/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: olemisc.cxx,v $
 * $Revision: 1.26 $
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
#include "precompiled_embeddedobj.hxx"
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <cppuhelper/interfacecontainer.h>

#include <oleembobj.hxx>
#include <olecomponent.hxx>

#include "ownview.hxx"

using namespace ::com::sun::star;

sal_Bool KillFile_Impl( const ::rtl::OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory );


//------------------------------------------------------
OleEmbeddedObject::OleEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                      const uno::Sequence< sal_Int8 >& aClassID,
                                      const ::rtl::OUString& aClassName )
: m_pOleComponent( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( -1 )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_aClassID( aClassID )
, m_aClassName( aClassName )
, m_bWaitSaveCompleted( sal_False )
, m_bNewVisReplInStream( sal_True )
, m_bStoreLoaded( sal_False )
, m_bVisReplInitialized( sal_False )
, m_bVisReplInStream( sal_False )
, m_bStoreVisRepl( sal_False )
, m_bIsLink( sal_False )
, m_bHasCachedSize( sal_False )
, m_nCachedAspect( 0 )
, m_bHasSizeToSet( sal_False )
, m_nAspectToSet( 0 )
, m_bGotStatus( sal_False )
, m_nStatus( 0 )
, m_nStatusAspect( 0 )
, m_pOwnView( NULL )
, m_bFromClipboard( sal_False )
{
}

//------------------------------------------------------
// In case of loading from persistent entry the classID of the object
// will be retrieved from the entry, during construction it is unknown
OleEmbeddedObject::OleEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory, sal_Bool bLink )
: m_pOleComponent( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( -1 )
, m_nTargetState( -1 )
, m_nUpdateMode( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_bWaitSaveCompleted( sal_False )
, m_bNewVisReplInStream( sal_True )
, m_bStoreLoaded( sal_False )
, m_bVisReplInitialized( sal_False )
, m_bVisReplInStream( sal_False )
, m_bStoreVisRepl( sal_False )
, m_bIsLink( bLink )
, m_bHasCachedSize( sal_False )
, m_nCachedAspect( 0 )
, m_bHasSizeToSet( sal_False )
, m_nAspectToSet( 0 )
, m_bGotStatus( sal_False )
, m_nStatus( 0 )
, m_nStatusAspect( 0 )
, m_pOwnView( NULL )
, m_bFromClipboard( sal_False )
{
}

//------------------------------------------------------
// this constructor let object be initialized from clipboard
OleEmbeddedObject::OleEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_pOleComponent( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( -1 )
, m_nTargetState( -1 )
, m_nUpdateMode( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_bWaitSaveCompleted( sal_False )
, m_bNewVisReplInStream( sal_True )
, m_bStoreLoaded( sal_False )
, m_bVisReplInitialized( sal_False )
, m_bVisReplInStream( sal_False )
, m_bStoreVisRepl( sal_False )
, m_bIsLink( sal_False )
, m_bHasCachedSize( sal_False )
, m_nCachedAspect( 0 )
, m_bHasSizeToSet( sal_False )
, m_nAspectToSet( 0 )
, m_bGotStatus( sal_False )
, m_nStatus( 0 )
, m_nStatusAspect( 0 )
, m_pOwnView( NULL )
, m_bFromClipboard( sal_True )
{
}

//------------------------------------------------------
OleEmbeddedObject::~OleEmbeddedObject()
{
    OSL_ENSURE( !m_pInterfaceContainer && !m_pOleComponent && !m_xObjectStream.is(),
                    "The object is not closed! DISASTER is possible!" );

    if ( m_pOleComponent || m_pInterfaceContainer || m_xObjectStream.is() )
    {
        // the component must be cleaned during closing
        m_refCount++; // to avoid crash
        try {
            Dispose();
        } catch( uno::Exception& ) {}
    }

    if ( m_aTempURL.getLength() )
           KillFile_Impl( m_aTempURL, m_xFactory );
}

//------------------------------------------------------
void OleEmbeddedObject::MakeEventListenerNotification_Impl( const ::rtl::OUString& aEventName )
{
    if ( m_pInterfaceContainer )
    {
           ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer(
                                    ::getCppuType( ( const uno::Reference< document::XEventListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            document::EventObject aEvent( static_cast< ::cppu::OWeakObject* >( this ), aEventName );
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    ((document::XEventListener*)pIterator.next())->notifyEvent( aEvent );
                }
                catch( uno::RuntimeException& )
                {
                }
            }
        }
    }
}

//----------------------------------------------
void OleEmbeddedObject::StateChangeNotification_Impl( sal_Bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer = m_pInterfaceContainer->getContainer(
                            ::getCppuType( ( const uno::Reference< embed::XStateChangeListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);

            while (pIterator.hasMoreElements())
            {
                if ( bBeforeChange )
                {
                    try
                    {
                        ((embed::XStateChangeListener*)pIterator.next())->changingState( aSource, nOldState, nNewState );
                    }
                    catch( uno::Exception& )
                    {
                        // even if the listener complains ignore it for now
                    }
                }
                else
                {
                       try
                    {
                        ((embed::XStateChangeListener*)pIterator.next())->stateChanged( aSource, nOldState, nNewState );
                    }
                    catch( uno::Exception& )
                    {
                        // if anything happened it is problem of listener, ignore it
                    }
                }
            }
        }
    }
}

//------------------------------------------------------
void OleEmbeddedObject::GetRidOfComponent()
{
#ifdef WNT
    if ( m_pOleComponent )
    {
        if ( m_nObjectState != -1 && m_nObjectState != embed::EmbedStates::LOADED )
            SaveObject_Impl();

        m_pOleComponent->removeCloseListener( m_xClosePreventer );
        try
        {
            m_pOleComponent->close( sal_False );
        }
        catch( uno::Exception& )
        {
            // TODO: there should be a special listener to wait for component closing
            //       and to notify object, may be object itself can be such a listener
            m_pOleComponent->addCloseListener( m_xClosePreventer );
            throw;
        }

        m_pOleComponent->disconnectEmbeddedObject();
        m_pOleComponent->release();
        m_pOleComponent = NULL;
    }
#endif
}

//------------------------------------------------------
void OleEmbeddedObject::Dispose()
{
    if ( m_pInterfaceContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
        m_pInterfaceContainer->disposeAndClear( aSource );
        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }

    if ( m_pOwnView )
    {
        m_pOwnView->Close();
        m_pOwnView->release();
        m_pOwnView = NULL;
    }

    if ( m_pOleComponent )
        try {
            GetRidOfComponent();
        } catch( uno::Exception& )
        {
            m_bDisposed = true;
            throw; // TODO: there should be a special listener that will close object when
                    // component is finally closed
        }

    if ( m_xObjectStream.is() )
    {
        uno::Reference< lang::XComponent > xComp( m_xObjectStream, uno::UNO_QUERY );
        OSL_ENSURE( xComp.is(), "Storage stream doesn't support XComponent!\n" );

        if ( xComp.is() )
        {
            try {
                xComp->dispose();
            } catch( uno::Exception& ) {}
        }
        m_xObjectStream = uno::Reference< io::XStream >();
    }

    m_xParentStorage = uno::Reference< embed::XStorage >();

    m_bDisposed = true;
}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OleEmbeddedObject::getClassID()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_aClassID;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObject::getClassName()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_aClassName;
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& /*aClassID*/, const ::rtl::OUString& /*aClassName*/ )
        throw ( lang::NoSupportException,
                uno::RuntimeException )
{
    // the object class info can not be changed explicitly
    throw lang::NoSupportException(); //TODO:
}

//------------------------------------------------------
uno::Reference< util::XCloseable > SAL_CALL OleEmbeddedObject::getComponent()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 ) // || m_nObjectState == embed::EmbedStates::LOADED )
    {
        // the object is still not running
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is not loaded!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    // if ( m_bWaitSaveCompleted )
    //  throw embed::WrongStateException(
    //              ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
    //              uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_pOleComponent )
    {
        // TODO/LATER: Is it correct???
        return uno::Reference< util::XCloseable >();
        // throw uno::RuntimeException(); // TODO
    }

    return uno::Reference< util::XCloseable >( static_cast< ::cppu::OWeakObject* >( m_pOleComponent ), uno::UNO_QUERY );
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
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
void SAL_CALL OleEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< embed::XStateChangeListener >*)0 ),
                                                xListener );
}


//----------------------------------------------
void SAL_CALL OleEmbeddedObject::close( sal_Bool bDeliverOwnership )
    throw ( util::CloseVetoException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
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
                catch( uno::RuntimeException& )
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
                catch( uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }
    }

    Dispose();
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
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
void SAL_CALL OleEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ),
                                                xListener );
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
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
void SAL_CALL OleEmbeddedObject::removeEventListener(
                const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ),
                                                xListener );
}

