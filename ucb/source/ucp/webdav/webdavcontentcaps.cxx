/*************************************************************************
 *
 *  $RCSfile: webdavcontentcaps.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2001-05-16 15:30:00 $
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

using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace rtl;
using namespace webdav_ucp;

//=========================================================================
//
// ContentProvider implementation.
//
//=========================================================================

bool ContentProvider::getProperty(
            const OUString & rPropName, Property & rProp, bool bStrict )
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
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                    -1,
                    getCppuBooleanType(),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                    -1,
                    getCppuBooleanType(),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND ) );

            // Optional UCB properties.

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
                    -1,
                    getCppuType( static_cast< const DateTime * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ),
                    -1,
                    getCppuType( static_cast< const DateTime * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
                    -1,
                    getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            // Standard DAV properties.

            m_pProps->insert(
                Property(
                    DAVProperties::CREATIONDATE,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    DAVProperties::DISPLAYNAME,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND ) );

            m_pProps->insert(
                Property(
                    DAVProperties::GETCONTENTLANGUAGE,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    DAVProperties::GETCONTENTLENGTH,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    DAVProperties::GETCONTENTTYPE ,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    DAVProperties::GETETAG,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                    DAVProperties::GETLASTMODIFIED,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                     DAVProperties::LOCKDISCOVERY,
                    -1,
                    getCppuType( static_cast< const Sequence< Lock > * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                     DAVProperties::RESOURCETYPE,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                     DAVProperties::SOURCE,
                    -1,
                    getCppuType(
                        static_cast< const Sequence< Link > * >( 0 ) ),
                    PropertyAttribute::BOUND ) );

            m_pProps->insert(
                Property(
                      DAVProperties::SUPPORTEDLOCK,
                    -1,
                    getCppuType(
                        static_cast< const Sequence< LockEntry > * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ) );

            m_pProps->insert(
                Property(
                      DAVProperties::EXECUTABLE,
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND ) );
        }
    }

    //////////////////////////////////////////////////////////////
    // Lookup property.
    //////////////////////////////////////////////////////////////

    Property aProp;
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
        rProp = Property(
                    rPropName,
                    - 1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND );
    }

    return true;
}

//=========================================================================
//
// Content implementation.
//
//=========================================================================

// virtual
Sequence< Property > Content::getProperties(
                            const Reference< XCommandEnvironment > & xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Obtain all properties supported for this resource from server.
    std::vector< OUString > aProps;
    try
    {
        std::vector< DAVResourceInfo > props;
        m_aResAccess.PROPFIND( ZERO, props, xEnv );

        // Note: vector always contains exactly one resource info, because
        //       we used a depth of ZERO for PROPFIND.
        aProps = (*props.begin()).properties;
    }
    catch ( DAVException const & )
    {
    }

    sal_Int32 nTotal = aProps.size() + 4; // number of mandatory props
    Sequence< Property > aProperties( nTotal );

    sal_Int32 nPos = 0;

    // Add DAV properties, map DAV properties to UCB properties.
    sal_Bool bHasCreationDate  = sal_False; // creationdate     <-> DateCreated
    sal_Bool bHasLastModified  = sal_False; // getlastmodified  <-> DateModified
    sal_Bool bHasContentType   = sal_False; // getcontenttype   <-> MediaType
    sal_Bool bHasContentLength = sal_False; // getcontentlength <-> Size

    Property aProp;

    std::vector< OUString >::const_iterator it  = aProps.begin();
    std::vector< OUString >::const_iterator end = aProps.end();
    while ( it != end )
    {
        m_pProvider->getProperty( (*it), aProp );

        aProperties[ nPos ] = aProp;

        if ( !bHasCreationDate &&
                ( aProp.Name == DAVProperties::CREATIONDATE ) )
        {
            bHasCreationDate = sal_True;
            nTotal++;
        }
        else if ( !bHasLastModified &&
                     ( aProp.Name == DAVProperties::GETLASTMODIFIED ) )
        {
            bHasLastModified = sal_True;
            nTotal++;
        }
        else if ( !bHasContentType &&
                     ( aProp.Name == DAVProperties::GETCONTENTTYPE ) )
        {
            bHasContentType = sal_True;
            nTotal++;
        }
        else if ( !bHasContentLength &&
                     ( aProp.Name == DAVProperties::GETCONTENTLENGTH ) )
        {
            bHasContentLength = sal_True;
            nTotal++;
        }

        it++;
        nPos++;
    }

    aProperties.realloc( nTotal );

    // Add mandatory properties.
    m_pProvider->getProperty(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ), aProp );
    aProperties[ nPos++ ] = aProp;

    m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ), aProp );
    aProperties[ nPos++ ] = aProp;

    m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ), aProp );
    aProperties[ nPos++ ] = aProp;

    m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aProp );
    aProperties[ nPos++ ] = aProp;

    // Add optional properties.
    if ( bHasCreationDate )
    {
        m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( bHasLastModified )
    {
        m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( bHasContentType )
    {
        m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    if ( bHasContentLength )
    {
        m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ), aProp );
        aProperties[ nPos++ ] = aProp;
    }

    return aProperties;
}

//=========================================================================
void Content::getProperties( const Reference< XCommandEnvironment > & xEnv,
                             PropertyMap & rProps )
{
    Sequence< Property > aProps = getProperties( xEnv );
    sal_Int32 nCount = aProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
        rProps.insert( aProps[ n ] );
}

//=========================================================================
// virtual
Sequence< CommandInfo > Content::getCommands(
                            const Reference< XCommandEnvironment > & xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

       if ( isFolder( xEnv ) )
    {
        //=================================================================
        //
        // Folder: Supported commands
        //
        //=================================================================

        static CommandInfo aFolderCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////

            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType( static_cast< Sequence< Property > * >( 0 ) )
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType( static_cast< Sequence< PropertyValue > * >( 0 ) )
            ),

            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////

            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuType( static_cast< InsertCommandArgument * >( 0 ) )
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast< TransferInfo * >( 0 ) )
            )

            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////

            /*
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "COPY" ) ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "MOVE" ) ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
            )
            */
        };

        return Sequence< CommandInfo >( aFolderCommandInfoTable, 8 );
    }
    else
    {
        //=================================================================
        //
        // Document: Supported commands
        //
        //=================================================================

        static CommandInfo aDocumentCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////

            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType( static_cast< Sequence< Property > * >( 0 ) )
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType( static_cast< Sequence< PropertyValue > * >( 0 ) )
            ),

            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////

            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuType( static_cast< InsertCommandArgument * >( 0 ) )
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
            )

            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////

            /*
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "COPY" ) ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
            ),
            CommandInfo(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "MOVE" ) ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
            )
            */
        };

        return Sequence< CommandInfo >( aDocumentCommandInfoTable, 7 );
    }
}

