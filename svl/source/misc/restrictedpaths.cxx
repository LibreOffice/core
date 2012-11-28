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

#include <svl/restrictedpaths.hxx>

#include <algorithm>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/syslocale.hxx>

namespace svt
{
    namespace
    {
        // ----------------------------------------------------------------
        /** retrieves the value of an environment variable
            @return <TRUE/> if and only if the retrieved string value is not empty
        */
        bool lcl_getEnvironmentValue( const sal_Char* _pAsciiEnvName, OUString& _rValue )
        {
            _rValue = OUString();
            OUString sEnvName = OUString::createFromAscii( _pAsciiEnvName );
            osl_getEnvironment( sEnvName.pData, &_rValue.pData );
            return !_rValue.isEmpty();
        }

        //-----------------------------------------------------------------
        void lcl_convertStringListToUrls( const OUString& _rColonSeparatedList, ::std::vector< OUString >& _rTokens )
        {
            const sal_Unicode cSeparator =
    #if defined(WNT)
                ';'
    #else
                ':'
    #endif
                ;
            sal_Int32 nIndex = 0;
            do
            {
                // the current token in the list
                OUString sCurrentToken = _rColonSeparatedList.getToken( 0, cSeparator, nIndex );
                if ( !sCurrentToken.isEmpty() )
                {
                    INetURLObject aCurrentURL;

                    OUString sURL;
                    if ( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sCurrentToken, sURL ) )
                        aCurrentURL = INetURLObject( sURL );
                    else
                    {
                        // smart URL parsing, assuming FILE protocol
                        aCurrentURL = INetURLObject( sCurrentToken, INET_PROT_FILE );
                    }

                    aCurrentURL.setFinalSlash( );
                    _rTokens.push_back( aCurrentURL.GetMainURL( INetURLObject::NO_DECODE ) );
                }
            }
            while ( nIndex >= 0 );
        }

    }

    //=====================================================================
    //= CheckURLAllowed
    //=====================================================================
    struct CheckURLAllowed
    {
    protected:
    #ifdef WNT
        SvtSysLocale    m_aSysLocale;
    #endif
        OUString        m_sCheckURL;    // the URL to check
        bool            m_bAllowParent;
    public:
        inline CheckURLAllowed( const OUString& _rCheckURL, bool bAllowParent = true )
            : m_sCheckURL( _rCheckURL )
            , m_bAllowParent( bAllowParent )
        {
    #ifdef WNT
            // on windows, assume that the relevant file systems are case insensitive,
            // thus normalize the URL
            m_sCheckURL = m_aSysLocale.GetCharClass().lowercase( m_sCheckURL, 0, m_sCheckURL.getLength() );
    #endif
        }

        bool operator()( const OUString& _rApprovedURL )
        {
    #ifdef WNT
            // on windows, assume that the relevant file systems are case insensitive,
            // thus normalize the URL
            OUString sApprovedURL( m_aSysLocale.GetCharClass().lowercase( _rApprovedURL, 0, _rApprovedURL.getLength() ) );
    #else
            OUString sApprovedURL( _rApprovedURL );
    #endif

            sal_Int32 nLenApproved = sApprovedURL.getLength();
            sal_Int32 nLenChecked  = m_sCheckURL.getLength();

            if ( nLenApproved > nLenChecked )
            {
                if ( m_bAllowParent )
                {
                    if ( sApprovedURL.indexOf( m_sCheckURL ) == 0 )
                    {
                        if ( ( m_sCheckURL[ nLenChecked - 1 ] == '/' )
                            || ( sApprovedURL[ nLenChecked ] == '/' ) )
                            return true;
                    }
                }
                else
                {
                    // just a difference in final slash?
                    if ( ( nLenApproved == ( nLenChecked + 1 ) ) &&
                        ( sApprovedURL[ nLenApproved - 1 ] == '/' ) )
                        return true;
                }
                return false;
            }
            else if ( nLenApproved < nLenChecked )
            {
                if ( m_sCheckURL.indexOf( sApprovedURL ) == 0 )
                {
                    if ( ( sApprovedURL[ nLenApproved - 1 ] == '/' )
                        || ( m_sCheckURL[ nLenApproved ] == '/' ) )
                        return true;
                }
                return false;
            }
            else
            {
                // strings have equal length
                return ( sApprovedURL == m_sCheckURL );
            }
        }
    };

    //=====================================================================
    //= RestrictedPaths
    //=====================================================================
    //---------------------------------------------------------------------
    RestrictedPaths::RestrictedPaths()
        :m_bFilterIsEnabled( true )
    {
        OUString sRestrictedPathList;
        if ( lcl_getEnvironmentValue( "RestrictedPath", sRestrictedPathList ) )
            // append a final slash. This ensures that when we later on check
            // for unrestricted paths, we don't allow paths like "/home/user35" just because
            // "/home/user3" is allowed - with the final slash, we make it "/home/user3/".
            lcl_convertStringListToUrls( sRestrictedPathList, m_aUnrestrictedURLs );
    }

    RestrictedPaths::~RestrictedPaths() {}

    // --------------------------------------------------------------------
    bool RestrictedPaths::isUrlAllowed( const OUString& _rURL ) const
    {
        if ( m_aUnrestrictedURLs.empty() || !m_bFilterIsEnabled )
            return true;

        ::std::vector< OUString >::const_iterator aApprovedURL = ::std::find_if(
            m_aUnrestrictedURLs.begin(),
            m_aUnrestrictedURLs.end(),
            CheckURLAllowed( _rURL, true )
        );

        return ( aApprovedURL != m_aUnrestrictedURLs.end() );
    }

    // --------------------------------------------------------------------
    bool RestrictedPaths::isUrlAllowed( const OUString& _rURL, bool allowParents ) const
    {
        if ( m_aUnrestrictedURLs.empty() || !m_bFilterIsEnabled )
            return true;

        ::std::vector< OUString >::const_iterator aApprovedURL = ::std::find_if(
            m_aUnrestrictedURLs.begin(),
            m_aUnrestrictedURLs.end(),
            CheckURLAllowed( _rURL, allowParents )
        );

        return ( aApprovedURL != m_aUnrestrictedURLs.end() );
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
