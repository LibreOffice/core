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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
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
        bool lcl_getEnvironmentValue( const sal_Char* _pAsciiEnvName, ::rtl::OUString& _rValue )
        {
            _rValue = ::rtl::OUString();
            ::rtl::OUString sEnvName = ::rtl::OUString::createFromAscii( _pAsciiEnvName );
            osl_getEnvironment( sEnvName.pData, &_rValue.pData );
            return _rValue.getLength() != 0;
        }

        //-----------------------------------------------------------------
        void lcl_convertStringListToUrls( const String& _rColonSeparatedList, ::std::vector< String >& _rTokens, bool _bFinalSlash )
        {
            const sal_Unicode s_cSeparator =
    #if defined(WNT)
                ';'
    #else
                ':'
    #endif
                ;
            xub_StrLen nTokens = _rColonSeparatedList.GetTokenCount( s_cSeparator );
            _rTokens.resize( 0 ); _rTokens.reserve( nTokens );
            for ( xub_StrLen i=0; i<nTokens; ++i )
            {
                // the current token in the list
                String sCurrentToken = _rColonSeparatedList.GetToken( i, s_cSeparator );
                if ( !sCurrentToken.Len() )
                    continue;

                INetURLObject aCurrentURL;

                String sURL;
                if ( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sCurrentToken, sURL ) )
                    aCurrentURL = INetURLObject( sURL );
                else
                {
                    // smart URL parsing, assuming FILE protocol
                    aCurrentURL = INetURLObject( sCurrentToken, INET_PROT_FILE );
                }

                if ( _bFinalSlash )
                    aCurrentURL.setFinalSlash( );
                else
                    aCurrentURL.removeFinalSlash( );
                _rTokens.push_back( aCurrentURL.GetMainURL( INetURLObject::NO_DECODE ) );
            }
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
        String          m_sCheckURL;    // the URL to check
        bool            m_bAllowParent;
    public:
        inline CheckURLAllowed( const String& _rCheckURL, bool bAllowParent = true )
            :m_sCheckURL( _rCheckURL ), m_bAllowParent( bAllowParent )
        {
    #ifdef WNT
            // on windows, assume that the relevant file systems are case insensitive,
            // thus normalize the URL
            m_sCheckURL = m_aSysLocale.GetCharClass().toLower( m_sCheckURL, 0, m_sCheckURL.Len() );
    #endif
        }

        bool operator()( const String& _rApprovedURL )
        {
    #ifdef WNT
            // on windows, assume that the relevant file systems are case insensitive,
            // thus normalize the URL
            String sApprovedURL( m_aSysLocale.GetCharClass().toLower( _rApprovedURL, 0, _rApprovedURL.Len() ) );
    #else
            String sApprovedURL( _rApprovedURL );
    #endif

            xub_StrLen nLenApproved = sApprovedURL.Len();
            xub_StrLen nLenChecked  = m_sCheckURL.Len();

            if ( nLenApproved > nLenChecked )
            {
                if ( m_bAllowParent )
                {
                    if ( sApprovedURL.Search( m_sCheckURL ) == 0 )
                    {
                        if ( ( m_sCheckURL.GetChar( nLenChecked - 1 ) == '/' )
                            || ( sApprovedURL.GetChar( nLenChecked ) == '/' ) )
                            return true;
                    }
                }
                else
                {
                    // just a difference in final slash?
                    if ( ( nLenApproved == ( nLenChecked + 1 ) ) &&
                        ( sApprovedURL.GetChar( nLenApproved - 1 ) == '/' ) )
                        return true;
                }
                return false;
            }
            else if ( nLenApproved < nLenChecked )
            {
                if ( m_sCheckURL.Search( sApprovedURL ) == 0 )
                {
                    if ( ( sApprovedURL.GetChar( nLenApproved - 1 ) == '/' )
                        || ( m_sCheckURL.GetChar( nLenApproved ) == '/' ) )
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
        ::rtl::OUString sRestrictedPathList;
        if ( lcl_getEnvironmentValue( "RestrictedPath", sRestrictedPathList ) )
            // append a final slash. This ensures that when we later on check
            // for unrestricted paths, we don't allow paths like "/home/user35" just because
            // "/home/user3" is allowed - with the final slash, we make it "/home/user3/".
            lcl_convertStringListToUrls( sRestrictedPathList, m_aUnrestrictedURLs, true );
    }

    RestrictedPaths::~RestrictedPaths() {}

    // --------------------------------------------------------------------
    bool RestrictedPaths::isUrlAllowed( const String& _rURL ) const
    {
        if ( m_aUnrestrictedURLs.empty() || !m_bFilterIsEnabled )
            return true;

        ::std::vector< String >::const_iterator aApprovedURL = ::std::find_if(
            m_aUnrestrictedURLs.begin(),
            m_aUnrestrictedURLs.end(),
            CheckURLAllowed( _rURL, true )
        );

        return ( aApprovedURL != m_aUnrestrictedURLs.end() );
    }

    // --------------------------------------------------------------------
    bool RestrictedPaths::isUrlAllowed( const String& _rURL, bool allowParents ) const
    {
        if ( m_aUnrestrictedURLs.empty() || !m_bFilterIsEnabled )
            return true;

        ::std::vector< String >::const_iterator aApprovedURL = ::std::find_if(
            m_aUnrestrictedURLs.begin(),
            m_aUnrestrictedURLs.end(),
            CheckURLAllowed( _rURL, allowParents )
        );

        return ( aApprovedURL != m_aUnrestrictedURLs.end() );
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
