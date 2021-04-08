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
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <comphelper/storagehelper.hxx>

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/mimeconfighelper.hxx>

#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

#include "persistence.hxx"

using namespace ::com::sun::star;


OCommonEmbeddedObject::OCommonEmbeddedObject( const uno::Reference< uno::XComponentContext >& rxContext,
                                                const uno::Sequence< beans::NamedValue >& aObjProps )
: m_bReadOnly( false )
, m_bDisposed( false )
, m_bClosed( false )
, m_nObjectState( -1 )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xContext( rxContext )
, m_nMiscStatus( 0 )
, m_bEmbeddedScriptSupport( true )
, m_bDocumentRecoverySupport( true )
, m_bWaitSaveCompleted( false )
, m_bIsLinkURL( false )
, m_bLinkTempFileChanged( false )
, m_bLinkHasPassword( false )
, m_aLinkTempFile( )
, m_bHasClonedSize( false )
, m_nClonedMapUnit( 0 )
{
    CommonInit_Impl( aObjProps );
}


OCommonEmbeddedObject::OCommonEmbeddedObject(
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Sequence< beans::NamedValue >& aObjProps,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& aObjectDescr )
: m_bReadOnly( false )
, m_bDisposed( false )
, m_bClosed( false )
, m_nObjectState( embed::EmbedStates::LOADED )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xContext( rxContext )
, m_nMiscStatus( 0 )
, m_bEmbeddedScriptSupport( true )
, m_bDocumentRecoverySupport( true )
, m_bWaitSaveCompleted( false )
, m_bIsLinkURL( true )
, m_bLinkTempFileChanged( false )
, m_bLinkHasPassword( false )
, m_aLinkTempFile( )
, m_bHasClonedSize( false )
, m_nClonedMapUnit( 0 )
{
    // linked object has no own persistence so it is in loaded state starting from creation
    LinkInit_Impl( aObjProps, aMediaDescr, aObjectDescr );
}


void OCommonEmbeddedObject::CommonInit_Impl( const uno::Sequence< beans::NamedValue >& aObjectProps )
{
    OSL_ENSURE( m_xContext.is(), "No ServiceFactory is provided!" );
    if ( !m_xContext.is() )
        throw uno::RuntimeException();

    m_xDocHolder = new DocumentHolder( m_xContext, this );

    // parse configuration entries
    // TODO/LATER: in future UI names can be also provided here
    for ( beans::NamedValue const & prop : aObjectProps )
    {
        if ( prop.Name == "ClassID" )
            prop.Value >>= m_aClassID;
        else if ( prop.Name == "ObjectDocumentServiceName" )
            prop.Value >>= m_aDocServiceName;
        else if ( prop.Name == "ObjectDocumentFilterName" )
            prop.Value >>= m_aPresetFilterName;
        else if ( prop.Name == "ObjectMiscStatus" )
            prop.Value >>= m_nMiscStatus;
        else if ( prop.Name == "ObjectVerbs" )
            prop.Value >>= m_aObjectVerbs;
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
    for ( auto const & verb : std::as_const(m_aObjectVerbs) )
    {
        if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_PRIMARY )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::UI_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_SHOW )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::UI_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_OPEN )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_IPACTIVATE )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::INPLACE_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_UIACTIVATE )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::UI_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_HIDE )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::RUNNING } );
        }
    }
}


