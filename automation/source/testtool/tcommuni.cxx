/*************************************************************************
 *
 *  $RCSfile: tcommuni.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 16:03:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
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

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include "rcontrol.hxx"
#include "tcommuni.hxx"
#include <basic/testtool.hxx>
#include <basic/process.hxx>

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


BOOL CommunicationManagerClientViaSocketTT::StartCommunication( String aApp, String aParams )
{
    aAppPath = aApp;
    aAppParams = aParams;
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
            pProcess->SetImage( aAppPath, aAppParams );

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
            for ( int i = 10 ; i-- ; )
                GetpApp()->Reschedule();
            return TRUE;
        }
        else
            return FALSE;
    }
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
            return aPortToTalk.ToInt64();
        }
    }

    ByteString abPortToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abPortToTalk, "TTPort", ByteString::CreateFromInt32( TESTTOOL_DEFAULT_PORT ) );
    return abPortToTalk.ToInt64();
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
            return aPortToTalk.ToInt64();
        }
    }

    ByteString abPortToTalk;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Communication");

    GETSET( abPortToTalk, "UnoPort", ByteString::CreateFromInt32( UNO_DEFAULT_PORT ) );
    return abPortToTalk.ToInt64();
}
