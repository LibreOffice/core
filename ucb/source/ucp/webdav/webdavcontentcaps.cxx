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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
/**************************************************************************
                               TODO
 **************************************************************************

 *************************************************************************/

#include <set>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/LockEntry.hpp>
#include "webdavcontent.hxx"
#include "webdavprovider.hxx"
#include "DAVSession.hxx"
#include "ContentProperties.hxx"

using namespace com::sun::star;
using namespace http_dav_ucp;

//=========================================================================
//
// ContentProvider implementation.
//
//=========================================================================

bool ContentProvider::getProperty(
        const rtl::OUString & rPropName, beans::Property & rProp, bool bStrict )
{
    if ( !m_pProps )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pProps )
        {
            m_pProps = new PropertyMap;

            //////////////////////////////////////////////////////////////
            // Fill map of known properties...
            //////////////////////////////////////////////////////////////

            // Mandatory UCB properties.
            m_pProps->insert(
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND ) );

            // Optional UCB properties.

            m_pProps->insert(
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
                    -1,
                    getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ),
                    -1,
                    getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
                    -1,
                    getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseURI" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "CreatableContentsInfo" ) ),
                    -1,
                    getCppuType( static_cast<
                        const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            // Standard DAV properties.

            m_pProps->insert(
                beans::Property(
                    DAVProperties::CREATIONDATE,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::DISPLAYNAME,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETCONTENTLANGUAGE,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETCONTENTLENGTH,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETCONTENTTYPE ,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETETAG,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETLASTMODIFIED,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::LOCKDISCOVERY,
                    -1,
                    getCppuType( static_cast<
                                    const uno::Sequence< ucb::Lock > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::RESOURCETYPE,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::SUPPORTEDLOCK,
                    -1,
                    getCppuType( static_cast<
                                    const uno::Sequence<
                                        ucb::LockEntry > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::EXECUTABLE,
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND ) );
        }
    }

    //////////////////////////////////////////////////////////////
    // Lookup property.
    //////////////////////////////////////////////////////////////

    beans::Property aProp;
    aProp.Name = rPropName;
    const PropertyMap::const_iterator it = m_pProps->find( aProp );
    if ( it != m_pProps->end() )
    {
        rProp = (*it);
    }
    else
    {
        if ( bStrict )
            return false;

        // All unknown props are treated as:
        rProp = beans::Property(
                    rPropName,
                    - 1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND );
    }

    return true;
}

//=========================================================================
//
// Content implementation.
//
//=========================================================================

// virtual
uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    sal_Bool bTransient;
    std::auto_ptr< DAVResourceAccess > xResAccess;
    std::auto_ptr< ContentProperties > xCachedProps;
    rtl::Reference< ContentProvider >  xProvider;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        bTransient = m_bTransient;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        if ( m_xCachedProps.get() )
            xCachedProps.reset(
                new ContentProperties( *m_xCachedProps.get() ) );
        xProvider.set( m_pProvider );
    }

    typedef std::set< rtl::OUString > StringSet;
    StringSet aPropSet;

    // No server access for just created (not yet committed) objects.
    // Only a minimal set of properties supported at this stage.
    if ( !bTransient )
    {
        // Obtain all properties supported for this resource from server.
        try
        {
            std::vector< DAVResourceInfo > props;
            xResAccess->PROPFIND( DAVZERO, props, xEnv );

            // Note: vector always contains exactly one resource info, because
            //       we used a depth of DAVZERO for PROPFIND.
            aPropSet.insert( (*props.begin()).properties.begin(),
                             (*props.begin()).properties.end() );
        }
        catch ( DAVException const & )
        {
        }
    }

    // Add DAV properties, map DAV properties to UCB properties.
    sal_Bool bHasCreationDate     = sal_False; // creationdate     <-> DateCreated
    sal_Bool bHasGetLastModified  = sal_False; // getlastmodified  <-> DateModified
    sal_Bool bHasGetContentType   = sal_False; // getcontenttype   <-> MediaType
    sal_Bool bHasGetContentLength = sal_False; // getcontentlength <-> Size

    sal_Bool bHasContentType      = sal_False;
    sal_Bool bHasIsDocument       = sal_False;
    sal_Bool bHasIsFolder         = sal_False;
    sal_Bool bHasTitle            = sal_False;
    sal_Bool bHasBaseURI          = sal_False;
    sal_Bool bHasDateCreated      = sal_False;
    sal_Bool bHasDateModified     = sal_False;
    sal_Bool bHasMediaType        = sal_False;
    sal_Bool bHasSize             = sal_False;
    sal_Bool bHasCreatableInfos   = sal_False;

    {
        std::set< rtl::OUString >::const_iterator it  = aPropSet.begin();
        std::set< rtl::OUString >::const_iterator end = aPropSet.end();
        while ( it != end )
        {
            if ( !bHasCreationDate &&
                 ( (*it) == DAVProperties::CREATIONDATE ) )
            {
                bHasCreationDate = sal_True;
            }
            else if ( !bHasGetLastModified &&
                      ( (*it) == DAVProperties::GETLASTMODIFIED ) )
            {
                bHasGetLastModified = sal_True;
            }
            else if ( !bHasGetContentType &&
                      ( (*it) == DAVProperties::GETCONTENTTYPE ) )
            {
                bHasGetContentType = sal_True;
            }
            else if ( !bHasGetContentLength &&
                      ( (*it) == DAVProperties::GETCONTENTLENGTH ) )
            {
                bHasGetContentLength = sal_True;
            }
            else if ( !bHasContentType &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
            {
                bHasContentType = sal_True;
            }
            else if ( !bHasIsDocument &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                bHasIsDocument = sal_True;
            }
            else if ( !bHasIsFolder &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                bHasIsFolder = sal_True;
            }
            else if ( !bHasTitle &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                bHasTitle = sal_True;
            }
            else if ( !bHasBaseURI &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "BaseURI" ) ) )
            {
                bHasBaseURI = sal_True;
            }
            else if ( !bHasDateCreated &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) ) )
            {
                bHasDateCreated = sal_True;
            }
            else if ( !bHasDateModified &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
            {
                bHasDateModified = sal_True;
            }
            else if ( !bHasMediaType &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
            {
                bHasMediaType = sal_True;
            }
            else if ( !bHasSize &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
            {
                bHasSize = sal_True;
            }
            else if ( !bHasCreatableInfos &&
                      (*it).equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "CreatableContentsInfo" ) ) )
            {
                bHasCreatableInfos = sal_True;
            }
            ++it;
        }
    }

    // Add mandatory properties.
    if ( !bHasContentType )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ) );

    if ( !bHasIsDocument )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ) );

    if ( !bHasIsFolder )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ) );

    if ( !bHasTitle )
    {
        // Always present since it can be calculated from content's URI.
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ) );
    }

    // Add optional properties.

    if ( !bHasBaseURI )
    {
        // Always present since it can be calculated from content's URI.
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseURI" ) ) );
    }

    if ( !bHasDateCreated && bHasCreationDate )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ) );

    if ( !bHasDateModified && bHasGetLastModified )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ) );

    if ( !bHasMediaType && bHasGetContentType )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) );

    if ( !bHasSize && bHasGetContentLength )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ) );

    if ( !bHasCreatableInfos )
        aPropSet.insert(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "CreatableContentsInfo" ) ) );

    // Add cached properties, if present and still missing.
    if ( xCachedProps.get() )
    {
        const std::set< rtl::OUString >::const_iterator set_end
            = aPropSet.end();

        const std::auto_ptr< PropertyValueMap > & xProps
            = xCachedProps->getProperties();

        PropertyValueMap::const_iterator       map_it  = xProps->begin();
        const PropertyValueMap::const_iterator map_end = xProps->end();

        while ( map_it != map_end )
        {
            if ( aPropSet.find( (*map_it).first ) == set_end )
                aPropSet.insert( (*map_it).first );

            ++map_it;
        }
    }

    // std::set -> uno::Sequence
    sal_Int32 nCount = aPropSet.size();
    uno::Sequence< beans::Property > aProperties( nCount );

    std::set< rtl::OUString >::const_iterator it = aPropSet.begin();
    beans::Property aProp;

    for ( sal_Int32 n = 0; n < nCount; ++n, ++it )
    {
        xProvider->getProperty( (*it), aProp );
        aProperties[ n ] = aProp;
    }

    return aProperties;
}

