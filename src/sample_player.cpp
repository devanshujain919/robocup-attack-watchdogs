// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

//bin/rione_player -team Ri-one -f conf/formations.conf -c conf/player.conf -num 1


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sample_player.h"

#include "strategy.h"
#include "field_analyzer.h"

#include "action_chain_holder.h"
#include "sample_field_evaluator.h"

#include "soccer_role.h"

#include "sample_communication.h"
#include "keepaway_communication.h"

#include "bhv_penalty_kick.h"
#include "bhv_set_play.h"
#include "bhv_set_play_kick_in.h"
#include "bhv_set_play_indirect_free_kick.h"
#include "bhv_basic_move.h" 

#include "bhv_custom_before_kick_off.h"
#include "bhv_strict_check_shoot.h"

#include "view_tactical.h"

#include "intention_receive.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/bhv_emergency.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/body_smart_kick.h>
#include <rcsc/action/body_kick_one_step.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_point.h> 
#include <rcsc/action/view_synch.h>
#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_dribble.h>

#include <rcsc/formation/formation.h>
#include <rcsc/action/kick_table.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/say_message_builder.h>
#include <rcsc/player/audio_sensor.h>
#include <rcsc/player/freeform_parser.h>
#include "bhv_chain_action.h"
#include <rcsc/action/body_advance_ball.h>
#include <rcsc/action/body_dribble.h>
#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_pass.h>
#include <rcsc/action/bhv_shoot.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include <rcsc/common/basic_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/audio_memory.h>
#include <rcsc/common/say_message_parser.h>
// #include <rcsc/common/free_message_parser.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector> 
#include <fstream>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
SamplePlayer::SamplePlayer()
    : PlayerAgent(),
      M_communication(),
      M_field_evaluator( createFieldEvaluator() ),
      M_action_generator( createActionGenerator() )
{
    boost::shared_ptr< AudioMemory > audio_memory( new AudioMemory );

    M_worldmodel.setAudioMemory( audio_memory );

    //
    // set communication message parser
    //
    addSayMessageParser( SayMessageParser::Ptr( new BallMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new PassMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new InterceptMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new GoalieMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new GoalieAndPlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new OffsideLineMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new DefenseLineMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new WaitRequestMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new PassRequestMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new DribbleMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new BallGoalieMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new OnePlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new TwoPlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new ThreePlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new SelfMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new TeammateMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new OpponentMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new BallPlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new StaminaMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new RecoveryMessageParser( audio_memory ) ) );

    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 9 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 8 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 7 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 6 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 5 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 4 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 3 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 2 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 1 >( audio_memory ) ) );

    //
    // set freeform message parser
    //
    setFreeformParser( FreeformParser::Ptr( new FreeformParser( M_worldmodel ) ) );

    //
    // set action generators
    //
    // M_action_generators.push_back( ActionGenerator::Ptr( new PassGenerator() ) );

    //
    // set communication planner
    //
    M_communication = Communication::Ptr( new SampleCommunication() );
}

/*-------------------------------------------------------------------*/
/*!

 */
SamplePlayer::~SamplePlayer()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SamplePlayer::initImpl( CmdLineParser & cmd_parser )
{
    bool result = PlayerAgent::initImpl( cmd_parser );

    // read additional options
    result &= Strategy::instance().init( cmd_parser );

    rcsc::ParamMap my_params( "Additional options" );
#if 0
    std::string param_file_path = "params";
    param_map.add()
        ( "param-file", "", &param_file_path, "specified parameter file" );
#endif

    cmd_parser.parse( my_params );

    if ( cmd_parser.count( "help" ) > 0 )
    {
        my_params.printHelp( std::cout );
        return false;
    }

    if ( cmd_parser.failed() )
    {
        std::cerr << "player: ***WARNING*** detected unsuppprted options: ";
        cmd_parser.print( std::cerr );
        std::cerr << std::endl;
    }

    if ( ! result )
    {
        return false;
    }

    if ( ! Strategy::instance().read( config().configDir() ) )
    {
        std::cerr << "***ERROR*** Failed to read team strategy." << std::endl;
        return false;
    }

    if ( KickTable::instance().read( config().configDir() + "/kick-table" ) )
    {
        std::cerr << "Loaded the kick table: ["
                  << config().configDir() << "/kick-table]"
                  << std::endl;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!
  main decision
  virtual method in super class
*/
void
SamplePlayer::actionImpl()
{
    //
    // update strategy and analyzer
    //
    Strategy::instance().update( world() );
    FieldAnalyzer::instance().update( world() );

    //
    // prepare action chain
    //
    M_field_evaluator = createFieldEvaluator();
    M_action_generator = createActionGenerator();

    ActionChainHolder::instance().setFieldEvaluator( M_field_evaluator );
    ActionChainHolder::instance().setActionGenerator( M_action_generator );

    //
    // special situations (tackle, objects accuracy, intention...)
    //
    const WorldModel & wm = this->world();

    if ( doPreprocess() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": preprocess done" );
        return;
    }

    //
    // update action chain
    //
    ActionChainHolder::instance().update( world() );

    //
    // create current role
    //
    std::string role_name;
    SoccerRole::Ptr role_ptr;
    {
        role_ptr = Strategy::i().createRole( world().self().unum(), world() );
        role_name = Strategy::i().getRoleName( world().self().unum(), world() );

        if ( ! role_ptr )
        {
            std::cerr << config().teamName() << ": "
                      << world().self().unum()
                      << " Error. Role is not registerd.\nExit ..."
                      << std::endl;
            M_client->setServerAlive( false );
            return;
        }
    }

    //
    // override execute if role accept
    //
    if ( role_ptr->acceptExecution( world() ) )
    {
        if(role_name=="Sample"){
            executeSampleRole(this);
            return;
        }
        role_ptr->execute( this );
        return;
    }

    //
    // play_on mode
    //
    if ( world().gameMode().type() == GameMode::PlayOn )
    {
        if(role_name=="Sample"){
            executeSampleRole(this);
            return;
        }
        role_ptr->execute( this );
        return;
    }

    if ( world().gameMode().type() == GameMode::KickOff_)
    {   
        mpIntransit = false;
        if(wm.self().unum()==10){
            mpIntransit = true;
            mpTarget = wm.ball().pos();
        }
        bool kickable = this->world().self().isKickable();
        if ( this->world().existKickableTeammate()
             && this->world().teammatesFromBall().front()->distFromBall()
             < this->world().ball().distFromSelf() )
        {
            kickable = false;
        }

        if ( kickable )
        {
            //Bhv_BasicMove().execute(this);
            Bhv_BasicOffensiveKick().execute(this);
        //    if(!PassToBestPlayer( this )){
        //        Body_HoldBall().execute( this );
                //doKick( this);
                //Bhv_BasicOffensiveKick().execute(this);
                //PassToBestPlayer(this);   
            //f}                
        }
        else
        {
            Bhv_BasicMove().execute(this);
        }
        return;
    }

    //
    // penalty kick mode
    //
    if ( world().gameMode().isPenaltyKickMode() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": penalty kick" );
        Bhv_PenaltyKick().execute( this );
        return;
    }

    //
    // other set play mode
    //
    Bhv_SetPlay().execute( this );
    return;

    if(role_name=="Sample"){
        executeSampleRole(this);
        return;
    }
}
   



/*--------------------------------------------------------------------------*/

