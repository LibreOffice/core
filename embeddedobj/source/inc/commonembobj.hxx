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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_INC_COMMONEMBOBJ_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_INC_COMMONEMBOBJ_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XEmbedPersist2.hpp>
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

class OCommonEmbeddedObject : public css::embed::XEmbeddedObject
                            , public css::embed::XEmbedPersist2
                            , public css::embed::XLinkageSupport
                            , public css::embed::XInplaceObject
                            , public css::container::XChild
                            , public css::chart2::XDefaultSizeTransmitter
                            , public ::cppu::OWeakObject
{
protected:
    ::osl::Mutex    m_aMutex;

    DocumentHolder* m_pDocHolder;

    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    bool m_bReadOnly;

    bool m_bDisposed;
    bool m_bClosed;

    sal_Int32 m_nObjectState;
    sal_Int32 m_nTargetState; // should be -1 except during state changing
    sal_Int32 m_nUpdateMode;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    css::uno::Sequence< css::beans::PropertyValue > m_aDocMediaDescriptor;

    css::uno::Sequence< sal_Int8 > m_aClassID;
    OUString m_aClassName;

    OUString m_aDocServiceName;
    OUString m_aPresetFilterName;

    sal_Int64 m_nMiscStatus;

    css::uno::Sequence< css::embed::VerbDescriptor > m_aObjectVerbs;

    css::uno::Sequence< sal_Int32 > m_aAcceptedStates;
    css::uno::Sequence< sal_Int32 > m_pIntermediateStatesSeqs[NUM_SUPPORTED_STATES][NUM_SUPPORTED_STATES];
    css::uno::Sequence< css::uno::Sequence< sal_Int32 > > m_aVerbTable;

    css::uno::Reference< css::embed::XEmbeddedClient > m_xClientSite;

    OUString m_aContainerName;
    OUString m_aDefaultParentBaseURL;
    OUString m_aModuleName;
    bool        m_bEmbeddedScriptSupport;
    bool        m_bDocumentRecoverySupport;

    // following information will be used between SaveAs and SaveCompleted
    bool m_bWaitSaveCompleted;
    OUString m_aNewEntryName;
    css::uno::Reference< css::embed::XStorage > m_xNewParentStorage;
    css::uno::Reference< css::embed::XStorage > m_xNewObjectStorage;
    css::uno::Sequence< css::beans::PropertyValue > m_aNewDocMediaDescriptor;

    css::uno::Reference< css::awt::XWindow > m_xClientWindow; // ???
    css::awt::Rectangle m_aOwnRectangle;
    css::awt::Rectangle m_aClipRectangle;

    bool m_bIsLink;

    // embedded object related stuff
    OUString m_aEntryName;
    css::uno::Reference< css::embed::XStorage > m_xParentStorage;
    css::uno::Reference< css::embed::XStorage > m_xObjectStorage;
    css::uno::Reference< css::embed::XStorage > m_xRecoveryStorage;

    // link related stuff
    OUString m_aLinkURL;
    OUString m_aLinkFilterName;
    bool        m_bLinkHasPassword;
    OUString m_aLinkPassword;

    css::uno::Reference< css::uno::XInterface > m_xParent;

    bool m_bHasClonedSize; // the object has cached size
    css::awt::Size m_aClonedSize;
    sal_Int32 m_nClonedMapUnit;
    css::awt::Size m_aDefaultSizeForChart_In_100TH_MM;//#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this member

private:
    void CommonInit_Impl( const css::uno::Sequence< css::beans::NamedValue >& aObjectProps );

    void LinkInit_Impl( const css::uno::Sequence< css::beans::NamedValue >& aObjectProps,
                        const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr,
                        const css::uno::Sequence< css::beans::PropertyValue >& aObjectDescr );


    void SwitchOwnPersistence(
                const css::uno::Reference< css::embed::XStorage >& xNewParentStorage,
                const css::uno::Reference< css::embed::XStorage >& xNewObjectStorage,
                const OUString& aNewName );

    void SwitchOwnPersistence(
                const css::uno::Reference< css::embed::XStorage >& xNewParentStorage,
                const OUString& aNewName );

    const OUString& GetDocumentServiceName() const { return m_aDocServiceName; }
    const OUString& GetPresetFilterName() const { return m_aPresetFilterName; }

    css::uno::Reference< css::io::XInputStream >
        StoreDocumentToTempStream_Impl( sal_Int32 nStorageFormat,
                                        const OUString& aBaseURL,
                                        const OUString& aHierarchName );

    sal_Int32 ConvertVerbToState_Impl( sal_Int32 nVerb );

    void Deactivate();

    void StateChangeNotification_Impl( bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState,::osl::ResettableMutexGuard& _rGuard );

    void SwitchStateTo_Impl( sal_Int32 nNextState );

    css::uno::Sequence< sal_Int32 > const & GetIntermediateStatesSequence_Impl( sal_Int32 nNewState );

    OUString GetFilterName( sal_Int32 nVersion ) const;
    css::uno::Reference< css::util::XCloseable > LoadDocumentFromStorage_Impl();

    css::uno::Reference< css::util::XCloseable > LoadLink_Impl();

    css::uno::Reference< css::util::XCloseable > InitNewDocument_Impl();

    void StoreDocToStorage_Impl(
        const css::uno::Reference<css::embed::XStorage>& xStorage,
        const css::uno::Sequence<css::beans::PropertyValue>& rMediaArgs,
        const css::uno::Sequence<css::beans::PropertyValue>& rObjArgs,
        sal_Int32 nStorageVersion,
        const OUString& aHierarchName,
        bool bAttachToStorage );

    void SwitchDocToStorage_Impl(
            const css::uno::Reference< css::document::XStorageBasedDocument >& xDoc,
            const css::uno::Reference< css::embed::XStorage >& xStorage );

    void FillDefaultLoadArgs_Impl(
            const css::uno::Reference< css::embed::XStorage >& i_rxStorage,
                  ::comphelper::NamedValueCollection& o_rLoadArgs
        ) const;

    void EmbedAndReparentDoc_Impl(
            const css::uno::Reference< css::util::XCloseable >& i_rxDocument
        ) const;

    css::uno::Reference< css::util::XCloseable > CreateDocFromMediaDescr_Impl(
                        const css::uno::Sequence< css::beans::PropertyValue >& aMedDescr );

    css::uno::Reference< css::util::XCloseable > CreateTempDocFromLink_Impl();

    OUString GetBaseURL_Impl() const;
    static OUString GetBaseURLFrom_Impl(
                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                    const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs );

public:
    OCommonEmbeddedObject(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Sequence< css::beans::NamedValue >& aObjectProps );

    // no persistence for linked objects, so the descriptors are provided in constructor
    OCommonEmbeddedObject(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Sequence< css::beans::NamedValue >& aObjectProps,
        const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr,
        const css::uno::Sequence< css::beans::PropertyValue >& aObjectDescr );

    virtual ~OCommonEmbeddedObject() override;

    void SaveObject_Impl();

    void requestPositioning( const css::awt::Rectangle& aRect );

    // not a real listener and should not be
    void PostEvent_Impl( const OUString& aEventName );

// XInterface

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException, std::exception ) override ;

    virtual void SAL_CALL acquire()
        throw() override;

    virtual void SAL_CALL release()
        throw() override;

