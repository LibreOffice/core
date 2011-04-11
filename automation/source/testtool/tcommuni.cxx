/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"


#include <tools/config.hxx>
#include <vcl/svapp.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>

#include <basic/ttstrhlp.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