bool
SamplePlayer::SampleDribble( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< PlayerObject * >( 0 )
            : opps.front() );
    const double nearest_opp_dist = ( nearest_opp
                                      ? nearest_opp->distFromSelf()
                                      : 1000.0 );
    const Vector2D nearest_opp_pos = ( nearest_opp
                                       ? nearest_opp->pos()
                                       : Vector2D( -1000.0, 0.0 ) );

    Vector2D pass_point;

    if ( Body_Pass::get_best_pass( wm, &pass_point, NULL, NULL ) )
    {
        if ( pass_point.x > wm.self().pos().x - 1.0 )
        {
            bool safety = true;
            const PlayerPtrCont::const_iterator opps_end = opps.end();
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                if ( (*it)->pos().dist( pass_point ) < 4.0 )
                {
                    safety = false;
                }
            }

            if ( safety )
            {
                Body_Pass().execute( agent );
                agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
                return true;
            }
        }
    }

    if ( nearest_opp_dist < 3.0 )
    {
        if ( Body_Pass().execute( agent ) )
        {
            agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
            return true;
        }
    }

    // dribble to my body dir
    if ( nearest_opp_dist < 5.0
         && nearest_opp_dist > ( ServerParam::i().tackleDist()
                                 + ServerParam::i().defaultPlayerSpeedMax() * 1.5 )
         && wm.self().body().abs() < 70.0 )
    {
        const Vector2D body_dir_drib_target
            = wm.self().pos()
            + Vector2D::polar2vector(5.0, wm.self().body());

        int max_dir_count = 0;
        wm.dirRangeCount( wm.self().body(), 20.0, &max_dir_count, NULL, NULL );

        if ( body_dir_drib_target.x < ServerParam::i().pitchHalfLength() - 1.0
             && body_dir_drib_target.absY() < ServerParam::i().pitchHalfWidth() - 1.0
             && max_dir_count < 3
             )
        {
            // check opponents
            // 10m, +-30 degree
            const Sector2D sector( wm.self().pos(),
                                   0.5, 10.0,
                                   wm.self().body() - 30.0,
                                   wm.self().body() + 30.0 );
            // opponent check with goalie
            if ( ! wm.existOpponentIn( sector, 10, true ) )
            {
                Body_Dribble( body_dir_drib_target,
                              1.0,
                              ServerParam::i().maxDashPower(),
                              2
                              ).execute( agent );
                agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
                return true;
            }
        }
    }

    Vector2D drib_target( 50.0, wm.self().pos().absY() );
    if ( drib_target.y < 20.0 ) drib_target.y = 20.0;
    if ( drib_target.y > 29.0 ) drib_target.y = 27.0;
    if ( wm.self().pos().y < 0.0 ) drib_target.y *= -1.0;
    const AngleDeg drib_angle = ( drib_target - wm.self().pos() ).th();

    // opponent is behind of me
    if ( nearest_opp_pos.x < wm.self().pos().x + 1.0 )
    {
        // check opponents
        // 15m, +-30 degree
        const Sector2D sector( wm.self().pos(),
                               0.5, 15.0,
                               drib_angle - 30.0,
                               drib_angle + 30.0 );
        // opponent check with goalie
        if ( ! wm.existOpponentIn( sector, 10, true ) )
        {
            const int max_dash_step
                = wm.self().playerType()
                .cyclesToReachDistance( wm.self().pos().dist( drib_target ) );
            if ( wm.self().pos().x > 35.0 )
            {
                drib_target.y *= ( 10.0 / drib_target.absY() );
            }
            Body_Dribble( drib_target,
                          1.0,
                          ServerParam::i().maxDashPower(),
                          std::min( 5, max_dash_step )
                          ).execute( agent );
        }
        else
        {
            Body_Dribble( drib_target,
                          1.0,
                          ServerParam::i().maxDashPower(),
                          2
                          ).execute( agent );

        }
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        return true;
    }

    // opp is far from me
    if ( nearest_opp_dist > 2.5 )
    {
        Body_Dribble( drib_target,
                      1.0,
                      ServerParam::i().maxDashPower(),
                      1
                      ).execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        return true;
    }

    // opp is near

    if ( Body_Pass().execute( agent ) )
    {
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        return true;
    }


    // opp is far from me
    if ( nearest_opp_dist > 3.0 )
    {
        Body_Dribble( drib_target,
                      1.0,
                      ServerParam::i().maxDashPower(),
                      1
                      ).execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        return true;
    }

    
    if ( nearest_opp_dist > 2.5 )
    {
        Body_HoldBall().execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        return true;
    }
    

    {
        Body_AdvanceBall().execute( agent );
        agent->setNeckAction( new Neck_ScanField() );
    }

    return true;
}

/*bool 
SamplePlayer::PassPlayersAvailable( PlayerAgent * agent ){
    const WorldModel & wm = agent->world();

    Vector2D myPosition = wm.self().pos();
    Vector2D currentHole = RoundToNearestHole(myPosition);
    Vector2D frontup = Vector2D(currentHole.x+10, currentHole.y-10);
    Vector2D backup = Vector2D(currentHole.x-10, currentHole.y-10);
    Vector2D frontdown = Vector2D(currentHole.x+10, currentHole.y+10);
    Vector2D backdown = Vector2D(currentHole.x-10, currentHole.y+10);

    Vector2D fronthor = Vector2D(currentHole.x+20, currentHole.y);
    Vector2D backhor = Vector2D(currentHole.x-20, currentHole.y);
    Vector2D upvert = Vector2D(currentHole.x, currentHole.y-20);
    Vector2D downvert = Vector2D(currentHole.x, currentHole.y+20);
    
    double buffer = 2.5;
    
    //TODO: Return true only if pass is advantageous
    
    if( IsOccupiedForPassing(agent, frontup, buffer)||
        IsOccupiedForPassing(agent, frontdown, buffer)||
        IsOccupiedForPassing(agent, backup, buffer)||
        IsOccupiedForPassing(agent, backdown, buffer)||
        IsOccupiedForPassing(agent, fronthor, buffer)||
        IsOccupiedForPassing(agent, upvert, buffer)||
        IsOccupiedForPassing(agent, downvert, buffer)||
        IsOccupiedForPassing(agent, backhor, buffer)
        ){
        return true;
    }

    /*
    const WorldModel & world = agent->world();
    const PlayerPtrCont::const_iterator 
    t_end = world.teammatesFromSelf().end();
    for ( PlayerPtrCont::const_iterator
              it = world.teammatesFromSelf().begin();
          it != t_end;
          ++it )
    {
        if(((*it)->pos().dist(world.ball().pos()))<=20.0){
            return false;
        }  
    }
    
    
    return false;   
}
*/

bool
SamplePlayer::IsSectorEmpty(PlayerAgent * agent, Vector2D target, double buf_degree){
    Neck_TurnToPoint( target ).execute(agent );
    const WorldModel & wm = agent->world();
    AngleDeg target_angle = (target - wm.self().pos()).dir();
    double target_dis = wm.self().pos().dist(target);
    const Sector2D sector( wm.self().pos(),
                            0.5, target_dis + 4,
                            target_angle - buf_degree,
                            target_angle + buf_degree );
    // opponent check without goalie
    if ( ! wm.existOpponentIn( sector, 10, false ) )
    {
        return true;
    }

    return false;
}



int
SamplePlayer::ClosestPlayerToBall(PlayerAgent * agent){
    double mindis = 999;
    int mindisunum = -1;
    for(int i=2; i<=11; i++){
        if(agent->world().ourPlayer(i)!=NULL){
            if(agent->world().ourPlayer(i)->distFromBall() < mindis){
                mindis = agent->world().ourPlayer(i)->distFromBall();
                mindisunum = i;
            }
        }
    }
    return mindisunum;
}



