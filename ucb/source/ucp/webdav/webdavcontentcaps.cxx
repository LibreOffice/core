/*************************************************************************
 *
 *  $RCSfile: webdavcontentcaps.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kso $ $Date: 2001-08-30 12:28:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_POSTCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HXX_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LINK_HPP_
#include <com/sun/star/ucb/Link.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LOCK_HPP_
#include <com/sun/star/ucb/Lock.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LOCKENTRY_HPP_
#include <com/sun/star/ucb/LockEntry.hpp>
#endif

#ifndef _WEBDAV_UCP_CONTENT_HXX
#include "webdavcontent.hxx"
#endif
#ifndef _WEBDAV_UCP_PROVIDFER_HXX
#include "webdavprovider.hxx"
#endif
#ifndef _WEBDAV_SESSION_HXX
#include "DAVSession.hxx"
#endif

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
        vos::OGuard aGuard( m_aMutex );
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
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Obtain all properties supported for this resource from server.
    std::vector< rtl::OUString > aProps;
    try
    {
        std::vector< DAVResourceInfo > props;
        m_xResAccess->PROPFIND( ZERO, props, xEnv );

        // Note: vector always contains exactly one resource info, because
        //       we used a depth of ZERO for PROPFIND.
        aProps = (*props.begin()).properties;
    }
    catch ( DAVException const & )
    {
    }

    sal_Int32 nTotal = aProps.size();
    uno::Sequence< beans::Property > aProperties( nTotal );

    sal_Int32 nPos = 0;

    // Add DAV properties, map DAV properties to UCB properties.
    sal_Bool bHasCreationDate     = sal_False; // creationdate     <-> DateCreated
    sal_Bool bHasGetLastModified  = sal_False; // getlastmodified  <-> DateModified
    sal_Bool bHasGetContentType   = sal_False; // getcontenttype   <-> MediaType
    sal_Bool bHasGetContentLength = sal_False; // getcontentlength <-> Size

    sal_Bool bHasContentType      = sal_False;
    sal_Bool bHasIsDocument       = sal_False;
    sal_Bool bHasIsFolder         = sal_False;
    sal_Bool bHasTitle            = sal_False;
    sal_Bool bHasDateCreated      = sal_False;
    sal_Bool bHasDateModified     = sal_False;
    sal_Bool bHasMediaType        = sal_False;
    sal_Bool bHasSize             = sal_False;

    beans::Property aProp;

    std::vector< rtl::OUString >::const_iterator it  = aProps.begin();
    std::vector< rtl::OUString >::const_iterator end = aProps.end();
    while ( it != end )
    {
        m_pProvider->getProperty( (*it), aProp );

        aProperties[ nPos ] = aProp;

        if ( !bHasCreationDate &&
                ( aProp.Name == DAVProperties::CREATIONDATE ) )
        {
            bHasCreationDate = sal_True;
        }
        else if ( !bHasGetLastModified &&
                     ( aProp.Name == DAVProperties::GETLASTMODIFIED ) )
        {
            bHasGetLastModified = sal_True;
        }
        else if ( !bHasGetContentType &&
                     ( aProp.Name == DAVProperties::GETCONTENTTYPE ) )
        {
            bHasGetContentType = sal_True;
        }
        else if ( !bHasGetContentLength &&
                     ( aProp.Name == DAVProperties::GETCONTENTLENGTH ) )
        {
            bHasGetContentLength = sal_True;
        }
        else if ( !bHasContentType &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
        {
            bHasContentType = sal_True;
        }
        else if ( !bHasIsDocument &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
        {
            bHasIsDocument = sal_True;
        }
        else if ( !bHasIsFolder &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
        {
            bHasIsFolder = sal_True;
        }
        else if ( !bHasTitle &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            bHasTitle = sal_True;
        }
        else if ( !bHasDateCreated &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) ) )
        {
            bHasDateCreated = sal_True;
        }
        else if ( !bHasDateModified &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
        {
            bHasDateModified = sal_True;
        }
        else if ( !bHasMediaType &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
        {
            bHasMediaType = sal_True;
        }
        else if ( !bHasSize &&
                  aProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
        {
            bHasSize = sal_True;
        }

        it++;
        nPos++;
    }

    if ( !bHasContentType )
        nTotal++;

    if ( !bHasIsDocument )
        nTotal++;

    if ( !bHasIsFolder )
        nTotal++;

    if ( !bHasTitle )
        nTotal++;

    if ( !bHasDateCreated && bHasCreationDate )
        nTotal++;

    if ( !bHasDateModified && bHasGetLastModified )
        nTotal++;

    if ( !bHasMediaType && bHasGetContentType )
        nTotal++;

    if ( !bHasSize && bHasGetContentLength )
        nTotal++;

    aProperties.realloc( nTotal );

    // Add mandatory properties.
    if ( !bHasContentType )
    {
        m_pProvider->getProperty(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( !bHasIsDocument )
    {
        m_pProvider->getProperty(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( !bHasIsFolder )
    {
        m_pProvider->getProperty(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( !bHasTitle )
    {
        m_pProvider->getProperty(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            aProp );
        aProperties[ nPos++ ] = aProp;
    }

    // Add optional properties.
    if ( !bHasDateCreated && bHasCreationDate )
    {
        m_pProvider->getProperty(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( !bHasDateModified && bHasGetLastModified )
    {
        m_pProvider->getProperty(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( !bHasMediaType && bHasGetContentType )
    {
        m_pProvider->getProperty(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( !bHasSize && bHasGetContentLength )
    {
        m_pProvider->getProperty(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "Size" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    return aProperties;
}

//=========================================================================
void Content::getProperties(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv,
    PropertyMap & rProps )
{
    uno::Sequence< beans::Property > aProps = getProperties( xEnv );
    sal_Int32 nCount = aProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
        rProps.insert( aProps[ n ] );
}

//=========================================================================
// virtual
uno::Sequence< com::sun::star::ucb::CommandInfo > Content::getCommands(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

       if ( isFolder( xEnv ) )
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

