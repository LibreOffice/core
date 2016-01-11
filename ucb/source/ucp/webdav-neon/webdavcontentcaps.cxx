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
#include <com/sun/star/ucb/PropertyCommandArgument.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/ucb/Link.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/LockEntry.hpp>
#include "webdavcontent.hxx"
#include "webdavprovider.hxx"
#include "DAVSession.hxx"
#include "ContentProperties.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;


// ContentProvider implementation.


bool ContentProvider::getProperty(
        const OUString & rPropName, beans::Property & rProp )
{
    if ( !m_pProps )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pProps )
        {
            m_pProps = new PropertyMap;


            // Fill map of known properties...


            // Mandatory UCB properties.
            m_pProps->insert(
                beans::Property(
                    OUString( "ContentType" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "IsDocument" ),
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "IsFolder" ),
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "Title" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND ) );

            // Optional UCB properties.

            m_pProps->insert(
                beans::Property(
                    OUString( "DateCreated" ),
                    -1,
                    cppu::UnoType<util::DateTime>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "DateModified" ),
                    -1,
                    cppu::UnoType<util::DateTime>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "MediaType" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "Size" ),
                    -1,
                    cppu::UnoType<sal_Int64>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString( "BaseURI" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ) );

            m_pProps->insert(
                beans::Property(
                    OUString(
                        "CreatableContentsInfo" ),
                    -1,
                    cppu::UnoType<
                        uno::Sequence< ucb::ContentInfo >>::get(),
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
                    cppu::UnoType<
                                    uno::Sequence< ucb::Lock >>::get(),
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
                    DAVProperties::SOURCE,
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::Link >>::get(),
                    beans::PropertyAttribute::BOUND ) );

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
        rProp = (*it);
    }
    else
    {
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

    typedef std::set< OUString > StringSet;
    StringSet aPropSet;

    // No server access for just created (not yet committed) objects.
    // Only a minimal set of properties supported at this stage.
    if ( !bTransient )
    {
        // Obtain all properties supported for this resource from server.
        DAVOptions aDAVOptions;
        getResourceOptions( xEnv, aDAVOptions, xResAccess );
        // only Class 1 is needed for PROPFIND
        if ( aDAVOptions.isClass1() )
        {
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
        std::set< OUString >::const_iterator it  = aPropSet.begin();
        std::set< OUString >::const_iterator end = aPropSet.end();
        while ( it != end )
        {
            if ( !bHasCreationDate &&
                 ( (*it) == DAVProperties::CREATIONDATE ) )
            {
                bHasCreationDate = true;
            }
            else if ( !bHasGetLastModified &&
                      ( (*it) == DAVProperties::GETLASTMODIFIED ) )
            {
                bHasGetLastModified = true;
            }
            else if ( !bHasGetContentType &&
                      ( (*it) == DAVProperties::GETCONTENTTYPE ) )
            {
                bHasGetContentType = true;
            }
            else if ( !bHasGetContentLength &&
                      ( (*it) == DAVProperties::GETCONTENTLENGTH ) )
            {
                bHasGetContentLength = true;
            }
            else if ( !bHasContentType && (*it) == "ContentType" )
            {
                bHasContentType = true;
            }
            else if ( !bHasIsDocument && (*it) == "IsDocument" )
            {
                bHasIsDocument = true;
            }
            else if ( !bHasIsFolder && (*it) == "IsFolder" )
            {
                bHasIsFolder = true;
            }
            else if ( !bHasTitle && (*it) == "Title" )
            {
                bHasTitle = true;
            }
            else if ( !bHasBaseURI && (*it) == "BaseURI" )
            {
                bHasBaseURI = true;
            }
            else if ( !bHasDateCreated && (*it) == "DateCreated" )
            {
                bHasDateCreated = true;
            }
            else if ( !bHasDateModified && (*it) == "DateModified" )
            {
                bHasDateModified = true;
            }
            else if ( !bHasMediaType && (*it) == "MediaType" )
            {
                bHasMediaType = true;
            }
            else if ( !bHasSize && (*it) == "Size" )
            {
                bHasSize = true;
            }
            else if ( !bHasCreatableInfos && (*it) == "CreatableContentsInfo" )
            {
                bHasCreatableInfos = true;
            }
            ++it;
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

    std::set< OUString >::const_iterator it = aPropSet.begin();
    beans::Property aProp;

    for ( sal_Int32 n = 0; n < nCount; ++n, ++it )
    {
        xProvider->getProperty( (*it), aProp );
        aProperties[ n ] = aProp;
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
                OUString( "getCommandInfo" ),
                -1,
                cppu::UnoType<void>::get() );
    aCmdInfo[ 1 ] =
            ucb::CommandInfo(
                OUString( "getPropertySetInfo" ),
                -1,
                cppu::UnoType<void>::get() );
    aCmdInfo[ 2 ] =
            ucb::CommandInfo(
                OUString( "getPropertyValues" ),
                -1,
                cppu::UnoType<uno::Sequence< beans::Property >>::get() );
    aCmdInfo[ 3 ] =
            ucb::CommandInfo(
                OUString( "setPropertyValues" ),
                -1,
                cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get() );


    // Optional standard commands


    aCmdInfo[ 4 ] =
            ucb::CommandInfo(
                OUString( "delete" ),
                -1,
                cppu::UnoType<bool>::get() );
    aCmdInfo[ 5 ] =
            ucb::CommandInfo(
                OUString( "insert" ),
                -1,
                cppu::UnoType<ucb::InsertCommandArgument>::get() );
    aCmdInfo[ 6 ] =
            ucb::CommandInfo(
                OUString( "open" ),
                -1,
                cppu::UnoType<ucb::OpenCommandArgument2>::get() );


    // New commands


    aCmdInfo[ 7 ] =
            ucb::CommandInfo(
                OUString( "post" ),
                -1,
                cppu::UnoType<ucb::PostCommandArgument2>::get() );
    aCmdInfo[ 8 ] =
            ucb::CommandInfo(
                OUString( "addProperty" ),
                -1,
                cppu::UnoType<ucb::PropertyCommandArgument>::get() );
    aCmdInfo[ 9 ] =
            ucb::CommandInfo(
                OUString( "removeProperty" ),
                -1,
                cppu::UnoType<rtl::OUString>::get() );

    bool bFolder = false;

    try
    {
        bFolder = isFolder( xEnv );
    }
    catch ( uno::Exception const & )
    {
        return aCmdInfo;
    }

    ResourceType eType = resourceTypeForLocks( xEnv );
    bool bSupportsLocking = ( eType == NOT_FOUND || eType == DAV );

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
                OUString( "transfer" ),
                -1,
                cppu::UnoType<ucb::TransferInfo>::get() );
        nPos++;
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                OUString(
                    "createNewContent" ),
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
                OUString( "lock" ),
                -1,
                cppu::UnoType<void>::get() );
        nPos++;
        aCmdInfo[ nPos ] =
            ucb::CommandInfo(
                OUString( "unlock" ),
                -1,
                cppu::UnoType<void>::get() );
        nPos++;
    }
    return aCmdInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
