/*************************************************************************
 *
 *  $RCSfile: profile.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: martin.maher $ $Date: 2000-09-29 14:39:31 $
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

#ifndef _OSL_PROFILE_HXX_
#define _OSL_PROFILE_HXX_

#include "profile.h"

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#include <list>

namespace osl {

    typedef oslProfileOption ProfileOption;

    const int Profile_DEFAULT   = osl_Profile_DEFAULT;
    const int Profile_SYSTEM    = osl_Profile_SYSTEM;    /* use system depended functinality */
    const int Profile_READLOCK  = osl_Profile_READLOCK;  /* lock file for reading            */
    const int Profile_WRITELOCK = osl_Profile_WRITELOCK; /* lock file for writing            */

    class Profile {
        oslProfile profile;

    public:
        /** Open or create a configuration profile.
            @return 0 if the profile could not be created, otherwise a handle to the profile.
        */
        Profile(const rtl::OUString strProfileName, oslProfileOption Options = Profile_DEFAULT )
        {
            profile = osl_openProfile(strProfileName.pData, Options);
            if( ! profile )
                throw std::exception();
        }


        /** Close the opened profile an flush all data to the disk.
            @param Profile handle to a opened profile.
        */
        ~Profile()
        {
            sal_Bool err = osl_closeProfile(profile);
        }


        sal_Bool flush()
        {
            return osl_flushProfile(profile);
        }

        rtl::OString readString( const rtl::OString& rSection, const rtl::OString& rEntry,
                                 const rtl::OString& rDefault)
        {
            sal_Char aBuf[1024];
            return osl_readProfileString( profile,
                                          rSection,
                                          rEntry,
                                          aBuf,
                                          sizeof( aBuf ),
                                          rDefault ) ? rtl::OString( aBuf ) : rtl::OString();

        }

        sal_Bool readBool( const rtl::OString& rSection, const rtl::OString& rEntry, sal_Bool bDefault )
        {
            return osl_readProfileBool( profile, rSection, rEntry, bDefault );
        }

        sal_uInt32 readIdent(const rtl::OString& rSection, const rtl::OString& rEntry,
                             sal_uInt32 nFirstId, const std::list< rtl::OString >& rStrings,
                             sal_uInt32 nDefault)
        {
            int nItems = rStrings.size();
            const sal_Char** pStrings = new const sal_Char*[ nItems+1 ];
            std::list< rtl::OString >::const_iterator it = rStrings.begin();
            nItems = 0;
            while( it != rStrings.end() )
            {
                pStrings[ nItems++ ] = *it;
                ++it;
            }
            pStrings[ nItems ] = NULL;
            sal_uInt32 nRet = osl_readProfileIdent(profile, rSection, rEntry, nFirstId, pStrings, nDefault);
            delete pStrings;
            return nRet;
        }

        sal_Bool writeString(const rtl::OString& rSection, const rtl::OString& rEntry,
                             const rtl::OString& rString)
        {
            return osl_writeProfileString(profile, rSection, rEntry, rString);
        }

        sal_Bool writeBool(const rtl::OString& rSection, const rtl::OString& rEntry, sal_Bool Value)
        {
            return osl_writeProfileBool(profile, rSection, rEntry, Value);
        }

        sal_Bool writeIdent(const rtl::OString& rSection, const rtl::OString& rEntry,
                            sal_uInt32 nFirstId, const std::list< rtl::OString >& rStrings,
                            sal_uInt32 nValue)
        {
            int nItems = rStrings.size();
            const sal_Char** pStrings = new const sal_Char*[ nItems+1 ];
            std::list< rtl::OString >::const_iterator it = rStrings.begin();
            nItems = 0;
            while( it != rStrings.end() )
            {
                pStrings[ nItems++ ] = *it;
                ++it;
            }
            pStrings[ nItems ] = NULL;
            sal_Bool bRet =
                osl_writeProfileIdent(profile, rSection, rEntry, nFirstId, pStrings, nValue );
            delete pStrings;
            return bRet;
        }

        /** Acquire the mutex, block if already acquired by another thread.
            @param Profile handle to a opened profile.
            @return False if section or entry could not be found.
        */
        sal_Bool removeEntry(const rtl::OString& rSection, const rtl::OString& rEntry)
        {
            return osl_removeProfileEntry(profile, rSection, rEntry);
        }

        /** Get all entries belonging to the specified section.
            @param Profile handle to a opened profile.
            @return Pointer to a array of pointers.
        */
        std::list< rtl::OString > getSectionEntries(const rtl::OString& rSection )
        {
            std::list< rtl::OString > aEntries;

            // count buffer size necessary
            int n = osl_getProfileSectionEntries( profile, rSection, NULL, 0 );
            if( n > 1 )
            {
                sal_Char* pBuf = new sal_Char[ n+1 ];
                osl_getProfileSectionEntries( profile, rSection, pBuf, n+1 );
                int nLen;
                for( n = 0; ( nLen = strlen( pBuf+n ) ); n += nLen+1 )
                    aEntries.push_back( rtl::OString( pBuf+n ) );
                delete pBuf;
            }

            return aEntries;
        }

        /** Get all section entries
            @param Profile handle to a opened profile.
            @return Pointer to a array of pointers.
        */
        std::list< rtl::OString > getSections()
        {
            std::list< rtl::OString > aSections;

            // count buffer size necessary
            int n = osl_getProfileSections( profile, NULL, 0 );
            if( n > 1 )
            {
                sal_Char* pBuf = new sal_Char[ n+1 ];
                osl_getProfileSections( profile, pBuf, n+1 );
                int nLen;
                for( n = 0; ( nLen = strlen( pBuf+n ) ); n += nLen+1 )
                    aSections.push_back( rtl::OString( pBuf+n ) );
                delete pBuf;
            }

            return aSections;
        }

        static rtl::OUString getName(const rtl::OUString& rPath, const rtl::OUString& rName)
        {
            rtl::OUString aProfileName;
            return osl_getProfileName( rPath.pData, rName.pData, &aProfileName.pData ) ? aProfileName : rtl::OUString();
        }

    };
}

#endif  /* _OSL_PROFILE_HXX_ */


