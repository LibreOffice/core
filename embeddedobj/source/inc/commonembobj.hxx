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

#ifndef _INC_COMMONEMBOBJ_HXX_
#define _INC_COMMONEMBOBJ_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/embed/XStateChangeBroadcaster.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/chart2/XDefaultSizeTransmitter.hpp>
#include <cppuhelper/weak.hxx>

namespace com { namespace sun { namespace star {
    namespace embed {
        class XStorage;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace util {
        class XCloseListener;
    }
    namespace beans {
        struct PropertyValue;
        struct NamedValue;
    }
}}}

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

namespace comphelper {
    class NamedValueCollection;
}

#define NUM_SUPPORTED_STATES 5
// #define NUM_SUPPORTED_VERBS 5

#include "docholder.hxx"

class Interceptor;

class OCommonEmbeddedObject : public ::com::sun::star::embed::XEmbeddedObject
                            , public ::com::sun::star::embed::XEmbedPersist
                            , public ::com::sun::star::embed::XLinkageSupport
                            , public ::com::sun::star::embed::XInplaceObject
                            , public ::com::sun::star::container::XChild
                            , public ::com::sun::star::chart2::XDefaultSizeTransmitter
                            , public ::cppu::OWeakObject
{
protected:
    ::osl::Mutex    m_aMutex;

    DocumentHolder* m_pDocHolder;

    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    sal_Bool m_bReadOnly;

    sal_Bool m_bDisposed;
    sal_Bool m_bClosed;

    sal_Int32 m_nObjectState;
    sal_Int32 m_nTargetState; // should be -1 exept during state changing
    sal_Int32 m_nUpdateMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aDocMediaDescriptor;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aClassID;
    ::rtl::OUString m_aClassName;

    ::rtl::OUString m_aDocServiceName;
    ::rtl::OUString m_aPresetFilterName;

    sal_Int64 m_nMiscStatus;

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > m_aObjectVerbs;

    ::com::sun::star::uno::Sequence< sal_Int32 > m_aAcceptedStates;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_pIntermediateStatesSeqs[NUM_SUPPORTED_STATES][NUM_SUPPORTED_STATES];
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > > m_aVerbTable;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient > m_xClientSite;

    ::rtl::OUString m_aContainerName;
    ::rtl::OUString m_aDefaultParentBaseURL;
    ::rtl::OUString m_aModuleName;
    sal_Bool        m_bEmbeddedScriptSupport;
    sal_Bool        m_bDocumentRecoverySupport;

    Interceptor* m_pInterceptor;

    // following information will be used between SaveAs and SaveCompleted
    sal_Bool m_bWaitSaveCompleted;
    ::rtl::OUString m_aNewEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xNewParentStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xNewObjectStorage;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aNewDocMediaDescriptor;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xClientWindow; // ???
    ::com::sun::star::awt::Rectangle m_aOwnRectangle;
    ::com::sun::star::awt::Rectangle m_aClipRectangle;

    sal_Bool m_bIsLink;

    // embedded object related stuff
    ::rtl::OUString m_aEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xParentStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xObjectStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xRecoveryStorage;

    // link related stuff
    ::rtl::OUString m_aLinkURL;
    ::rtl::OUString m_aLinkFilterName;
    sal_Bool        m_bLinkHasPassword;
    ::rtl::OUString m_aLinkPassword;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;

    sal_Bool m_bHasClonedSize; // the object has cached size
    ::com::sun::star::awt::Size m_aClonedSize;
    sal_Int32 m_nClonedMapUnit;
    ::com::sun::star::awt::Size m_aDefaultSizeForChart_In_100TH_MM;//#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this member

private:
    void CommonInit_Impl( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aObjectProps );

    void LinkInit_Impl( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aObjectProps,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aObjectDescr );


    void SwitchOwnPersistence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewParentStorage,
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewObjectStorage,
                const ::rtl::OUString& aNewName );

    void SwitchOwnPersistence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewParentStorage,
                const ::rtl::OUString& aNewName );

    ::rtl::OUString GetDocumentServiceName() const { return m_aDocServiceName; }
    ::rtl::OUString GetPresetFilterName() const { return m_aPresetFilterName; }

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        StoreDocumentToTempStream_Impl( sal_Int32 nStorageFormat,
                                        const ::rtl::OUString& aBaseURL,
                                        const ::rtl::OUString& aHierarchName );

    sal_Int32 ConvertVerbToState_Impl( sal_Int32 nVerb );

    void Deactivate();

    void StateChangeNotification_Impl( sal_Bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState,::osl::ResettableMutexGuard& _rGuard );

    void SwitchStateTo_Impl( sal_Int32 nNextState );

    ::com::sun::star::uno::Sequence< sal_Int32 > GetIntermediateStatesSequence_Impl( sal_Int32 nNewState );

    ::rtl::OUString GetFilterName( sal_Int32 nVersion ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > LoadDocumentFromStorage_Impl();

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > LoadLink_Impl();

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > InitNewDocument_Impl();

    void StoreDocToStorage_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                sal_Int32 nStorageVersion,
                                const ::rtl::OUString& aBaseURL,
                                const ::rtl::OUString& aHierarchName,
                                sal_Bool bAttachToStorage );

    void SwitchDocToStorage_Impl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageBasedDocument >& xDoc,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    void FillDefaultLoadArgs_Impl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rxStorage,
                  ::comphelper::NamedValueCollection& o_rLoadArgs
        ) const;

    void EmbedAndReparentDoc_Impl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >& i_rxDocument
        ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > CreateDocFromMediaDescr_Impl(
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMedDescr );

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > CreateTempDocFromLink_Impl();

    ::rtl::OUString GetBaseURL_Impl() const;
    ::rtl::OUString GetBaseURLFrom_Impl(
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs );

public:
    OCommonEmbeddedObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aObjectProps );

    // no persistance for linked objects, so the descriptors are provided in constructor
    OCommonEmbeddedObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aObjectProps,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aObjectDescr );

    virtual ~OCommonEmbeddedObject();

    void SaveObject_Impl();

    void requestPositioning( const ::com::sun::star::awt::Rectangle& aRect );

    // not a real listener and should not be
    void PostEvent_Impl( const ::rtl::OUString& aEventName );

// XInterface

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException ) ;

    virtual void SAL_CALL acquire()
        throw();

    virtual void SAL_CALL release()
        throw();

// XTypeProvider

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException ) ;

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException ) ;

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


// XInplaceObject

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

// XClassifiedObject

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getClassID()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getClassName()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setClassInfo(
                const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const ::rtl::OUString& aClassName )
        throw ( ::com::sun::star::lang::NoSupportException,
                ::com::sun::star::uno::RuntimeException );


// XComponentSupplier

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent()
        throw ( ::com::sun::star::uno::RuntimeException );

// XStateChangeBroadcaster
    virtual void SAL_CALL addStateChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStateChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

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

    // XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // XDefaultSizeTransmitter
    //#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    virtual void SAL_CALL setDefaultSize( const ::com::sun::star::awt::Size& rSize_100TH_MM ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
