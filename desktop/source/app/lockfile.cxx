/*************************************************************************
 *
 *  $RCSfile: lockfile.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:45:22 $
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
 *  Contributor(s): lars.oppermann@sun.com
 *
 *
 ************************************************************************/
#include <stdlib.h>
#include <time.h>
#ifdef WNT
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
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


namespace desktop {

    // initialize static members...
    // lock suffix
    const OUString Lockfile::Suffix()
        { return OUString::createFromAscii( "/.lock" ); }
    // values for datafile
    const ByteString Lockfile::Group()
        { return ByteString( "Lockdata" ); }
    const ByteString Lockfile::Userkey()
        { return ByteString( "User" ); }
    const ByteString Lockfile::Hostkey()
        { return ByteString( "Host" ); }
    const ByteString Lockfile::Stampkey()
        { return ByteString( "Stamp" ); }
    const ByteString Lockfile::Timekey()
        { return ByteString( "Time" ); }
    const ByteString Lockfile::IPCkey()
        { return ByteString( "IPCServer" ); }

    Lockfile::Lockfile( bool bIPCserver )
    :m_bRemove(sal_False)
    ,m_bIsLocked(sal_False)
    ,m_bIPCserver(bIPCserver)
    {
        // build the file-url to use for the lock
        OUString aUserPath;
        Bootstrap::locateUserInstallation( aUserPath );
        m_aLockname = aUserPath + Suffix();

        // generate ID
        const int nIdBytes = 16;
        char tmpId[nIdBytes*2+1];
        time_t t;
        srand( (unsigned)(t = time( NULL )) );
        int tmpByte = 0;
        for (int i = 0; i<nIdBytes; i++) {
            tmpByte = rand( ) % 0xFF;
            sprintf( tmpId+i*2, "%02X", tmpByte ); // #100211# - checked
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
        if (aFile.open( OpenFlag_Create ) == File::E_EXIST) {
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
                aFile.open( OpenFlag_Create );
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
        String aLockname = m_aLockname;
        Config aConfig(aLockname);
        aConfig.SetGroup(Group());
        ByteString aIPCserver  = aConfig.ReadKey( IPCkey() );
        if (! aIPCserver.EqualsIgnoreCaseAscii( "true" ))
            return false;

        ByteString aHost  = aConfig.ReadKey( Hostkey() );
        ByteString aUser  = aConfig.ReadKey( Userkey() );
        // lockfile from same host?
        ByteString myHost;
#ifdef WNT
        /*
          prevent windows from connecting to the net to get it's own
          hostname by using the netbios name
        */
        sal_Int32 sz = MAX_COMPUTERNAME_LENGTH + 1;
        char* szHost = new char[sz];
        if (GetComputerName(szHost, (LPDWORD)&sz))
            myHost = OString(szHost);
        else
            myHost = OString("UNKNOWN");
        delete[] szHost;
#else
        oslSocketResult sRes;
        myHost  = OUStringToOString(
            SocketAddr::getLocalHostname( &sRes ), RTL_TEXTENCODING_ASCII_US );
#endif
        if (aHost == myHost) {
            // lockfile by same UID
            OUString myUserName;
            Security aSecurity;
            aSecurity.getUserName( myUserName );
            ByteString myUser  = OUStringToOString( myUserName, RTL_TEXTENCODING_ASCII_US );
            if (aUser == myUser)
                return sal_True;
        }
        return sal_False;
    }

    void Lockfile::syncToFile( void ) const
    {
        String aLockname = m_aLockname;
        Config aConfig(aLockname);
        aConfig.SetGroup(Group());

        // get information
        ByteString aHost;
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
        oslSocketResult sRes;
        aHost  = OUStringToOString(
            SocketAddr::getLocalHostname( &sRes ), RTL_TEXTENCODING_ASCII_US );
#endif
        OUString aUserName;
        Security aSecurity;
        aSecurity.getUserName( aUserName );
        ByteString aUser  = OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US );
        ByteString aTime  = OUStringToOString( m_aDate, RTL_TEXTENCODING_ASCII_US );
        ByteString aStamp = OUStringToOString( m_aId, RTL_TEXTENCODING_ASCII_US );

        // write information
        aConfig.WriteKey( Userkey(),  aUser );
        aConfig.WriteKey( Hostkey(),  aHost );
        aConfig.WriteKey( Stampkey(), aStamp );
        aConfig.WriteKey( Timekey(),  aTime );
        aConfig.WriteKey(
            IPCkey(),
            m_bIPCserver ? ByteString("true") : ByteString("false") );
        aConfig.Flush( );
    }

    void Lockfile::clean( void )
    {
        if ( m_bRemove )
        {
            File::remove( m_aLockname );
            m_bRemove = sal_False;
        }
    }

    Lockfile::~Lockfile( void )
    {
        // unlock userdata by removing file
        if ( m_bRemove )
            File::remove( m_aLockname );
    }
}









