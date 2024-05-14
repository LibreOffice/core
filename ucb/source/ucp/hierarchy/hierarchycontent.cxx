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


/**************************************************************************
                                TODO
 **************************************************************************

 - optimize transfer command. "Move" should be implementable much more
   efficient!

 **************************************************************************

 - Root Folder vs. 'normal' Folder
    - root doesn't support command 'delete'
    - root doesn't support command 'insert'
    - root needs not created via XContentCreator - queryContent with root
      folder id ( HIERARCHY_ROOT_FOLDER_URL ) always returns a value != 0
    - root has no parent.

 *************************************************************************/
#include <osl/diagnose.h>

#include <rtl/ustring.hxx>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/macros.hxx>
#include <utility>
#include "hierarchycontent.hxx"
#include "hierarchyprovider.hxx"
#include "dynamicresultset.hxx"
#include "hierarchyuri.hxx"

#include "../inc/urihelper.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;


// HierarchyContent Implementation.


// static ( "virtual" ctor )
rtl::Reference<HierarchyContent> HierarchyContent::create(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    // Fail, if content does not exist.
    HierarchyContentProperties aProps;
    if ( !loadData( rxContext, pProvider, Identifier, aProps ) )
        return nullptr;

    return new HierarchyContent( rxContext, pProvider, Identifier, std::move(aProps) );
}


// static ( "virtual" ctor )
rtl::Reference<HierarchyContent> HierarchyContent::create(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            const ucb::ContentInfo& Info )
{
    if ( Info.Type.isEmpty() )
        return nullptr;

    if ( Info.Type != HIERARCHY_FOLDER_CONTENT_TYPE && Info.Type != HIERARCHY_LINK_CONTENT_TYPE )
        return nullptr;

    return new HierarchyContent( rxContext, pProvider, Identifier, Info );
}


HierarchyContent::HierarchyContent(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties aProps )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_aProps(std::move( aProps )),
  m_eState( PERSISTENT ),
  m_pProvider( pProvider ),
  m_bCheckedReadOnly( false ),
  m_bIsReadOnly( true )
{
    setKind( Identifier );
}


HierarchyContent::HierarchyContent(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            const ucb::ContentInfo& Info )
  : ContentImplHelper( rxContext, pProvider, Identifier ),
  m_aProps( Info.Type == HIERARCHY_FOLDER_CONTENT_TYPE ? HierarchyEntryData::FOLDER : HierarchyEntryData::LINK ),
  m_eState( TRANSIENT ),
  m_pProvider( pProvider ),
  m_bCheckedReadOnly( false ),
  m_bIsReadOnly( true )
{
    setKind( Identifier );
}


// virtual
HierarchyContent::~HierarchyContent()
{
}


// XInterface methods.


// virtual
void SAL_CALL HierarchyContent::acquire()
    noexcept
{
    ContentImplHelper::acquire();
}


// virtual
void SAL_CALL HierarchyContent::release()
    noexcept
{
    ContentImplHelper::release();
}


// virtual
uno::Any SAL_CALL HierarchyContent::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = ContentImplHelper::queryInterface( rType );

    if ( !aRet.hasValue() )
    {
        // Note: isReadOnly may be relative expensive. So avoid calling it
        //       unless it is really necessary.
        aRet = cppu::queryInterface(
                rType, static_cast< ucb::XContentCreator * >( this ) );
        if ( aRet.hasValue() )
        {
            if ( !isFolder() || isReadOnly() )
                return uno::Any();
        }
    }

    return aRet;
}


// XTypeProvider methods.


XTYPEPROVIDER_COMMON_IMPL( HierarchyContent );


// virtual
uno::Sequence< uno::Type > SAL_CALL HierarchyContent::getTypes()
{
    if ( isFolder() && !isReadOnly() )
    {
        static cppu::OTypeCollection s_aFolderTypes(
                    CPPU_TYPE_REF( lang::XTypeProvider ),
                    CPPU_TYPE_REF( lang::XServiceInfo ),
                    CPPU_TYPE_REF( lang::XComponent ),
                    CPPU_TYPE_REF( ucb::XContent ),
                    CPPU_TYPE_REF( ucb::XCommandProcessor ),
                    CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                    CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                    CPPU_TYPE_REF( beans::XPropertyContainer ),
                    CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                    CPPU_TYPE_REF( container::XChild ),
                    CPPU_TYPE_REF( ucb::XContentCreator ) );


        return s_aFolderTypes.getTypes();
    }
    else
    {
        static cppu::OTypeCollection s_aDocumentTypes(
                    CPPU_TYPE_REF( lang::XTypeProvider ),
                    CPPU_TYPE_REF( lang::XServiceInfo ),
                    CPPU_TYPE_REF( lang::XComponent ),
                    CPPU_TYPE_REF( ucb::XContent ),
                    CPPU_TYPE_REF( ucb::XCommandProcessor ),
                    CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                    CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                    CPPU_TYPE_REF( beans::XPropertyContainer ),
                    CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                    CPPU_TYPE_REF( container::XChild ) );

        return s_aDocumentTypes.getTypes();
    }
}


