/*************************************************************************
 *
 *  $RCSfile: lockfile.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 13:51:15 $
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
#include <sal/types.h>
#include <osl/file.hxx>
#include <osl/socket.hxx>
#include <osl/security.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/msgbox.hxx>
#include <tools/string.hxx>
#include <tools/config.hxx>

#include "desktopresid.hxx"
#include "lockfile.hxx"
#include "desktop.hrc"

using namespace ::osl;
using namespace ::rtl;
using namespace ::utl;


namespace desktop {

    // initialize static members...
    // lock suffix
    const OUString Lockfile::m_aSuffix = OUString::createFromAscii( "/.lock" );
    // values for datafile
    const ByteString Lockfile::m_aGroup( "Lockdata" );
    const ByteString Lockfile::m_aUserkey( "User" );
    const ByteString Lockfile::m_aHostkey( "Host" );
    const ByteString Lockfile::m_aStampkey( "Stamp" );
    const ByteString Lockfile::m_aTimekey( "Time" );

    Lockfile::Lockfile(void)
    :m_bRemove(sal_False)
    ,m_bIsLocked(sal_False)
    {
        // build the file-url to use for the lock
        OUString aUserPath;
        Bootstrap::locateUserInstallation( aUserPath );
        m_aLockname = aUserPath + m_aSuffix;

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
        tmpTime[24] = 0x00; // buffer is always 26 chars, remove '\n'
        m_aDate = OUString::createFromAscii( tmpTime );

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

    sal_Bool Lockfile::check( void )
    {

        if (m_bIsLocked) {
            // lock existed, ask user what to do
            if (isStale() || execWarning( ) == RET_YES) {
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
        // to assume that it is a stale lookfile which can be overwritten
        String aLockname = m_aLockname;
        Config aConfig(aLockname);
        aConfig.SetGroup(m_aGroup);
        ByteString aHost  = aConfig.ReadKey( m_aHostkey );
        ByteString aUser  = aConfig.ReadKey( m_aUserkey );
        // lockfile from same host?
        oslSocketResult sRes;
        ByteString myHost  = OUStringToOString(
            SocketAddr::getLocalHostname( &sRes ), RTL_TEXTENCODING_ASCII_US );
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
        aConfig.SetGroup(m_aGroup);

        // get information
        oslSocketResult sRes;
        ByteString aHost  = OUStringToOString(
            SocketAddr::getLocalHostname( &sRes ), RTL_TEXTENCODING_ASCII_US );
        OUString aUserName;
        Security aSecurity;
        aSecurity.getUserName( aUserName );
        ByteString aUser  = OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US );
        ByteString aTime  = OUStringToOString( m_aDate, RTL_TEXTENCODING_ASCII_US );
        ByteString aStamp = OUStringToOString( m_aId, RTL_TEXTENCODING_ASCII_US );

        // write information
        aConfig.WriteKey( m_aUserkey,  aUser );
        aConfig.WriteKey( m_aHostkey,  aHost );
        aConfig.WriteKey( m_aStampkey, aStamp );
        aConfig.WriteKey( m_aTimekey,  aTime );
        aConfig.Flush( );
    }

    short Lockfile::execWarning(void) const
    {
        // read information from lock
        String aLockname = m_aLockname;
        Config aConfig(aLockname);
        aConfig.SetGroup(m_aGroup);
        ByteString aHost  = aConfig.ReadKey( m_aHostkey );
        ByteString aUser  = aConfig.ReadKey( m_aUserkey );
        ByteString aStamp = aConfig.ReadKey( m_aStampkey );
        ByteString aTime  = aConfig.ReadKey( m_aTimekey );

        // display warning and return response
        QueryBox aBox( NULL, DesktopResId( QBX_USERDATALOCKED ) );
        // set box title
        String aTitle = String( DesktopResId( STR_TITLE_USERDATALOCKED ));
        aBox.SetText( aTitle );
        // insert values...
        String aMsgText = aBox.GetMessText( );
        aMsgText.SearchAndReplaceAscii( "$u", String( aUser, RTL_TEXTENCODING_ASCII_US) );
        aMsgText.SearchAndReplaceAscii( "$h", String( aHost, RTL_TEXTENCODING_ASCII_US) );
        aMsgText.SearchAndReplaceAscii( "$t", String( aTime, RTL_TEXTENCODING_ASCII_US) );
        aBox.SetMessText(aMsgText);
        // do it
        return aBox.Execute( );
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
