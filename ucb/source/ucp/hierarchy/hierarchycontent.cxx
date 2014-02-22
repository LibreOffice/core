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

#include "osl/doublecheckedlocking.h"
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include "hierarchycontent.hxx"
#include "hierarchyprovider.hxx"
#include "dynamicresultset.hxx"
#include "hierarchyuri.hxx"

#include "../inc/urihelper.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;



//

//




HierarchyContent* HierarchyContent::create(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    
    HierarchyContentProperties aProps;
    if ( !loadData( rxContext, pProvider, Identifier, aProps ) )
        return 0;

    return new HierarchyContent( rxContext, pProvider, Identifier, aProps );
}



HierarchyContent* HierarchyContent::create(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            const ucb::ContentInfo& Info )
{
    if ( Info.Type.isEmpty() )
        return 0;

    if ( Info.Type != HIERARCHY_FOLDER_CONTENT_TYPE && Info.Type != HIERARCHY_LINK_CONTENT_TYPE )
        return 0;

    return new HierarchyContent( rxContext, pProvider, Identifier, Info );
}


HierarchyContent::HierarchyContent(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            const HierarchyContentProperties& rProps )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_aProps( rProps ),
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



HierarchyContent::~HierarchyContent()
{
}


//

//



void SAL_CALL HierarchyContent::acquire()
    throw( )
{
    ContentImplHelper::acquire();
}



void SAL_CALL HierarchyContent::release()
    throw( )
{
    ContentImplHelper::release();
}



uno::Any SAL_CALL HierarchyContent::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet = ContentImplHelper::queryInterface( rType );

    if ( !aRet.hasValue() )
    {
        
        
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


//

//


XTYPEPROVIDER_COMMON_IMPL( HierarchyContent );



uno::Sequence< uno::Type > SAL_CALL HierarchyContent::getTypes()
    throw( uno::RuntimeException )
{
    cppu::OTypeCollection * pCollection = 0;

    if ( isFolder() && !isReadOnly() )
    {
        static cppu::OTypeCollection* pFolderTypes = 0;

        pCollection = pFolderTypes;
        if ( !pCollection )
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

            pCollection = pFolderTypes;
            if ( !pCollection )
            {
                static cppu::OTypeCollection aCollection(
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
                pCollection = &aCollection;
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                pFolderTypes = pCollection;
            }
        }
        else {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
    }
    else
    {
        static cppu::OTypeCollection* pDocumentTypes = 0;

        pCollection = pDocumentTypes;
        if ( !pCollection )
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

            pCollection = pDocumentTypes;
            if ( !pCollection )
            {
                static cppu::OTypeCollection aCollection(
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
                pCollection = &aCollection;
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                pDocumentTypes = pCollection;
            }
        }
        else {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
    }

    return (*pCollection).getTypes();
}


//

//



OUString SAL_CALL HierarchyContent::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.ucb.HierarchyContent" );
}



uno::Sequence< OUString > SAL_CALL
HierarchyContent::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );

    if ( m_eKind == LINK )
        aSNS.getArray()[ 0 ] = HIERARCHY_LINK_CONTENT_SERVICE_NAME;
    else if ( m_eKind == FOLDER )
        aSNS.getArray()[ 0 ] = HIERARCHY_FOLDER_CONTENT_SERVICE_NAME;
    else
        aSNS.getArray()[ 0 ] = HIERARCHY_ROOT_FOLDER_CONTENT_SERVICE_NAME;

    return aSNS;
}


//

//



OUString SAL_CALL HierarchyContent::getContentType()
    throw( uno::RuntimeException )
{
    return m_aProps.getContentType();
}



uno::Reference< ucb::XContentIdentifier > SAL_CALL
HierarchyContent::getIdentifier()
    throw( uno::RuntimeException )
{
    
    if ( m_eState == TRANSIENT )
    {
        
        return uno::Reference< ucb::XContentIdentifier >();
    }

    return ContentImplHelper::getIdentifier();
}


//

//