// XServiceInfo methods.


// virtual
OUString SAL_CALL HierarchyContent::getImplementationName()
{
    return u"com.sun.star.comp.ucb.HierarchyContent"_ustr;
}


// virtual
uno::Sequence< OUString > SAL_CALL
HierarchyContent::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSNS( 1 );

    if ( m_eKind == LINK )
        aSNS.getArray()[ 0 ] = "com.sun.star.ucb.HierarchyLinkContent";
    else if ( m_eKind == FOLDER )
        aSNS.getArray()[ 0 ] = "com.sun.star.ucb.HierarchyFolderContent";
    else
        aSNS.getArray()[ 0 ] = "com.sun.star.ucb.HierarchyRootFolderContent";

    return aSNS;
}


// XContent methods.


// virtual
OUString SAL_CALL HierarchyContent::getContentType()
{
    return m_aProps.getContentType();
}


// virtual
uno::Reference< ucb::XContentIdentifier > SAL_CALL
HierarchyContent::getIdentifier()
{
    // Transient?
    if ( m_eState == TRANSIENT )
    {
        // Transient contents have no identifier.
        return uno::Reference< ucb::XContentIdentifier >();
    }

    return ContentImplHelper::getIdentifier();
}


// XCommandProcessor methods.


// virtual
uno::Any SAL_CALL HierarchyContent::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {

        // getPropertyValues


        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {

        // setPropertyValues


        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.hasElements() )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"No properties!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {

        // getPropertySetInfo


        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {

        // getCommandInfo


        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name == "open" && isFolder() )
    {

        // open command for a folder content


        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet( m_xContext, this, aOpenCommand );
        aRet <<= xSet;
    }
    else if ( aCommand.Name == "insert" && ( m_eKind != ROOT ) && !isReadOnly() )
    {

        // insert
        //  ( Not available at root folder )


        ucb::InsertCommandArgument aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        sal_Int32 nNameClash = aArg.ReplaceExisting
                             ? ucb::NameClash::OVERWRITE
                             : ucb::NameClash::ERROR;
        insert( nNameClash, Environment );
    }
    else if ( aCommand.Name == "delete" && ( m_eKind != ROOT ) && !isReadOnly() )
    {

        // delete
        //  ( Not available at root folder )


        bool bDeletePhysical = false;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical, Environment );

        // Remove own and all children's persistent data.
        if ( !removeData() )
        {
            uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
            {
                {"Uri", uno::Any(m_xIdentifier->getContentIdentifier())}
            }));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_WRITE,
                aArgs,
                Environment,
                u"Cannot remove persistent data!"_ustr,
                this );
            // Unreachable
        }

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet();
    }
    else if ( aCommand.Name == "transfer" && isFolder() && !isReadOnly() )
    {

        // transfer
        //      ( Not available at link objects )


        ucb::TransferInfo aInfo;
        if ( !( aCommand.Argument >>= aInfo ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        transfer( aInfo, Environment );
    }
    else if ( aCommand.Name == "createNewContent" && isFolder() && !isReadOnly() )
    {

        // createNewContent
        //      ( Not available at link objects )


        ucb::ContentInfo aInfo;
        if ( !( aCommand.Argument >>= aInfo ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= createNewContent( aInfo );
    }
    else
    {

        // Unsupported command


        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                                OUString(),
                                getXWeak() ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}


// virtual
void SAL_CALL HierarchyContent::abort( sal_Int32 /*CommandId*/ )
{
    // @@@ Generally, no action takes much time...
}


// XContentCreator methods.


// virtual
uno::Sequence< ucb::ContentInfo > SAL_CALL
HierarchyContent::queryCreatableContentsInfo()
{
    return m_aProps.getCreatableContentsInfo();
}


// virtual
uno::Reference< ucb::XContent > SAL_CALL
HierarchyContent::createNewContent( const ucb::ContentInfo& Info )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( Info.Type.isEmpty() )
            return uno::Reference< ucb::XContent >();

        bool bCreateFolder = Info.Type == HIERARCHY_FOLDER_CONTENT_TYPE;

        if ( !bCreateFolder && Info.Type != HIERARCHY_LINK_CONTENT_TYPE )
            return uno::Reference< ucb::XContent >();

        OUString aURL = m_xIdentifier->getContentIdentifier();

        OSL_ENSURE( !aURL.isEmpty(),
                    "HierarchyContent::createNewContent - empty identifier!" );

        if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
            aURL += "/";

        if ( bCreateFolder )
            aURL += "New_Folder";
        else
            aURL += "New_Link";

        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aURL );

        return create( m_xContext, m_pProvider, xId, Info );
    }
    else
    {
        OSL_FAIL( "createNewContent called on non-folder object!" );
        return uno::Reference< ucb::XContent >();
    }
}


// virtual
OUString HierarchyContent::getParentURL()
{
    HierarchyUri aUri( m_xIdentifier->getContentIdentifier() );
    return aUri.getParentUri();
}


//static
bool HierarchyContent::hasData(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    OUString aURL = Identifier->getContentIdentifier();

    // Am I a root folder?
    HierarchyUri aUri( aURL );
    if ( aUri.isRootFolder() )
    {
        // hasData must always return 'true' for root folder
        // even if no persistent data exist!!!
        return true;
    }

    return HierarchyEntry( rxContext, pProvider, aURL ).hasData();
}


//static
bool HierarchyContent::loadData(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties& rProps )
{
    OUString aURL = Identifier->getContentIdentifier();

    // Am I a root folder?
    HierarchyUri aUri( aURL );
    if ( aUri.isRootFolder() )
    {
        rProps = HierarchyContentProperties( HierarchyEntryData::FOLDER );
    }
    else
    {
        HierarchyEntry aEntry( rxContext, pProvider, aURL );
        HierarchyEntryData aData;
        if ( !aEntry.getData( aData ) )
            return false;

        rProps = HierarchyContentProperties( aData );
    }
    return true;
}


bool HierarchyContent::storeData()
{
    HierarchyEntry aEntry(
            m_xContext, m_pProvider, m_xIdentifier->getContentIdentifier() );
    return aEntry.setData( m_aProps.getHierarchyEntryData() );
}


void HierarchyContent::renameData(
            const uno::Reference< ucb::XContentIdentifier >& xOldId,
            const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    HierarchyEntry aEntry(
            m_xContext, m_pProvider, xOldId->getContentIdentifier() );
    aEntry.move( xNewId->getContentIdentifier(),
                        m_aProps.getHierarchyEntryData() );
}


bool HierarchyContent::removeData()
{
    HierarchyEntry aEntry(
        m_xContext, m_pProvider, m_xIdentifier->getContentIdentifier() );
    return aEntry.remove();
}


void HierarchyContent::setKind(
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    if ( m_aProps.getIsFolder() )
    {
        // Am I a root folder?
        HierarchyUri aUri( Identifier->getContentIdentifier() );
        if ( aUri.isRootFolder() )
            m_eKind = ROOT;
        else
            m_eKind = FOLDER;
    }
    else
        m_eKind = LINK;
}


bool HierarchyContent::isReadOnly()
{
    if ( !m_bCheckedReadOnly )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if ( !m_bCheckedReadOnly )
        {
            m_bCheckedReadOnly = true;
            m_bIsReadOnly      = true;

            HierarchyUri aUri( m_xIdentifier->getContentIdentifier() );
            uno::Reference< lang::XMultiServiceFactory > xConfigProv
                = m_pProvider->getConfigProvider( aUri.getService() );
            if ( xConfigProv.is() )
            {
                uno::Sequence< OUString > aNames
                    = xConfigProv->getAvailableServiceNames();
                m_bIsReadOnly = comphelper::findValue(aNames, "com.sun.star.ucb.HierarchyDataReadWriteAccess") == -1;
            }
        }
    }

    return m_bIsReadOnly;
}


