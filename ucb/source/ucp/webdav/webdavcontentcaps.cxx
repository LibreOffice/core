/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: webdavcontentcaps.cxx,v $
 * $Revision: 1.21.20.1 $
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
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HXX_
#include <com/sun/star/util/DateTime.hpp>
#endif
#include <com/sun/star/ucb/Link.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/LockEntry.hpp>
#include "webdavcontent.hxx"
#ifndef _WEBDAV_UCP_PROVIDFER_HXX
#include "webdavprovider.hxx"
#endif
#ifndef _WEBDAV_SESSION_HXX
#include "DAVSession.hxx"
#endif
#include "ContentProperties.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;

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
                                    const uno::Sequence<
                                        com::sun::star::ucb::Lock > * >( 0 ) ),
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
                     DAVProperties::SOURCE,
                    -1,
                    getCppuType( static_cast<
                                    const uno::Sequence<
                                        com::sun::star::ucb::Link > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND ) );

            m_pProps->insert(
                beans::Property(
                      DAVProperties::SUPPORTEDLOCK,
                    -1,
                    getCppuType( static_cast<
                                    const uno::Sequence<
                                        com::sun::star::ucb::LockEntry > * >(
                                            0 ) ),
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
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
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
            xCachedProps.reset( new ContentProperties( *m_xCachedProps.get() ) );
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

            it++;
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
uno::Sequence< com::sun::star::ucb::CommandInfo > Content::getCommands(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    sal_Bool bFolder = sal_False;

    try
    {
        bFolder = isFolder( xEnv );
    }
    catch ( uno::Exception const & )
    {
        static com::sun::star::ucb::CommandInfo aDefaultCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Just mandatory commands avail.
            ///////////////////////////////////////////////////////////////

            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                                uno::Sequence< beans::Property > * >( 0 ) )
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                                uno::Sequence< beans::PropertyValue > * >( 0 ) )
            )
        };
        return uno::Sequence< com::sun::star::ucb::CommandInfo >(
                                            aDefaultCommandInfoTable, 4 );
    }

    if ( bFolder )
    {
        //=================================================================
        //
        // Folder: Supported commands
        //
        //=================================================================

        static com::sun::star::ucb::CommandInfo aFolderCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////

            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                                uno::Sequence< beans::Property > * >( 0 ) )
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                                uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),

            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////

            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuType( static_cast<
                        com::sun::star::ucb::InsertCommandArgument * >( 0 ) )
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType( static_cast<
                        com::sun::star::ucb::OpenCommandArgument2 * >( 0 ) )
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast<
                                com::sun::star::ucb::TransferInfo * >( 0 ) )
            )

            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////

            /*
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "COPY" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MOVE" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            )
            */
        };

        return uno::Sequence< com::sun::star::ucb::CommandInfo >(
                                            aFolderCommandInfoTable, 8 );
    }
    else
    {
        //=================================================================
        //
        // Document: Supported commands
        //
        //=================================================================

        static com::sun::star::ucb::CommandInfo aDocumentCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////

            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                                uno::Sequence< beans::Property > * >( 0 ) )
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType( static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),

            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////

            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuType( static_cast<
                    com::sun::star::ucb::InsertCommandArgument * >( 0 ) )
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType( static_cast<
                    com::sun::star::ucb::OpenCommandArgument2 * >( 0 ) )
            ),

            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////

            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "post" ) ),
                -1,
                getCppuType( static_cast<
                    com::sun::star::ucb::PostCommandArgument2 * >( 0 ) )
            )

            /*
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "COPY" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            ),
            com::sun::star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MOVE" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            )
            */
        };

        return uno::Sequence< com::sun::star::ucb::CommandInfo >(
                                            aDocumentCommandInfoTable, 8 );
    }
}

