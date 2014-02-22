/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "rtl/ustrbuf.hxx"
#include "osl/diagnose.h"
#include "comphelper/storagehelper.hxx"

#include "../inc/urihelper.hxx"

#include "pkguri.hxx"

using namespace package_ucp;




//

//



static void normalize( OUString& rURL )
{
    sal_Int32 nPos = 0;
    do
    {
        nPos = rURL.indexOf( '%', nPos );
        if ( nPos != -1 )
        {
            if ( nPos < ( rURL.getLength() - 2 ) )
            {
                OUString aTmp = rURL.copy( nPos + 1, 2 );
                rURL = rURL.replaceAt( nPos + 1, 2, aTmp.toAsciiUpperCase() );
                nPos++;
            }
        }
    }
    while ( nPos != -1 );
}


void PackageUri::init() const
{
    
    if ( !m_aUri.isEmpty() && m_aPath.isEmpty() )
    {
        
        m_aPackage = m_aParentUri = m_aName = m_aParam = m_aScheme
            = OUString();

        
        if ( ( m_aUri.getLength() < PACKAGE_URL_SCHEME_LENGTH + 4 ) )
        {
            
            m_aPath = "/";
            return;
        }

        
        if ( ( m_aUri[ PACKAGE_URL_SCHEME_LENGTH ] != ':' )
             ||
             ( m_aUri[ PACKAGE_URL_SCHEME_LENGTH + 1 ] != '/' )
             ||
             ( m_aUri[ PACKAGE_URL_SCHEME_LENGTH + 2 ] != '/' ) )
        {
            
            m_aPath = "/";
            return;
        }

        OUString aPureUri;
        sal_Int32 nParam = m_aUri.indexOf( '?' );
        if( nParam >= 0 )
        {
            m_aParam = m_aUri.copy( nParam );
            aPureUri = m_aUri.copy( 0, nParam );
        }
        else
            aPureUri = m_aUri;

        
        m_aScheme = aPureUri.copy(
            0, PACKAGE_URL_SCHEME_LENGTH ).toAsciiLowerCase();

        if ( m_aScheme == PACKAGE_URL_SCHEME || m_aScheme == PACKAGE_ZIP_URL_SCHEME )
        {
            if ( m_aScheme == PACKAGE_ZIP_URL_SCHEME )
            {
                m_aParam +=
                    ( !m_aParam.isEmpty()
                      ? OUString( "&purezip" )
                      : OUString( "?purezip" ) );
            }

            aPureUri = aPureUri.replaceAt( 0,
                                           m_aScheme.getLength(),
                                           m_aScheme );

            sal_Int32 nStart = PACKAGE_URL_SCHEME_LENGTH + 3;
            sal_Int32 nEnd   = aPureUri.lastIndexOf( '/' );
            if ( nEnd == PACKAGE_URL_SCHEME_LENGTH + 3 )
            {
                

                
                m_aPath = "/";
                return;
            }
            else if ( nEnd == ( aPureUri.getLength() - 1 ) )
            {
                if ( aPureUri[ aPureUri.getLength() - 2 ] == '/' )
                {
                    

                    
                    m_aPath = "/";
                    return;
                }

                
                aPureUri = aPureUri.copy( 0, nEnd );
            }


            nEnd = aPureUri.indexOf( '/', nStart );
            if ( nEnd == -1 )
            {
                

                OUString aNormPackage = aPureUri.copy( nStart );
                normalize( aNormPackage );

                aPureUri = aPureUri.replaceAt(
                    nStart, aPureUri.getLength() - nStart, aNormPackage );
                m_aPackage
                    = ::ucb_impl::urihelper::decodeSegment( aNormPackage );
                m_aPath = "/";
                m_aUri = m_aUri.replaceAt( 0,
                                           ( nParam >= 0 )
                                           ? nParam
                                           : m_aUri.getLength(), aPureUri );

                sal_Int32 nLastSlash = m_aPackage.lastIndexOf( '/' );
                if ( nLastSlash != -1 )
                    m_aName = ::ucb_impl::urihelper::decodeSegment(
                        m_aPackage.copy( nLastSlash + 1 ) );
                else
                    m_aName
                        = ::ucb_impl::urihelper::decodeSegment( m_aPackage );
            }
            else
            {
                m_aPath = aPureUri.copy( nEnd + 1 );

                
                
                
                
                
                if ( m_aPath.indexOf( "
                  || m_aPath.indexOf( "%2F" ) != -1
                  || m_aPath.indexOf( "%2f" ) != -1
                  || ::comphelper::OStorageHelper::PathHasSegment( m_aPath, OUString( ".." ) )
                  || ::comphelper::OStorageHelper::PathHasSegment( m_aPath, OUString( "." ) ) )
                {
                    
                    m_aPath = "/";
                    return;
                }

                OUString aNormPackage = aPureUri.copy( nStart, nEnd - nStart );
                normalize( aNormPackage );

                aPureUri = aPureUri.replaceAt(
                    nStart, nEnd - nStart, aNormPackage );
                aPureUri = aPureUri.replaceAt(
                    nEnd + 1,
                    aPureUri.getLength() - nEnd - 1,
                    ::ucb_impl::urihelper::encodeURI( m_aPath ) );

                m_aPackage
                    = ::ucb_impl::urihelper::decodeSegment( aNormPackage );
                m_aPath = ::ucb_impl::urihelper::decodeSegment( m_aPath );
                m_aUri = m_aUri.replaceAt( 0,
                                           ( nParam >= 0 )
                                           ? nParam
                                           : m_aUri.getLength(), aPureUri );

                sal_Int32 nLastSlash = aPureUri.lastIndexOf( '/' );
                if ( nLastSlash != -1 )
                {
                    m_aParentUri = aPureUri.copy( 0, nLastSlash );
                    m_aName = ::ucb_impl::urihelper::decodeSegment(
                        aPureUri.copy( nLastSlash + 1 ) );
                }
            }

            
            m_bValid = true;
        }
        else
        {
            
            m_aPath = "/";
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