uno::Reference< ucb::XContentIdentifier >
HierarchyContent::makeNewIdentifier( const OUString& rTitle )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Assemble new content identifier...
    HierarchyUri aUri( m_xIdentifier->getContentIdentifier() );
    OUString aNewURL = aUri.getParentUri() + "/" +
        ::ucb_impl::urihelper::encodeSegment( rTitle );

    return uno::Reference< ucb::XContentIdentifier >(
        new ::ucbhelper::ContentIdentifier( aNewURL ) );
}


void HierarchyContent::queryChildren( HierarchyContentRefVector& rChildren )
{
    if ( ( m_eKind != FOLDER ) && ( m_eKind != ROOT ) )
        return;

    // Obtain a list with a snapshot of all currently instantiated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += "/";
    }

    sal_Int32 nLen = aURL.getLength();

    for ( const auto& rContent : aAllContents )
    {
        ::ucbhelper::ContentImplHelperRef xChild = rContent;
        OUString aChildURL
            = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.startsWith( aURL ) ) )
        {
            sal_Int32 nPos = aChildURL.indexOf( '/', nLen );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes/ only a final slash. It's a child!
                rChildren.emplace_back(
                        static_cast< HierarchyContent * >( xChild.get() ) );
            }
        }
    }
}


bool HierarchyContent::exchangeIdentity(
            const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return false;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    // Already persistent?
    if ( m_eState != PERSISTENT )
    {
        OSL_FAIL( "HierarchyContent::exchangeIdentity - Not persistent!" );
        return false;
    }

    // Am I the root folder?
    if ( m_eKind == ROOT )
    {
        OSL_FAIL( "HierarchyContent::exchangeIdentity - "
                               "Not supported by root folder!" );
        return false;
    }

    // Exchange own identity.

    // Fail, if a content with given id already exists.
    if ( !hasData( xNewId ) )
    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            if ( m_eKind == FOLDER )
            {
                // Process instantiated children...

                HierarchyContentRefVector aChildren;
                queryChildren( aChildren );

                for ( const auto& rChild : aChildren )
                {
                    HierarchyContentRef xChild = rChild;

                    // Create new content identifier for the child...
                    uno::Reference< ucb::XContentIdentifier > xOldChildId
                                                    = xChild->getIdentifier();
                    OUString aOldChildURL
                        = xOldChildId->getContentIdentifier();
                    OUString aNewChildURL
                        = aOldChildURL.replaceAt(
                                        0,
                                        aOldURL.getLength(),
                                        xNewId->getContentIdentifier() );
                    uno::Reference< ucb::XContentIdentifier > xNewChildId
                        = new ::ucbhelper::ContentIdentifier( aNewChildURL );

                    if ( !xChild->exchangeIdentity( xNewChildId ) )
                        return false;
                }
            }
            return true;
        }
    }

    OSL_FAIL( "HierarchyContent::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return false;
}