uno::Any SAL_CALL HierarchyContent::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        
        
        

        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        
        
        

        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "No properties!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {
        
        
        

        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {
        
        
        

        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name == "open" && isFolder() )
    {
        
        
        

        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet( m_xContext, this, aOpenCommand );
        aRet <<= xSet;
    }
    else if ( aCommand.Name == "insert" && ( m_eKind != ROOT ) && !isReadOnly() )
    {
        
        
        
        

        ucb::InsertCommandArgument aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        sal_Int32 nNameClash = aArg.ReplaceExisting
                             ? ucb::NameClash::OVERWRITE
                             : ucb::NameClash::ERROR;
        insert( nNameClash, Environment );
    }
    else if ( aCommand.Name == "delete" && ( m_eKind != ROOT ) && !isReadOnly() )
    {
        
        
        
        

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical, Environment );

        
        if ( !removeData() )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             OUString( "Uri"),
                             -1,
                             uno::makeAny(m_xIdentifier->
                                              getContentIdentifier()),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_WRITE,
                uno::Sequence< uno::Any >(&aProps, 1),
                Environment,
                OUString( "Cannot remove persistent data!" ),
                this );
            
        }

        
        removeAdditionalPropertySet( true );
    }
    else if ( aCommand.Name == "transfer" && isFolder() && !isReadOnly() )
    {
        
        
        
        

        ucb::TransferInfo aInfo;
        if ( !( aCommand.Argument >>= aInfo ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        transfer( aInfo, Environment );
    }
    else if ( aCommand.Name == "createNewContent" && isFolder() && !isReadOnly() )
    {
        
        
        
        

        ucb::ContentInfo aInfo;
        if ( !( aCommand.Argument >>= aInfo ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        aRet <<= createNewContent( aInfo );
    }
    else
    {
        
        
        

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        
    }

    return aRet;
}



void SAL_CALL HierarchyContent::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException )
{
    
}


//

//



uno::Sequence< ucb::ContentInfo > SAL_CALL
HierarchyContent::queryCreatableContentsInfo()
    throw( uno::RuntimeException )
{
    return m_aProps.getCreatableContentsInfo();
}



uno::Reference< ucb::XContent > SAL_CALL
HierarchyContent::createNewContent( const ucb::ContentInfo& Info )
    throw( uno::RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( Info.Type.isEmpty() )
            return uno::Reference< ucb::XContent >();

        sal_Bool bCreateFolder = Info.Type == HIERARCHY_FOLDER_CONTENT_TYPE;

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



OUString HierarchyContent::getParentURL()
{
    HierarchyUri aUri( m_xIdentifier->getContentIdentifier() );
    return aUri.getParentUri();
}



sal_Bool HierarchyContent::hasData(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    OUString aURL = Identifier->getContentIdentifier();

    
    HierarchyUri aUri( aURL );
    if ( aUri.isRootFolder() )
    {
        
        
        return sal_True;
    }

    return HierarchyEntry( rxContext, pProvider, aURL ).hasData();
}



sal_Bool HierarchyContent::loadData(
            const uno::Reference< uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties& rProps )
{
    OUString aURL = Identifier->getContentIdentifier();

    
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
            return sal_False;

        rProps = HierarchyContentProperties( aData );
    }
    return sal_True;
}


sal_Bool HierarchyContent::storeData()
{
    HierarchyEntry aEntry(
            m_xContext, m_pProvider, m_xIdentifier->getContentIdentifier() );
    return aEntry.setData( m_aProps.getHierarchyEntryData(), sal_True );
}


sal_Bool HierarchyContent::renameData(
            const uno::Reference< ucb::XContentIdentifier >& xOldId,
            const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    HierarchyEntry aEntry(
            m_xContext, m_pProvider, xOldId->getContentIdentifier() );
    return aEntry.move( xNewId->getContentIdentifier(),
                        m_aProps.getHierarchyEntryData() );
}


sal_Bool HierarchyContent::removeData()
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
                sal_Int32 nCount = aNames.getLength();
                for ( sal_Int32 n = 0; n < nCount; ++n )
                {
                    if ( aNames[ n ] == "com.sun.star.ucb.HierarchyDataReadWriteAccess" )
                    {
                        m_bIsReadOnly = false;
                        break;
                    }
                }
            }
        }
    }

    return m_bIsReadOnly;
}


