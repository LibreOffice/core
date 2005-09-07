/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tcommuni.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:33:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#include <vcl/timer.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include "rcontrol.hxx"
#include "tcommuni.hxx"
#include <basic/testtool.hxx>

CommunicationManagerClientViaSocketTT::CommunicationManagerClientViaSocketTT()
: CommunicationManagerClientViaSocket( TRUE )
, aAppPath()
, aAppParams()
, pProcess( NULL )
{
}


BOOL CommunicationManagerClientViaSocketTT::StartCommunication()
{
    bApplicationStarted = FALSE;
    return CommunicationManagerClientViaSocket::StartCommunication( ByteString( GetHostConfig(), RTL_TEXTENCODING_UTF8 ), GetTTPortConfig() );
}


BOOL CommunicationManagerClientViaSocketTT::StartCommunication( String aApp, String aParams, Environment *pChildEnv )
{
    aAppPath = aApp;
    aAppParams = aParams;
    aAppEnv = (*pChildEnv);
    return StartCommunication();
}


BOOL CommunicationManagerClientViaSocketTT::RetryConnect()
{
    if ( !bApplicationStarted )
    {
        // Die App ist wohl nicht da. Starten wir sie mal.
        if ( aAppPath.Len() )
        {
            delete pProcess;

            pProcess = new Process();
            pProcess->SetImage( aAppPath, aAppParams, &aAppEnv );

            BOOL bSucc = pProcess->Start();
            bApplicationStarted = TRUE;

            if ( bSucc )
            {
                aFirstRetryCall = Time() + Time( 0, 1 );    // Max eine Minute Zeit
                for ( int i = 10 ; i-- ; )
                    GetpApp()->Reschedule();
            }
            return bSucc;
        }
        return FALSE;
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
            return TRUE;
        }
        else
            return FALSE;
    }
}

BOOL CommunicationManagerClientViaSocketTT::KillApplication()
{
    if ( pProcess )
        return pProcess->Terminate();
    return TRUE;
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

    for ( int i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
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


ULONG GetTTPortConfig()
{
    String aPortToTalk;

    for ( int i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,6).CompareIgnoreCaseToAscii("-port=") == COMPARE_EQUAL
#ifndef UNX
          || Application::GetCommandLineParam( i ).Copy(0,6).CompareIgnoreCaseToAscii("/port=") == COMPARE_EQUAL
#endif
          )
        {
            aPortToTalk = Application::GetCommandLineParam( i ).Copy(6);
            return (ULONG)aPortToTalk.ToInt64();
        }
    }

    ByteString abPortToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abPortToTalk, "TTPort", ByteString::CreateFromInt32( TESTTOOL_DEFAULT_PORT ) );
    return (ULONG)abPortToTalk.ToInt64();
}


ULONG GetUnoPortConfig()
{
    String aPortToTalk;

    for ( int i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,9).CompareIgnoreCaseToAscii("-unoport=") == COMPARE_EQUAL
#ifndef UNX
          || Application::GetCommandLineParam( i ).Copy(0,9).CompareIgnoreCaseToAscii("/unoport=") == COMPARE_EQUAL
#endif
          )
        {
            aPortToTalk = Application::GetCommandLineParam( i ).Copy(6);
            return (ULONG)aPortToTalk.ToInt64();
        }
    }

    ByteString abPortToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abPortToTalk, "UnoPort", ByteString::CreateFromInt32( UNO_DEFAULT_PORT ) );
    return (ULONG)abPortToTalk.ToInt64();
}