void OCommonEmbeddedObject::LinkInit_Impl(
                                const uno::Sequence< beans::NamedValue >& aObjectProps,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                const uno::Sequence< beans::PropertyValue >& aObjectDescr )
{
    // setPersistance has no effect on own links, so the complete initialization must be done here

    for ( beans::PropertyValue const & prop : aMediaDescr )
        if ( prop.Name == "URL" )
            prop.Value >>= m_aLinkURL;
        else if ( prop.Name == "FilterName" )
            prop.Value >>= m_aLinkFilterName;

    OSL_ENSURE( m_aLinkURL.getLength() && m_aLinkFilterName.getLength(), "Filter and URL must be provided!" );

    m_bReadOnly = true;
    if ( m_aLinkFilterName.getLength() )
    {
        ::comphelper::MimeConfigurationHelper aHelper( m_xContext );
        OUString aExportFilterName = aHelper.GetExportFilterFromImportFilter( m_aLinkFilterName );
        m_bReadOnly = aExportFilterName != m_aLinkFilterName;
    }

    if(m_bIsLinkURL && !m_bReadOnly)
    {
        // tdf#141529 we have a linked OLE object. To prevent the original OLE
        // data to be changed each time the OLE gets changed (at deactivate), copy it to
        // a temporary file. That file will be changed on activated OLE changes then.
        // The moment the original gets changed itself will now be associated with the
        // file/document embedding the OLE being changed (see other additions to the
        // task-ID above)
        //
        // open OLE original data as read input file
        uno::Reference< ucb::XSimpleFileAccess3 > xTempAccess( ucb::SimpleFileAccess::create( m_xContext ) );
        uno::Reference< io::XInputStream > xInStream( xTempAccess->openFileRead( m_aLinkURL ) );

        if(xInStream.is())
        {
            // create temporary file
            m_aLinkTempFile = io::TempFile::create(m_xContext);

            if(m_aLinkTempFile.is())
            {
                // completely copy content of original OLE data
                uno::Reference < io::XOutputStream > xTempOut = m_aLinkTempFile->getOutputStream();
                ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xTempOut );
                xTempOut->flush();
                xTempOut->closeOutput();

                // reset flag m_bLinkTempFileChanged, so it will also work for multiple
                // save op's of the containing file/document
                m_bLinkTempFileChanged = false;
            }
        }
    }

    if(m_aLinkTempFile.is())
    {
        uno::Sequence< beans::PropertyValue > aAlternativeMediaDescr(aMediaDescr.getLength());

        for ( sal_Int32 a(0); a < aMediaDescr.getLength(); a++ )
        {
            const beans::PropertyValue& rSource(aMediaDescr[a]);
            beans::PropertyValue& rDestination(aAlternativeMediaDescr[a]);

            rDestination.Name = rSource.Name;
            if(rSource.Name == "URL")
                rDestination.Value <<= m_aLinkTempFile->getUri();
            else
                rDestination.Value = rSource.Value;
        }

        m_aDocMediaDescriptor = GetValuableArgs_Impl( aAlternativeMediaDescr, false );
    }
    else
    {
        m_aDocMediaDescriptor = GetValuableArgs_Impl( aMediaDescr, false );
    }

    uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
    for ( beans::PropertyValue const & prop : aObjectDescr )
        if ( prop.Name == "OutplaceDispatchInterceptor" )
        {
            prop.Value >>= xDispatchInterceptor;
            break;
        }
        else if ( prop.Name == "Parent" )
        {
            prop.Value >>= m_xParent;
        }

    CommonInit_Impl( aObjectProps );

    if ( xDispatchInterceptor.is() )
        m_xDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );
}


OCommonEmbeddedObject::~OCommonEmbeddedObject()
{
    if ( !(m_pInterfaceContainer || m_xDocHolder.is()) )
        return;

    osl_atomic_increment(&m_refCount);
    if ( m_pInterfaceContainer )
    {
        try {
            lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
            m_pInterfaceContainer->disposeAndClear( aSource );
        } catch( const uno::Exception& ) {}
        m_pInterfaceContainer.reset();
    }

    try {
        if ( m_xDocHolder.is() )
        {
            m_xDocHolder->CloseFrame();
            try {
                m_xDocHolder->CloseDocument( true, true );
            } catch ( const uno::Exception& ) {}
            m_xDocHolder->FreeOffice();

            m_xDocHolder.clear();
        }
    } catch( const uno::Exception& ) {}
}


void OCommonEmbeddedObject::requestPositioning( const awt::Rectangle& aRect )
{
    // the method is called in case object is inplace active and the object window was resized

    OSL_ENSURE( m_xClientSite.is(), "The client site must be set for inplace active object!" );
    if ( !m_xClientSite.is() )
        return;

    uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );

    OSL_ENSURE( xInplaceClient.is(), "The client site must support XInplaceClient to allow inplace activation!" );
    if ( xInplaceClient.is() )
    {
        try {
            xInplaceClient->changedPlacement( aRect );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "embeddedobj", "Exception on request to resize!" );
        }
    }
}