uno::Reference< ucb::XContentIdentifier >
HierarchyContent::makeNewIdentifier( const OUString& rTitle )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    
    HierarchyUri aUri( m_xIdentifier->getContentIdentifier() );
    OUString aNewURL = aUri.getParentUri();
    aNewURL += "/";
    aNewURL += ::ucb_impl::urihelper::encodeSegment( rTitle );

    return uno::Reference< ucb::XContentIdentifier >(
        new ::ucbhelper::ContentIdentifier( aNewURL ) );
}


void HierarchyContent::queryChildren( HierarchyContentRefList& rChildren )
{
    if ( ( m_eKind != FOLDER ) && ( m_eKind != ROOT ) )
        return;

    
    
    

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
    {
        
        aURL += "/";
    }

    sal_Int32 nLen = aURL.getLength();

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucbhelper::ContentImplHelperRef xChild = (*it);
        OUString aChildURL
            = xChild->getIdentifier()->getContentIdentifier();

        
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                
                rChildren.push_back(
                    HierarchyContentRef(
                        static_cast< HierarchyContent * >( xChild.get() ) ) );
            }
        }
        ++it;
    }
}


sal_Bool HierarchyContent::exchangeIdentity(
            const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    
    if ( m_eState != PERSISTENT )
    {
        OSL_FAIL( "HierarchyContent::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    
    if ( m_eKind == ROOT )
    {
        OSL_FAIL( "HierarchyContent::exchangeIdentity - "
                               "Not supported by root folder!" );
        return sal_False;
    }

    

    
    if ( !hasData( xNewId ) )
    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            if ( m_eKind == FOLDER )
            {
                

                HierarchyContentRefList aChildren;
                queryChildren( aChildren );

                HierarchyContentRefList::const_iterator it  = aChildren.begin();
                HierarchyContentRefList::const_iterator end = aChildren.end();

                while ( it != end )
                {
                    HierarchyContentRef xChild = (*it);

                    
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
                        return sal_False;

                    ++it;
                }
            }
            return sal_True;
        }
    }

    OSL_FAIL( "HierarchyContent::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return sal_False;
}



uno::Reference< sdbc::XRow > HierarchyContent::getPropertyValues(
                const uno::Reference< uno::XComponentContext >& rxContext,
                const uno::Sequence< beans::Property >& rProperties,
                const HierarchyContentProperties& rData,
                HierarchyContentProvider* pProvider,
                const OUString& rContentId )
{
    

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
        = new ::ucbhelper::PropertyValueSet( rxContext );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditionalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            

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
                    rProp, uno::makeAny( rData.getCreatableContentsInfo() ) );
            }
            else if ( rProp.Name == "TargetURL" )
            {
                

                if ( rData.getIsDocument() )
                    xRow->appendString( rProp, rData.getTargetURL() );
                else
                    xRow->appendVoid( rProp );
            }
            else
            {
                

                if ( !bTriedToGetAdditionalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
                            pProvider->getAdditionalPropertySet( rContentId,
                                                                 false ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditionalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    if ( !xRow->appendPropertySetValue(
                                                xAdditionalPropSet,
                                                rProp ) )
                    {
                        
                        xRow->appendVoid( rProp );
                    }
                }
                else
                {
                    
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        
        xRow->appendString (
            beans::Property( OUString("ContentType"),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getContentType() );
        xRow->appendString (
            beans::Property( OUString("Title"),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                          
                      beans::PropertyAttribute::BOUND ),
            rData.getTitle() );
        xRow->appendBoolean(
            beans::Property( OUString("IsDocument"),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getIsDocument() );
        xRow->appendBoolean(
            beans::Property( OUString("IsFolder"),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.getIsFolder() );

        if ( rData.getIsDocument() )
            xRow->appendString(
                beans::Property( OUString("TargetURL"),
                          -1,
                          getCppuType(
                            static_cast< const OUString * >( 0 ) ),
                          
                          beans::PropertyAttribute::BOUND ),
                rData.getTargetURL() );
        xRow->appendObject(
            beans::Property(
                OUString("CreatableContentsInfo"),
                -1,
                getCppuType( static_cast<
                        const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY ),
            uno::makeAny( rData.getCreatableContentsInfo() ) );

        

        uno::Reference< beans::XPropertySet > xSet(
            pProvider->getAdditionalPropertySet( rContentId, false ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
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
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;

    aEvent.PropertyHandle = -1;



    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditionalPropSet = sal_False;

    sal_Bool bExchange = sal_False;
    OUString aOldTitle;
    OUString aOldName;
    sal_Int32 nTitlePos = -1;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name == "ContentType" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name == "IsDocument" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name == "IsFolder" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name == "CreatableContentsInfo" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name == "Title" )
        {
            if ( isReadOnly() )
            {
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                OUString aNewValue;
                if ( rValue.Value >>= aNewValue )
                {
                    
                    if ( !aNewValue.isEmpty() )
                    {
                        if ( aNewValue != m_aProps.getTitle() )
                        {
                            
                            if ( m_eState == PERSISTENT )
                                bExchange = sal_True;

                            aOldTitle = m_aProps.getTitle();
                            aOldName  = m_aProps.getName();

                            m_aProps.setTitle( aNewValue );
                            m_aProps.setName(
                                ::ucb_impl::urihelper::encodeSegment(
                                    aNewValue ) );

                            

                            
                            
                            nTitlePos = n;
                        }
                    }
                    else
                    {
                        aRet[ n ] <<= lang::IllegalArgumentException(
                                    OUString( "Empty title not allowed!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 );
                    }
                }
                else
                {
                    aRet[ n ] <<= beans::IllegalTypeException(
                                OUString( "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                }
            }
        }
        else if ( rValue.Name == "TargetURL" )
        {
            if ( isReadOnly() )
            {
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                

                if ( m_eKind == LINK )
                {
                    OUString aNewValue;
                    if ( rValue.Value >>= aNewValue )
                    {
                        
                        if ( !aNewValue.isEmpty() )
                        {
                            if ( aNewValue != m_aProps.getTargetURL() )
                            {
                                aEvent.PropertyName = rValue.Name;
                                aEvent.OldValue
                                    = uno::makeAny( m_aProps.getTargetURL() );
                                aEvent.NewValue
                                    = uno::makeAny( aNewValue );

                                aChanges.getArray()[ nChanged ] = aEvent;

                                m_aProps.setTargetURL( aNewValue );
                                nChanged++;
                            }
                        }
                        else
                        {
                            aRet[ n ] <<= lang::IllegalArgumentException(
                                    OUString( "Empty target URL not allowed!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 );
                        }
                    }
                    else
                    {
                        aRet[ n ] <<= beans::IllegalTypeException(
                                OUString( "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                    }
                }
                else
                {
                    aRet[ n ] <<= beans::UnknownPropertyException(
                                OUString( "TargetURL only supported by links!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                }
            }
        }
        else
        {
            

            if ( !bTriedToGetAdditionalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( false );
                bTriedToGetAdditionalPropSet = sal_True;
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
                    aRet[ n ] <<= e;
                }
                catch ( lang::WrappedTargetException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( beans::PropertyVetoException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::IllegalArgumentException const & e )
                {
                    aRet[ n ] <<= e;
                }
            }
            else
            {
                aRet[ n ] <<= uno::Exception(
                                OUString( "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
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
            
            renameData( xOldId, xNewId );

            
            renameAdditionalPropertySet( xOldId->getContentIdentifier(),
                                         xNewId->getContentIdentifier(),
                                         true );
        }
        else
        {
            
            m_aProps.setTitle( aOldTitle );
            m_aProps.setName ( aOldName );

            aOldTitle = aOldName = "";

            
            aRet[ nTitlePos ] <<= uno::Exception(
                    OUString("Exchange failed!"),
                    static_cast< cppu::OWeakObject * >( this ) );
        }
    }

    if ( !aOldTitle.isEmpty() )
    {
        aEvent.PropertyName = "Title";
        aEvent.OldValue     = uno::makeAny( aOldTitle );
        aEvent.NewValue     = uno::makeAny( m_aProps.getTitle() );

        aChanges.getArray()[ nChanged ] = aEvent;
        nChanged++;
    }

    if ( nChanged > 0 )
    {
        
        if ( !bExchange && ( m_eState == PERSISTENT ) )
        {
            if ( !storeData() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 OUString( "Uri"),
                                 -1,
                                 uno::makeAny(m_xIdentifier->
                                                  getContentIdentifier()),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_WRITE,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    xEnv,
                    OUString( "Cannot store persistent data!" ),
                    this );
                
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
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    
    if ( m_eKind == ROOT )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                                OUString( "Not supported by root folder!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        
    }

    
    if ( m_aProps.getTitle().isEmpty() )
    {
        uno::Sequence< OUString > aProps( 1 );
        aProps[ 0 ] = "Title";
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            xEnv );
        
    }

    

    uno::Reference< ucb::XContentIdentifier > xId
        = makeNewIdentifier( m_aProps.getTitle() );

    

    switch ( nNameClashResolve )
    {
        
        case ucb::NameClash::ERROR:
            if ( hasData( xId ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::NameClashException(
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            task::InteractionClassification_ERROR,
                            m_aProps.getTitle() ) ),
                    xEnv );
                
            }
            break;

        
        case ucb::NameClash::OVERWRITE:
            break;

        
        case ucb::NameClash::RENAME:
            if ( hasData( xId ) )
            {
                sal_Int32 nTry = 0;

                do
                {
                    OUString aNewId = xId->getContentIdentifier();
                    aNewId += "_";
                    aNewId += OUString::number( ++nTry );
                    xId = new ::ucbhelper::ContentIdentifier( aNewId );
                }
                while ( hasData( xId ) && ( nTry < 1000 ) );

                if ( nTry == 1000 )
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::UnsupportedNameClashException(
                                OUString( "Unable to resolve name clash!" ),
                                static_cast< cppu::OWeakObject * >( this ),
                                nNameClashResolve ) ),
                    xEnv );
                
                }
                else
                {
                    OUString aNewTitle( m_aProps.getTitle() );
                    aNewTitle += "_";
                    aNewTitle += OUString::number( nTry );
                    m_aProps.setTitle( aNewTitle );
                }
            }
            break;

        case ucb::NameClash::KEEP: 
        case ucb::NameClash::ASK:
        default:
            if ( hasData( xId ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::UnsupportedNameClashException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                nNameClashResolve ) ),
                    xEnv );
                
            }
            break;
    }

    
    sal_Bool bNewId = ( xId->getContentIdentifier()
                            != m_xIdentifier->getContentIdentifier() );
    m_xIdentifier = xId;

    if ( !storeData() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(m_xIdentifier->
                                                   getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            OUString("Cannot store persistent data!"),
            this );
        
    }

    m_eState = PERSISTENT;

    if ( bNewId )
    {
        aGuard.clear();
        inserted();
    }
}


void HierarchyContent::destroy( sal_Bool bDeletePhysical,
                                const uno::Reference<
                                    ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    
    if ( m_eState != PERSISTENT )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                                OUString( "Not persistent!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        
    }

    
    if ( m_eKind == ROOT )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                                OUString( "Not supported by root folder!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        
    }

    m_eState = DEAD;

    aGuard.clear();
    deleted();

    if ( m_eKind == FOLDER )
    {
        

        HierarchyContentRefList aChildren;
        queryChildren( aChildren );

        HierarchyContentRefList::const_iterator it  = aChildren.begin();
        HierarchyContentRefList::const_iterator end = aChildren.end();

        while ( it != end )
        {
            (*it)->destroy( bDeletePhysical, xEnv );
            ++it;
        }
    }
}


void HierarchyContent::transfer(
            const ucb::TransferInfo& rInfo,
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    
    if ( m_eState != PERSISTENT )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                                OUString( "Not persistent!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        
    }

    
    if ( !rInfo.SourceURL.startsWith( HIERARCHY_URL_SCHEME ":/" ) )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::InteractiveBadTransferURLException(
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
        
    }

    
    OUString aId = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aId.lastIndexOf( '/' );
    if ( nPos != ( aId.getLength() - 1 ) )
    {
        
        aId += "/";
    }

    if ( rInfo.SourceURL.getLength() <= aId.getLength() )
    {
        if ( aId.compareTo(
                rInfo.SourceURL, rInfo.SourceURL.getLength() ) == 0 )
        {
            uno::Any aProps
                = uno::makeAny(beans::PropertyValue(
                                      OUString( "Uri"),
                                      -1,
                                      uno::makeAny(rInfo.SourceURL),
                                      beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_RECURSIVE,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                OUString( "Target is equal to or is a child of source!" ),
                this );
            
        }
    }

    
    
    

    uno::Reference< ucb::XContentIdentifier > xId
        = new ::ucbhelper::ContentIdentifier( rInfo.SourceURL );

    
    
    rtl::Reference< HierarchyContent > xSource;

    try
    {
        xSource = static_cast< HierarchyContent * >(
                        m_xProvider->queryContent( xId ).get() );
    }
    catch ( ucb::IllegalIdentifierException const & )
    {
        
    }

    if ( !xSource.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(xId->getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            OUString( "Cannot instanciate source object!" ),
            this );
        
    }

    
    
    

    OUString aType = xSource->isFolder()
        ? OUString( HIERARCHY_FOLDER_CONTENT_TYPE )
        : OUString( HIERARCHY_LINK_CONTENT_TYPE );
    ucb::ContentInfo aContentInfo;
    aContentInfo.Type = aType;
    aContentInfo.Attributes = 0;

    
    
    rtl::Reference< HierarchyContent > xTarget
        = static_cast< HierarchyContent * >(
            createNewContent( aContentInfo ).get() );
    if ( !xTarget.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Folder"),
                                  -1,
                                  uno::makeAny(aId),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            OUString( "XContentCreator::createNewContent failed!" ),
            this );
        
    }

    
    
    

    uno::Sequence< beans::Property > aSourceProps
                    = xSource->getPropertySetInfo( xEnv )->getProperties();
    sal_Int32 nCount = aSourceProps.getLength();

    if ( nCount )
    {
        sal_Bool bHadTitle = rInfo.NewTitle.isEmpty();

        
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
                
                bHadTitle = sal_True;
                rValue.Value <<= rInfo.NewTitle;
            }
            else
                rValue.Value = xRow->getObject(
                                n + 1,
                                uno::Reference< container::XNameAccess >() );

            rValue.State = beans::PropertyState_DIRECT_VALUE;

            if ( rProp.Attributes & beans::PropertyAttribute::REMOVABLE )
            {
                
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

        
        xTarget->setPropertyValues( aValues, xEnv );
    }

    
    
    

    xTarget->insert( rInfo.NameClash, xEnv );

    
    
    

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
            aInfo.MoveData  = sal_False;
            aInfo.NewTitle  = "";
            aInfo.SourceURL = aChildId;
            aInfo.NameClash = rInfo.NameClash;

            
            xTarget->transfer( aInfo, xEnv );
        }
    }

    
    
    

    if ( rInfo.MoveData )
    {
        xSource->destroy( sal_True, xEnv );

        
        if ( !xSource->removeData() )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             OUString( "Uri"),
                             -1,
                             uno::makeAny(
                                 xSource->m_xIdentifier->
                                              getContentIdentifier()),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_WRITE,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                OUString( "Cannot remove persistent data of source object!" ),
                this );
            
        }

        
        xSource->removeAdditionalPropertySet( true );
    }
}



//

//



uno::Sequence< ucb::ContentInfo >
HierarchyContentProperties::getCreatableContentsInfo() const
{
    if ( getIsFolder() )
    {
        uno::Sequence< ucb::ContentInfo > aSeq( 2 );

        
        aSeq.getArray()[ 0 ].Type = HIERARCHY_FOLDER_CONTENT_TYPE;
        aSeq.getArray()[ 0 ].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;

        uno::Sequence< beans::Property > aFolderProps( 1 );
        aFolderProps.getArray()[ 0 ] = beans::Property(
                    OUString("Title"),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND );
        aSeq.getArray()[ 0 ].Properties = aFolderProps;

        
        aSeq.getArray()[ 1 ].Type = HIERARCHY_LINK_CONTENT_TYPE;
        aSeq.getArray()[ 1 ].Attributes = ucb::ContentInfoAttribute::KIND_LINK;

        uno::Sequence< beans::Property > aLinkProps( 2 );
        aLinkProps.getArray()[ 0 ] = beans::Property(
                    OUString("Title"),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND );
        aLinkProps.getArray()[ 1 ] = beans::Property(
                    OUString("TargetURL"),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
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
