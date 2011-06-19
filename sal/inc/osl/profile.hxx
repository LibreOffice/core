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

#ifndef _OSL_PROFILE_HXX_
#define _OSL_PROFILE_HXX_

#include "profile.h"
#include <rtl/ustring.hxx>
#include <string.h>
#include <list>

namespace osl {

    typedef oslProfileOption ProfileOption;

    const int Profile_DEFAULT   = osl_Profile_DEFAULT;
    const int Profile_SYSTEM    = osl_Profile_SYSTEM;    /* use system depended functinality */
    const int Profile_READLOCK  = osl_Profile_READLOCK;  /* lock file for reading            */
    const int Profile_WRITELOCK = osl_Profile_WRITELOCK; /* lock file for writing            */

    /** Deprecated API.
        @deprecated
    */
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
            osl_closeProfile(profile);
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
                                          rSection.getStr(),
                                          rEntry.getStr(),
                                          aBuf,
                                          sizeof( aBuf ),
                                          rDefault.getStr() ) ? rtl::OString( aBuf ) : rtl::OString();

        }

        sal_Bool readBool( const rtl::OString& rSection, const rtl::OString& rEntry, sal_Bool bDefault )
        {
            return osl_readProfileBool( profile, rSection.getStr(), rEntry.getStr(), bDefault );
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
                pStrings[ nItems++ ] = it->getStr();
                ++it;
            }
            pStrings[ nItems ] = NULL;
            sal_uInt32 nRet = osl_readProfileIdent(profile, rSection.getStr(), rEntry.getStr(), nFirstId, pStrings, nDefault);
            delete pStrings;
            return nRet;
        }

        sal_Bool writeString(const rtl::OString& rSection, const rtl::OString& rEntry,
                             const rtl::OString& rString)
        {
            return osl_writeProfileString(profile, rSection.getStr(), rEntry.getStr(), rString.getStr());
        }

        sal_Bool writeBool(const rtl::OString& rSection, const rtl::OString& rEntry, sal_Bool Value)
        {
            return osl_writeProfileBool(profile, rSection.getStr(), rEntry.getStr(), Value);
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
                pStrings[ nItems++ ] = it->getStr();
                ++it;
            }
            pStrings[ nItems ] = NULL;
            sal_Bool bRet =
                osl_writeProfileIdent(profile, rSection.getStr(), rEntry.getStr(), nFirstId, pStrings, nValue );
            delete pStrings;
            return bRet;
        }

        /** Acquire the mutex, block if already acquired by another thread.
            @param Profile handle to a opened profile.
            @return False if section or entry could not be found.
        */
        sal_Bool removeEntry(const rtl::OString& rSection, const rtl::OString& rEntry)
        {
            return osl_removeProfileEntry(profile, rSection.getStr(), rEntry.getStr());
        }

        /** Get all entries belonging to the specified section.
            @param Profile handle to a opened profile.
            @return Pointer to a array of pointers.
        */
        std::list< rtl::OString > getSectionEntries(const rtl::OString& rSection )
        {
            std::list< rtl::OString > aEntries;

            // count buffer size necessary
            int n = osl_getProfileSectionEntries( profile, rSection.getStr(), NULL, 0 );
            if( n > 1 )
            {
                sal_Char* pBuf = new sal_Char[ n+1 ];
                osl_getProfileSectionEntries( profile, rSection.getStr(), pBuf, n+1 );
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
    };
}

#endif  /* _OSL_PROFILE_HXX_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
