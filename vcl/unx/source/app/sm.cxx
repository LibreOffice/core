/*************************************************************************
 *
 *  $RCSfile: sm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <unistd.h>
#include <stdio.h>

#include <vos/process.hxx>
#include <vos/security.hxx>

#ifndef _VCL_SM_HXX
#include <sm.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_CONFIG_HXX
#include <config.hxx>
#endif

//#define USE_SM_EXTENSION

BOOL ICEConnectionObserver::bIsWatching = FALSE;
SmcConn SessionManagerClient::aSmcConnection = NULL;
char* SessionManagerClient::pClientID = NULL;

static SmProp*  pSmProps = NULL;
static SmProp** ppSmProps = NULL;
static int      nSmProps = 0;

static void BuildSmPropertyList()
{
    if( ! pSmProps )
    {
        ByteString aExec( SessionManagerClient::getExecName(), gsl_getSystemTextEncoding() );

        nSmProps = 4;
        pSmProps = new SmProp[ nSmProps ];

        pSmProps[ 0 ].name      = "SmCloneCommand";
        pSmProps[ 0 ].type      = "SmLISTofARRAY8";
        pSmProps[ 0 ].num_vals  = 1;
        pSmProps[ 0 ].vals      = new SmPropValue;
        pSmProps[ 0 ].vals->length  = aExec.Len();
        pSmProps[ 0 ].vals->value   = strdup( aExec.GetBuffer() );

        pSmProps[ 1 ].name      = "SmProgram";
        pSmProps[ 1 ].type      = "SmARRAY8";
        pSmProps[ 1 ].num_vals  = 1;
        pSmProps[ 1 ].vals      = new SmPropValue;
        pSmProps[ 1 ].vals->length  = aExec.Len();
        pSmProps[ 1 ].vals->value   = strdup( aExec.GetBuffer() );

        pSmProps[ 2 ].name      = "SmRestartCommand";
        pSmProps[ 2 ].type      = "SmLISTofARRAY8";
        pSmProps[ 2 ].num_vals  = 1;
        pSmProps[ 2 ].vals      = new SmPropValue;
        pSmProps[ 2 ].vals->length  = aExec.Len();
        pSmProps[ 2 ].vals->value   = strdup( aExec.GetBuffer() );

        NAMESPACE_VOS(OSecurity) aSecurity;
        ::rtl::OUString aUserName;
        aSecurity.getUserName( aUserName );
        ::rtl::OString aUser( ::rtl::OUStringToOString( aUserName, gsl_getSystemTextEncoding() ) );

        pSmProps[ 3 ].name      = "SmUserID";
        pSmProps[ 3 ].type      = "SmARRAY8";
        pSmProps[ 3 ].num_vals  = 1;
        pSmProps[ 3 ].vals      = new SmPropValue;
        pSmProps[ 3 ].vals->value   = strdup( aUser.getStr() );
        pSmProps[ 3 ].vals->length  = strlen( (char *)pSmProps[ 3 ].vals->value );

        ppSmProps = new SmProp*[ nSmProps ];
        for( int i = 0; i < nSmProps; i++ )
            ppSmProps[ i ] = &pSmProps[i];
    }
}


void SessionManagerClient::SaveYourselfProc(
    SmcConn connection,
    SmPointer client_data,
    int save_type,
    Bool shutdown,
    int interact_style,
    Bool fast
    )
{
    BuildSmPropertyList();
#ifdef USE_SM_EXTENSION
    SmcSetProperties( aSmcConnection, nSmProps, ppSmProps );
    SmcSaveYourselfDone( aSmcConnection, True );
#endif
}

void SessionManagerClient::DieProc(
    SmcConn connection,
    SmPointer client_data
    )
{
#ifdef USE_SM_EXTENSION
    SmcCloseConnection( connection, 0, NULL );
#endif
    if( connection == aSmcConnection )
        aSmcConnection = NULL;
}

void SessionManagerClient::SaveCompleteProc(
    SmcConn connection,
    SmPointer client_data
    )
{
}

void SessionManagerClient::ShutdownCanceledProc(
    SmcConn connection,
    SmPointer client_data )
{
}

void SessionManagerClient::open()
{
    static SmcCallbacks aCallbacks;

#ifdef USE_SM_EXTENSION
    // erst scharf schalten wenn getestet

    // this is the way Xt does it, so we can too
    if( ! aSmcConnection && getenv( "SESSION_MANAGER" ) )
    {
        char aErrBuf[1024];
        String aFilename( getenv( "HOME" ) );
        aFilename += "/.so_lastSessionID";

        ICEConnectionObserver::activate();

        aCallbacks.save_yourself.callback           = SaveYourselfProc;
        aCallbacks.save_yourself.client_data        = NULL;
        aCallbacks.die.callback                     = DieProc;
        aCallbacks.die.client_data                  = NULL;
        aCallbacks.save_complete.callback           = SaveCompleteProc;
        aCallbacks.save_complete.client_data        = NULL;
        aCallbacks.shutdown_cancelled.callback      = ShutdownCanceledProc;
        aCallbacks.shutdown_cancelled.client_data   = NULL;
        aSmcConnection = SmcOpenConnection( NULL,
                                            NULL,
                                            SmProtoMajor,
                                            SmProtoMinor,
                                            SmcSaveYourselfProcMask         |
                                            SmcDieProcMask                  |
                                            SmcSaveCompleteProcMask         |
                                            SmcShutdownCancelledProcMask    ,
                                            &aCallbacks,
                                            getPreviousSessionID(),
                                            &pClientID,
                                            sizeof( aErrBuf ),
                                            aErrBuf );
#if defined DEBUG || defined DBG_UTIL
        if( ! aSmcConnection )
            fprintf( stderr, "SmcOpenConnection failed: %s\n", aErrBuf );
#endif
        setPreviousSessionID( pClientID );
    }
#endif
}

void SessionManagerClient::close()
{
    if( aSmcConnection )
    {
#ifdef USE_SM_EXTENSION
        SmcCloseConnection( aSmcConnection, 0, NULL );
#endif
        aSmcConnection = NULL;
        ICEConnectionObserver::deactivate();
    }
}


String SessionManagerClient::getExecName()
{
    static NAMESPACE_VOS( OStartupInfo ) aStartupInfo;

    ::rtl::OUString aExec;
    aStartupInfo.getExecutableFile( aExec );

    int nPos = aExec.search( ::rtl::OUString::createFromAscii( ".bin" ) );
    if( nPos != -1 )
        aExec = aExec.copy( 0, nPos );
    return aExec;
}


char* SessionManagerClient::getPreviousSessionID()
{
    static char aID[1024];

    String aFilename( getenv( "HOME" ), gsl_getSystemTextEncoding() );
    aFilename.AppendAscii( "/.sosessions" );

    ByteString aExec( getExecName(), gsl_getSystemTextEncoding() );
    aExec = aExec.GetToken( aExec.GetTokenCount( '/' )-1, '/' );

    Config aConfig( aFilename );
    aConfig.SetGroup( "Sessions" );
    ByteString aSessionID = aConfig.ReadKey( aExec );
    if( aSessionID.Len() )
    {
        strncpy( aID, aSessionID.GetBuffer(), sizeof( aID ) );
        return aID;
    }
    return NULL;
}

void SessionManagerClient::setPreviousSessionID( const ByteString& rID )
{
    static char aID[1024];

    String aFilename( getenv( "HOME" ), gsl_getSystemTextEncoding() );
    aFilename.AppendAscii( "/.sosessions" );

    ByteString aExec( getExecName(), gsl_getSystemTextEncoding() );
    aExec = aExec.GetToken( aExec.GetTokenCount( '/' )-1, '/' );

    Config aConfig( aFilename );
    aConfig.SetGroup( "Sessions" );
    aConfig.WriteKey( aExec, rID );
}

void ICEConnectionObserver::activate()
{
    if( ! bIsWatching )
    {
        bIsWatching = TRUE;
#ifdef USE_SM_EXTENSION
        IceAddConnectionWatch( ICEWatchProc, NULL );
#endif
    }
}

void ICEConnectionObserver::deactivate()
{
    if( bIsWatching )
    {
        bIsWatching = FALSE;
#ifdef USE_SM_EXTENSION
        IceRemoveConnectionWatch( ICEWatchProc, NULL );
#endif
    }
}

void ICEConnectionObserver::ICEWatchProc(
    IceConn connection,
    IcePointer client_data,
    Bool opening,
    IcePointer* watch_data
    )
{
#ifdef USE_SM_EXTENSION
    if( opening )
        GetSalData()->GetLib()->Insert( IceConnectionNumber( connection ),
                                        connection,
                                        (YieldFunc)Pending,
                                        (YieldFunc)Queued,
                                        (YieldFunc)HandleEvents );
    else
        GetSalData()->GetLib()->Remove( IceConnectionNumber( connection ) );
#endif
}

int ICEConnectionObserver::Pending( int fd, void* data )
{
    return 1;
}

int ICEConnectionObserver::Queued( int fd, void* data )
{
    return 1;
}

int ICEConnectionObserver::HandleEvents( int fd, void* data )
{
#ifdef USE_SM_EXTENSION
    IceProcessMessages( (IceConn)data, NULL, NULL );
#endif
    return 0;
}