void OCommonEmbeddedObject::PostEvent_Impl( const OUString& aEventName )
{
    if ( !m_pInterfaceContainer )
        return;

    ::cppu::OInterfaceContainerHelper* pIC = m_pInterfaceContainer->getContainer(
                                        cppu::UnoType<document::XEventListener>::get());
    if( !pIC )
        return;

    document::EventObject aEvent;
    aEvent.EventName = aEventName;
    aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
    // For now all the events are sent as object events
    // aEvent.Source = ( xSource.is() ? xSource
    //                       : uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ) );
    ::cppu::OInterfaceIteratorHelper aIt( *pIC );
    while( aIt.hasMoreElements() )
    {
        try
        {
            static_cast<document::XEventListener *>(aIt.next())->notifyEvent( aEvent );
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


uno::Any SAL_CALL OCommonEmbeddedObject::queryInterface( const uno::Type& rType )
{
    uno::Any aReturn;

    if ( rType == cppu::UnoType<embed::XEmbeddedObject>::get() )
    {
        void * p = static_cast< embed::XEmbeddedObject * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<embed::XEmbedPersist2>::get())
    {
        void* p = static_cast<embed::XEmbedPersist2*>(this);
        return uno::Any(&p, rType);
    }
    else
        aReturn = ::cppu::queryInterface(
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


void SAL_CALL OCommonEmbeddedObject::acquire()
        throw()
{
    ::cppu::OWeakObject::acquire() ;
}


void SAL_CALL OCommonEmbeddedObject::release()
        throw()
{
    ::cppu::OWeakObject::release() ;
}


uno::Sequence< sal_Int8 > SAL_CALL OCommonEmbeddedObject::getClassID()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    return m_aClassID;
}

OUString SAL_CALL OCommonEmbeddedObject::getClassName()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    return m_aClassName;
}

void SAL_CALL OCommonEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& /*aClassID*/, const OUString& /*aClassName*/ )
{
    // the object class info can not be changed explicitly
    throw lang::NoSupportException(); //TODO:
}


uno::Reference< util::XCloseable > SAL_CALL OCommonEmbeddedObject::getComponent()
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // add an exception
    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw uno::RuntimeException( "Can't store object without persistence!",
                                     static_cast< ::cppu::OWeakObject* >(this) );
    }

    return m_xDocHolder->GetComponent();
}


void SAL_CALL OCommonEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                        xListener );
}


void SAL_CALL OCommonEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                xListener );
}


void SAL_CALL OCommonEmbeddedObject::close( sal_Bool bDeliverOwnership )
{
    SolarMutexGuard aGuard;
    if ( m_bClosed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    static_cast<util::XCloseListener*>(pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( const uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    static_cast<util::XCloseListener*>(pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( const uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }

        m_pInterfaceContainer->disposeAndClear( aSource );
        m_pInterfaceContainer.reset();
    }

    m_bDisposed = true; // the object is disposed now for outside

    // it is possible that the document can not be closed, in this case if the argument is false
    // the exception will be thrown otherwise in addition to exception the object must register itself
    // as termination listener and listen for document events

    if ( m_xDocHolder.is() )
    {
        m_xDocHolder->CloseFrame();

        try {
            m_xDocHolder->CloseDocument( bDeliverOwnership, bDeliverOwnership );
        }
        catch( const uno::Exception& )
        {
            if ( bDeliverOwnership )
            {
                m_xDocHolder.clear();
                m_bClosed = true;
            }

            throw;
        }

        m_xDocHolder->FreeOffice();

        m_xDocHolder.clear();
    }

    // TODO: for now the storage will be disposed by the object, but after the document
    // will use the storage, the storage will be disposed by the document and recreated by the object
    if ( m_xObjectStorage.is() )
    {
        try {
            m_xObjectStorage->dispose();
        } catch ( const uno::Exception& ) {}

        m_xObjectStorage.clear();
        m_xRecoveryStorage.clear();
    }

    m_bClosed = true; // the closing succeeded
}


void SAL_CALL OCommonEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new ::cppu::OMultiTypeInterfaceContainerHelper(m_aMutex));

    m_pInterfaceContainer->addInterface( cppu::UnoType<util::XCloseListener>::get(), xListener );
}


void SAL_CALL OCommonEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<util::XCloseListener>::get(),
                                                xListener );
}


void SAL_CALL OCommonEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new ::cppu::OMultiTypeInterfaceContainerHelper(m_aMutex));

    m_pInterfaceContainer->addInterface( cppu::UnoType<document::XEventListener>::get(), xListener );
}


void SAL_CALL OCommonEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<document::XEventListener>::get(),
                                                xListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