// static
uno::Reference< sdbc::XRow > HierarchyContent::getPropertyValues(
                const uno::Reference< uno::XComponentContext >& rxContext,
                const uno::Sequence< beans::Property >& rProperties,
                const HierarchyContentProperties& rData,
                HierarchyContentProvider* pProvider,
                const OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
        = new ::ucbhelper::PropertyValueSet( rxContext );

    if ( rProperties.hasElements() )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        bool bTriedToGetAdditionalPropSet = false;

        for ( const beans::Property& rProp : rProperties )
        {
            // Process Core properties.

            if ( rProp.Name == "ContentType" )
            {
                xRow->appendString ( rProp, rData.getContentType() );
            }
            else if ( rProp.Name == "Title" )
            {
                xRow->appendString ( rProp, rData.getTitle() );
            }
            else if ( rProp.Name == "IsDocument" )
            {
                xRow->appendBoolean( rProp, rData.getIsDocument() );
            }
            else if ( rProp.Name == "IsFolder" )
            {
                xRow->appendBoolean( rProp, rData.getIsFolder() );
            }
            else if ( rProp.Name == "CreatableContentsInfo" )
            {
                xRow->appendObject(
                    rProp, uno::Any( rData.getCreatableContentsInfo() ) );
            }
            else if ( rProp.Name == "TargetURL" )
            {
                // TargetURL is only supported by links.

                if ( rData.getIsDocument() )
                    xRow->appendString( rProp, rData.getTargetURL() );
                else
                    xRow->appendVoid( rProp );
            }
            else
            {
                // Not a Core Property! Maybe it's an Additional Core Property?!

                if ( !bTriedToGetAdditionalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet =
                            pProvider->getAdditionalPropertySet( rContentId,
                                                                 false );
                    bTriedToGetAdditionalPropSet = true;
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
            beans::Property( u"ContentType"_ustr,
                      -1,
                      cppu::UnoType<OUString>::get(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getContentType() );
        xRow->appendString (
            beans::Property( u"Title"_ustr,
                      -1,
                      cppu::UnoType<OUString>::get(),
                          // @@@ Might actually be read-only!
                      beans::PropertyAttribute::BOUND ),
            rData.getTitle() );
        xRow->appendBoolean(
            beans::Property( u"IsDocument"_ustr,
                      -1,
                      cppu::UnoType<bool>::get(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getIsDocument() );
        xRow->appendBoolean(
            beans::Property( u"IsFolder"_ustr,
                      -1,
                      cppu::UnoType<bool>::get(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getIsFolder() );

        if ( rData.getIsDocument() )
            xRow->appendString(
                beans::Property( u"TargetURL"_ustr,
                          -1,
                          cppu::UnoType<OUString>::get(),
                          // @@@ Might actually be read-only!
                          beans::PropertyAttribute::BOUND ),
                rData.getTargetURL() );
        xRow->appendObject(
            beans::Property(
                u"CreatableContentsInfo"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY ),
            uno::Any( rData.getCreatableContentsInfo() ) );

        // Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet =
            pProvider->getAdditionalPropertySet( rContentId, false );
        xRow->appendPropertySet( xSet );
    }

    return xRow;
}


uno::Reference< sdbc::XRow > HierarchyContent::getPropertyValues(
                        const uno::Sequence< beans::Property >& rProperties )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xContext,
                              rProperties,
                              m_aProps,
                              m_pProvider,
                              m_xIdentifier->getContentIdentifier() );
}


uno::Sequence< uno::Any > HierarchyContent::setPropertyValues(
        const uno::Sequence< beans::PropertyValue >& rValues,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    osl::ResettableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    auto aRetRange = asNonConstRange(aRet);
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = getXWeak();
    aEvent.Further        = false;
//    aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//    aEvent.OldValue       =
//    aEvent.NewValue       =

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
    bool bTriedToGetAdditionalPropSet = false;

    bool bExchange = false;
    OUString aOldTitle;
    OUString aOldName;
    sal_Int32 nTitlePos = -1;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name == "ContentType" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                            u"Property is read-only!"_ustr,
                            getXWeak() );
        }
        else if ( rValue.Name == "IsDocument" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                            u"Property is read-only!"_ustr,
                            getXWeak() );
        }
        else if ( rValue.Name == "IsFolder" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                            u"Property is read-only!"_ustr,
                            getXWeak() );
        }
        else if ( rValue.Name == "CreatableContentsInfo" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                            u"Property is read-only!"_ustr,
                            getXWeak() );
        }
        else if ( rValue.Name == "Title" )
        {
            if ( isReadOnly() )
            {
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
            }
            else
            {
                OUString aNewValue;
                if ( rValue.Value >>= aNewValue )
                {
                    // No empty titles!
                    if ( !aNewValue.isEmpty() )
                    {
                        if ( aNewValue != m_aProps.getTitle() )
                        {
                            // modified title -> modified URL -> exchange !
                            if ( m_eState == PERSISTENT )
                                bExchange = true;

                            aOldTitle = m_aProps.getTitle();
                            aOldName  = m_aProps.getName();

                            m_aProps.setTitle( aNewValue );
                            m_aProps.setName(
                                ::ucb_impl::urihelper::encodeSegment(
                                    aNewValue ) );

                            // property change event will be set later...

                            // remember position within sequence of values
                            // (for error handling).
                            nTitlePos = n;
                        }
                    }
                    else
                    {
                        aRetRange[ n ] <<= lang::IllegalArgumentException(
                                    u"Empty title not allowed!"_ustr,
                                    getXWeak(),
                                    -1 );
                    }
                }
                else
                {
                    aRetRange[ n ] <<= beans::IllegalTypeException(
                                u"Property value has wrong type!"_ustr,
                                getXWeak() );
                }
            }
        }
        else if ( rValue.Name == "TargetURL" )
        {
            if ( isReadOnly() )
            {
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
            }
            else
            {
                // TargetURL is only supported by links.

                if ( m_eKind == LINK )
                {
                    OUString aNewValue;
                    if ( rValue.Value >>= aNewValue )
                    {
                        // No empty target URL's!
                        if ( !aNewValue.isEmpty() )
                        {
                            if ( aNewValue != m_aProps.getTargetURL() )
                            {
                                aEvent.PropertyName = rValue.Name;
                                aEvent.OldValue <<= m_aProps.getTargetURL();
                                aEvent.NewValue <<= aNewValue;

                                aChanges.getArray()[ nChanged ] = aEvent;

                                m_aProps.setTargetURL( aNewValue );
                                nChanged++;
                            }
                        }
                        else
                        {
                            aRetRange[ n ] <<= lang::IllegalArgumentException(
                                    u"Empty target URL not allowed!"_ustr,
                                    getXWeak(),
                                    -1 );
                        }
                    }
                    else
                    {
                        aRetRange[ n ] <<= beans::IllegalTypeException(
                                u"Property value has wrong type!"_ustr,
                                getXWeak() );
                    }
                }
                else
                {
                    aRetRange[ n ] <<= beans::UnknownPropertyException(
                                u"TargetURL only supported by links!"_ustr,
                                getXWeak() );
                }
            }
        }
        else
        {
            // Not a Core Property! Maybe it's an Additional Core Property?!

            if ( !bTriedToGetAdditionalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( false );
                bTriedToGetAdditionalPropSet = true;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    uno::Any aOldValue = xAdditionalPropSet->getPropertyValue(
                                                                rValue.Name );
                    if ( aOldValue != rValue.Value )
                    {
                        xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = aOldValue;
                        aEvent.NewValue     = rValue.Value;

                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                }
                catch ( beans::UnknownPropertyException const & e )
                {
                    aRetRange[ n ] <<= e;
                }
                catch ( lang::WrappedTargetException const & e )
                {
                    aRetRange[ n ] <<= e;
                }
                catch ( beans::PropertyVetoException const & e )
                {
                    aRetRange[ n ] <<= e;
                }
                catch ( lang::IllegalArgumentException const & e )
                {
                    aRetRange[ n ] <<= e;
                }
            }
            else
            {
                aRetRange[ n ] <<= uno::Exception(
                                u"No property set for storing the value!"_ustr,
                                getXWeak() );
            }
        }
    }

    if ( bExchange )
    {
        uno::Reference< ucb::XContentIdentifier > xOldId
            = m_xIdentifier;
        uno::Reference< ucb::XContentIdentifier > xNewId
            = makeNewIdentifier( m_aProps.getTitle() );

        aGuard.clear();
        if ( exchangeIdentity( xNewId ) )
        {
            // Adapt persistent data.
            renameData( xOldId, xNewId );

            // Adapt Additional Core Properties.
            renameAdditionalPropertySet( xOldId->getContentIdentifier(),
                                         xNewId->getContentIdentifier() );
        }
        else
        {
            // Roll-back.
            m_aProps.setTitle( aOldTitle );
            m_aProps.setName ( aOldName );

            aOldTitle.clear();
            aOldName.clear();

            // Set error .
            aRetRange[ nTitlePos ] <<= uno::Exception(
                    u"Exchange failed!"_ustr,
                    getXWeak() );
        }
        aGuard.reset();
    }

    if ( !aOldTitle.isEmpty() )
    {
        aEvent.PropertyName = "Title";
        aEvent.OldValue     <<= aOldTitle;
        aEvent.NewValue     <<= m_aProps.getTitle();

        aChanges.getArray()[ nChanged ] = aEvent;
        nChanged++;
    }

    if ( nChanged > 0 )
    {
        // Save changes, if content was already made persistent.
        if ( !bExchange && ( m_eState == PERSISTENT ) )
        {
            if ( !storeData() )
            {
                uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
                {
                    {"Uri", uno::Any(m_xIdentifier->getContentIdentifier())}
                }));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_WRITE,
                    aArgs,
                    xEnv,
                    u"Cannot store persistent data!"_ustr,
                    this );
                // Unreachable
            }
        }

        aChanges.realloc( nChanged );

        aGuard.clear();
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}


