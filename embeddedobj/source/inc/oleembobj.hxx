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

#ifndef _INC_OLEEMBOBJ_HXX_
#define _INC_OLEEMBOBJ_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <cppuhelper/implbase5.hxx>

#include <osl/thread.h>

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class VerbExecutionController
{
    // the following mutex is allowed to be locked only for variables initialization, so no deadlock can be caused
    ::osl::Mutex    m_aVerbExecutionMutex;

    sal_Bool m_bVerbExecutionInProgress;
#ifdef WNT
    oslThreadIdentifier m_nVerbExecutionThreadIdentifier;
    sal_Bool m_bChangedOnVerbExecution;
#endif

    sal_Bool m_bWasEverActive;
    sal_Int32 m_nNotificationLock;

public:

    VerbExecutionController()
    : m_bVerbExecutionInProgress( sal_False )
#ifdef WNT
    , m_nVerbExecutionThreadIdentifier( 0 )
    , m_bChangedOnVerbExecution( sal_False )
#endif
    , m_bWasEverActive( sal_False )
    , m_nNotificationLock( 0 )
    {}
#ifdef WNT
    void StartControlExecution();
    sal_Bool EndControlExecution_WasModified();
    void ModificationNotificationIsDone();
#endif
    void LockNotification();
    void UnlockNotification();

    // no need to lock anything to check the value of the numeric members
    sal_Bool CanDoNotification() { return ( !m_bVerbExecutionInProgress && !m_bWasEverActive && !m_nNotificationLock ); }
    // ... or to change it
    void ObjectIsActive() { m_bWasEverActive = sal_True; }
};

class VerbExecutionControllerGuard
{
    VerbExecutionController& m_rController;
public:

    VerbExecutionControllerGuard( VerbExecutionController& rController )
    : m_rController( rController )
    {
        m_rController.LockNotification();
    }

    ~VerbExecutionControllerGuard()
    {
        m_rController.UnlockNotification();
    }
};


class OleComponent;
class OwnView_Impl;
class OleEmbeddedObject : public ::cppu::WeakImplHelper5
                        < ::com::sun::star::embed::XEmbeddedObject
                        , ::com::sun::star::embed::XEmbedPersist
                        , ::com::sun::star::embed::XLinkageSupport
                        , ::com::sun::star::embed::XInplaceObject
                        , ::com::sun::star::container::XChild >
{
    friend class OleComponent;

    ::osl::Mutex    m_aMutex;

    OleComponent*   m_pOleComponent;

    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    sal_Bool m_bReadOnly;

    sal_Int32 m_bDisposed;
    sal_Int32 m_nObjectState;
    sal_Int32 m_nTargetState;
    sal_Int32 m_nUpdateMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aClassID;
    ::rtl::OUString m_aClassName;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient > m_xClientSite;

    ::rtl::OUString m_aContainerName;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener > m_xClosePreventer;

    sal_Bool m_bWaitSaveCompleted;
    sal_Bool m_bNewVisReplInStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xNewCachedVisRepl;
    ::rtl::OUString m_aNewEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xNewParentStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xNewObjectStream;
    sal_Bool m_bStoreLoaded;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xCachedVisualRepresentation;
    sal_Bool m_bVisReplInitialized;
    sal_Bool m_bVisReplInStream;
    sal_Bool m_bStoreVisRepl;

    sal_Bool m_bIsLink;

    // TODO/LATER: may need to cache more than one aspect in future
    sal_Bool m_bHasCachedSize; // the object has cached size
    ::com::sun::star::awt::Size m_aCachedSize;
    sal_Int64 m_nCachedAspect;

    sal_Bool m_bHasSizeToSet;  // the object has cached size that should be set to OLE component
    ::com::sun::star::awt::Size m_aSizeToSet; // this size might be different from the cached one ( scaling is applied )
    sal_Int64 m_nAspectToSet;


    // cache the status of the object
    // TODO/LATER: may need to cache more than one aspect in future
    sal_Bool m_bGotStatus;
    sal_Int64 m_nStatus;
    sal_Int64 m_nStatusAspect;

    // embedded object related stuff
    ::rtl::OUString m_aEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xParentStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xObjectStream;

    // link related stuff
    ::rtl::OUString m_aLinkURL; // ???

    // points to own view provider if the the object has no server
    OwnView_Impl*   m_pOwnView;

    // whether the object should be initialized from clipboard in case of default initialization
    sal_Bool m_bFromClipboard;

    ::rtl::OUString m_aTempURL;

    ::rtl::OUString m_aTempDumpURL;

    // STAMPIT solution
    // the following member is used during verb execution to detect whether the verb execution modifies the object
    VerbExecutionController m_aVerbExecutionController;

    // if the following member is set, the object works in wrapper mode
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > m_xWrappedObject;
    sal_Bool m_bTriedConversion;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > TryToGetAcceptableFormat_Impl(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream )
        throw ( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetNewFilledTempStream_Impl(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream )
        throw( ::com::sun::star::io::IOException );
#ifdef WNT
    void SwitchComponentToRunningState_Impl();
#endif
    void MakeEventListenerNotification_Impl( const ::rtl::OUString& aEventName );
#ifdef WNT
    void StateChangeNotification_Impl( sal_Bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > GetStreamForSaving();


    ::com::sun::star::uno::Sequence< sal_Int32 > GetIntermediateVerbsSequence_Impl( sal_Int32 nNewState );

    ::com::sun::star::uno::Sequence< sal_Int32 > GetReachableStatesList_Impl(
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor >& aVerbList );
#endif

    void CloseComponent();
    void Dispose();

    void SwitchOwnPersistence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewParentStorage,
                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xNewObjectStream,
                const ::rtl::OUString& aNewName );

    void SwitchOwnPersistence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewParentStorage,
                const ::rtl::OUString& aNewName );

    void GetRidOfComponent();

    void StoreToLocation_Impl(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs,
                            sal_Bool bSaveAs )
        throw ( ::com::sun::star::uno::Exception );
#ifdef WNT
    void StoreObjectToStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStream )
        throw ( ::com::sun::star::uno::Exception );
