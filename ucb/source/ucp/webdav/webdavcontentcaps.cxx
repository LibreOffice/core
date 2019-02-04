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

#include <memory>
#include <set>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/PropertyCommandArgument.hpp>
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


// ContentProvider implementation.


bool ContentProvider::getProperty(
        const OUString & rPropName, beans::Property & rProp, bool bStrict )
{
    if ( !m_pProps )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pProps )
        {
            m_pProps = std::make_unique<PropertyMap>();


            // Fill map of known properties...


            // Mandatory UCB properties.
            m_pProps->insert(
                beans::Property(
                    "ContentType",
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "IsDocument",
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "IsFolder",
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "Title",
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND ) );

            // Optional UCB properties.

            m_pProps->insert(
                beans::Property(
                    "DateCreated",
                    -1,
                    cppu::UnoType<util::DateTime>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "DateModified",
                    -1,
                    cppu::UnoType<util::DateTime>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "MediaType",
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "Size",
                    -1,
                    cppu::UnoType<sal_Int64>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "BaseURI",
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    "CreatableContentsInfo",
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            // Standard DAV properties.

            m_pProps->insert(
                beans::Property(
                    DAVProperties::CREATIONDATE,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::DISPLAYNAME,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETCONTENTLANGUAGE,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETCONTENTLENGTH,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETCONTENTTYPE ,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETETAG,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::GETLASTMODIFIED,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::LOCKDISCOVERY,
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::Lock >>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::RESOURCETYPE,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::SUPPORTEDLOCK,
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::LockEntry >>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    DAVProperties::EXECUTABLE,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND ) );
        }
    }


    // Lookup property.


    beans::Property aProp;
    aProp.Name = rPropName;
    const PropertyMap::const_iterator it = m_pProps->find( aProp );
    if ( it != m_pProps->end() )
    {
        rProp = *it;
    }
    else
    {
        if ( bStrict )
            return false;

        // All unknown props are treated as:
        rProp = beans::Property(
                    rPropName,
                    - 1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND );
    }

    return true;
}


// Content implementation.


// virtual
uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    bool bTransient;
    std::unique_ptr< DAVResourceAccess > xResAccess;
    std::unique_ptr< ContentProperties > xCachedProps;
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

    std::set< OUString > aPropSet;

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
    bool bHasCreationDate     = false; // creationdate     <-> DateCreated
    bool bHasGetLastModified  = false; // getlastmodified  <-> DateModified
    bool bHasGetContentType   = false; // getcontenttype   <-> MediaType
    bool bHasGetContentLength = false; // getcontentlength <-> Size

    bool bHasContentType      = false;
    bool bHasIsDocument       = false;
    bool bHasIsFolder         = false;
    bool bHasTitle            = false;
    bool bHasBaseURI          = false;
    bool bHasDateCreated      = false;
    bool bHasDateModified     = false;
    bool bHasMediaType        = false;
    bool bHasSize             = false;
    bool bHasCreatableInfos   = false;

    {
        for ( const auto& rProp : aPropSet )
        {
            if ( !bHasCreationDate &&
                 ( rProp == DAVProperties::CREATIONDATE ) )
            {
                bHasCreationDate = true;
            }
            else if ( !bHasGetLastModified &&
                      ( rProp == DAVProperties::GETLASTMODIFIED ) )
            {
                bHasGetLastModified = true;
            }
            else if ( !bHasGetContentType &&
                      ( rProp == DAVProperties::GETCONTENTTYPE ) )
            {
                bHasGetContentType = true;
            }
            else if ( !bHasGetContentLength &&
                      ( rProp == DAVProperties::GETCONTENTLENGTH ) )
            {
                bHasGetContentLength = true;
            }
            else if ( !bHasContentType && rProp == "ContentType" )
            {
                bHasContentType = true;
            }
            else if ( !bHasIsDocument && rProp == "IsDocument" )
            {
                bHasIsDocument = true;
            }
            else if ( !bHasIsFolder && rProp == "IsFolder" )
            {
                bHasIsFolder = true;
            }
            else if ( !bHasTitle && rProp == "Title" )
            {
                bHasTitle = true;
            }
            else if ( !bHasBaseURI && rProp == "BaseURI" )
            {
                bHasBaseURI = true;
            }
            else if ( !bHasDateCreated && rProp == "DateCreated" )
            {
                bHasDateCreated = true;
            }
            else if ( !bHasDateModified && rProp == "DateModified" )
            {
                bHasDateModified = true;
            }
            else if ( !bHasMediaType && rProp == "MediaType" )
            {
                bHasMediaType = true;
            }
            else if ( !bHasSize && rProp == "Size" )
            {
                bHasSize = true;
            }
            else if ( !bHasCreatableInfos && rProp == "CreatableContentsInfo" )
            {
                bHasCreatableInfos = true;
            }
        }
    }

    // Add mandatory properties.
    if ( !bHasContentType )
        aPropSet.insert(
            OUString( "ContentType" ) );

    if ( !bHasIsDocument )
        aPropSet.insert(
            OUString( "IsDocument" ) );

    if ( !bHasIsFolder )
        aPropSet.insert(
            OUString( "IsFolder" ) );

    if ( !bHasTitle )
    {
        // Always present since it can be calculated from content's URI.
        aPropSet.insert(
            OUString( "Title" ) );
    }

    // Add optional properties.

    if ( !bHasBaseURI )
    {
        // Always present since it can be calculated from content's URI.
        aPropSet.insert(
            OUString( "BaseURI" ) );
    }

    if ( !bHasDateCreated && bHasCreationDate )
        aPropSet.insert(
            OUString( "DateCreated" ) );

    if ( !bHasDateModified && bHasGetLastModified )
        aPropSet.insert(
            OUString( "DateModified" ) );

    if ( !bHasMediaType && bHasGetContentType )
        aPropSet.insert(
            OUString( "MediaType" ) );

    if ( !bHasSize && bHasGetContentLength )
        aPropSet.insert(
            OUString( "Size" ) );

    if ( !bHasCreatableInfos )
        aPropSet.insert(
            OUString(
                "CreatableContentsInfo" ) );

    // Add cached properties, if present and still missing.
    if ( xCachedProps.get() )
    {
        const std::set< OUString >::const_iterator set_end
            = aPropSet.end();

        const std::unique_ptr< PropertyValueMap > & xProps
            = xCachedProps->getProperties();

        for ( const auto& rEntry : *xProps )
        {
            if ( aPropSet.find( rEntry.first ) == set_end )
                aPropSet.insert( rEntry.first );
        }
    }

    // std::set -> uno::Sequence
    sal_Int32 nCount = aPropSet.size();
    uno::Sequence< beans::Property > aProperties( nCount );

    beans::Property aProp;
    sal_Int32 n = 0;

    for ( const auto& rProp : aPropSet )
    {
        xProvider->getProperty( rProp, aProp );
        aProperties[ n++ ] = aProp;
    }

    return aProperties;
}


