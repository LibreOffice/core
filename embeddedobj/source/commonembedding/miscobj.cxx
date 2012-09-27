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

#include <commonembobj.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/mimeconfighelper.hxx>

#include "closepreventer.hxx"
#include "intercept.hxx"

using namespace ::com::sun::star;


uno::Sequence< beans::PropertyValue > GetValuableArgs_Impl( const uno::Sequence< beans::PropertyValue >& aMedDescr,
                                                            sal_Bool bCanUseDocumentBaseURL );

//------------------------------------------------------
OCommonEmbeddedObject::OCommonEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                                const uno::Sequence< beans::NamedValue >& aObjProps )
: m_pDocHolder( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_bClosed( sal_False )
, m_nObjectState( -1 )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_nMiscStatus( 0 )
, m_bEmbeddedScriptSupport( sal_True )
, m_bDocumentRecoverySupport( sal_True )
, m_bWaitSaveCompleted( sal_False )
, m_bIsLink( sal_False )
, m_bLinkHasPassword( sal_False )
, m_bHasClonedSize( sal_False )
, m_nClonedMapUnit( 0 )
{
    CommonInit_Impl( aObjProps );
}

//------------------------------------------------------
OCommonEmbeddedObject::OCommonEmbeddedObject(
        const uno::Reference< lang::XMultiServiceFactory >& xFactory,
        const uno::Sequence< beans::NamedValue >& aObjProps,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& aObjectDescr )
: m_pDocHolder( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_bClosed( sal_False )
, m_nObjectState( embed::EmbedStates::LOADED )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_nMiscStatus( 0 )
, m_bEmbeddedScriptSupport( sal_True )
, m_bDocumentRecoverySupport( sal_True )
, m_bWaitSaveCompleted( sal_False )
, m_bIsLink( sal_True )
, m_bLinkHasPassword( sal_False )
, m_bHasClonedSize( sal_False )
, m_nClonedMapUnit( 0 )
{
    // linked object has no own persistence so it is in loaded state starting from creation
    LinkInit_Impl( aObjProps, aMediaDescr, aObjectDescr );
}

//------------------------------------------------------
void OCommonEmbeddedObject::CommonInit_Impl( const uno::Sequence< beans::NamedValue >& aObjectProps )
{
    OSL_ENSURE( m_xFactory.is(), "No ServiceFactory is provided!\n" );
    if ( !m_xFactory.is() )
        throw uno::RuntimeException();

    m_pDocHolder = new DocumentHolder( m_xFactory, this );
    m_pDocHolder->acquire();

    // parse configuration entries
    // TODO/LATER: in future UI names can be also provided here
    for ( sal_Int32 nInd = 0; nInd < aObjectProps.getLength(); nInd++ )
    {
        if ( aObjectProps[nInd].Name == "ClassID" )
            aObjectProps[nInd].Value >>= m_aClassID;
        else if ( aObjectProps[nInd].Name == "ObjectDocumentServiceName" )
            aObjectProps[nInd].Value >>= m_aDocServiceName;
        else if ( aObjectProps[nInd].Name == "ObjectDocumentFilterName" )
            aObjectProps[nInd].Value >>= m_aPresetFilterName;
        else if ( aObjectProps[nInd].Name == "ObjectMiscStatus" )
            aObjectProps[nInd].Value >>= m_nMiscStatus;
        else if ( aObjectProps[nInd].Name == "ObjectVerbs" )
            aObjectProps[nInd].Value >>= m_aObjectVerbs;
    }

    if ( m_aClassID.getLength() != 16 /*|| !m_aDocServiceName.getLength()*/ )
        throw uno::RuntimeException(); // something goes really wrong

    // accepted states
    m_aAcceptedStates.realloc( NUM_SUPPORTED_STATES );

    m_aAcceptedStates[0] = embed::EmbedStates::LOADED;
    m_aAcceptedStates[1] = embed::EmbedStates::RUNNING;
    m_aAcceptedStates[2] = embed::EmbedStates::INPLACE_ACTIVE;
    m_aAcceptedStates[3] = embed::EmbedStates::UI_ACTIVE;
    m_aAcceptedStates[4] = embed::EmbedStates::ACTIVE;


    // intermediate states
    // In the following table the first index points to starting state,
    // the second one to the target state, and the sequence referenced by
    // first two indexes contains intermediate states, that should be
    // passed by object to reach the target state.
    // If the sequence is empty that means that indirect switch from start
    // state to the target state is forbidden, only if direct switch is possible
    // the state can be reached.

    m_pIntermediateStatesSeqs[0][2].realloc( 1 );
    m_pIntermediateStatesSeqs[0][2][0] = embed::EmbedStates::RUNNING;

    m_pIntermediateStatesSeqs[0][3].realloc( 2 );
    m_pIntermediateStatesSeqs[0][3][0] = embed::EmbedStates::RUNNING;
    m_pIntermediateStatesSeqs[0][3][1] = embed::EmbedStates::INPLACE_ACTIVE;

    m_pIntermediateStatesSeqs[0][4].realloc( 1 );
    m_pIntermediateStatesSeqs[0][4][0] = embed::EmbedStates::RUNNING;

    m_pIntermediateStatesSeqs[1][3].realloc( 1 );
    m_pIntermediateStatesSeqs[1][3][0] = embed::EmbedStates::INPLACE_ACTIVE;

    m_pIntermediateStatesSeqs[2][0].realloc( 1 );
    m_pIntermediateStatesSeqs[2][0][0] = embed::EmbedStates::RUNNING;

    m_pIntermediateStatesSeqs[3][0].realloc( 2 );
    m_pIntermediateStatesSeqs[3][0][0] = embed::EmbedStates::INPLACE_ACTIVE;
    m_pIntermediateStatesSeqs[3][0][1] = embed::EmbedStates::RUNNING;

    m_pIntermediateStatesSeqs[3][1].realloc( 1 );
    m_pIntermediateStatesSeqs[3][1][0] = embed::EmbedStates::INPLACE_ACTIVE;

    m_pIntermediateStatesSeqs[4][0].realloc( 1 );
    m_pIntermediateStatesSeqs[4][0][0] = embed::EmbedStates::RUNNING;

    // verbs table
    sal_Int32 nVerbTableSize = 0;
    for ( sal_Int32 nVerbInd = 0; nVerbInd < m_aObjectVerbs.getLength(); nVerbInd++ )
    {
        if ( m_aObjectVerbs[nVerbInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_PRIMARY )
        {
            m_aVerbTable.realloc( ++nVerbTableSize );
            m_aVerbTable[nVerbTableSize - 1].realloc( 2 );
            m_aVerbTable[nVerbTableSize - 1][0] = m_aObjectVerbs[nVerbInd].VerbID;
            m_aVerbTable[nVerbTableSize - 1][1] = embed::EmbedStates::UI_ACTIVE;
        }
        else if ( m_aObjectVerbs[nVerbInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_SHOW )
        {
            m_aVerbTable.realloc( ++nVerbTableSize );
            m_aVerbTable[nVerbTableSize - 1].realloc( 2 );
            m_aVerbTable[nVerbTableSize - 1][0] = m_aObjectVerbs[nVerbInd].VerbID;
            m_aVerbTable[nVerbTableSize - 1][1] = embed::EmbedStates::UI_ACTIVE;
        }
        else if ( m_aObjectVerbs[nVerbInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_OPEN )
        {
            m_aVerbTable.realloc( ++nVerbTableSize );
            m_aVerbTable[nVerbTableSize - 1].realloc( 2 );
            m_aVerbTable[nVerbTableSize - 1][0] = m_aObjectVerbs[nVerbInd].VerbID;
            m_aVerbTable[nVerbTableSize - 1][1] = embed::EmbedStates::ACTIVE;
        }
        else if ( m_aObjectVerbs[nVerbInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_IPACTIVATE )
        {
            m_aVerbTable.realloc( ++nVerbTableSize );
            m_aVerbTable[nVerbTableSize - 1].realloc( 2 );
            m_aVerbTable[nVerbTableSize - 1][0] = m_aObjectVerbs[nVerbInd].VerbID;
            m_aVerbTable[nVerbTableSize - 1][1] = embed::EmbedStates::INPLACE_ACTIVE;
        }
        else if ( m_aObjectVerbs[nVerbInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_UIACTIVATE )
        {
            m_aVerbTable.realloc( ++nVerbTableSize );
            m_aVerbTable[nVerbTableSize - 1].realloc( 2 );
            m_aVerbTable[nVerbTableSize - 1][0] = m_aObjectVerbs[nVerbInd].VerbID;
            m_aVerbTable[nVerbTableSize - 1][1] = embed::EmbedStates::UI_ACTIVE;
        }
        else if ( m_aObjectVerbs[nVerbInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_HIDE )
        {
            m_aVerbTable.realloc( ++nVerbTableSize );
            m_aVerbTable[nVerbTableSize - 1].realloc( 2 );
            m_aVerbTable[nVerbTableSize - 1][0] = m_aObjectVerbs[nVerbInd].VerbID;
            m_aVerbTable[nVerbTableSize - 1][1] = embed::EmbedStates::RUNNING;
        }
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::LinkInit_Impl(
                                const uno::Sequence< beans::NamedValue >& aObjectProps,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                const uno::Sequence< beans::PropertyValue >& aObjectDescr )
{
    // setPersistance has no effect on own links, so the complete initialization must be done here

    for ( sal_Int32 nInd = 0; nInd < aMediaDescr.getLength(); nInd++ )
        if ( aMediaDescr[nInd].Name == "URL" )
            aMediaDescr[nInd].Value >>= m_aLinkURL;
        else if ( aMediaDescr[nInd].Name == "FilterName" )
            aMediaDescr[nInd].Value >>= m_aLinkFilterName;

    OSL_ENSURE( m_aLinkURL.getLength() && m_aLinkFilterName.getLength(), "Filter and URL must be provided!\n" );

    m_bReadOnly = sal_True;
    if ( m_aLinkFilterName.getLength() )
    {
        ::comphelper::MimeConfigurationHelper aHelper( m_xFactory );
        ::rtl::OUString aExportFilterName = aHelper.GetExportFilterFromImportFilter( m_aLinkFilterName );
        m_bReadOnly = !( aExportFilterName.equals( m_aLinkFilterName ) );
    }

    m_aDocMediaDescriptor = GetValuableArgs_Impl( aMediaDescr, sal_False );

    uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
    for ( sal_Int32 nObjInd = 0; nObjInd < aObjectDescr.getLength(); nObjInd++ )
        if ( aObjectDescr[nObjInd].Name == "OutplaceDispatchInterceptor" )
        {
            aObjectDescr[nObjInd].Value >>= xDispatchInterceptor;
            break;
        }
        else if ( aObjectDescr[nObjInd].Name == "Parent" )
        {
            aObjectDescr[nObjInd].Value >>= m_xParent;
        }

    CommonInit_Impl( aObjectProps );

    if ( xDispatchInterceptor.is() )
        m_pDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );
}

//------------------------------------------------------
OCommonEmbeddedObject::~OCommonEmbeddedObject()
{
    if ( m_pInterfaceContainer || m_pDocHolder )
    {
        m_refCount++;
        try {
            lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

            if ( m_pInterfaceContainer )
            {
                m_pInterfaceContainer->disposeAndClear( aSource );

                delete m_pInterfaceContainer;
                m_pInterfaceContainer = NULL;
            }
        } catch( const uno::Exception& ) {}

        try {
            if ( m_pDocHolder )
            {
                m_pDocHolder->CloseFrame();
                try {
                    m_pDocHolder->CloseDocument( sal_True, sal_True );
                } catch ( const uno::Exception& ) {}
                m_pDocHolder->FreeOffice();

                m_pDocHolder->release();
                m_pDocHolder = NULL;
            }
        } catch( const uno::Exception& ) {}
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::requestPositioning( const awt::Rectangle& aRect )
{
    // the method is called in case object is inplace active and the object window was resized

    OSL_ENSURE( m_xClientSite.is(), "The client site must be set for inplace active object!\n" );
    if ( m_xClientSite.is() )
    {
        uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );

        OSL_ENSURE( xInplaceClient.is(), "The client site must support XInplaceClient to allow inplace activation!\n" );
        if ( xInplaceClient.is() )
        {
            try {
                xInplaceClient->changedPlacement( aRect );
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "Exception on request to resize!\n" );
            }
        }
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::PostEvent_Impl( const ::rtl::OUString& aEventName )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pIC = m_pInterfaceContainer->getContainer(
                                            ::getCppuType((const uno::Reference< document::XEventListener >*)0) );
        if( pIC )
        {
            document::EventObject aEvent;
            aEvent.EventName = aEventName;
            aEvent.Source = uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) );
            // For now all the events are sent as object events
            // aEvent.Source = ( xSource.is() ? xSource
            //                       : uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ) );
            ::cppu::OInterfaceIteratorHelper aIt( *pIC );
            while( aIt.hasMoreElements() )
            {
                try
                {
                    ((document::XEventListener *)aIt.next())->notifyEvent( aEvent );
                }
                catch( const uno::RuntimeException& )
                {
                    aIt.remove();
                }

                // the listener could dispose the object.
                if ( m_bDisposed )
                    return;
            }
        }
    }
}

//------------------------------------------------------
uno::Any SAL_CALL OCommonEmbeddedObject::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    if ( rType == ::getCppuType( (uno::Reference< embed::XEmbeddedObject > const *)0 ))
    {
        void * p = static_cast< embed::XEmbeddedObject * >( this );
        return uno::Any( &p, rType );
    }
    else
        aReturn <<= ::cppu::queryInterface(
                    rType,
                    static_cast< embed::XInplaceObject* >( this ),
                    static_cast< embed::XVisualObject* >( this ),
                    static_cast< embed::XCommonEmbedPersist* >( static_cast< embed::XEmbedPersist* >( this ) ),
                    static_cast< embed::XEmbedPersist* >( this ),
                    static_cast< embed::XLinkageSupport* >( this ),
                    static_cast< embed::XStateChangeBroadcaster* >( this ),
                    static_cast< embed::XClassifiedObject* >( this ),
                    static_cast< embed::XComponentSupplier* >( this ),
                    static_cast< util::XCloseable* >( this ),
                    static_cast< container::XChild* >( this ),
                    static_cast< chart2::XDefaultSizeTransmitter* >( this ),
                    static_cast< document::XEventBroadcaster* >( this ) );

    if ( aReturn.hasValue() )
        return aReturn;
    else
        return ::cppu::OWeakObject::queryInterface( rType ) ;

}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::acquire()
        throw()
{
    ::cppu::OWeakObject::acquire() ;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::release()
        throw()
{
    ::cppu::OWeakObject::release() ;
}

//------------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OCommonEmbeddedObject::getTypes()
        throw( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL;

    if ( !pTypeCollection )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pTypeCollection )
        {
            if ( m_bIsLink )
            {
                static ::cppu::OTypeCollection aTypeCollection(
                                            ::getCppuType( (const uno::Reference< lang::XTypeProvider >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XEmbeddedObject >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XInplaceObject >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XCommonEmbedPersist >*)NULL ),
                                            ::getCppuType( (const uno::Reference< container::XChild >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XLinkageSupport >*)NULL ) );

                pTypeCollection = &aTypeCollection ;
            }
            else
            {
                   static ::cppu::OTypeCollection aTypeCollection(
                                            ::getCppuType( (const uno::Reference< lang::XTypeProvider >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XEmbeddedObject >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XInplaceObject >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XCommonEmbedPersist >*)NULL ),
                                            ::getCppuType( (const uno::Reference< container::XChild >*)NULL ),
                                            ::getCppuType( (const uno::Reference< embed::XEmbedPersist >*)NULL ) );

                pTypeCollection = &aTypeCollection ;
            }
        }
    }

    return pTypeCollection->getTypes() ;

}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OCommonEmbeddedObject::getImplementationId()
        throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( !pID )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;
        if ( !pID )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OCommonEmbeddedObject::getClassID()
        throw ( uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    return m_aClassID;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OCommonEmbeddedObject::getClassName()
        throw ( uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    return m_aClassName;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& /*aClassID*/, const ::rtl::OUString& /*aClassName*/ )
        throw ( lang::NoSupportException,
                uno::RuntimeException )
{
    // the object class info can not be changed explicitly
    throw lang::NoSupportException(); //TODO:
}

//------------------------------------------------------
uno::Reference< util::XCloseable > SAL_CALL OCommonEmbeddedObject::getComponent()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // add an exception
    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw uno::RuntimeException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't store object without persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    return uno::Reference< util::XCloseable >( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< embed::XStateChangeListener >*)0 ),
                                                        xListener );
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< embed::XStateChangeListener >*)0 ),
                                                xListener );
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::close( sal_Bool bDeliverOwnership )
    throw ( util::CloseVetoException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bClosed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( const uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( const uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }

        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_bDisposed = sal_True; // the object is disposed now for outside

    // it is possible that the document can not be closed, in this case if the argument is false
    // the exception will be thrown otherwise in addition to exception the object must register itself
    // as termination listener and listen for document events

    if ( m_pDocHolder )
    {
        m_pDocHolder->CloseFrame();

        try {
            m_pDocHolder->CloseDocument( bDeliverOwnership, bDeliverOwnership );
        }
        catch( const uno::Exception& )
        {
            if ( bDeliverOwnership )
            {
                m_pDocHolder->release();
                m_pDocHolder = NULL;
                m_bClosed = sal_True;
            }

            throw;
        }

        m_pDocHolder->FreeOffice();

        m_pDocHolder->release();
        m_pDocHolder = NULL;
    }

    // TODO: for now the storage will be disposed by the object, but after the document
    // will use the storage, the storage will be disposed by the document and recreated by the object
    if ( m_xObjectStorage.is() )
    {
        uno::Reference< lang::XComponent > xComp( m_xObjectStorage, uno::UNO_QUERY );
        OSL_ENSURE( xComp.is(), "Storage does not support XComponent!\n" );

        if ( xComp.is() )
        {
            try {
                xComp->dispose();
            } catch ( const uno::Exception& ) {}
        }

        m_xObjectStorage.clear();
        m_xRecoveryStorage.clear();
    }

    m_bClosed = sal_True; // the closing succeeded
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ), xListener );
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ),
                                                xListener );
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ), xListener );
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ),
                                                xListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