void HierarchyContent::insert( sal_Int32 nNameClashResolve,
                               const uno::Reference<
                                    ucb::XCommandEnvironment > & xEnv )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Am I the root folder?
    if ( m_eKind == ROOT )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                                u"Not supported by root folder!"_ustr,
                                getXWeak() ) ),
            xEnv );
        // Unreachable
    }

    // Check, if all required properties were set.
    if ( m_aProps.getTitle().isEmpty() )
    {
        uno::Sequence<OUString> aProps { u"Title"_ustr };
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::MissingPropertiesException(
                                OUString(),
                                getXWeak(),
                                aProps ) ),
            xEnv );
        // Unreachable
    }

    // Assemble new content identifier...

    uno::Reference< ucb::XContentIdentifier > xId
        = makeNewIdentifier( m_aProps.getTitle() );

    // Handle possible name clash...

    switch ( nNameClashResolve )
    {
        // fail.
        case ucb::NameClash::ERROR:
            if ( hasData( xId ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::Any(
                        ucb::NameClashException(
                            OUString(),
                            getXWeak(),
                            task::InteractionClassification_ERROR,
                            m_aProps.getTitle() ) ),
                    xEnv );
                // Unreachable
            }
            break;

        // replace existing object.
        case ucb::NameClash::OVERWRITE:
            break;

        // "invent" a new valid title.
        case ucb::NameClash::RENAME:
            if ( hasData( xId ) )
            {
                sal_Int32 nTry = 0;

                do
                {
                    OUString aNewId = xId->getContentIdentifier() + "_" + OUString::number( ++nTry );
                    xId = new ::ucbhelper::ContentIdentifier( aNewId );
                }
                while ( hasData( xId ) && ( nTry < 1000 ) );

                if ( nTry == 1000 )
                {
                    ucbhelper::cancelCommandExecution(
                        uno::Any(
                            ucb::UnsupportedNameClashException(
                                u"Unable to resolve name clash!"_ustr,
                                getXWeak(),
                                nNameClashResolve ) ),
                    xEnv );
                // Unreachable
                }
                else
                {
                    OUString aNewTitle( m_aProps.getTitle() + "_" + OUString::number( nTry ) );
                    m_aProps.setTitle( aNewTitle );
                }
            }
            break;

        case ucb::NameClash::KEEP: // deprecated
        case ucb::NameClash::ASK:
        default:
            if ( hasData( xId ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::Any(
                        ucb::UnsupportedNameClashException(
                                OUString(),
                                getXWeak(),
                                nNameClashResolve ) ),
                    xEnv );
                // Unreachable
            }
            break;
    }

    // Identifier changed?
    bool bNewId = ( xId->getContentIdentifier()
                            != m_xIdentifier->getContentIdentifier() );
    m_xIdentifier = xId;

    if ( !storeData() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(m_xIdentifier->getContentIdentifier())}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_WRITE,
            aArgs,
            xEnv,
            u"Cannot store persistent data!"_ustr,
            this );
        // Unreachable
    }

    m_eState = PERSISTENT;

    if ( bNewId )
    {
        aGuard.clear();
        inserted();
    }
}


