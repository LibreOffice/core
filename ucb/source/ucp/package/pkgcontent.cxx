/*************************************************************************
 *
 *  $RCSfile: pkgcontent.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-28 14:20:41 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************
 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif

#ifndef _PKGCONTENT_HXX
#include "pkgcontent.hxx"
#endif
#ifndef _PKGPROVIDER_HXX
#include "pkgprovider.hxx"
#endif
#ifndef _PKGRESULTSET_HXX
#include "pkgresultset.hxx"
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;
using namespace rtl;

using namespace package_ucp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

// static ( "virtual" ctor )
Content* Content::create( const Reference< XMultiServiceFactory >& rxSMgr,
                          ::ucb::ContentProviderImplHelper* pProvider,
                          const Reference< XContentIdentifier >& Identifier )
{
    PackageUri aURI( Identifier->getContentIdentifier() );
    ContentProperties aProps;
    Reference< XHierarchicalNameAccess > xPackage;

    if ( loadData( rxSMgr, aURI, aProps, xPackage ) )
    {
        // resource exists

        Reference< XContentIdentifier > xId = Identifier;
        OUString aURL = xId->getContentIdentifier();
        sal_Int32 nLastSlash = aURL.lastIndexOf( '/' );
        if ( ( nLastSlash + 1 ) == aURL.getLength() )
        {
            // Client explicitely requested a folder!
            if ( !aProps.bIsFolder )
                return 0;

            // Note: Internally ids have no trailing slash.
            aURL = aURL.copy( 0, nLastSlash );
            xId = new ::ucb::ContentIdentifier( rxSMgr, aURL );
        }

        return new Content( rxSMgr, pProvider, xId, xPackage, aURI, aProps );
    }
    else
    {
        // resource doesn't exist

        sal_Bool bFolder = sal_False;

        Reference< XContentIdentifier > xId = Identifier;
        OUString aURL = xId->getContentIdentifier();
        sal_Int32 nLastSlash = aURL.lastIndexOf( '/' );
        if ( ( nLastSlash + 1 ) == aURL.getLength() )
        {
            bFolder = sal_True;

            // Note: Internally ids have no trailing slash.
            aURL = aURL.copy( 0, nLastSlash );
            xId = new ::ucb::ContentIdentifier( rxSMgr, aURL );
        }

        // Guess type according to URI.
        ContentInfo aInfo;
        if ( bFolder )
            aInfo.Type = OUString::createFromAscii(
                            PACKAGE_FOLDER_CONTENT_TYPE );
        else if ( aURI.getPath().compareToAscii( "/" ) == 0 )
            aInfo.Type = OUString::createFromAscii(
                            PACKAGE_FOLDER_CONTENT_TYPE ); // root folder
        else
            aInfo.Type = OUString::createFromAscii(
                            PACKAGE_STREAM_CONTENT_TYPE );

        return new Content( rxSMgr, pProvider, xId, xPackage, aURI, aInfo );
    }
}

//=========================================================================
// static ( "virtual" ctor )
Content* Content::create( const Reference< XMultiServiceFactory >& rxSMgr,
                          ::ucb::ContentProviderImplHelper* pProvider,
                          const Reference< XContentIdentifier >& Identifier,
                          const ContentInfo& Info )
{
    if ( !Info.Type.getLength() )
        return 0;

    if ( ( Info.Type.compareToAscii( PACKAGE_FOLDER_CONTENT_TYPE ) != 0 ) &&
         ( Info.Type.compareToAscii( PACKAGE_STREAM_CONTENT_TYPE ) != 0 ) )
        return 0;

    PackageUri aURI( Identifier->getContentIdentifier() );

#if 0
    // Fail, if content does exist.
    if ( hasData( rxSMgr, aURI ) )
        return 0;
#endif

    Reference< XHierarchicalNameAccess > xPackage;
    return new Content( rxSMgr, pProvider, Identifier, xPackage, aURI, Info );
}

//=========================================================================
Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                  ::ucb::ContentProviderImplHelper* pProvider,
                  const Reference< XContentIdentifier >& Identifier,
                  const Reference< XHierarchicalNameAccess >& Package,
                  const PackageUri& rUri,
                  const ContentProperties& rProps )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_xPackage( Package ),
  m_aUri( rUri ),
  m_aProps( rProps ),
  m_eState( PERSISTENT )
{
}

//=========================================================================
Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                  ::ucb::ContentProviderImplHelper* pProvider,
                  const Reference< XContentIdentifier >& Identifier,
                  const Reference< XHierarchicalNameAccess >& Package,
                  const PackageUri& rUri,
                  const ContentInfo& Info )
: ContentImplHelper( rxSMgr, pProvider, Identifier, sal_False ),
  m_xPackage( Package ),
  m_aUri( rUri ),
  m_eState( TRANSIENT )
{
    if ( Info.Type.compareToAscii( PACKAGE_FOLDER_CONTENT_TYPE ) == 0 )
    {
        // New folder...
        m_aProps.aContentType = Info.Type;
//      m_aProps.aTitle       =
//      m_aProps.aMediaType   =
        m_aProps.bIsFolder    = sal_True;
        m_aProps.bIsDocument  = sal_False;
        m_aProps.nSize        = 0;
    }
    else
    {
//      VOS_ENSURE(
//          Info.Type.compareToAscii( PACKAGE_STREAM_CONTENT_TYPE ) == 0,
//          "Content::Content - Wrong content info!" );

        // New stream...
        m_aProps.aContentType = Info.Type;
//      m_aProps.aTitle       =
//      m_aProps.aMediaType   =
        m_aProps.bIsFolder    = sal_False;
        m_aProps.bIsDocument  = sal_True;
        m_aProps.nSize        = 0;
    }
}

//=========================================================================
// virtual
Content::~Content()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire()
    throw( RuntimeException )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw( RuntimeException )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
Any SAL_CALL Content::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet;

    if ( isFolder() )
          aRet = cppu::queryInterface( rType,
                                     static_cast< XContentCreator * >( this ) );

     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
Sequence< Type > SAL_CALL Content::getTypes()
    throw( RuntimeException )
{
    static OTypeCollection* pCollection = NULL;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
            if ( isFolder() )
            {
                  static OTypeCollection aCollection(
                    CPPU_TYPE_REF( XTypeProvider ),
                       CPPU_TYPE_REF( XServiceInfo ),
                       CPPU_TYPE_REF( XComponent ),
                       CPPU_TYPE_REF( XContent ),
                       CPPU_TYPE_REF( XCommandProcessor ),
                       CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                       CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                       CPPU_TYPE_REF( XPropertyContainer ),
                       CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                       CPPU_TYPE_REF( XChild ),
                       CPPU_TYPE_REF( XContentCreator ) );  // !!
                  pCollection = &aCollection;
            }
            else
            {
                  static OTypeCollection aCollection(
                    CPPU_TYPE_REF( XTypeProvider ),
                       CPPU_TYPE_REF( XServiceInfo ),
                       CPPU_TYPE_REF( XComponent ),
                       CPPU_TYPE_REF( XContent ),
                       CPPU_TYPE_REF( XCommandProcessor ),
                       CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                       CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                       CPPU_TYPE_REF( XPropertyContainer ),
                       CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                       CPPU_TYPE_REF( XChild ) );
                  pCollection = &aCollection;
            }
        }
    }

    return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getImplementationName()
    throw( RuntimeException )
{
    return OUString::createFromAscii( "PackageContent" );
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    if ( isFolder() )
        aSNS.getArray()[ 0 ]
            = OUString::createFromAscii( PACKAGE_FOLDER_CONTENT_SERVICE_NAME );
    else
        aSNS.getArray()[ 0 ]
            = OUString::createFromAscii( PACKAGE_STREAM_CONTENT_SERVICE_NAME );

    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getContentType()
    throw( RuntimeException )
{
    return m_aProps.aContentType;
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
Any SAL_CALL Content::execute( const Command& aCommand,
                                sal_Int32 CommandId,
                                const Reference<
                                        XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    Any aRet;

    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" );
            return Any();
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" );
            return Any();
        }

        if ( !aProperties.getLength() )
        {
            VOS_ENSURE( sal_False, "No properties!" );
            return Any();
        }

        setPropertyValues( aProperties );
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo();
    }
    else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo();
    }
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 )
    {
        OpenCommandArgument2 aOpenCommand;
        if ( aCommand.Argument >>= aOpenCommand )
        {
            aRet = open( aOpenCommand, Environment );
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "Content::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else if ( aCommand.Name.compareToAscii( "insert" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        InsertCommandArgument aArg;
        if ( aCommand.Argument >>= aArg )
        {
            sal_Int32 nNameClash = aArg.ReplaceExisting
                                 ? NameClash::OVERWRITE
                                 : NameClash::ERROR;
            insert( aArg.Data, nNameClash );
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "Content::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else if ( aCommand.Name.compareToAscii( "delete" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical );

        // Remove own and all children's persistent data.
        removeData();

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );
    }
    else if ( aCommand.Name.compareToAscii( "transfer" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // transfer
        //  ( Not available at stream objects )
        //////////////////////////////////////////////////////////////////

        TransferInfo aInfo;
        if ( aCommand.Argument >>= aInfo )
        {
            transfer( aInfo );
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "Content::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else if ( aCommand.Name.compareToAscii( "flush" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // flush
        //  ( Not available at stream objects )
        //////////////////////////////////////////////////////////////////

        flushData();
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unknown command
        //////////////////////////////////////////////////////////////////

        VOS_ENSURE( sal_False,
                    "Content::execute - unknown command!" );
        throw CommandAbortedException();
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
    // @@@ Implement logic to abort running commands, if this makes
    //     sense for your content.
}

//=========================================================================
//
// XContentCreator methods.
//
//=========================================================================

// virtual
Sequence< ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
    throw( RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        Sequence< Property > aProps( 1 );
        aProps.getArray()[ 0 ] = Property(
                    OUString::createFromAscii( "Title" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND );

        Sequence< ContentInfo > aSeq( 2 );

        // Folder.
        aSeq.getArray()[ 0 ].Type
            = OUString::createFromAscii( PACKAGE_FOLDER_CONTENT_TYPE );
        aSeq.getArray()[ 0 ].Attributes = 0;
        aSeq.getArray()[ 0 ].Properties = aProps;

        // Stream.
        aSeq.getArray()[ 1 ].Type
            = OUString::createFromAscii( PACKAGE_STREAM_CONTENT_TYPE );
        aSeq.getArray()[ 1 ].Attributes
            = ContentInfoAttribute::INSERT_WITH_INPUTSTREAM;
        aSeq.getArray()[ 1 ].Properties = aProps;

        return aSeq;
    }
    else
    {
        VOS_ENSURE( sal_False,
                    "queryCreatableContentsInfo called on non-folder object!" );

        return Sequence< ContentInfo >( 0 );
    }
}

//=========================================================================
// virtual
Reference< XContent > SAL_CALL Content::createNewContent(
                                                const ContentInfo& Info )
    throw( RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !Info.Type.getLength() )
            return Reference< XContent >();

        if ( ( Info.Type.compareToAscii( PACKAGE_FOLDER_CONTENT_TYPE ) != 0 )
              &&
              ( Info.Type.compareToAscii( PACKAGE_STREAM_CONTENT_TYPE ) != 0 ) )
            return Reference< XContent >();

        OUString aURL = m_xIdentifier->getContentIdentifier();

        VOS_ENSURE( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ),
                    "Content::createNewContent - invalid URL!" );

        aURL += OUString::createFromAscii( "/" );

        if ( Info.Type.compareToAscii( PACKAGE_FOLDER_CONTENT_TYPE ) == 0 )
            aURL += OUString::createFromAscii( "New_Folder" );
        else
            aURL += OUString::createFromAscii( "New_Stream" );

        Reference< XContentIdentifier > xId(
                        new ::ucb::ContentIdentifier( m_xSMgr, aURL ) );

        return create( m_xSMgr, m_xProvider.getBodyPtr(), xId, Info );
    }
    else
    {
        VOS_ENSURE( sal_False,
                    "createNewContent called on non-folder object!" );
        return Reference< XContent >();
    }
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// virtual
OUString Content::getParentURL()
{
    return m_aUri.getParentUri();
}

//=========================================================================
// static
Reference< XRow > Content::getPropertyValues(
                const Reference< XMultiServiceFactory >& rSMgr,
                const Sequence< Property >& rProperties,
                const vos::ORef< ucb::ContentProviderImplHelper >& rProvider,
                const OUString& rContentId )
{
    ContentProperties aData;
    Reference< XHierarchicalNameAccess > xPackage;
    if ( loadData( rSMgr, PackageUri( rContentId ), aData, xPackage ) )
    {
        return getPropertyValues( rSMgr,
                                  rProperties,
                                  aData,
                                  rProvider,
                                  rContentId );
    }
    else
    {
        vos::ORef< ::ucb::PropertyValueSet > xRow
                                = new ::ucb::PropertyValueSet( rSMgr );

        sal_Int32 nCount = rProperties.getLength();
        if ( nCount )
        {
            const Property* pProps = rProperties.getConstArray();
            for ( sal_Int32 n = 0; n < nCount; ++n )
                xRow->appendVoid( pProps[ n ] );
        }

        return Reference< XRow >( xRow.getBodyPtr() );
    }
}

//=========================================================================
// static
Reference< XRow > Content::getPropertyValues(
                const Reference< XMultiServiceFactory >& rSMgr,
                const Sequence< Property >& rProperties,
                const ContentProperties& rData,
                const vos::ORef< ucb::ContentProviderImplHelper >& rProvider,
                const OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    vos::ORef< ::ucb::PropertyValueSet > xRow
                                = new ::ucb::PropertyValueSet( rSMgr );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        Reference< XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name.compareToAscii( "ContentType" ) == 0 )
            {
                xRow->appendString ( rProp, rData.aContentType );
            }
            else if ( rProp.Name.compareToAscii( "Title" ) == 0 )
            {
                xRow->appendString ( rProp, rData.aTitle );
            }
            else if ( rProp.Name.compareToAscii( "IsDocument" ) == 0 )
            {
                xRow->appendBoolean( rProp, rData.bIsDocument );
            }
            else if ( rProp.Name.compareToAscii( "IsFolder" ) == 0 )
            {
                xRow->appendBoolean( rProp, rData.bIsFolder );
            }
            else if ( rProp.Name.compareToAscii( "MediaType" ) == 0 )
            {
                xRow->appendString ( rProp, rData.aMediaType );
            }
            else if ( rProp.Name.compareToAscii( "Size" ) == 0 )
            {
                xRow->appendLong   ( rProp, rData.nSize );
            }
            else
            {
                // Not a Core Property! Maybe it's an Additional Core Property?!

                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = Reference< XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            UNO_QUERY );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    if ( !xRow->appendPropertySetValue(
                                                xAdditionalPropSet,
                                                rProp ) )
                    {
                        // Append empty entry.
                        xRow->appendVoid( rProp );
                    }
                }
                else
                {
                    // Append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            Property( OUString::createFromAscii( "ContentType" ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.aContentType );
        xRow->appendString (
            Property( OUString::createFromAscii( "Title" ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND ),
            rData.aTitle );
        xRow->appendBoolean(
            Property( OUString::createFromAscii( "IsDocument" ),
                      -1,
                      getCppuBooleanType(),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.bIsDocument );
        xRow->appendBoolean(
            Property( OUString::createFromAscii( "IsFolder" ),
                      -1,
                      getCppuBooleanType(),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.bIsFolder );
        xRow->appendString (
            Property( OUString::createFromAscii( "MediaType" ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND ),
            rData.aMediaType );
        xRow->appendLong   (
            Property( OUString::createFromAscii( "Size" ),
                      -1,
                      getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.nSize );

        // Append all Additional Core Properties.

        Reference< XPropertySet > xSet(
            rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return Reference< XRow >( xRow.getBodyPtr() );
}

//=========================================================================
Reference< XRow > Content::getPropertyValues(
                                const Sequence< Property >& rProperties )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              m_xProvider,
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
void Content::setPropertyValues( const Sequence< PropertyValue >& rValues )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    Reference< XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;
    sal_Bool bExchange = sal_False;
    sal_Bool bStore    = sal_False;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.compareToAscii( "ContentType" ) == 0 )
        {
            // Read-only property!
        }
        else if ( rValue.Name.compareToAscii( "IsDocument" ) == 0 )
        {
            // Read-only property!
        }
        else if ( rValue.Name.compareToAscii( "IsFolder" ) == 0 )
        {
            // Read-only property!
        }
        else if ( rValue.Name.compareToAscii( "Title" ) == 0 )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                if ( aNewValue != m_aProps.aTitle )
                {
                    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );
                    m_aProps.aTitle = aNewValue;

                    // modified title -> modified URL -> exchange !
                    if ( m_eState == PERSISTENT )
                        bExchange = sal_True;

                    aGuard.clear();

                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = makeAny( m_aProps.aTitle );
                    aEvent.NewValue     = makeAny( aNewValue );

                    aChanges.getArray()[ nChanged ] = aEvent;
                    nChanged++;
                }
            }
        }
        else if ( rValue.Name.compareToAscii( "MediaType" ) == 0 )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                if ( aNewValue != m_aProps.aMediaType )
                {
                    osl::Guard< osl::Mutex > aGuard( m_aMutex );
                    m_aProps.aMediaType = aNewValue;
                      nChanged++;
                    bStore = sal_True;
                }
            }
        }
        else if ( rValue.Name.compareToAscii( "Size" ) == 0 )
        {
            // Read-only property!
        }
        else
        {
            // Not a Core Property! Maybe it's an Additional Core Property?!

            if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                bTriedToGetAdditonalPropSet = sal_True;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    Any aOldValue = xAdditionalPropSet->getPropertyValue(
                                                                rValue.Name );
                    xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                    if ( aOldValue != rValue.Value )
                    {
                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = aOldValue;
                        aEvent.NewValue     = rValue.Value;

                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                }
                catch ( UnknownPropertyException )
                {
                }
                catch ( WrappedTargetException )
                {
                }
                catch ( PropertyVetoException )
                {
                }
                catch ( IllegalArgumentException )
                {
                }
            }
        }
    }

    if ( bExchange )
    {
        Reference< XContentIdentifier > xOldId = m_xIdentifier;

        // Assemble new content identifier...
        OUString aNewURL = m_aUri.getParentUri();
        aNewURL += m_aProps.aTitle;
        Reference< XContentIdentifier > xNewId
                        = new ::ucb::ContentIdentifier( m_xSMgr, aNewURL );

        aGuard.clear();
        if ( exchangeIdentity( xNewId ) )
        {
            // Adapt persistent data.
            renameData( xOldId, xNewId );

            // Adapt Additional Core Properties.
            renameAdditionalPropertySet(
                            xOldId->getContentIdentifier(),
                            xNewId->getContentIdentifier(),
                            sal_True );
        }
    }

    if ( nChanged > 0 )
    {
        // Save changes, if content was already made persistent.
        if ( bStore && ( m_eState == PERSISTENT  ) )
            storeData( Reference< XInputStream >() );

        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }
}

//=========================================================================
Any Content::open( const OpenCommandArgument2& rArg,
                   const Reference< XCommandEnvironment >& xEnv )
    throw( CommandAbortedException )
{
      if ( isFolder() )
    {
        //////////////////////////////////////////////////////////////////
        // open command for a folder content
        //////////////////////////////////////////////////////////////////

        Reference< XDynamicResultSet > xSet
                    = new DynamicResultSet( m_xSMgr, this, rArg, xEnv );
        Any aRet;
        aRet <<= xSet;
        return aRet;
      }
      else
    {
        //////////////////////////////////////////////////////////////////
        // open command for a document content
        //////////////////////////////////////////////////////////////////

        if ( ( rArg.Mode == OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
             ( rArg.Mode == OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
        {
            // Currently(?) unsupported.
              throw CommandAbortedException();
        }

        OUString aURL = m_xIdentifier->getContentIdentifier();
        Reference< XOutputStream > xOut
              = Reference< XOutputStream >( rArg.Sink, UNO_QUERY );
        if ( xOut.is() )
          {
            // PUSH: write data into xOut

               Reference< XInputStream > xIn = getInputStream();
            if ( !xIn.is() )
                  throw CommandAbortedException();

            try
            {
                Sequence< sal_Int8 > aBuffer( 65536 );
                sal_Int32  nRead = xIn->readSomeBytes( aBuffer, 65536 );

                while ( nRead > 0 )
                {
                    aBuffer.realloc( nRead );
                    xOut->writeBytes( aBuffer );
                    aBuffer.realloc( 65536 );

                    nRead = xIn->readSomeBytes( aBuffer, 65536 );
                }

                xOut->closeOutput();
            }
            catch ( NotConnectedException & )
            {
                // closeOutput, readSomeBytes, writeBytes
            }
            catch ( BufferSizeExceededException & )
            {
                // closeOutput, readSomeBytes, writeBytes
            }
            catch ( IOException & )
            {
                // closeOutput, readSomeBytes, writeBytes
            }
          }
        else
          {
            Reference< XActiveDataSink > xDataSink
                  = Reference< XActiveDataSink >(   rArg.Sink, UNO_QUERY );
              if ( xDataSink.is() )
            {
                  // PULL: wait for client read

                Reference< XInputStream > xIn = getInputStream();
                if ( !xIn.is() )
                      throw CommandAbortedException();

                // Done.
                xDataSink->setInputStream( xIn );
            }
              else
            {
                  VOS_ENSURE( sal_False,
                              "Content::execute - invalid parameter!" );
                  throw CommandAbortedException();
            }
          }
    }

    return Any();
}

//=========================================================================
void Content::insert(
        const Reference< XInputStream >& xStream, sal_Int32 nNameClashResolve )
    throw( CommandAbortedException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( isFolder() )
    {
        // Required: Title

        if ( !m_aProps.aTitle.getLength() )
        {
            VOS_ENSURE( sal_False,
                        "Content::insert - property value missing!" );
            throw CommandAbortedException();
        }
    }
    else
    {
        // Required: rArg.Data

        if ( !xStream.is() )
        {
            VOS_ENSURE( sal_False,
                        "Content::insert - input stream missing!" );
            throw CommandAbortedException();
        }

        // Required: Title

        if ( !m_aProps.aTitle.getLength() )
        {
            VOS_ENSURE( sal_False,
                        "Content::insert - property value missing!" );
            throw CommandAbortedException();
        }
    }

    OUString aNewURL = m_aUri.getParentUri();
    aNewURL += m_aProps.aTitle;
    PackageUri aNewUri( aNewURL );

    // Handle possible name clash...
    switch ( nNameClashResolve )
    {
        // fail.
        case NameClash::ERROR:
            if ( hasData( m_xSMgr, aNewUri ) )
                throw CommandAbortedException();

            break;

        // replace (possibly) existing object.
        case NameClash::OVERWRITE:
            break;

        // "invent" a new valid title.
        case NameClash::RENAME:
            if ( hasData( m_xSMgr, aNewUri ) )
            {
                sal_Int32 nTry = 0;

                do
                {
                    OUString aNew = aNewUri.getUri();
                    aNew += OUString::createFromAscii( "_" );
                    aNew += OUString::valueOf( ++nTry );
                    aNewUri.setUri( aNew );
                }
                while ( hasData( m_xSMgr, aNewUri ) && ( nTry < 100000 ) );

                if ( nTry == 100000 )
                {
                    VOS_ENSURE( sal_False,
                                "Content::insert - "
                                "Unable to resolve name clash" );
                    throw CommandAbortedException();
                }
                else
                {
                    m_aProps.aTitle += OUString::createFromAscii( "_" );
                    m_aProps.aTitle += OUString::valueOf( nTry );
                }
            }
            break;

        // keep existing sub-objects, transfer non-clashing sub-objects.
        case NameClash::KEEP:
            // @@@

        default:
            throw CommandAbortedException();
    }

    m_xIdentifier = new ::ucb::ContentIdentifier( m_xSMgr, aNewURL );
    m_aUri = aNewUri;

    storeData( xStream );

    if ( m_eState == TRANSIENT )
    {
        m_eState = PERSISTENT;

        aGuard.clear();
        inserted();
    }
}

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( CommandAbortedException )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Reference< XContent > xThis = this;

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        VOS_ENSURE( sal_False, "Content::destroy - Not persistent!" );
        throw CommandAbortedException();
    }

    m_eState = DEAD;

    aGuard.clear();
    deleted();

    if ( isFolder() )
    {
        // Process instanciated children...

        ContentRefList aChildren;
        queryChildren( aChildren );

        ContentRefList::const_iterator it  = aChildren.begin();
        ContentRefList::const_iterator end = aChildren.end();

        while ( it != end )
        {
            (*it)->destroy( bDeletePhysical );
            ++it;
        }
    }
}

//=========================================================================
void Content::transfer( const TransferInfo& rInfo )
    throw( CommandAbortedException )
{
    // targeturl->XNamed::setParent( this )

    // setParent hat weder returnwert noch wirft es exceptions
    // --> nach setparent checken, ob neues elem existiert und
    //     by move checken, ob altes elem weg ist.

    // dynamische props nicht vergessen!

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        VOS_ENSURE( sal_False, "Content::transfer - Not persistent!" );
        throw CommandAbortedException();
    }

    if ( rInfo.SourceURL.getLength() == 0 )
        throw CommandAbortedException();

    // Is source a hierarchy content?
    if ( rInfo.SourceURL.compareToAscii(
            PACKAGE_URL_SCHEME "://", PACKAGE_URL_SCHEME_LENGTH + 3 ) != 0 )
        throw CommandAbortedException();

    // Is source not a parent of me / not me?
    OUString aId = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aId.lastIndexOf( '/' );
    if ( nPos != ( aId.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aId += OUString::createFromAscii( "/" );
    }

    if ( rInfo.SourceURL.getLength() <= aId.getLength() )
    {
        if ( aId.compareTo(
                rInfo.SourceURL, rInfo.SourceURL.getLength() ) == 0 )
            throw CommandAbortedException();
    }

    try
    {
        //////////////////////////////////////////////////////////////////
        // 0) Obtain content object for source.
        //////////////////////////////////////////////////////////////////

        Reference< XContentIdentifier > xId =
                    new ::ucb::ContentIdentifier( m_xSMgr, rInfo.SourceURL );

        // Note: The static cast is okay here, because its sure that
        //       m_xProvider is always the PackageContentProvider.
        vos::ORef< Content > xSource
            = static_cast< Content * >(
                m_xProvider->queryContent( xId ).get() );
        if ( !xSource.isValid() )
            throw CommandAbortedException();

        //////////////////////////////////////////////////////////////////
        // 1) Create new child content.
        //////////////////////////////////////////////////////////////////

        OUString aType = xSource->isFolder()
                       ? OUString::createFromAscii(
                               PACKAGE_FOLDER_CONTENT_SERVICE_NAME )
                       : OUString::createFromAscii(
                               PACKAGE_STREAM_CONTENT_SERVICE_NAME );
        ContentInfo aInfo;
        aInfo.Type = aType;
        aInfo.Attributes = 0;

        // Note: The static cast is okay here, because its sure that
        //       createNewContent always creates a Content.
        vos::ORef< Content > xTarget = static_cast< Content * >(
                                            createNewContent( aInfo ).get() );
        if ( !xTarget.isValid() )
            throw CommandAbortedException();

        //////////////////////////////////////////////////////////////////
        // 2) Copy data from source content to child content.
        //////////////////////////////////////////////////////////////////

        Sequence< Property > aProps
                        = xSource->getPropertySetInfo()->getProperties();
        sal_Int32 nCount = aProps.getLength();

        if ( nCount )
        {
            sal_Bool bHadTitle = ( rInfo.NewTitle.getLength() == 0 );

            // Get all source values.
            Reference< XRow > xRow = xSource->getPropertyValues( aProps );

            Sequence< PropertyValue > aValues( nCount );
            PropertyValue* pValues = aValues.getArray();

            const Property* pProps = aProps.getConstArray();
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                const Property& rProp  = pProps[ n ];
                PropertyValue&  rValue = pValues[ n ];

                rValue.Name   = rProp.Name;
                rValue.Handle = rProp.Handle;

                if ( !bHadTitle && rProp.Name.compareToAscii( "Title" ) == 0 )
                {
                    // Set new title instead of original.
                    bHadTitle = sal_True;
                    rValue.Value <<= rInfo.NewTitle;
                }
                else
                    rValue.Value
                        = xRow->getObject( n + 1, Reference< XNameAccess >() );

                rValue.State = PropertyState_DIRECT_VALUE;

                if ( rProp.Attributes & PropertyAttribute::REMOVABLE )
                {
                    // Add Additional Core Property.
                    try
                    {
                        xTarget->addProperty( rProp.Name,
                                              rProp.Attributes,
                                              rValue.Value );
                    }
                    catch ( PropertyExistException & )
                    {
                    }
                    catch ( IllegalTypeException & )
                    {
                    }
                    catch ( IllegalArgumentException & )
                    {
                    }
                }
            }

            // Set target values.
            xTarget->setPropertyValues( aValues );
        }

        //////////////////////////////////////////////////////////////////
        // 3) Commit (insert) child.
        //////////////////////////////////////////////////////////////////

        xTarget->insert( xSource->getInputStream(), rInfo.NameClash );

        //////////////////////////////////////////////////////////////////
        // 4) Transfer (copy) children of source.
        //////////////////////////////////////////////////////////////////

        if ( xSource->isFolder() )
        {
            Reference< XEnumeration > xIter = xSource->getIterator();
            if ( xIter.is() )
            {
                while ( xIter->hasMoreElements() )
                {
                    try
                    {
                        Reference< XNamed > xNamed;
                        xIter->nextElement() >>= xNamed;

                        if ( !xNamed.is() )
                        {
                            VOS_ENSURE( sal_False,
                                        "Content::transfer - Got no XNamed!" );
                            break;
                        }

                        OUString aName = xNamed->getName();

                        if ( !aName.getLength() )
                        {
                            VOS_ENSURE( sal_False,
                                        "Content::transfer - Empty name!" );
                            break;
                        }

                        OUString aChildId = xId->getContentIdentifier();
                        if ( ( aChildId.lastIndexOf( '/' ) + 1 )
                                                    != aChildId.getLength() )
                            aChildId += OUString::createFromAscii( "/" );

                        aChildId += aName;

                        Reference< XContentIdentifier > xChildId
                            = new ::ucb::ContentIdentifier( m_xSMgr, aChildId );

                        vos::ORef< Content > xChild
                            = static_cast< Content * >(
                                m_xProvider->queryContent( xChildId ).get() );

                        TransferInfo aInfo;
                        aInfo.MoveData  = sal_False;
                        aInfo.NewTitle  = OUString();
                        aInfo.SourceURL = aChildId;
                        aInfo.NameClash = rInfo.NameClash;

                        // Transfer child to target.
                        xTarget->transfer( aInfo );
                    }
                    catch ( NoSuchElementException & )
                    {
                    }
                    catch ( WrappedTargetException & )
                    {
                    }
                }
            }
        }

        //////////////////////////////////////////////////////////////////
        // 5) Destroy source ( when moving only ) .
        //////////////////////////////////////////////////////////////////

        if ( rInfo.MoveData )
        {
            xSource->destroy( sal_True );

            // Remove all persistent data of source and its children.
            xSource->removeData();

            // Remove own and all children's Additional Core Properties.
            xSource->removeAdditionalPropertySet( sal_True );
        }
    }
    catch ( IllegalIdentifierException & )
    {
        // queryContent
        VOS_ENSURE( sal_False, "Content::transfer - "
                               "Caught IllegalIdentifierException!" );
        throw CommandAbortedException();
    }
}

//=========================================================================
sal_Bool Content::exchangeIdentity(
                        const Reference< XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Reference< XContent > xThis = this;

    // Already persistent?
    if ( m_eState != PERSISTENT )
    {
        VOS_ENSURE( sal_False,
                    "Content::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
    PackageUri aNewUri( xNewId->getContentIdentifier() );
    if ( !hasData( m_xSMgr, aNewUri ) )
    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            m_aUri = aNewUri;
            if ( isFolder() )
            {
                // Process instanciated children...

                ContentRefList aChildren;
                queryChildren( aChildren );

                ContentRefList::const_iterator it  = aChildren.begin();
                ContentRefList::const_iterator end = aChildren.end();

                while ( it != end )
                {
                    ContentRef xChild = (*it);

                    // Create new content identifier for the child...
                    Reference< XContentIdentifier > xOldChildId
                                                    = xChild->getIdentifier();
                    OUString aOldChildURL = xOldChildId->getContentIdentifier();
                    OUString aNewChildURL
                        = aOldChildURL.replaceAt(
                                        0,
                                        aOldURL.getLength(),
                                        xNewId->getContentIdentifier() );
                    Reference< XContentIdentifier > xNewChildId
                        = new ::ucb::ContentIdentifier( m_xSMgr, aNewChildURL );

                    if ( !xChild->exchangeIdentity( xNewChildId ) )
                        return sal_False;

                    ++it;
                }
            }
            return sal_True;
        }
    }

    VOS_ENSURE( sal_False,
                "Content::exchangeIdentity - Panic! Cannot exchange identity!" );
    return sal_False;
}

//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    // @@@ Adapt method to your URL scheme...

    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucb::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();

    VOS_ENSURE( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ),
                "Content::queryChildren - Invalid URL!" );

    aURL += OUString::createFromAscii( "/" );

    sal_Int32 nLen = aURL.getLength();

    ::ucb::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucb::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucb::ContentImplHelperRef xChild = (*it);
        OUString aChildURL = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            if ( aChildURL.indexOf( '/', nLen ) == -1 )
            {
                // No further slashes. It's a child!
                rChildren.push_back(
                    ContentRef(
                        static_cast< Content * >( xChild.getBodyPtr() ) ) );
            }
        }
        ++it;
    }
}

//=========================================================================
Reference< XHierarchicalNameAccess > Content::getPackage()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_xPackage.is() )
        m_xPackage = getPackage( m_xSMgr, m_aUri );

    return m_xPackage;
}

//=========================================================================
// static
Reference< XHierarchicalNameAccess > Content::getPackage(
                             const Reference< XMultiServiceFactory > & rxSMgr,
                            const PackageUri& rURI )
{
    Reference< XHierarchicalNameAccess > xNameAccess;

    OUString aPackageName = rURI.getPackage();
    if ( !aPackageName.getLength() )
    {
        VOS_ENSURE( sal_False, "Content::getPackage - Invalid URL!" );
        return xNameAccess;
    }

    try
    {
        Sequence< Any > aArguments( 1 );
        aArguments[ 0 ] <<= aPackageName;

        Reference< XInterface > xIfc
            = rxSMgr->createInstanceWithArguments(
                OUString::createFromAscii( "com.sun.star.package.Package" ),
                aArguments );

        if ( xIfc.is() )
        {
            xNameAccess
                = Reference< XHierarchicalNameAccess >( xIfc, UNO_QUERY );

            VOS_ENSURE( xNameAccess.is(),
                        "Content::getPackage - "
                        "Got no hierarchical name access!" );
        }
    }
    catch ( RuntimeException & )
    {
        throw;
    }
    catch ( Exception & )
    {
        // createInstanceWithArguemts
    }

    return xNameAccess;
}

//=========================================================================
// static
sal_Bool Content::hasData( const Reference< XMultiServiceFactory >& rxSMgr,
                           const PackageUri& rURI )
{
    Reference< XHierarchicalNameAccess > xNA = getPackage( rxSMgr, rURI );
    if ( !xNA.is() )
        return sal_False;

    return xNA->hasByHierarchicalName( rURI.getPath() );
}

//=========================================================================
//static
sal_Bool Content::loadData( const Reference< XMultiServiceFactory >& rxSMgr,
                            const PackageUri& rURI,
                            ContentProperties& rProps,
                            Reference< XHierarchicalNameAccess > & rxPackage )
{
    rxPackage = getPackage( rxSMgr, rURI );
    if ( !rxPackage.is() )
        return sal_False;

    try
    {
        Any aEntry = rxPackage->getByHierarchicalName( rURI.getPath() );
        if ( aEntry.hasValue() )
        {
            Reference< XPropertySet > xPropSet;
            aEntry >>= xPropSet;

            if ( !xPropSet.is() )
            {
                VOS_ENSURE( sal_False,
                            "Content::loadData - Got no XPropertySet interface!" );
                return sal_False;
            }

            // Title
            rProps.aTitle = rURI.getName();

            // MediaType
            try
            {
                Any aMediaType
                    = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "MediaType" ) );
                if ( !( aMediaType >>= rProps.aMediaType ) )
                {
                    VOS_ENSURE( sal_False,
                                "Content::loadData - Got no MediaType value!" );
                    return sal_False;
                }
            }
            catch ( UnknownPropertyException & )
            {
                VOS_ENSURE( sal_False,
                            "Content::loadData - Got no MediaType value!" );
                return sal_False;
            }
            catch ( WrappedTargetException & )
            {
                VOS_ENSURE( sal_False,
                            "Content::loadData - Got no MediaType value!" );
                return sal_False;
            }

            // Size
            try
            {
                Any aSize
                    = xPropSet->getPropertyValue(
                        OUString::createFromAscii( "Size" ) );
                if ( !( aSize >>= rProps.nSize ) )
                {
                    VOS_ENSURE( sal_False,
                                "Content::loadData - Got no Size value!" );
                    return sal_False;
                }
            }
            catch ( UnknownPropertyException & )
            {
                VOS_ENSURE( sal_False,
                            "Content::loadData - Got no Size value!" );
                return sal_False;
            }
            catch ( WrappedTargetException & )
            {
                VOS_ENSURE( sal_False,
                            "Content::loadData - Got no Size value!" );
                return sal_False;
            }

            Reference< XEnumerationAccess > xEnumAccess;
            aEntry >>= xEnumAccess;

            // ContentType / IsFolder / IsDocument
            if ( xEnumAccess.is() )
            {
                // folder
                rProps.aContentType
                    = OUString::createFromAscii( PACKAGE_FOLDER_CONTENT_TYPE );
                rProps.bIsDocument = sal_False;
                rProps.bIsFolder = sal_True;
            }
            else
            {
                // stream
                rProps.aContentType
                    = OUString::createFromAscii( PACKAGE_STREAM_CONTENT_TYPE );
                rProps.bIsDocument = sal_True;
                rProps.bIsFolder = sal_False;
            }

            return sal_True;
        }
    }
    catch ( NoSuchElementException & )
    {
        // getByHierarchicalName
    }

    return sal_False;
}

//=========================================================================
sal_Bool Content::renameData( const Reference< XContentIdentifier >& xOldId,
                               const Reference< XContentIdentifier >& xNewId )
{
    PackageUri aURI( xOldId->getContentIdentifier() );
    Reference< XHierarchicalNameAccess > xNA = getPackage( m_xSMgr, aURI );
    if ( !xNA.is() )
        return sal_False;

    try
    {
        Any aEntry = xNA->getByHierarchicalName( aURI.getPath() );
        Reference< XNamed > xNamed;
        aEntry >>= xNamed;

        if ( !xNamed.is() )
        {
            VOS_ENSURE( sal_False,
                        "Content::renameData - Got no XNamed interface!" );
            return sal_False;
        }

        PackageUri aNewURI( xNewId->getContentIdentifier() );

        // No success indicator!? No return value / exceptions specified.
        xNamed->setName( aNewURI.getName() );
        return sal_True;
    }
    catch ( NoSuchElementException & )
    {
        // getByHierarchicalName
    }

    return sal_False;
}

//=========================================================================
sal_Bool Content::storeData( const Reference< XInputStream >& xStream )
{
    Reference< XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return sal_False;

    if ( !xNA->hasByHierarchicalName( m_aUri.getPath() ) )
    {
//      if ( !bCreate )
//          return sal_True;

        try
        {
            // Create new resource...
            Reference< XSingleServiceFactory > xFac( xNA, UNO_QUERY );
            if ( !xFac.is() )
            {
                VOS_ENSURE( sal_False,
                            "Content::storeData - "
                            "Got no XSingleServiceFactory interface!" );
                return sal_False;
            }

            Sequence< Any > aArgs( 1 );
            aArgs[ 0 ]  <<= isFolder();

            Reference< XInterface > xNew
                = xFac->createInstanceWithArguments( aArgs );

            if ( !xNew.is() )
            {
                VOS_ENSURE( sal_False,
                            "Content::storeData - createInstance failed!" );
                return sal_False;
            }

            PackageUri aParentUri( getParentURL() );
            Any aEntry = xNA->getByHierarchicalName( aParentUri.getPath() );
            Reference< XNameContainer > xParentContainer;
            aEntry >>= xParentContainer;

            if ( !xParentContainer.is() )
            {
                VOS_ENSURE( sal_False,
                            "Content::storeData - "
                            "Got no XNameContainer interface!" );
                return sal_False;
            }

            xParentContainer->insertByName( m_aProps.aTitle, makeAny( xNew ) );
        }
        catch ( RuntimeException & )
        {
            throw;
        }
        catch ( IllegalArgumentException & )
        {
            // insertByName
            VOS_ENSURE( sal_False, "Content::storeData - insertByName failed!" );
            return sal_False;
        }
        catch ( ElementExistException & )
        {
            // insertByName
            VOS_ENSURE( sal_False, "Content::storeData - insertByName failed!" );
            return sal_False;
        }
        catch ( WrappedTargetException & )
        {
            // insertByName
            VOS_ENSURE( sal_False, "Content::storeData - insertByName failed!" );
            return sal_False;
        }
        catch ( Exception & )
        {
            // createInstanceWithArguments
            VOS_ENSURE( sal_False, "Content::storeData - Error!" );
            return sal_False;
        }
    }

    try
    {
        Any aEntry = xNA->getByHierarchicalName( m_aUri.getPath() );
        Reference< XPropertySet > xPropSet;
        aEntry >>= xPropSet;

        if ( !xPropSet.is() )
        {
            VOS_ENSURE( sal_False,
                        "Content::storeData - Got no XPropertySet interface!" );
            return sal_False;
        }

        //////////////////////////////////////////////////////////////////
        // Store property values...
        //////////////////////////////////////////////////////////////////

        xPropSet->setPropertyValue( OUString::createFromAscii( "MediaType" ),
                                    makeAny( m_aProps.aMediaType ) );

        //////////////////////////////////////////////////////////////////
        // Store data stream...
        //////////////////////////////////////////////////////////////////

        if ( xStream.is() && !isFolder() )
        {
            Reference< XActiveDataSink > xSink( xPropSet, UNO_QUERY );

            if ( !xSink.is() )
            {
                VOS_ENSURE( sal_False,
                            "Content::storeData - "
                            "Got no XActiveDataSink interface!" );
                return sal_False;
            }

            xSink->setInputStream( xStream );

            // flushData();
        }

        return sal_True;
    }
    catch ( NoSuchElementException & )
    {
        // getByHierarchicalName
    }
    catch ( UnknownPropertyException & )
    {
        // setPropertyValue
    }
    catch ( PropertyVetoException & )
    {
        // setPropertyValue
    }
    catch ( IllegalArgumentException & )
    {
        // setPropertyValue
    }
    catch ( WrappedTargetException & )
    {
        // setPropertyValue
    }

    VOS_ENSURE( sal_False, "Content::storeData - Error!" );
    return sal_False;
}

//=========================================================================
sal_Bool Content::removeData()
{
    Reference< XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return sal_False;

    try
    {
        PackageUri aParentUri( getParentURL() );
        Any aEntry = xNA->getByHierarchicalName( aParentUri.getPath() );
        Reference< XNameContainer > xContainer;
        aEntry >>= xContainer;

        if ( !xContainer.is() )
        {
            VOS_ENSURE( sal_False,
                        "Content::removeData - "
                        "Got no XNameContainer interface!" );
            return sal_False;
        }

        xContainer->removeByName( m_aUri.getName() );
        return sal_True;
    }
    catch ( NoSuchElementException & )
    {
        // getByHierarchicalName, removeByName
    }
    catch ( WrappedTargetException & )
    {
        // removeByName
    }

    VOS_ENSURE( sal_False, "Content::removeData - Error!" );
    return sal_False;
}

//=========================================================================
sal_Bool Content::flushData()
{
    // Note: XChangesBatch is only implemented by the package itself, not
    //       by the single entries. Maybe this has to change...

    Reference< XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return sal_False;

    Reference< XChangesBatch > xBatch( xNA, UNO_QUERY );
    if ( !xBatch.is() )
    {
        VOS_ENSURE( sal_False,
                    "Content::flushData - Got no XChangesBatch interface!" );
        return sal_False;
    }

    try
    {
        xBatch->commitChanges();
        return sal_True;
    }
    catch ( WrappedTargetException & )
    {
    }

    VOS_ENSURE( sal_False, "Content::flushData - Error!" );
    return sal_False;
}

//=========================================================================
Reference< XInputStream > Content::getInputStream()
{
    Reference< XInputStream > xStream;
    Reference< XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return xStream;

    try
    {
        Any aEntry = xNA->getByHierarchicalName( m_aUri.getPath() );
        Reference< XActiveDataSink > xSink;
        aEntry >>= xSink;

        if ( !xSink.is() )
        {
            VOS_ENSURE( sal_False,
                        "Content::getInputStream - "
                        "Got no XActiveDataSink interface!" );
            return xStream;
        }

        xStream = xSink->getInputStream();

        VOS_ENSURE( xStream.is(),
                    "Content::getInputStream - Got no stream!" );
    }
    catch ( NoSuchElementException & )
    {
        // getByHierarchicalName
    }

    return xStream;
}

//=========================================================================
Reference< XEnumeration > Content::getIterator()
{
    Reference< XEnumeration > xIter;
    Reference< XHierarchicalNameAccess > xNA = getPackage();
    if ( !xNA.is() )
        return xIter;

    try
    {
        Any aEntry = xNA->getByHierarchicalName( m_aUri.getPath() );
        Reference< XEnumerationAccess > xIterFac;
        aEntry >>= xIterFac;

        if ( !xIterFac.is() )
        {
            VOS_ENSURE( sal_False,
                        "Content::getIterator - "
                        "Got no XEnumerationAccess interface!" );
            return xIter;
        }

        xIter = xIterFac->createEnumeration();

        VOS_ENSURE( xIter.is(),
                    "Content::getIterator - Got no iterator!" );
    }
    catch ( NoSuchElementException & )
    {
        // getByHierarchicalName
    }

    return xIter;
}