// XEmbeddedObject

    virtual void SAL_CALL changeState( sal_Int32 nNewState )
        throw ( css::embed::UnreachableStateException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getReachableStates()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getCurrentState()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL doVerb( sal_Int32 nVerbID )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::embed::UnreachableStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< css::embed::VerbDescriptor > SAL_CALL getSupportedVerbs()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setClientSite(
                const css::uno::Reference< css::embed::XEmbeddedClient >& xClient )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::embed::XEmbeddedClient > SAL_CALL getClientSite()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL update()
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setUpdateMode( sal_Int32 nMode )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int64 SAL_CALL getStatus( sal_Int64 nAspect )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setContainerName( const OUString& sName )
        throw ( css::uno::RuntimeException, std::exception ) override;


// XVisualObject

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
        throw ( css::uno::Exception,
                css::uno::RuntimeException, std::exception) override;

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
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL storeToEntry( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& lArguments, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL storeAsEntry(
                const css::uno::Reference< css::embed::XStorage >& xStorage,
                const OUString& sEntName,
                const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL saveCompleted( sal_Bool bUseNew )
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasEntry()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL getEntryName()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

// XLinkageSupport

    virtual void SAL_CALL breakLink( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                     const OUString& sEntName ) override;

    virtual sal_Bool SAL_CALL isLink() override;

    virtual OUString SAL_CALL getLinkURL()
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception) override;


// XCommonEmbedPersist

    virtual void SAL_CALL storeOwn()
        throw ( css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isReadonly()
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL reload(
                const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

// XEmbedPersist2

    virtual sal_Bool SAL_CALL isStored()
        throw (css::uno::RuntimeException, std::exception) override;

// XInplaceObject

    virtual void SAL_CALL setObjectRectangles( const css::awt::Rectangle& aPosRect,
                                          const css::awt::Rectangle& aClipRect )
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL enableModeless( sal_Bool bEnable )
        throw ( css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL translateAccelerators(
                    const css::uno::Sequence< css::awt::KeyEvent >& aKeys )
        throw ( css::embed::WrongStateException,
                css::uno::RuntimeException, std::exception ) override;

// XClassifiedObject

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getClassID()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL getClassName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setClassInfo(
                const css::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName )
        throw ( css::lang::NoSupportException,
                css::uno::RuntimeException, std::exception ) override;


// XComponentSupplier

    virtual css::uno::Reference< css::util::XCloseable > SAL_CALL getComponent()
        throw ( css::uno::RuntimeException, std::exception ) override;

// XStateChangeBroadcaster
    virtual void SAL_CALL addStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

// XCloseable

    virtual void SAL_CALL close( sal_Bool DeliverOwnership )
        throw ( css::util::CloseVetoException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addCloseListener(
                const css::uno::Reference< css::util::XCloseListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeCloseListener(
                const css::uno::Reference< css::util::XCloseListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) override;

// XEventBroadcaster
    virtual void SAL_CALL addEventListener(
                const css::uno::Reference< css::document::XEventListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeEventListener(
                const css::uno::Reference< css::document::XEventListener >& Listener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // XDefaultSizeTransmitter
    //#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    virtual void SAL_CALL setDefaultSize( const css::awt::Size& rSize_100TH_MM ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