#endif
    void InsertVisualCache_Impl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xCachedVisualRepresentation )
        throw ( ::com::sun::star::uno::Exception );

    void RemoveVisualCache_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream )
        throw ( ::com::sun::star::uno::Exception );

    void SetVisReplInStream( sal_Bool bExists );
    sal_Bool HasVisReplInStream();

    ::com::sun::star::embed::VisualRepresentation GetVisualRepresentationInNativeFormat_Impl(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xCachedVisRepr )
        throw ( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > TryToRetrieveCachedVisualRepresentation_Impl(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream,
                    sal_Bool bAllowRepair50 = sal_False )
        throw ();
#ifdef WNT
    sal_Bool SaveObject_Impl();
    sal_Bool OnShowWindow_Impl( sal_Bool bShow );
    void CreateOleComponent_Impl( OleComponent* pOleComponent = NULL );
    void CreateOleComponentAndLoad_Impl( OleComponent* pOleComponent = NULL );
    void CreateOleComponentFromClipboard_Impl( OleComponent* pOleComponent = NULL );
#endif
    void SetObjectIsLink_Impl( sal_Bool bIsLink ) { m_bIsLink = bIsLink; }

#ifdef WNT
    ::rtl::OUString CreateTempURLEmpty_Impl();
    ::rtl::OUString GetTempURL_Impl();
#endif
    ::rtl::OUString GetContainerName_Impl() { return m_aContainerName; }

    // the following 4 methods are related to switch to wrapping mode
    void MoveListeners();
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > CreateTemporarySubstorage( ::rtl::OUString& o_aStorageName );
    ::rtl::OUString MoveToTemporarySubstream();
    sal_Bool TryToConvertToOOo();

public:
    // in case a new object must be created the class ID must be specified
    OleEmbeddedObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID,
                        const ::rtl::OUString& aClassName );

    // in case object will be loaded from a persistent entry or from a file the class ID will be detected on loading
    // factory can do it for OOo objects, but for OLE objects OS dependent code is required
    OleEmbeddedObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                        sal_Bool bLink );
#ifdef WNT
    // this constructor let object be initialized from clipboard
    OleEmbeddedObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
#endif

    virtual ~OleEmbeddedObject();

#ifdef WNT
    void OnIconChanged_Impl();
    void OnViewChanged_Impl();
    void OnClosed_Impl();
#endif

// XEmbeddedObject

    virtual void SAL_CALL changeState( sal_Int32 nNewState )
        throw ( ::com::sun::star::embed::UnreachableStateException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getReachableStates()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getCurrentState()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL doVerb( sal_Int32 nVerbID )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::embed::UnreachableStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > SAL_CALL getSupportedVerbs()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setClientSite(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient >& xClient )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient > SAL_CALL getClientSite()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL update()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setUpdateMode( sal_Int32 nMode )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL getStatus( sal_Int64 nAspect )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setContainerName( const ::rtl::OUString& sName )
        throw ( ::com::sun::star::uno::RuntimeException );


// XVisualObject

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException);


// XEmbedPersist

    virtual void SAL_CALL setPersistentEntry(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeToEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::rtl::OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeAsEntry(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                const ::rtl::OUString& sEntName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL saveCompleted( sal_Bool bUseNew )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasEntry()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getEntryName()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

// XLinkageSupport

    virtual void SAL_CALL breakLink( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                     const ::rtl::OUString& sEntName )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isLink()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getLinkURL()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException);

// XCommonEmbedPersist
    virtual void SAL_CALL storeOwn()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isReadonly()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL reload(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

// XClassifiedObject

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getClassID()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getClassName()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setClassInfo(
                const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const ::rtl::OUString& aClassName )
        throw ( ::com::sun::star::lang::NoSupportException,
                ::com::sun::star::uno::RuntimeException );

// XStateChangeBroadcaster
    virtual void SAL_CALL addStateChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStateChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);


// XComponentSupplier

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent()
        throw ( ::com::sun::star::uno::RuntimeException );

// XCloseable

    virtual void SAL_CALL close( sal_Bool DeliverOwnership )
        throw ( ::com::sun::star::util::CloseVetoException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addCloseListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeCloseListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

// XEventBroadcaster
    virtual void SAL_CALL addEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

// XInplaceObject ( only for wrapping scenario here )

    virtual void SAL_CALL setObjectRectangles( const ::com::sun::star::awt::Rectangle& aPosRect,
                                          const ::com::sun::star::awt::Rectangle& aClipRect )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL enableModeless( sal_Bool bEnable )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL translateAccelerators(
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyEvent >& aKeys )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    // XChild ( only for wrapping scenario here )
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
