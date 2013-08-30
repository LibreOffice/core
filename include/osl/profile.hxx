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
            size_t nItems = rStrings.size();
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
            size_t nItems = rStrings.size();
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

        /** Remove an entry from a section.
            @param rSection Name of the section.
            @param rEntry Name of the entry to remove.
            @return False if section or entry could not be found.
        */
        sal_Bool removeEntry(const rtl::OString& rSection, const rtl::OString& rEntry)
        {
            return osl_removeProfileEntry(profile, rSection.getStr(), rEntry.getStr());
        }

        /** Get all entries belonging to the specified section.
            @param rSection Name of the section.
            @return Pointer to a array of pointers.
        */
        std::list< rtl::OString > getSectionEntries(const rtl::OString& rSection )
        {
            std::list< rtl::OString > aEntries;

            // count buffer size necessary
            size_t n = osl_getProfileSectionEntries( profile, rSection.getStr(), NULL, 0 );
            if( n > 1 )
            {
                sal_Char* pBuf = new sal_Char[ n+1 ];
                osl_getProfileSectionEntries( profile, rSection.getStr(), pBuf, n+1 );
                size_t nLen;
                for( n = 0; ( nLen = strlen( pBuf+n ) ); n += nLen+1 )
                    aEntries.push_back( rtl::OString( pBuf+n ) );
                delete pBuf;
            }

            return aEntries;
        }

        /** Get all section entries
            @return Pointer to a array of pointers.
        */
        std::list< rtl::OString > getSections()
        {
            std::list< rtl::OString > aSections;

            // count buffer size necessary
            size_t n = osl_getProfileSections( profile, NULL, 0 );
            if( n > 1 )
            {
                sal_Char* pBuf = new sal_Char[ n+1 ];
                osl_getProfileSections( profile, pBuf, n+1 );
                size_t nLen;
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