/*
    START EDITTING<---------------------------------------------
*/
std::string
SamplePlayer::createState(PlayerAgent *agent)
{
    const WorldModel &wm = agent->world();
    Vector2D myPos = wm.self().pos();

    /* Iterator over all the players and obtain their relative position */
    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerPtrCont::const_iterator end = opps.end();
    std::vector<Vector2D> oppositionPos;
    for(PlayerPtrCont::const_iterator it = opps.begin() ; it != end ; ++it)
    {
        const Vector2D &u = (*it)->rpos();
        const Vector2D v = u + myPos;
        oppositionPos.push_back(v);
    }

    const PlayerPtrCont & team = wm.teammatesFromSelf();
    const PlayerPtrCont::const_iterator end2 = team.end();
    std::vector<Vector2D> teamPos;
    for(PlayerPtrCont::const_iterator it = team.begin() ; it != end2 ; ++it)
    {
        const Vector2D &u = (*it)->rpos();
        const Vector2D v = u + myPos;
        teamPos.push_back(v);
    }

    /* Following variables required to obtain the region in which the other players are */
    const BallObject &ball = wm.ball();
    const Vector2D ballPos = ball.pos();

    int halfWidth = ServerParam::i().pitchHalfWidth();
    int halfLength = ServerParam::i().pitchHalfLength();

    int regionsHalfWidth = 9;
    int regionsHalfLength = 12;

    int widthEachCell = halfWidth/regionsHalfWidth;
    int lengthEachCell = halfLength/regionsHalfLength;

    /* File pointer */
    std::stringstream ss;

    /*  --  Start recording the values for the state --  */

    /* Recording Format -> "[(T:x1,y1)(T:x2,y2)....(O:x1,y1)(O:x2,y2)....[(B:x,y)]" */

    /* Record team player's position values */
    ss << "[";
    for ( std::vector<Vector2D>::iterator it = teamPos.begin() ; it != teamPos.end(); ++it)
    {
        int x = (*it).x/lengthEachCell ;
        int y = (*it).y/widthEachCell ;
        ss << "(T:" << x << "," << y << ")";
    }

    /* Record opponent player's position values */
    for (std::vector<Vector2D>::iterator it = oppositionPos.begin() ; it != oppositionPos.end(); ++it)
    {
        int x = (*it).x/lengthEachCell ;
        int y = (*it).y/widthEachCell ;
        ss << "(O:" << x << "," << y << ")";
    }
    
    /* Record ball's position */
    int x = ballPos.x/lengthEachCell ;
    int y = ballPos.y/widthEachCell ;
    ss << "(B:" << x << "," << y << ")]";

    return ss.str();
}

void 
SamplePlayer::writeState(PlayerAgent *agent, std::string prevState, int action, int action_arg, double q_value)
{

    std::cout << "----------------------> Writing <------------------------" ;

    const WorldModel &wm = agent->world();

    /* Write the recorded states to a file */
    std::stringstream ss;
    ss << prevState;

    /* Write the action */
    ss << "[A:" << action;
    if(action == Pass)
    {
        ss << ":" << action_arg;
    }
    ss << "]";

    std::string stateAction_new = ss.str();

    /* Write the q-value */
    int p = q_value * 100;
    float q = p/100;
    ss << "[Q:" << q << "]";

    std::string result = ss.str();
    int length = result.length();

    /*  350 : maximum number of characters in a line 
                            =
        11*11 + 11*11 + 1*11 + for_action + for_q_value + some_safety margin 
    */
    int remainingLength = 350 - length;
    for(int i = 0 ; i < remainingLength-1 ; i ++)
    {
        ss << "]";
    }
    ss << "\n";
    result = ss.str();

    std::cout << "Writing to the file hahahahahahahahaha!!!!!\n";
    std::fstream myfile;
        
    std::stringstream sstm;
    sstm << "state_" << wm.self().unum() << ".txt";
    std::string res = sstm.str();
    myfile.open (res.c_str(), std::ios::in);
        
    if(!myfile.is_open())
    {
        std::cout << "file for search does not exist!!!\n\n\n";
    }
    int flag = 0;
    std::streampos writePos = 0;
    std::string line;
    char line_file[351];
    std::cout << "Comparing it with : " << stateAction_new << "in file : " << res << "\n\n\n";
    int pq = 1;

    if(myfile.is_open())
    {
        myfile.seekg(0);
        while(myfile.getline(line_file, 351)) // error in this statement not able to read...
        {
            line = std::string(line_file);
            std::cout << "Read the line in side the while loop: " << line_file << "\n\n\n";
            std::cout << "--------------------------------> SEARCHING <------------------------------------" ;
            int p = line.find_first_of("]");
            int p2 = line.substr(p+1, line.size()).find_first_of("]");
            std::string stateAction_inFile = line.substr(0, p2+1);

            int comparable = stateAction_new.compare(stateAction_inFile);
            if(comparable == 0)
            {
                /* State-Action pair already exists in the file */
                std::cout << "Its true!!!\n\n\n";
                flag = 1;
                break;
            }
            writePos = myfile.tellg();
        }
        myfile.close();
        std::cout << "Read the line: " << line_file << "\n\n\n";
    }

    if(flag == 1)
    {
        std::cout << "I found it in the file !!! at position: " << writePos << "\n\n\n";
        myfile.open (res.c_str(), std::ios::out);
        myfile.seekp(writePos);
        myfile << result ;
        myfile.close();
    }
    else
    {
        std::cout << "I have to append to the file !!! at position: " << writePos << "\n\n\n";
        myfile.open (res.c_str(), std::ios::out | std::ios::app);
        myfile << result ;
        myfile.close();
    }

    std::cout << "Written to the file " << res << "\n" << "Text is : " << result << "\n\n";

}

std::string SamplePlayer::chooseAction(PlayerAgent *agent, double *q_value, std::string ss)
{
    const WorldModel &wm = agent->world();

    /* Read data and obtain the appropriate action */
    std::ifstream myfile;
    std::stringstream sstm;
    sstm << "state_" << wm.self().unum() << ".txt";
    std::string res = sstm.str();
    myfile.open (res.c_str(), std::ios::in);
    if(myfile.is_open())
    {
        std::string line;
        int flag = 0;
        double maxQ = 0;
        std::string action = "NULL"; 
        while(getline(myfile, line))
        {

            std::cout << "----------------------> Choosing the action <---------------------------" ;

            /* -- Parse the lines into positions, actions and q-values -- */

            /* First get positions of all players and the ball */
            int pos = line.find_first_of("]");
            std::string state = line.substr(0,pos+1);
            int comparable = ss.compare(state);
            if(comparable == 0)
            {
                
                /* Obtain action */
                int pos2 = line.substr(pos+1,line.size()).find_first_of(":");
                int pos3 = line.substr(pos+1,line.size()).find_first_of("]");
                std::string a = line.substr(pos2+1, pos3);
                
                /* Obtain q-value */
                int pos4 = line.substr(pos3+1,line.size()).find_first_of(":");
                int pos5 = line.substr(pos3+1,line.size()).find_first_of("]");
                std::string q = line.substr(pos4+1, pos5);

                //int act = stoi(a_act);
                //int act_arg = stoi(a_arg);
                double qval = std::strtod(q.c_str(), NULL);
                if(!flag)
                {
                    maxQ = qval;
                    action = a;
                    flag = 1;
                }
                else if(maxQ < qval)
                {
                    maxQ = qval;
                    action = a;
                }
                else
                {
                    // do nothing
                }
            }
        }
        if(!flag)
        {
            q_value = NULL;
            return "NULL";
        }
        else
        {   
            *q_value = maxQ;
            return action;
        }
    }
    else
    {
        std::cout << "making it null\n\n\n";
        q_value = NULL;
        return "NULL";
    }
}

