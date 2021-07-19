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

#include <resource/sharedresources.hxx>

#include <unotools/resmgr.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

namespace connectivity
{
    namespace {

    class SharedResources_Impl
    {
    private:
        static  SharedResources_Impl*   s_pInstance;
        static  oslInterlockedCount     s_nClients;

    private:
        std::locale                     m_aLocale;

    public:
        static void     registerClient();
        static void     revokeClient();

        static SharedResources_Impl&
                        getInstance();

        OUString getResourceString(TranslateId pId);

    private:
        SharedResources_Impl();

        static ::osl::Mutex& getMutex()
        {
            static ::osl::Mutex s_aMutex;
            return s_aMutex;
        }
    };

    }

    SharedResources_Impl*   SharedResources_Impl::s_pInstance( nullptr );
    oslInterlockedCount     SharedResources_Impl::s_nClients( 0 );

    SharedResources_Impl::SharedResources_Impl()
        : m_aLocale(Translate::Create("cnr"))
    {
    }

    OUString SharedResources_Impl::getResourceString(TranslateId pId)
    {
        return Translate::get(pId, m_aLocale);
    }

    void SharedResources_Impl::registerClient()
    {
        osl_atomic_increment( &s_nClients );
    }

    void SharedResources_Impl::revokeClient()
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( 0 == osl_atomic_decrement( &s_nClients ) )
        {
            delete s_pInstance;
            s_pInstance = nullptr;
        }
    }


    SharedResources_Impl& SharedResources_Impl::getInstance()
    {
        ::osl::MutexGuard aGuard( getMutex() );
        OSL_ENSURE( s_nClients > 0, "SharedResources_Impl::getInstance: no active clients!" );

        if ( !s_pInstance )
            s_pInstance = new SharedResources_Impl;

        return *s_pInstance;
    }

    namespace
    {
        size_t lcl_substitute( OUString& _inout_rString,
            const char* _pAsciiPattern, const OUString& _rReplace )
        {
            size_t nOccurrences = 0;

            OUString sPattern( OUString::createFromAscii( _pAsciiPattern ) );
            sal_Int32 nIndex = 0;
            while ( ( nIndex = _inout_rString.indexOf( sPattern ) ) > -1 )
            {
                ++nOccurrences;
                _inout_rString = _inout_rString.replaceAt( nIndex, sPattern.getLength(), _rReplace );
            }

            return nOccurrences;
        }
    }

    SharedResources::SharedResources()
    {
        SharedResources_Impl::registerClient();
    }


    SharedResources::~SharedResources()
    {
        SharedResources_Impl::revokeClient();
    }


    OUString SharedResources::getResourceString(TranslateId pResId) const
    {
        return SharedResources_Impl::getInstance().getResourceString(pResId);
    }


    OUString SharedResources::getResourceStringWithSubstitution(TranslateId pResId,
                const char* _pAsciiPatternToReplace, const OUString& _rStringToSubstitute ) const
    {
        OUString sString( SharedResources_Impl::getInstance().getResourceString(pResId) );
        if ( !lcl_substitute( sString, _pAsciiPatternToReplace, _rStringToSubstitute ) )
            SAL_WARN("connectivity.resource", "Unable to substitute " << _pAsciiPatternToReplace << " with " << _rStringToSubstitute);
        return sString;
    }


    OUString SharedResources::getResourceStringWithSubstitution(TranslateId pResId,
                const char* _pAsciiPatternToReplace1, const OUString& _rStringToSubstitute1,
                const char* _pAsciiPatternToReplace2, const OUString& _rStringToSubstitute2 ) const
    {
        OUString sString( SharedResources_Impl::getInstance().getResourceString(pResId) );
        if( !lcl_substitute( sString, _pAsciiPatternToReplace1, _rStringToSubstitute1 ) )
            SAL_WARN("connectivity.resource", "Unable to substitute " << _pAsciiPatternToReplace1 << " with " << _rStringToSubstitute1);
        if( !lcl_substitute( sString, _pAsciiPatternToReplace2, _rStringToSubstitute2 ) )
            SAL_WARN("connectivity.resource", "Unable to substitute " << _pAsciiPatternToReplace2 << " with " << _rStringToSubstitute2);
        return sString;
    }


    OUString SharedResources::getResourceStringWithSubstitution(TranslateId pResId,
                const char* _pAsciiPatternToReplace1, const OUString& _rStringToSubstitute1,
                const char* _pAsciiPatternToReplace2, const OUString& _rStringToSubstitute2,
                const char* _pAsciiPatternToReplace3, const OUString& _rStringToSubstitute3 ) const
    {
        OUString sString( SharedResources_Impl::getInstance().getResourceString(pResId) );
        if( !lcl_substitute( sString, _pAsciiPatternToReplace1, _rStringToSubstitute1 ) )
            SAL_WARN("connectivity.resource", "Unable to substitute " << _pAsciiPatternToReplace1 << " with " << _rStringToSubstitute1);
        if( !lcl_substitute( sString, _pAsciiPatternToReplace2, _rStringToSubstitute2 ) )
            SAL_WARN("connectivity.resource", "Unable to substitute " << _pAsciiPatternToReplace2 << " with " << _rStringToSubstitute2);
        if( !lcl_substitute( sString, _pAsciiPatternToReplace3, _rStringToSubstitute3 ) )
            SAL_WARN("connectivity.resource", "Unable to substitute " << _pAsciiPatternToReplace3 << " with " << _rStringToSubstitute3);
        return sString;
    }

    OUString SharedResources::getResourceStringWithSubstitution(TranslateId pResId,
                    const std::vector< std::pair<const char* , OUString > >& _rStringToSubstitutes) const
    {
        OUString sString( SharedResources_Impl::getInstance().getResourceString(pResId) );
        for(const auto& [rPattern, rReplace] : _rStringToSubstitutes)
            if( !lcl_substitute( sString, rPattern, rReplace ) )
                SAL_WARN("connectivity.resource", "Unable to substitute " << rPattern << " with " << rReplace);

        return sString;
    }


} // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