// virtual
uno::Sequence< ucb::CommandInfo > Content::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< ucb::CommandInfo > aCmdInfo( 10 );


    // Mandatory commands


    aCmdInfo[ 0 ] =
            ucb::CommandInfo(
                "getCommandInfo",
                -1,
                cppu::UnoType<void>::get() );
    aCmdInfo[ 1 ] =
            ucb::CommandInfo(
                "getPropertySetInfo",
                -1,
                cppu::UnoType<void>::get() );
    aCmdInfo[ 2 ] =
            ucb::CommandInfo(
                "getPropertyValues",
                -1,
                cppu::UnoType<uno::Sequence< beans::Property >>::get());
    aCmdInfo[ 3 ] =
            ucb::CommandInfo(
                "setPropertyValues",
                -1,
                cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get());


    // Optional standard commands


    aCmdInfo[ 4 ] =
            ucb::CommandInfo(
                "delete",
                -1,
                cppu::UnoType<bool>::get() );
    aCmdInfo[ 5 ] =
            ucb::CommandInfo(
                "insert",
                -1,
                cppu::UnoType<ucb::InsertCommandArgument>::get() );
    aCmdInfo[ 6 ] =
            ucb::CommandInfo(
                "open",
                -1,
                cppu::UnoType<ucb::OpenCommandArgument2>::get() );


    // New commands


    aCmdInfo[ 7 ] =
            ucb::CommandInfo(
                "post",
                -1,
                cppu::UnoType<ucb::PostCommandArgument2>::get() );
    aCmdInfo[ 8 ] =
            ucb::CommandInfo(
                "addProperty",
                -1,
                cppu::UnoType<ucb::PropertyCommandArgument>::get() );
    aCmdInfo[ 9 ] =
            ucb::CommandInfo(
                "removeProperty",
                -1,
                cppu::UnoType<OUString>::get() );

    bool bFolder = false;

    try
    {
        bFolder = isFolder( xEnv );
    }
    catch ( uno::Exception const & )
    {
        return aCmdInfo;
    }

    bool bSupportsLocking = supportsExclusiveWriteLock( xEnv );

    sal_Int32 nPos = aCmdInfo.getLength();
    sal_Int32 nMoreCmds = ( bFolder ? 2 : 0 ) + ( bSupportsLocking ? 2 : 0 );
    if ( nMoreCmds )
        aCmdInfo.realloc( nPos + nMoreCmds );
    else
        return aCmdInfo;

    if ( bFolder )
    {

        // Optional standard commands


        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                "transfer",
                -1,
                cppu::UnoType<ucb::TransferInfo>::get() );
        nPos++;
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                "createNewContent",
                -1,
                cppu::UnoType<ucb::ContentInfo>::get() );
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
                "lock",
                -1,
                cppu::UnoType<void>::get() );
        nPos++;
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                "unlock",
                -1,
                cppu::UnoType<void>::get() );
        nPos++;
    }
    return aCmdInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
