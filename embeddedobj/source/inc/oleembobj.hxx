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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_INC_OLEEMBOBJ_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_INC_OLEEMBOBJ_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
#include <cppuhelper/implbase.hxx>

#include <osl/thread.h>

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class VerbExecutionController
{
    // the following mutex is allowed to be locked only for variables initialization, so no deadlock can be caused
    ::osl::Mutex    m_aVerbExecutionMutex;

    sal_Int32 m_nNotificationLock;

#ifdef WNT
    bool m_bWasEverActive;
    bool m_bVerbExecutionInProgress;
    oslThreadIdentifier m_nVerbExecutionThreadIdentifier;
    sal_Bool m_bChangedOnVerbExecution;
#endif

public:

    VerbExecutionController()
    : m_nNotificationLock( 0 )
#ifdef WNT
    , m_bWasEverActive( false )
    , m_bVerbExecutionInProgress( false )
    , m_nVerbExecutionThreadIdentifier( 0 )
    , m_bChangedOnVerbExecution( sal_False )
#endif
    {}

#ifdef WNT
    void StartControlExecution();
    sal_Bool EndControlExecution_WasModified();
    void ModificationNotificationIsDone();
    // no need to lock anything to check the value of the numeric members
    bool CanDoNotification() { return ( !m_bVerbExecutionInProgress && !m_bWasEverActive && !m_nNotificationLock ); }
    // ... or to change it
    void ObjectIsActive() { m_bWasEverActive = true; }
#endif
    void LockNotification();
    void UnlockNotification();
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
class OleEmbeddedObject : public ::cppu::WeakImplHelper
                        < css::embed::XEmbeddedObject
                        , css::embed::XEmbedPersist
                        , css::embed::XLinkageSupport
                        , css::embed::XInplaceObject
                        , css::container::XChild >
{
    friend class OleComponent;

    ::osl::Mutex    m_aMutex;

    OleComponent*   m_pOleComponent;

    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    bool m_bReadOnly;

    bool m_bDisposed;
    sal_Int32 m_nObjectState;
    sal_Int32 m_nTargetState;
    sal_Int32 m_nUpdateMode;

    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

    css::uno::Sequence< sal_Int8 > m_aClassID;
    OUString m_aClassName;

    css::uno::Reference< css::embed::XEmbeddedClient > m_xClientSite;

    OUString m_aContainerName;

    css::uno::Reference< css::util::XCloseListener > m_xClosePreventer;

    bool m_bWaitSaveCompleted;
    bool m_bNewVisReplInStream;
    css::uno::Reference< css::io::XStream > m_xNewCachedVisRepl;
    OUString m_aNewEntryName;
    css::uno::Reference< css::embed::XStorage > m_xNewParentStorage;
    css::uno::Reference< css::io::XStream > m_xNewObjectStream;
    bool m_bStoreLoaded;

    css::uno::Reference< css::io::XStream > m_xCachedVisualRepresentation;
    bool m_bVisReplInitialized;
    bool m_bVisReplInStream;
    bool m_bStoreVisRepl;

    bool m_bIsLink;

    // TODO/LATER: may need to cache more than one aspect in future
    bool m_bHasCachedSize; // the object has cached size
    css::awt::Size m_aCachedSize;
    sal_Int64 m_nCachedAspect;

    bool m_bHasSizeToSet;  // the object has cached size that should be set to OLE component
    css::awt::Size m_aSizeToSet; // this size might be different from the cached one ( scaling is applied )
    sal_Int64 m_nAspectToSet;


    // cache the status of the object
    // TODO/LATER: may need to cache more than one aspect in future
    bool m_bGotStatus;
    sal_Int64 m_nStatus;
    sal_Int64 m_nStatusAspect;

    // embedded object related stuff
    OUString m_aEntryName;
    css::uno::Reference< css::embed::XStorage > m_xParentStorage;
    css::uno::Reference< css::io::XStream > m_xObjectStream;

    // link related stuff
    OUString m_aLinkURL; // ???

    // points to own view provider if the object has no server
    OwnView_Impl*   m_pOwnView;

    // whether the object should be initialized from clipboard in case of default initialization
    bool m_bFromClipboard;

    OUString m_aTempURL;

    OUString m_aTempDumpURL;

    // STAMPIT solution
    // the following member is used during verb execution to detect whether the verb execution modifies the object
    VerbExecutionController m_aVerbExecutionController;

    // if the following member is set, the object works in wrapper mode
    css::uno::Reference< css::embed::XEmbeddedObject > m_xWrappedObject;
    bool m_bTriedConversion;
    OUString m_aFilterName; // if m_bTriedConversion, then the filter detected by that

    css::uno::Reference< css::uno::XInterface > m_xParent;

protected:

    css::uno::Reference< css::io::XStream > TryToGetAcceptableFormat_Impl(
                                    const css::uno::Reference< css::io::XStream >& xStream )
        throw ( css::uno::Exception );

    css::uno::Reference< css::io::XStream > GetNewFilledTempStream_Impl(
                                    const css::uno::Reference< css::io::XInputStream >& xInStream )
        throw( css::io::IOException, css::uno::RuntimeException );
#ifdef WNT
    void SwitchComponentToRunningState_Impl();
#endif
    void MakeEventListenerNotification_Impl( const OUString& aEventName );
#ifdef WNT
    void StateChangeNotification_Impl( sal_Bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState );
    css::uno::Reference< css::io::XOutputStream > GetStreamForSaving();


    css::uno::Sequence< sal_Int32 > GetIntermediateVerbsSequence_Impl( sal_Int32 nNewState );

    css::uno::Sequence< sal_Int32 > GetReachableStatesList_Impl(
                        const css::uno::Sequence< css::embed::VerbDescriptor >& aVerbList );
#endif

    void Dispose();

    void SwitchOwnPersistence(
                const css::uno::Reference< css::embed::XStorage >& xNewParentStorage,
                const css::uno::Reference< css::io::XStream >& xNewObjectStream,
                const OUString& aNewName );

    void SwitchOwnPersistence(
                const css::uno::Reference< css::embed::XStorage >& xNewParentStorage,
                const OUString& aNewName );

    void GetRidOfComponent();

    void StoreToLocation_Impl(
                            const css::uno::Reference< css::embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs,
                            bool bSaveAs )
        throw ( css::uno::Exception );
#ifdef WNT
    void StoreObjectToStream( css::uno::Reference< css::io::XOutputStream > xOutStream )
        throw ( css::uno::Exception );
#endif
    void InsertVisualCache_Impl(
            const css::uno::Reference< css::io::XStream >& xTargetStream,
            const css::uno::Reference< css::io::XStream >& xCachedVisualRepresentation )
        throw ( css::uno::Exception );

    void RemoveVisualCache_Impl( const css::uno::Reference< css::io::XStream >& xTargetStream )
        throw ( css::uno::Exception );

    void SetVisReplInStream( bool bExists );
    bool HasVisReplInStream();

    css::embed::VisualRepresentation GetVisualRepresentationInNativeFormat_Impl(
                    const css::uno::Reference< css::io::XStream > xCachedVisRepr )
        throw ( css::uno::Exception );

    css::uno::Reference< css::io::XStream > TryToRetrieveCachedVisualRepresentation_Impl(
                    const css::uno::Reference< css::io::XStream >& xStream,
                    bool bAllowRepair50 = false )
        throw ();
#ifdef WNT
    sal_Bool SaveObject_Impl();
    sal_Bool OnShowWindow_Impl( sal_Bool bShow );
    void CreateOleComponent_Impl( OleComponent* pOleComponent = NULL );
    void CreateOleComponentAndLoad_Impl( OleComponent* pOleComponent = NULL );
    void CreateOleComponentFromClipboard_Impl( OleComponent* pOleComponent = NULL );
    OUString CreateTempURLEmpty_Impl();
    OUString GetTempURL_Impl();
    void SetObjectIsLink_Impl( bool bIsLink ) { m_bIsLink = bIsLink; }
#endif

    // the following 4 methods are related to switch to wrapping mode
    void MoveListeners();
    css::uno::Reference< css::embed::XStorage > CreateTemporarySubstorage( OUString& o_aStorageName );
    OUString MoveToTemporarySubstream();
    bool TryToConvertToOOo();

public:
    // in case a new object must be created the class ID must be specified
    OleEmbeddedObject( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                        const css::uno::Sequence< sal_Int8 >& aClassID,
                        const OUString& aClassName );

    // in case object will be loaded from a persistent entry or from a file the class ID will be detected on loading
    // factory can do it for OOo objects, but for OLE objects OS dependent code is required
    OleEmbeddedObject( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                        bool bLink );
#ifdef WNT
    // this constructor let object be initialized from clipboard
    OleEmbeddedObject( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
#endif

    virtual ~OleEmbeddedObject();

#ifdef WNT
    void OnIconChanged_Impl();
    void OnViewChanged_Impl();
    void OnClosed_Impl();
#endif

// XEmbeddedObject

    virtual void SAL_CALL changeState( sal_Int32 nNewState )
        throw ( css::embed::UnreachableStateException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getReachableStates()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getCurrentState()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL doVerb( sal_Int32 nVerbID )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::embed::UnreachableStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual css::uno::Sequence< css::embed::VerbDescriptor > SAL_CALL getSupportedVerbs()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL setClientSite(
                const css::uno::Reference< css::embed::XEmbeddedClient >& xClient )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual css::uno::Reference< css::embed::XEmbeddedClient > SAL_CALL getClientSite()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL update()
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL setUpdateMode( sal_Int32 nMode )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Int64 SAL_CALL getStatus( sal_Int64 nAspect )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL setContainerName( const OUString& sName )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


// XVisualObject

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual css::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
        throw ( css::uno::Exception,
                css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


// XEmbedPersist

    virtual void SAL_CALL setPersistentEntry(
                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                    const OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                    const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL storeToEntry( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& lArguments, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL storeAsEntry(
                const css::uno::Reference< css::embed::XStorage >& xStorage,
                const OUString& sEntName,
                const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL saveCompleted( sal_Bool bUseNew )
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasEntry()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual OUString SAL_CALL getEntryName()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XLinkageSupport

    virtual void SAL_CALL breakLink( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                     const OUString& sEntName )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isLink()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getLinkURL()
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XCommonEmbedPersist
    virtual void SAL_CALL storeOwn()
        throw ( css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isReadonly()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL reload(
                const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XClassifiedObject

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getClassID()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual OUString SAL_CALL getClassName()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL setClassInfo(
                const css::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName )
        throw ( css::lang::NoSupportException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XStateChangeBroadcaster
    virtual void SAL_CALL addStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


// XComponentSupplier

    virtual css::uno::Reference< css::util::XCloseable > SAL_CALL getComponent()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XCloseable

    virtual void SAL_CALL close( sal_Bool DeliverOwnership )
        throw ( css::util::CloseVetoException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL addCloseListener(
                const css::uno::Reference< css::util::XCloseListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL removeCloseListener(
                const css::uno::Reference< css::util::XCloseListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XEventBroadcaster
    virtual void SAL_CALL addEventListener(
                const css::uno::Reference< css::document::XEventListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL removeEventListener(
                const css::uno::Reference< css::document::XEventListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XInplaceObject ( only for wrapping scenario here )

    virtual void SAL_CALL setObjectRectangles( const css::awt::Rectangle& aPosRect,
                                          const css::awt::Rectangle& aClipRect )
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL enableModeless( sal_Bool bEnable )
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL translateAccelerators(
                    const css::uno::Sequence< css::awt::KeyEvent >& aKeys )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XChild ( only for wrapping scenario here )
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
