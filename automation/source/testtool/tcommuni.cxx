/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"


#include <tools/config.hxx>
#include <vcl/svapp.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include "rcontrol.hxx"
#include "tcommuni.hxx"
#include <basic/testtool.hxx>

CommunicationManagerClientViaSocketTT::CommunicationManagerClientViaSocketTT()
: CommunicationManagerClientViaSocket( sal_True )
, aAppPath()
, aAppParams()
, pProcess( NULL )
{
}


sal_Bool CommunicationManagerClientViaSocketTT::StartCommunication()
{
    bApplicationStarted = sal_False;
    return CommunicationManagerClientViaSocket::StartCommunication( ByteString( GetHostConfig(), RTL_TEXTENCODING_UTF8 ), GetTTPortConfig() );
}


sal_Bool CommunicationManagerClientViaSocketTT::StartCommunication( String aApp, String aParams, Environment *pChildEnv )
{
    aAppPath = aApp;
    aAppParams = aParams;
    aAppEnv = (*pChildEnv);
    return StartCommunication();
}


sal_Bool CommunicationManagerClientViaSocketTT::RetryConnect()
{
    if ( !bApplicationStarted )
    {
        // Die App ist wohl nicht da. Starten wir sie mal.
        if ( aAppPath.Len() )
        {
            delete pProcess;

            pProcess = new Process();
            pProcess->SetImage( aAppPath, aAppParams, &aAppEnv );

            sal_Bool bSucc = pProcess->Start();
            bApplicationStarted = sal_True;

            if ( bSucc )
            {
                aFirstRetryCall = Time() + Time( 0, 1 );    // Max eine Minute Zeit
                for ( int i = 10 ; i-- ; )
                    GetpApp()->Reschedule();
            }
            return bSucc;
        }
        return sal_False;
    }
    else
    {
        if ( aFirstRetryCall > Time() )
        {
            Timer aWait;
            aWait.SetTimeout( 500 );         // Max 500 mSec
            aWait.Start();
            while ( aWait.IsActive() )
                GetpApp()->Yield();
            return sal_True;
        }
        else
            return sal_False;
    }
}

sal_Bool CommunicationManagerClientViaSocketTT::KillApplication()
{
    if ( pProcess )
        return pProcess->Terminate();
    return sal_True;
}

#define GETSET(aVar, KeyName, Dafault)                 \
    aVar = aConf.ReadKey(KeyName,"No Entry");          \
    if ( aVar.CompareTo("No Entry") == COMPARE_EQUAL ) \
    {                                                  \
        aVar = ByteString(Dafault);                    \
        aConf.WriteKey(KeyName, aVar);                 \
    }


String GetHostConfig()
{
    String aHostToTalk;

    for ( sal_uInt16 i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,6).CompareIgnoreCaseToAscii("-host=") == COMPARE_EQUAL
#ifndef UNX
          || Application::GetCommandLineParam( i ).Copy(0,6).CompareIgnoreCaseToAscii("/host=") == COMPARE_EQUAL
#endif
          )
            return Application::GetCommandLineParam( i ).Copy(6);
    }

    ByteString abHostToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abHostToTalk, "Host", DEFAULT_HOST );
    return UniString( abHostToTalk, RTL_TEXTENCODING_UTF8 );
}


sal_uLong GetTTPortConfig()
{
    String aPortToTalk;

    for ( sal_uInt16 i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,6).CompareIgnoreCaseToAscii("-port=") == COMPARE_EQUAL
#ifndef UNX
          || Application::GetCommandLineParam( i ).Copy(0,6).CompareIgnoreCaseToAscii("/port=") == COMPARE_EQUAL
#endif
          )
        {
            aPortToTalk = Application::GetCommandLineParam( i ).Copy(6);
            return (sal_uLong)aPortToTalk.ToInt64();
        }
    }

    ByteString abPortToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abPortToTalk, "TTPort", ByteString::CreateFromInt32( TESTTOOL_DEFAULT_PORT ) );
    return (sal_uLong)abPortToTalk.ToInt64();
}


sal_uLong GetUnoPortConfig()
{
    String aPortToTalk;

    for ( sal_uInt16 i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,9).CompareIgnoreCaseToAscii("-unoport=") == COMPARE_EQUAL
#ifndef UNX
          || Application::GetCommandLineParam( i ).Copy(0,9).CompareIgnoreCaseToAscii("/unoport=") == COMPARE_EQUAL
#endif
          )
        {
            aPortToTalk = Application::GetCommandLineParam( i ).Copy(6);
            return (sal_uLong)aPortToTalk.ToInt64();
        }
    }

    ByteString abPortToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abPortToTalk, "UnoPort", ByteString::CreateFromInt32( UNO_DEFAULT_PORT ) );
    return (sal_uLong)abPortToTalk.ToInt64();
}