double
SamplePlayer::findQ( PlayerAgent *agent, std::string stateAction )
{
    const WorldModel &wm = agent->world();

    /* Finding the q-value from the file given the agent and state-action */
    std::ifstream myfile;
    std::stringstream sstm;
    sstm << "state_" << wm.self().unum() << ".txt";
    std::string res = sstm.str();
    myfile.open (res.c_str(), std::ios::in);
    if(myfile.is_open())
    {
        std::string line;
        int q_val = 0;
        while(getline(myfile, line))
        {
            int pos = line.find_first_of("]");
            int pos2 = line.substr(pos+1, line.size()).find_first_of("]");
            std::string stateAction2 = line.substr(0,pos2+1);
            int comparable = stateAction.compare(stateAction2);
            if(comparable == 0)
            {               
                int pos3 = line.substr(pos2+1,line.size()).find_first_of(":");
                int pos4 = line.substr(pos2+1,line.size()).find_first_of("]");
                std::string q = line.substr(pos3+1, pos4);
                q_val = std::strtod(q.c_str(), NULL);
                break;
            }
        }
        return q_val;
    }
    else
    {
        return 0;
    }   
}

double
SamplePlayer::obtainReward(rcsc::PlayerAgent *agent, std::string prevState, std::string newState, int action, bool prev_team_has_ball, bool prev_opponent_has_ball)
{
    //TODO
    const WorldModel & wm = this->world();

    int side = wm.gameMode().side();
    int time_passed = wm.gameMode().time().cycle();
    int totalTime = ServerParam::i().DEFAULT_HALF_TIME * 2;
    int scoreLeft = wm.gameMode().scoreLeft();
    int scoreRight = wm.gameMode().scoreRight();
    int scoreline = scoreLeft - scoreRight;

    /*enum SideID {
        LEFT = 1,
        NEUTRAL = 0,
        RIGHT = -1,
    };

    in rcsc/types.h
    */

    if(side == 1)
    {
        // left
    }
    if(side == -1)
    {
        // right
        scoreline *= -1;
    }

    if(wm.gameMode().type() == GameMode::AfterGoal_)
    {
        // check for goal

        Vector2D ballPos = wm.ball().pos();
        if(side == 1)
        {
            // left
            if(ballPos.x > 0)
            {
                // goal is made at opposition
                return 1.0;
            }
            else
            {
                // goal is made at my goal
                return -1.0;
            }
        }
        if(side == -1)
        {
            // right
            if(ballPos.x > 0)
            {
                // goal is made at my goal
                return -1.0;
            }
            else
            {
                // goal is made at opposition
                return 1.0;
            }
        }
    }

    else if(action >= 2 && action <= 6)
    {
        // check for possession

        double f_value, arg0, arg1;

        double reward = 0;

        bool team_has_ball = false, opponent_has_ball = false;

        team_has_ball = wm.existKickableTeammate();
        opponent_has_ball = wm.existKickableOpponent();

        if(prev_team_has_ball && !prev_opponent_has_ball && opponent_has_ball && !team_has_ball)
        {
            /* change of possession: ----> earlier I had the ball, opposition didn't
                                     ----> Now, Opposition has the ball
                                     ----> We don't have the ball
            */
            // case of being tackled

            if(scoreline != 0)
            {
                arg0 = time_passed/totalTime;
                int sl = (scoreline > 0) ? scoreline : -1 * scoreline;
                arg1 = sl/20;
            }   
            else
            {
                arg0 = time_passed/totalTime;
                arg1 = 1/40;
            }

            f_value = arg0 * arg1;

            Vector2D opponentGoal = ServerParam::i().theirTeamGoalPos();
            Vector2D ballPos = wm.ball().pos();
            Vector2D ballToOpponentGoal = ballPos - opponentGoal;

            double distance = ballToOpponentGoal.norm();

            reward = -1 * f_value * distance;
            return reward;
        }
        else if(!prev_team_has_ball && prev_opponent_has_ball && !opponent_has_ball && team_has_ball)
        {
            /* change of possession: ----> earlier we didn't have the ball, opposition had it
                                     ----> Now, Opposition doesn't have the ball
                                     ----> We possess the ball
            */
            // case of tackling

            if(scoreline != 0)
            {
                arg0 = time_passed/totalTime;
                int sl = (scoreline > 0) ? scoreline : -1 * scoreline;
                arg1 = sl/20;
            }   
            else
            {
                arg0 = time_passed/totalTime;
                arg1 = 1/40;
            }

            f_value = arg0 * arg1;

            Vector2D opponentGoal = ServerParam::i().theirTeamGoalPos();
            Vector2D ballPos = wm.ball().pos();
            Vector2D ballToOpponentGoal = ballPos - opponentGoal;

            double distance = ballToOpponentGoal.norm();

            reward = f_value * distance;
            return reward;
        }
        else if(prev_team_has_ball && !prev_opponent_has_ball && !opponent_has_ball && team_has_ball)
        {
            /* no change of possession: ----> I still possess the ball
            */
            // case of passing

             double f_value = 0, arg0, arg1;
            if(scoreline < 0)
            {
                arg0 = time_passed/totalTime;
                arg1 = -1 * scoreline/20;
            }
            else if(scoreline > 0)
            {
                arg0 = 1 - (time_passed/totalTime);
                arg1 = scoreline/20;
            }
            else
            {
                arg0 = time_passed/totalTime;
                arg1 = 1/40;
            }
            f_value = arg0 * arg1;

            Vector2D ourTeamGoalPos = ServerParam::i().ourTeamGoalPos();
            Vector2D myPos = wm.self().pos();

            Vector2D goalToMe = myPos - ourTeamGoalPos;

            double distance = goalToMe.norm();

            double reward = f_value * distance;

            return reward;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

bool
SamplePlayer::SampleMove(rcsc::PlayerAgent *agent)
{
    const WorldModel &wm = agent->world();

    Vector2D v; // store the resultant vector
    int count = 0;

    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerPtrCont::const_iterator end = opps.end();
    for(PlayerPtrCont::const_iterator it = opps.begin() ; it != end ; ++it)
    {
        const Vector2D &u = (*it)->rpos();
        v = v - (u/(u.norm2()));
        count ++;
    }

    const PlayerPtrCont & team = wm.teammatesFromSelf();
    const PlayerPtrCont::const_iterator end2 = team.end();
    for(PlayerPtrCont::const_iterator it = team.begin() ; it != end2 ; ++it)
    {
        const Vector2D &u = (*it)->rpos();
        v = v - (u/(u.norm2()));
        count ++;
    }

    v = v/count;
    
    const Vector2D &u = wm.ball().rpos();
    v = v + (u/(u.norm2()));

    Vector2D p = ServerParam::i().theirTeamGoalPos();
    Vector2D myPos = wm.self().pos();
    Vector2D rpos = p - myPos;
    v = v + (rpos/rpos.norm2());

    Body_GoToPoint( myPos + v, 0.5, ServerParam::i().maxDashPower()).execute(agent);

    return true;
}

//main function that will be used.

bool
SamplePlayer::executeSampleRole( PlayerAgent * agent )
{
    const WorldModel &wm = agent->world();

    /*
            Body_GoToPoint: rcsc/action/body_gotopoint
            
            \param point target point to be reached
            \param dist_thr distance threshold to the target point
            \param max_dash_power maximum dash power parameter (have to be a positive value)
            \param dash_speed prefered dash speed. negative value means no preferred speed
            \param cycle preferred reach cycle
            \param save_recovery if this is true, player always saves its recoverry.
            \param dir_thr turn angle threshold
    
    Body_GoToPoint2010( const Vector2D & point,
                        const double & dist_thr,
                        const double & max_dash_power,
                        const double & dash_speed = -1.0,
                        const int cycle = 100,
                        const bool save_recovery = true,
                        const double & dir_thr = 15.0 )
        : M_target_point( point ),
          M_dist_thr( dist_thr ),
          M_max_dash_power( std::fabs( max_dash_power ) ),
          M_dash_speed( dash_speed ),
          M_cycle( cycle ),
          M_save_recovery( save_recovery ),
          M_dir_thr( dir_thr ),
          M_back_mode( false )
      { }

        Vector2D(x,y): rcsc/geom

        ServerParam: rcsc/common : i(): returns a reference to the instance
        */

    
    if(agent->config().teamName() == "opp"){
        
        Body_GoToPoint( Vector2D(-50,0), 0.0, ServerParam::i().maxDashPower(), -1, 4, true, 60).execute( agent );
        return true;
    } 

    //Setting up of different flags.

    bool kickable = agent->world().self().isKickable();
    
    // If there is a teammate who can kick the ball and who is closer to the ball than I am.
    if ( agent->world().existKickableTeammate()
         && agent->world().teammatesFromBall().front()->distFromBall()
         < agent->world().ball().distFromSelf() )
    {
        kickable = false;
    }


    if(agent->world().existKickableTeammate()){
        //passHeard = false;
        Opponenthasball = false;
    }
    /* Why the semicolon after else? */
    else{};
        //Opponenthasball = true;
    

    if(agent->world().existKickableOpponent()){
        Opponenthasball = true;
    }

    bool team_has_ball, opponent_has_ball;
    team_has_ball = agent->world().existKickableTeammate();
    opponent_has_ball = agent->world().existKickableOpponent();

    //----------XX------------//
    //If you have taken attack, you will have comment out the attach functions are replace
    //them with your own, similarly if you have taken defense, you need to comment out the existing
    //defence function and replace it with your own.
    //------------xx------------//


    //ATTACK STARTS HERE
    // I have the ball, what to do?

    //This is for off the ball movement which attacking, where to go for passes etc.
    if (!kickable && !Opponenthasball)
    {   
        //RunThrough(agent);
        doMove(this);
        return true;
    } 

    double alpha = 0.5;                             // importance given to the new learnings
    
    /* Implementation of q-learning */
    /* First, we need to play with some order of randomness in order to learn */
    /* After certain knowledge, we will be able to create a policy and follow it */

    std::srand (time(NULL));
    int random = rand() % 100 + 1;                  // exploration vs exploitation

    int act, act_arg;                               // act:action and act_arg : useful for passing (to which player)
    
    double q_val;                                   // q_value already in the file
    double *ptr_q_val = &q_val;                     // pointer to the above

    std::string str = createState(agent);

    std::stringstream prevState ;                   // current state
    prevState << str;

    if(random <= 30)
    {
        // do random action
        std::cout << "I am choosing some random action \n\n\n";

        if(kickable && !Opponenthasball)
        {
            act = rand() % 4 + 1; // action ball possession
        }
        if(!kickable && Opponenthasball)
        {
            act = rand() % 2 + 5; // action no ball possession
        }

        if(act == Pass)
        {
            act_arg = rand() % 10 + 2;              // player to pass to : 2 to 11
        }
        else
        {
            act_arg = -1;
        }

        std::stringstream stateAction;
        stateAction << prevState.str();
        stateAction << "[A:" << act;
        if(act == Pass)
        {
            stateAction << ":" << act_arg;
        }
        stateAction << "]";

        q_val = findQ(agent, stateAction.str());    // find if there is a qvalue for that state and action

        if(q_val == -1)
        {
            /* Could not open the file */
            std::cout << "Its NULL \n\n\n";
            return false;
        }

    }
    else
    {
        /* choose action according to policy */

        std::cout << "I am choosing some meaningful action Q-learning\n\n\n";
        
        std::string action = chooseAction(agent, ptr_q_val, prevState.str());
        if(action.compare("") == 0)
        {
            std::cout << "Some error in opening files \n\n\n";
            return false;
        }
        if(action.compare("NULL") == 0)
        {
            std::cout << "nothing choosen, now taking some random\n\n\n";
            /* if no such found, then choose an action randomly */
            if(kickable && !Opponenthasball)
            {
                act = rand() % 4 + 1; // action ball possession
            }
            if(!kickable && Opponenthasball)
            {
                act = rand() % 2 + 5; // action no ball possession
            }

            if(act == Pass)
            {
                act_arg = rand() % 10 + 2;              // player to pass to : 2 to 11
            }
            else
            {
                act_arg = -1;
            }

            std::cout << "After a lot of deliberation, action choosen: " << act << " with arg as : " << act_arg << "\n\n\n";
        }
        else
        {
            int p = action.find_first_of(":");          // pass:i  or move or something else ---> action arg only in pass
            if(p == -1)
            {
                act = std::strtod(action.c_str(), NULL);
            }    
            else
            {
                act = std::strtod(action.substr(0, p+1).c_str(), NULL);
                act_arg = std::strtod(action.substr(p+1, action.size()).c_str(), NULL);
            }
        }
    }

    // TODO: here execute the action

    if(act == Pass)
    {
        std::cout << "pass";
    }

    if(kickable && !Opponenthasball)
    {

        switch(act)
        {
            case Pass:      {
                                Vector2D pass_point;
                                Body_Pass::get_best_pass( agent->world(), &pass_point, NULL, NULL );
                                Body_Pass().execute(agent);
                                agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
                                std::cout << "Passed the ball to point: " << pass_point ;
                                break;
                            }
            case Hold:      {
                                std::cout << "Holding the ball!!!!" ;
                                Body_HoldBall().execute(agent);
                                break;
                            }
            case Dribble:   {
                                std::cout << "Dribbling the ball!!!!" ;
                                SampleDribble(agent);
                                break;
                            }
            case Goal:      {
                                std::cout << "Shooting the ball!!!!" ;
                                Vector2D ball_pos = wm.ball().pos();
                                Body_GoToPoint( ball_pos, 0.5, ServerParam::i().maxDashPower()).execute(agent);
                                Bhv_Shoot().execute(agent);
                                break;
                            }
            default:        {
                                std::cout << "Wrong action!!!!!!!!!!!!!! action choosed = " << act ;
                                break;
                            }
        }

    }

    if(!kickable && Opponenthasball)
    {

        switch(act)
        {
            case Move:      {
                                std::cout << "Moving the agent!!!!" ;
                                SampleMove(agent);
                                break;
                            }
            case Intercept: {
                                Body_Intercept().execute( agent );
                                agent->setNeckAction( new Neck_OffensiveInterceptNeck() );
                                break;
                            }
            default:        {
                                std::cout << "Wrong action!!!!!!!!!!!!!!action choosed = " << act ;
                                break;
                            }
        }
        
    }

    std::cout << "Creating the state\n\n\n";
    str = createState(agent);
    std::stringstream newState ;                    // get new state of the player
    newState << str;
    
    std::cout << "Obtaining reward\n\n\n";
    double reward = obtainReward(agent, prevState.str(), newState.str(), act, team_has_ball, opponent_has_ball); // TODO: obtain reward according to it

    /* Apply the formula to compute the q value for previous state */
    double q_value, newState_q_value;
    double *ptr_newState_q_value = &newState_q_value;
    chooseAction(agent, ptr_newState_q_value, newState.str()); // gets the maximum q-value for that state
    if(ptr_newState_q_value == NULL)
    {
        /* Didn't find the state */
        newState_q_value = 0;
    }

    std::cout << "Calculating\n\n\n";
    q_value = (1-alpha)*q_val  + alpha*(reward + newState_q_value);

    /* Write the new state value to the file */

    std::cout << "-------------------------------->Writing to the file <-----------------------------------------------" ;
    writeState(agent, prevState.str(), act, act_arg, q_value);
    
    /*
    //ATTACK ENDS HERE
    //--------XX----------XX--------//
    // DEFENCE STARTS HERE
    //The defense call
    else
    {
        //Uncomment this for defense.
        //If I can kick the ball, but opponent has it. Common ball.
        if (kickable && Opponenthasball){
            if ( Bhv_ChainAction().execute( agent ) )
            {
                dlog.addText( Logger::TEAM,
                      __FILE__": (execute) do chain action" );
            agent->debugClient().addMessage( "ChainAction" );
            return true;
            }

            Bhv_BasicOffensiveKick().execute( agent );
            return true;

            }

        // I don't have the ball, opponent has it, off the ball movement while defending.
        //falling back etc.     
        else if (!kickable && Opponenthasball){
            Bhv_BasicMove().execute(agent);
        }
        return true;
    };
    */
    //DEFENSE ENDS HERE.
    
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/

bool SamplePlayer::SamplePass(PlayerAgent *agent, const PlayerObject *target_mate)
{
    const double target_mate_distance = (target_mate ? target_mate->distFromSelf() : 1000.0);

    const Vector2D target_mate_pos = (target_mate ? target_mate->pos() : Vector2D(-1000.0, 0.0));

    Body_SmartKick(target_mate_pos, ServerParam::i().ballSpeedMax()*0.9, ServerParam::i().ballSpeedMax(), 8).execute(agent);

    int unum = getUnum(agent, target_mate_pos);
    if(unum == -1)
    {
        return false;
    }
    agent->addSayMessage( new PassMessage( unum, target_mate_pos, agent->effector().queuedNextBallPos(), agent->effector().queuedNextBallVel() ) );

    return true;

}

bool SamplePlayer::GiveThrough(PlayerAgent *agent, const PlayerObject *target_mate)
{
    const WorldModel &wm = agent->world();

    Vector2D mate_pos = target_mate->pos();

    double *dist;

    const PlayerObject *opponent = wm.getOpponentNearestTo(target_mate, 10, dist);
    Vector2D opponent_pos;

    if(opponent == NULL)
        opponent_pos = Vector2D(mate_pos.x + 10.0, mate_pos.y);

    else
        opponent_pos = opponent->pos();

    Vector2D vecbetween = mate_pos - opponent_pos;

    Vector2D target = wm.self().pos() + opponent_pos + (vecbetween*1.5);

    return Body_KickOneStep(target, ServerParam::i().ballSpeedMax()*0.8, false).execute(agent);

}

bool SamplePlayer::RunThrough(PlayerAgent *agent)
{
    const WorldModel &wm = agent->world();
    const PlayerObject *player = wm.getTeammateNearestToBall(5, true);
    if(AreSamePoints(wm.self().pos(), wm.self().pos()+player->pos(), 5))
    {
        // same player
        Vector2D ball_pos = wm.ball().pos();
        Body_GoToPoint( ball_pos, 0.5, ServerParam::i().maxDashPower()).execute(agent);
    }
}

bool SamplePlayer::impl_dribble(PlayerAgent *agent)
{
    const WorldModel &wm = agent->world();
    Vector2D pos = Vector2D(ServerParam::i().pitchHalfLength(), 0.0);
    Body_KickOneStep(pos, ServerParam::i().ballSpeedMax()*0.8, false).execute(agent);
    Body_GoToPoint( wm.ball().pos(), 0.5, ServerParam::i().maxDashPower()).execute(agent);
}

int SamplePlayer::getUnum(PlayerAgent *agent, Vector2D target)
{
    const WorldModel & wm = agent->world();
    for(int i=2; i<=11; i++)
    {
        if(wm.ourPlayer(i)!=NULL)
        {
            Vector2D player_pos = wm.ourPlayer(i)->pos();
            if( AreSamePoints(player_pos, target, 5) && i!=wm.self().unum() )
                return i;
        }
    }
    return -1;
}


/*
    END EDITTING <------------------------------------------------
*/



void
SamplePlayer::doKick( PlayerAgent * agent )
{
    
    if ( Bhv_ChainAction().execute( agent ) )
    {
        return;
    }

    Bhv_BasicOffensiveKick().execute( agent );
}



/*-------------------------------------------------------------------*/
/*!

*/
void
SamplePlayer::doMove( PlayerAgent * agent )
{
        Bhv_BasicMove().execute(agent);
        return;
}

bool
SamplePlayer::BasicMove(PlayerAgent * agent){
    const WorldModel & wm = agent->world();
    
    //-----------------------------------------------
    // tackle
    
    if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    {   
        return true;
    }

    /*--------------------------------------------------------*/
    // chase ball
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    //Intercept
    if ( ! wm.existKickableTeammate()
         && ( self_min <= 3
              || ( self_min <= mate_min
                   && self_min < opp_min + 3 )
              )
         )
    {
        std::cout<<"body intercept called for player - "<<wm.self().unum()<<std::endl;

    
        dlog.addText( Logger::TEAM,
                      __FILE__": intercept");
        Body_Intercept().execute( agent );
        agent->setNeckAction( new Neck_OffensiveInterceptNeck() );
        return true;
    }
    
    //Check if ball has been passed
    /*
    if(wm.existKickableTeammate()){
        int CurrentBH = GetBHUnum(agent);
        if(CurrentBH!=LastBH && CurrentBH!=-1){
            FoundNewBH = true;
            LastBH = CurrentBH;
        }
        else
            FoundNewBH = false;

        //Opponenthasball=false;
    }
    */
    
    const Vector2D target_point = Vector2D(0,0);
    const double dash_power = ServerParam::i().maxDashPower();

    double dist_thr = wm.ball().distFromSelf() * 0.1;
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    

    if ( wm.existKickableOpponent()
         && wm.ball().distFromSelf() < 18.0 )
    {
        agent->setNeckAction( new Neck_TurnToBall() );
        return true;
    }
    else
    {
        agent->setNeckAction( new Neck_TurnToBallOrScan() );
    }

    return true;
}






//centreback







bool
SamplePlayer::isKickable(PlayerAgent * agent, int unum){
    const WorldModel & wm = agent->world();
    if(wm.ourPlayer(unum)!=NULL){
        if(wm.ourPlayer(unum)->distFromBall() < ServerParam::i().defaultKickableArea()){
            return true;
        }
        return false;
    }
    else{
        return false;
    }
}

bool 
SamplePlayer::AreSamePoints(Vector2D A, Vector2D B, double buffer){
    //Check if and b are the same points +/- buffer
    if(A.dist(B)<buffer)
        return true;
    return false;
}

double 
SamplePlayer::abs(double d){
    if (d>0.00)
        return d;
    else
        return d*(-1.00);
}


bool
SamplePlayer::AreSameNos(double A, double B, double buffer){
    if( abs(A-B) < buffer)
        return true;
    return false;
}



double DistanceBetweenPoints(double x1,double y1,double x2,double y2){
 
        double distance = sqrt(pow((x1-x2),2)+pow((y1-y2),2));

        return distance;
    }



    double AngleBetweenPoints(double x1,double y1,double x2,double y2,double x3,double y3){

        double angle = acos((pow(DistanceBetweenPoints(x1,y1,x2,y2),2)+pow(DistanceBetweenPoints(x1,y1,x3,y3),2)-pow(DistanceBetweenPoints(x2,y2,x3,y3),2))/(2*DistanceBetweenPoints(x1,y1,x2,y2)*DistanceBetweenPoints(x1,y1,x3,y3)));
        
        return angle;
    }

    static bool compare_first(const std::pair<double,double>& i, const std::pair<double,double>& j)
    {
        return i.first > j.first;
    }


    static bool compare_second(const std::pair<double,double>& i, const std::pair<double,double>& j)
    {
        return i.second > j.second;
    }

    double slope(double x1,double y1,double x2,double y2){

        double slope_of_line = (y2-y1)/(x2-x1);

        return slope_of_line;
    }

    double constant(double x1,double y1,double slope_of_line){

        return (y1-slope_of_line*x1);
    }

    double bisector_line_const(double c1,double c2,double a1,double a2){


        return ((c2-c1)*sqrt((pow(a2,2)+1)*(pow(a1,2)+1)));
    }

    double bisector_line_x_const(double a1,double a2){

        return ((a2*sqrt(pow(a1,2)+1))-(a1*sqrt(pow(a2,2)+1)));
    }

    double bisector_line_y_const(double a1,double a2){


        return (sqrt(pow(a2,2)+1)-sqrt(pow(a1,2)+1));
    }

    double intersecting_point_x(double c1, double c2, double a, double b){

        double x = ((c2*b - c1*a)/((a*a)+(b*b)));
        
        return x;
    }



    double intersecting_point_y(double c1, double c2, double a, double b){

        double y = ((c2*a + c1*b)/((a*a)+(b*b)));
        
        return y;
    } 

    double angle_between_two_lines(const Line2D & line1,const Line2D & line2 ){

        double theta_1 = atan(line1.getB()/line1.getA()); 
    
        double theta_2 = atan(line2.getB()/line2.getA());

        return (theta_2-theta_1);
    }


    static
    Line2D angle_bisectorof( const Vector2D & origin,
                           const AngleDeg & left,
                           const AngleDeg & right )
      {
          return Line2D( origin, AngleDeg::bisect( left, right ) );
      }





/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleActionStart()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleActionEnd()
{
    if ( world().self().posValid() )
    {
#if 0
        const ServerParam & SP = ServerParam::i();
        //
        // inside of pitch
        //

        // top,lower
        debugClient().addLine( Vector2D( world().ourOffenseLineX(),
                                         -SP.pitchHalfWidth() ),
                               Vector2D( world().ourOffenseLineX(),
                                         -SP.pitchHalfWidth() + 3.0 ) );
        // top,lower
        debugClient().addLine( Vector2D( world().ourDefenseLineX(),
                                         -SP.pitchHalfWidth() ),
                               Vector2D( world().ourDefenseLineX(),
                                         -SP.pitchHalfWidth() + 3.0 ) );

        // bottom,upper
        debugClient().addLine( Vector2D( world().theirOffenseLineX(),
                                         +SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().theirOffenseLineX(),
                                         +SP.pitchHalfWidth() ) );
        //
        debugClient().addLine( Vector2D( world().offsideLineX(),
                                         world().self().pos().y - 15.0 ),
                               Vector2D( world().offsideLineX(),
                                         world(Opponenthasball
Opponenthasball
Opponenthasball
Opponenthasball
Opponenthasball
Opponenthasball).self().pos().y + 15.0 ) );

        // outside of pitch

        // top,upper
        debugClient().addLine( Vector2D( world().ourOffensePlayerLineX(),
                                         -SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().ourOffensePlayerLineX(),
                                         -SP.pitchHalfWidth() ) );
        // top,upper
        debugClient().addLine( Vector2D( world().ourDefensePlayerLineX(),
                                         -SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().ourDefensePlayerLineX(),
                                         -SP.pitchHalfWidth() ) );
        // bottom,lower
        debugClient().addLine( Vector2D( world(Opponenthasball
O).theirOffensePlayerLineX(),
                                         +SP.pitchHalfWidth() ),
                               Vector2D( world().theirOffensePlayerLineX(),
                                         +SP.pitchHalfWidth() + 3.0 ) );
        // bottom,lower
        debugClient().addLine( Vector2D( world().theirDefensePlayerLineX(),
                                         +SP.pitchHalfWidth() ),
                               Vector2D( world().theirDefensePlayerLineX(),
                                         +SP.pitchHalfWidth() + 3.0 ) );
#else
        // top,lower
        debugClient().addLine( Vector2D( world().ourDefenseLineX(),
                                         world().self().pos().y - 2.0 ),
                               Vector2D( world().ourDefenseLineX(),
                                         world().self().pos().y + 2.0 ) );

        //
        debugClient().addLine( Vector2D( world().offsideLineX(),
                                         world().self().pos().y - 15.0 ),
                               Vector2D( world().offsideLineX(),
                                         world().self().pos().y + 15.0 ) );
#endif
    }

    //
    // ball position & velocity
    //
    dlog.addText( Logger::WORLD,
                  "WM: BALL pos=(%lf, %lf), vel=(%lf, %lf, r=%lf, ang=%lf)",
                  world().ball().pos().x,
                  world().ball().pos().y,
                  world().ball().vel().x,
                  world().ball().vel().y,
                  world().ball().vel().r(),
                  world().ball().vel().th().degree() );


    dlog.addText( Logger::WORLD,
                  "WM: SELF move=(%lf, %lf, r=%lf, th=%lf)",
                  world().self().lastMove().x,
                  world().self().lastMove().y,
                  world().self().lastMove().r(),
                  world().self().lastMove().th().degree() );

    Vector2D diff = world().ball().rpos() - world().ball().rposPrev();
    dlog.addText( Logger::WORLD,
                  "WM: BALL rpos=(%lf %lf) prev_rpos=(%lf %lf) diff=(%lf %lf)",
                  world().ball().rpos().x,
                  world().ball().rpos().y,
                  world().ball().rposPrev().x,
                  world().ball().rposPrev().y,
                  diff.x,
                  diff.y );

    Vector2D ball_move = diff + world().self().lastMove();
    Vector2D diff_vel = ball_move * ServerParam::i().ballDecay();
    dlog.addText( Logger::WORLD,
                  "---> ball_move=(%lf %lf) vel=(%lf, %lf, r=%lf, th=%lf)",
                  ball_move.x,
                  ball_move.y,
                  diff_vel.x,
                  diff_vel.y,
                  diff_vel.r(),
                  diff_vel.th().degree() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleServerParam()
{
    if ( KickTable::instance().createTables() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << " KickTable created."
                  << std::endl;
    }
    else
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << " KickTable failed..."
                  << std::endl;
        M_client->setServerAlive( false );
    }


    if ( ServerParam::i().keepawayMode() )
    {
        std::cerr << "set Keepaway mode communication." << std::endl;
        M_communication = Communication::Ptr( new KeepawayCommunication() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handlePlayerParam()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handlePlayerType()
{

}

/*-------------------------------------------------------------------*/
/*!
  communication decision.
  virtual method in super class
*/
void
SamplePlayer::communicationImpl()
{
    if ( M_communication )
    {
        M_communication->execute( this );
    }
}

/*-------------------------------------------------------------------*/
/*!
*/
bool
SamplePlayer::doPreprocess()
{
    // check tackle expires
    // check self position accuracy
    // ball search
    // check queued intention
    // check simultaneous kick

    const WorldModel & wm = this->world();

    dlog.addText( Logger::TEAM,
                  __FILE__": (doPreProcess)" );

    //
    // freezed by tackle effect
    //
        if ( wm.self().isFrozen() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": tackle wait. expires= %d",
                      wm.self().tackleExpires() );
        // face neck to ball
        this->setViewAction( new View_Tactical() );
        this->setNeckAction( new Neck_TurnToBallOrScan() );
        return true;
    }
    

    //
    // BeforeKickOff or AfterGoal. jump to the initial position
    //
    if ( wm.gameMode().type() == GameMode::BeforeKickOff
         || wm.gameMode().type() == GameMode::AfterGoal_ )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": before_kick_off" );
        Vector2D move_point =  Strategy::i().getPosition( wm.self().unum() );
        Bhv_CustomBeforeKickOff( move_point ).execute( this );
        this->setViewAction( new View_Tactical() );
        return true;
    }

    //
    // self localization error
    //
    
    if ( ! wm.self().posValid() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": invalid my pos" );
        Bhv_Emergency().execute( this ); // includes change view
        return true;
    }
    

    //
    // ball localization error
    //
    
    const int count_thr = ( wm.self().goalie()
                            ? 10
                            : 5 );
    if ( wm.ball().posCount() > count_thr
         || ( wm.gameMode().type() != GameMode::PlayOn
              && wm.ball().seenPosCount() > count_thr + 10 ) )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": search ball" );
        this->setViewAction( new View_Tactical() );
        Bhv_NeckBodyToBall().execute( this );
        return true;
    }
    

    //
    // set default change view
    //

    this->setViewAction( new View_Tactical() );

    //
    // check shoot chance
    //
    
    if ( doShoot() )
    {
        std::cout<<"doShoot"<<std::endl;
        std::cout<<"*******************************************************************************"<<std::endl;

        return true;
    }
    

    //
    // check queued action
    //
    
    if ( this->doIntention() )
    {   
        std::cout<<"doIntention------------------------------------------------------------"<<std::endl;
        std::cout<<"*******************************************************************************"<<std::endl;
        dlog.addText( Logger::TEAM,
                      __FILE__": do queued intention" );
        return true;
    }

    //
    // check simultaneous kick
    //
    
    if ( doForceKick() )
    {   
        std::cout<<"doForceKick--------------------------------------------------------------"<<std::endl;
        std::cout<<"*******************************************************************************"<<std::endl;
        return true;
    }
    

    //
    // check pass message
    //
    
    if ( doHeardPassReceive() )
    {
        std::cout<<"doHeardPassReceive------------------------------------------------------"<<std::endl;
        std::cout<<"*******************************************************************************"<<std::endl;
        return true;
    }
    

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SamplePlayer::doShoot()
{
    const WorldModel & wm = this->world();

    if ( wm.gameMode().type() != GameMode::IndFreeKick_
         && wm.time().stopped() == 0
         && wm.self().isKickable()
         && Bhv_StrictCheckShoot().execute( this ) )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": shooted" );

        // reset intention
        this->setIntention( static_cast< SoccerIntention * >( 0 ) );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SamplePlayer::doForceKick()
{
    const WorldModel & wm = this->world();

    if ( wm.gameMode().type() == GameMode::PlayOn
         && ! wm.self().goalie()
         && wm.self().isKickable()
         && wm.existKickableOpponent() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": simultaneous kick" );
        this->debugClient().addMessage( "SimultaneousKick" );
        Vector2D goal_pos( ServerParam::i().pitchHalfLength(), 0.0 );

        if ( wm.self().pos().x > 36.0
             && wm.self().pos().absY() > 10.0 )
        {
            goal_pos.x = 45.0;
            dlog.addText( Logger::TEAM,
                          __FILE__": simultaneous kick cross type" );
        }
        Body_KickOneStep( goal_pos,
                          ServerParam::i().ballSpeedMax()
                          ).execute( this );
        this->setNeckAction( new Neck_ScanField() );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SamplePlayer::doHeardPassReceive()
{
    const WorldModel & wm = this->world();

    if ( wm.audioMemory().passTime() != wm.time()
         || wm.audioMemory().pass().empty()
         || wm.audioMemory().pass().front().receiver_ != wm.self().unum() )
    {
        return false;
    }

    //passHeard = true;

    //Vector2D closestOppPos = wm.getOpponentNearestToSelf(5, false)->pos();

    //closestOppDis = static_cast<int>(wm.self().pos().dist(closestOppPos));


    int self_min = wm.interceptTable()->selfReachCycle();
    Vector2D intercept_pos = wm.ball().inertiaPoint( self_min );
    Vector2D heard_pos = wm.audioMemory().pass().front().receive_pos_;

    dlog.addText( Logger::TEAM,
                  __FILE__":  (doHeardPassReceive) heard_pos(%.2f %.2f) intercept_pos(%.2f %.2f)",
                  heard_pos.x, heard_pos.y,
                  intercept_pos.x, intercept_pos.y );

    if ( ! wm.existKickableTeammate()
         && wm.ball().posCount() <= 1
         && wm.ball().velCount() <= 1
         && self_min < 20
         //&& intercept_pos.dist( heard_pos ) < 3.0 ) //5.0 )
         )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": (doHeardPassReceive) intercept cycle=%d. intercept",
                      self_min );
        this->debugClient().addMessage( "Comm:Receive:Intercept" );
        Body_Intercept().execute( this );
        this->setNeckAction( new Neck_TurnToBall() );
    }
    else
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": (doHeardPassReceive) intercept cycle=%d. go to receive point",
                      self_min );
        this->debugClient().setTarget( heard_pos );
        this->debugClient().addMessage( "Comm:Receive:GoTo" );
        Body_GoToPoint( heard_pos,
                    0.5,
                        ServerParam::i().maxDashPower()
                        ).execute( this );
        this->setNeckAction( new Neck_TurnToBall() );
    }

    this->setIntention( new IntentionReceive( heard_pos,
                                              ServerParam::i().maxDashPower(),
                                              0.9,
                                              5,
                                              wm.time() ) );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldEvaluator::ConstPtr
SamplePlayer::getFieldEvaluator() const
{
    return M_field_evaluator;
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldEvaluator::ConstPtr
SamplePlayer::createFieldEvaluator() const
{
    return FieldEvaluator::ConstPtr( new SampleFieldEvaluator );
}


/*-------------------------------------------------------------------*/
/*!
*/
#include "actgen_cross.h"
#include "actgen_direct_pass.h"
#include "actgen_self_pass.h"
#include "actgen_strict_check_pass.h"
#include "actgen_short_dribble.h"
#include "actgen_simple_dribble.h"
#include "actgen_shoot.h"
#include "actgen_action_chain_length_filter.h"

ActionGenerator::ConstPtr
SamplePlayer::createActionGenerator() const
{
    CompositeActionGenerator * g = new CompositeActionGenerator();

    //
    // shoot
    //
    g->addGenerator( new ActGen_RangeActionChainLengthFilter
                     ( new ActGen_Shoot(),
                       2, ActGen_RangeActionChainLengthFilter::MAX ) );

    //
    // strict check pass
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_StrictCheckPass(), 1 ) );

    //
    // cross
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_Cross(), 1 ) );

    //
    // direct pass
    //
    // g->addGenerator( new ActGen_RangeActionChainLengthFilter
    //                  ( new ActGen_DirectPass(),
    //                    2, ActGen_RangeActionChainLengthFilter::MAX ) );

    //
    // short dribble
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_ShortDribble(), 1 ) );

    //
    // self pass (long dribble)
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_SelfPass(), 1 ) );

    //
    // simple dribble
    //
    // g->addGenerator( new ActGen_RangeActionChainLengthFilter
    //                  ( new ActGen_SimpleDribble(),
    //                    2, ActGen_RangeActionChainLengthFilter::MAX ) );

    return ActionGenerator::ConstPtr( g );
}

