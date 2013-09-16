/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdlib.h>
#include <time.h>
#ifndef WNT
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <sal/types.h>
#include <osl/file.hxx>
#include <osl/socket.hxx>
#include <osl/security.hxx>
#include <unotools/bootstrap.hxx>
#include <tools/string.hxx>
#include <tools/config.hxx>

#include "lockfile.hxx"


using namespace ::osl;
using namespace ::rtl;
using namespace ::utl;


static OString impl_getHostname()
{
    OString aHost;
#ifdef WNT
    /*
       prevent windows from connecting to the net to get it's own
       hostname by using the netbios name
       */
    sal_Int32 sz = MAX_COMPUTERNAME_LENGTH + 1;
    char* szHost = new char[sz];
    if (GetComputerName(szHost, (LPDWORD)&sz))
        aHost = OString(szHost);
    else
        aHost = OString("UNKNOWN");
    delete[] szHost;
#else
    /* Don't do dns lookup on Linux either */
    sal_Char pHostName[1024];

    if ( gethostname( pHostName, sizeof( pHostName ) - 1 ) == 0 )
    {
        pHostName[sizeof( pHostName ) - 1] = '\0';
        aHost = OString( pHostName );
    }
    else
        aHost = OString("UNKNOWN");
#endif

    return aHost;
}

namespace desktop {

    Lockfile::Lockfile( bool bIPCserver )
    :m_bIPCserver(bIPCserver)
    ,m_bRemove(sal_False)
    ,m_bIsLocked(sal_False)
    {
        // build the file-url to use for the lock
        OUString aUserPath;
        utl::Bootstrap::locateUserInstallation( aUserPath );
        m_aLockname = aUserPath + LOCKFILE_SUFFIX;

        // generate ID
        const int nIdBytes = 16;
        char tmpId[nIdBytes*2+1];
        time_t t;
        srand( (unsigned)(t = time( NULL )) );
        int tmpByte = 0;
        for (int i = 0; i<nIdBytes; i++) {
            tmpByte = rand( ) % 0xFF;
            sprintf( tmpId+i*2, "%02X", tmpByte );
        }
        tmpId[nIdBytes*2]=0x00;
        m_aId = OUString::createFromAscii( tmpId );

        // generate date string
        char *tmpTime = ctime( &t );
        if (tmpTime != NULL) {
            m_aDate = OUString::createFromAscii( tmpTime );
            sal_Int32 i = m_aDate.indexOf('\n');
            if (i > 0)
                m_aDate = m_aDate.copy(0, i);
        }


        // try to create file
        File aFile(m_aLockname);
        if (aFile.open( osl_File_OpenFlag_Create ) == File::E_EXIST) {
            m_bIsLocked = sal_True;
        } else {
            // new lock created
            aFile.close( );
            syncToFile( );
            m_bRemove = sal_True;
        }
    }

    sal_Bool Lockfile::check( fpExecWarning execWarning )
    {

        if (m_bIsLocked) {
            // lock existed, ask user what to do
            if (isStale() ||
                (execWarning != 0 && (*execWarning)( this ))) {
                // remove file and create new
                File::remove( m_aLockname );
                File aFile(m_aLockname);
                aFile.open( osl_File_OpenFlag_Create );
                aFile.close( );
                syncToFile( );
                m_bRemove = sal_True;
                return sal_True;
            } else {
                //leave alone and return false
                m_bRemove = sal_False;
                return sal_False;
            }
        } else {
            // lock was created by us
            return sal_True;
        }
    }

    sal_Bool Lockfile::isStale( void ) const
    {
        // this checks whether the lockfile was created on the same
        // host by the same user. Should this be the case it is safe
        // to assume that it is a stale lockfile which can be overwritten
        OUString aLockname = m_aLockname;
        Config aConfig(aLockname);
        aConfig.SetGroup(LOCKFILE_GROUP);
        OString aIPCserver  = aConfig.ReadKey( LOCKFILE_IPCKEY );
        if (!aIPCserver.equalsIgnoreAsciiCase(OString("true")))
            return false;

        OString aHost = aConfig.ReadKey( LOCKFILE_HOSTKEY );
        OString aUser = aConfig.ReadKey( LOCKFILE_USERKEY );

        // lockfile from same host?
        OString myHost( impl_getHostname() );
        if (aHost == myHost) {
            // lockfile by same UID
            OUString myUserName;
            Security aSecurity;
            aSecurity.getUserName( myUserName );
            OString myUser(OUStringToOString(myUserName, RTL_TEXTENCODING_ASCII_US));
            if (aUser == myUser)
                return sal_True;
        }
        return sal_False;
    }

    void Lockfile::syncToFile( void ) const
    {
        OUString aLockname = m_aLockname;
        Config aConfig(aLockname);
        aConfig.SetGroup(LOCKFILE_GROUP);

        // get information
        OString aHost( impl_getHostname() );
        OUString aUserName;
        Security aSecurity;
        aSecurity.getUserName( aUserName );
        OString aUser  = OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US );
        OString aTime  = OUStringToOString( m_aDate, RTL_TEXTENCODING_ASCII_US );
        OString aStamp = OUStringToOString( m_aId, RTL_TEXTENCODING_ASCII_US );

        // write information
        aConfig.WriteKey( LOCKFILE_USERKEY,  aUser );
        aConfig.WriteKey( LOCKFILE_HOSTKEY,  aHost );
        aConfig.WriteKey( LOCKFILE_STAMPKEY, aStamp );
        aConfig.WriteKey( LOCKFILE_TIMEKEY,  aTime );
        aConfig.WriteKey(
            LOCKFILE_IPCKEY,
            m_bIPCserver ? OString("true") : OString("false") );
        aConfig.Flush( );
    }

    Lockfile::~Lockfile( void )
    {
        // unlock userdata by removing file
        if ( m_bRemove )
            File::remove( m_aLockname );
    }
}









/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