void HierarchyContent::destroy( bool bDeletePhysical,
                                const uno::Reference<
                                    ucb::XCommandEnvironment > & xEnv )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                                u"Not persistent!"_ustr,
                                getXWeak() ) ),
            xEnv );
        // Unreachable
    }

    // Am I the root folder?
    if ( m_eKind == ROOT )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                                u"Not supported by root folder!"_ustr,
                                getXWeak() ) ),
            xEnv );
        // Unreachable
    }

    m_eState = DEAD;

    aGuard.clear();
    deleted();

    if ( m_eKind == FOLDER )
    {
        // Process instantiated children...

        HierarchyContentRefVector aChildren;
        queryChildren( aChildren );

        for ( auto & child : aChildren)
        {
            child->destroy( bDeletePhysical, xEnv );
        }
    }
}


void HierarchyContent::transfer(
            const ucb::TransferInfo& rInfo,
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                                u"Not persistent!"_ustr,
                                getXWeak() ) ),
            xEnv );
        // Unreachable
    }

    // Is source a hierarchy content?
    if ( !rInfo.SourceURL.startsWith( HIERARCHY_URL_SCHEME ":/" ) )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::InteractiveBadTransferURLException(
                            OUString(),
                            getXWeak() ) ),
            xEnv );
        // Unreachable
    }

    // Is source not a parent of me / not me?
    OUString aId = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aId.lastIndexOf( '/' );
    if ( nPos != ( aId.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aId += "/";
    }

    if ( rInfo.SourceURL.getLength() <= aId.getLength() )
    {
        if ( aId.startsWith( rInfo.SourceURL ) )
        {
            uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
            {
                {"Uri", uno::Any(rInfo.SourceURL)}
            }));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_RECURSIVE,
                aArgs,
                xEnv,
                u"Target is equal to or is a child of source!"_ustr,
                this );
            // Unreachable
        }
    }


    // 0) Obtain content object for source.


    uno::Reference< ucb::XContentIdentifier > xId
        = new ::ucbhelper::ContentIdentifier( rInfo.SourceURL );

    // Note: The static cast is okay here, because its sure that
    //       m_xProvider is always the HierarchyContentProvider.
    rtl::Reference< HierarchyContent > xSource;

    try
    {
        xSource = static_cast< HierarchyContent * >(
                        m_xProvider->queryContent( xId ).get() );
    }
    catch ( ucb::IllegalIdentifierException const & )
    {
        // queryContent
    }

    if ( !xSource.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(xId->getContentIdentifier())}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Cannot instantiate source object!"_ustr,
            this );
        // Unreachable
    }


    // 1) Create new child content.


    ucb::ContentInfo aContentInfo;
    aContentInfo.Type = xSource->isFolder()
        ? HIERARCHY_FOLDER_CONTENT_TYPE
        : HIERARCHY_LINK_CONTENT_TYPE;
    aContentInfo.Attributes = 0;

    // Note: The static cast is okay here, because its sure that
    //       createNewContent always creates a HierarchyContent.
    rtl::Reference< HierarchyContent > xTarget
        = static_cast< HierarchyContent * >(
            createNewContent( aContentInfo ).get() );
    if ( !xTarget.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Folder", uno::Any(aId)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            aArgs,
            xEnv,
            u"XContentCreator::createNewContent failed!"_ustr,
            this );
        // Unreachable
    }


    // 2) Copy data from source content to child content.


    uno::Sequence< beans::Property > aSourceProps
                    = xSource->getPropertySetInfo( xEnv )->getProperties();
    sal_Int32 nCount = aSourceProps.getLength();

    if ( nCount )
    {
        bool bHadTitle = rInfo.NewTitle.isEmpty();

        // Get all source values.
        uno::Reference< sdbc::XRow > xRow
            = xSource->getPropertyValues( aSourceProps );

        uno::Sequence< beans::PropertyValue > aValues( nCount );
        beans::PropertyValue* pValues = aValues.getArray();

        const beans::Property* pProps = aSourceProps.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp  = pProps[ n ];
            beans::PropertyValue&  rValue = pValues[ n ];

            rValue.Name   = rProp.Name;
            rValue.Handle = rProp.Handle;

            if ( !bHadTitle && rProp.Name == "Title" )
            {
                // Set new title instead of original.
                bHadTitle = true;
                rValue.Value <<= rInfo.NewTitle;
            }
            else
                rValue.Value = xRow->getObject(
                                n + 1,
                                uno::Reference< container::XNameAccess >() );

            rValue.State = beans::PropertyState_DIRECT_VALUE;

            if ( rProp.Attributes & beans::PropertyAttribute::REMOVABLE )
            {
                // Add Additional Core Property.
                try
                {
                    xTarget->addProperty( rProp.Name,
                                          rProp.Attributes,
                                          rValue.Value );
                }
                catch ( beans::PropertyExistException const & )
                {
                }
                catch ( beans::IllegalTypeException const & )
                {
                }
                catch ( lang::IllegalArgumentException const & )
                {
                }
            }
        }

        // Set target values.
        xTarget->setPropertyValues( aValues, xEnv );
    }


    // 3) Commit (insert) child.


    xTarget->insert( rInfo.NameClash, xEnv );


    // 4) Transfer (copy) children of source.


    if ( xSource->isFolder() )
    {
        HierarchyEntry aFolder(
            m_xContext, m_pProvider, xId->getContentIdentifier() );
        HierarchyEntry::iterator it;

        while ( aFolder.next( it ) )
        {
            const HierarchyEntryData& rResult = *it;

            OUString aChildId = xId->getContentIdentifier();
            if ( ( aChildId.lastIndexOf( '/' ) + 1 ) != aChildId.getLength() )
                aChildId += "/";

            aChildId += rResult.getName();

            ucb::TransferInfo aInfo;
            aInfo.MoveData  = false;
            aInfo.NewTitle.clear();
            aInfo.SourceURL = aChildId;
            aInfo.NameClash = rInfo.NameClash;

            // Transfer child to target.
            xTarget->transfer( aInfo, xEnv );
        }
    }


    // 5) Destroy source ( when moving only ) .


    if ( !rInfo.MoveData )
        return;

    xSource->destroy( true, xEnv );

    // Remove all persistent data of source and its children.
    if ( !xSource->removeData() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(xSource->m_xIdentifier->getContentIdentifier())}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_WRITE,
            aArgs,
            xEnv,
            u"Cannot remove persistent data of source object!"_ustr,
            this );
        // Unreachable
    }

    // Remove own and all children's Additional Core Properties.
    xSource->removeAdditionalPropertySet();
}