//=========================================================================
// virtual
uno::Sequence< ucb::CommandInfo > Content::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< ucb::CommandInfo > aCmdInfo( 8 );

    ///////////////////////////////////////////////////////////////
    // Mandatory commands
    ///////////////////////////////////////////////////////////////

    aCmdInfo[ 0 ] =
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType() );
    aCmdInfo[ 1 ] =
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType() );
    aCmdInfo[ 2 ] =
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                                uno::Sequence< beans::Property > * >( 0 ) ) );
    aCmdInfo[ 3 ] =
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                    uno::Sequence< beans::PropertyValue > * >( 0 ) ) );

    ///////////////////////////////////////////////////////////////
    // Optional standard commands
    ///////////////////////////////////////////////////////////////

    aCmdInfo[ 4 ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType() );
    aCmdInfo[ 5 ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuType( static_cast<
                    ucb::InsertCommandArgument * >( 0 ) ) );
    aCmdInfo[ 6 ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType( static_cast<
                    ucb::OpenCommandArgument2 * >( 0 ) ) );

    ///////////////////////////////////////////////////////////////
    // New commands
    ///////////////////////////////////////////////////////////////

    aCmdInfo[ 7 ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "post" ) ),
                -1,
                getCppuType( static_cast<
                    ucb::PostCommandArgument2 * >( 0 ) ) );

    sal_Bool bFolder = sal_False;

    try
    {
        bFolder = isFolder( xEnv );
    }
    catch ( uno::Exception const & )
    {
        return aCmdInfo;
    }

    sal_Bool bSupportsLocking = supportsExclusiveWriteLock( xEnv );

    sal_Int32 nPos = aCmdInfo.getLength();
    sal_Int32 nMoreCmds = ( bFolder ? 2 : 0 ) + ( bSupportsLocking ? 2 : 0 );
    if ( nMoreCmds )
        aCmdInfo.realloc( nPos + nMoreCmds );
    else
        return aCmdInfo;

    if ( bFolder )
    {
        ///////////////////////////////////////////////////////////////
        // Optional standard commands
        ///////////////////////////////////////////////////////////////

        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast< ucb::TransferInfo * >( 0 ) ) );
        nPos++;
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "createNewContent" ) ),
                -1,
                getCppuType( static_cast< ucb::ContentInfo * >( 0 ) ) );
        nPos++;
    }
    else
    {
        // no document-only commands at the moment.
    }

    if ( bSupportsLocking )
    {
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "lock" ) ),
                -1,
                getCppuVoidType() );
        nPos++;
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "unlock" ) ),
                -1,
                getCppuVoidType() );
        nPos++;
    }
    return aCmdInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