// HierarchyContentProperties Implementation.


uno::Sequence< ucb::ContentInfo >
HierarchyContentProperties::getCreatableContentsInfo() const
{
    if ( getIsFolder() )
    {
        uno::Sequence< ucb::ContentInfo > aSeq( 2 );

        // Folder.
        aSeq.getArray()[ 0 ].Type = HIERARCHY_FOLDER_CONTENT_TYPE;
        aSeq.getArray()[ 0 ].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;

        uno::Sequence< beans::Property > aFolderProps( 1 );
        aFolderProps.getArray()[ 0 ] = beans::Property(
                    u"Title"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND );
        aSeq.getArray()[ 0 ].Properties = aFolderProps;

        // Link.
        aSeq.getArray()[ 1 ].Type = HIERARCHY_LINK_CONTENT_TYPE;
        aSeq.getArray()[ 1 ].Attributes = ucb::ContentInfoAttribute::KIND_LINK;

        uno::Sequence< beans::Property > aLinkProps( 2 );
        aLinkProps.getArray()[ 0 ] = beans::Property(
                    u"Title"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND );
        aLinkProps.getArray()[ 1 ] = beans::Property(
                    u"TargetURL"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND );
        aSeq.getArray()[ 1 ].Properties = aLinkProps;

        return aSeq;
    }
    else
    {
        return uno::Sequence< ucb::ContentInfo >( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
