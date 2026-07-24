/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist2.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/chart2/XDefaultSizeTransmitter.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/weak.hxx>
#include <embeddedobj/embeddedupdate.hxx>
#include <rtl/ref.hxx>
#include <map>
#include <memory>
#include <svtools/filechangedchecker.hxx>
#include <unotools/resmgr.hxx>

namespace com::sun::star {
    namespace embed {
        class XStorage;
    }
    namespace beans {
        struct PropertyValue;
        struct NamedValue;
    }
}

namespace comphelper {
    class OMultiTypeInterfaceContainerHelper2;
}

namespace comphelper {
    class NamedValueCollection;
}

#define NUM_SUPPORTED_STATES 5
// #define NUM_SUPPORTED_VERBS 5

#include "docholder.hxx"

/**
 * Represents an OLE object that has native data and we loaded that data into a
 * document model successfully.
 */
class OCommonEmbeddedObject : public css::embed::XEmbeddedObject
                            , public ::embeddedobj::EmbeddedUpdate
                            , public css::embed::XEmbedPersist2
                            , public css::embed::XLinkageSupport
                            , public css::embed::XInplaceObject
                            , public css::container::XChild
                            , public css::chart2::XDefaultSizeTransmitter
                            , public css::lang::XServiceInfo
                            , public css::lang::XInitialization
                            , public css::lang::XTypeProvider
                            , public ::cppu::OWeakObject
{
protected:
    ::osl::Mutex    m_aMutex;

    rtl::Reference<embeddedobj::DocumentHolder> m_xDocHolder;

    std::unique_ptr<::comphelper::OMultiTypeInterfaceContainerHelper2> m_pInterfaceContainer;

    bool m_bReadOnly;

    bool m_bDisposed;
    bool m_bClosed;

    sal_Int32 m_nObjectState;
    sal_Int32 m_nTargetState; // should be -1 except during state changing
    sal_Int32 m_nUpdateMode;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    cpo::uno::Sequence< css::beans::PropertyValue > m_aDocMediaDescriptor;

    cpo::uno::Sequence< sal_Int8 > m_aClassID;
    OUString m_aClassName;

    OUString m_aDocServiceName;
    OUString m_aPresetFilterName;

    sal_Int64 m_nMiscStatus;

    cpo::uno::Sequence< css::embed::VerbDescriptor > m_aObjectVerbs;

    std::map< sal_Int32, sal_Int32 > m_aVerbTable;

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
    cpo::uno::Sequence< css::beans::PropertyValue > m_aNewDocMediaDescriptor;

    css::uno::Reference< css::awt::XWindow > m_xClientWindow; // ???
    css::awt::Rectangle m_aOwnRectangle;
    css::awt::Rectangle m_aClipRectangle;

    bool m_bIsLinkURL;
    bool m_bLinkTempFileChanged;
    ::std::unique_ptr< FileChangedChecker > m_pLinkFile;
    bool m_bOleUpdate;
    bool m_bInHndFunc;

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

    // tdf#141529 hold a cc of a linked OLE
    css::uno::Reference < css::io::XTempFile > m_aLinkTempFile;

    css::uno::Reference< css::uno::XInterface > m_xParent;

    bool m_bHasClonedSize; // the object has cached size
    css::awt::Size m_aClonedSize;
    sal_Int32 m_nClonedMapUnit;
    css::awt::Size m_aDefaultSizeForChart_In_100TH_MM;//#i103460# charts do not necessarily have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this member

private:
    void CommonInit_Impl( const cpo::uno::Sequence< css::beans::NamedValue >& aObjectProps );

    void LinkInit_Impl( const cpo::uno::Sequence< css::beans::NamedValue >& aObjectProps,
                        const cpo::uno::Sequence< css::beans::PropertyValue >& aMediaDescr,
                        const cpo::uno::Sequence< css::beans::PropertyValue >& aObjectDescr );


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

    // when State = CopyTempToLink        -> the user pressed the save button
    //                                       when change in embedded part then copy to the linked-file
    //              CopyLinkToTemp        -> the user pressed the refresh button
    //                                       when change in linked-file then copy to the embedded part (temp-file)
    //              CopyLinkToTempInit    -> create the temp file
    //              CopyLinkToTempRefresh -> when save and Link change but not temp then update temp
    enum class CopyBackToOLELink {NoCopy, CopyTempToLink, CopyLinkToTemp, CopyLinkToTempInit, CopyLinkToTempRefresh};

    void handleLinkedOLE( CopyBackToOLELink eState );

    void StateChangeNotification_Impl( bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState,::osl::ResettableMutexGuard& _rGuard );

    void SwitchStateTo_Impl( sal_Int32 nNextState );

    cpo::uno::Sequence< sal_Int32 > const & GetIntermediateStatesSequence_Impl( sal_Int32 nNewState );

    OUString GetFilterName( sal_Int32 nVersion ) const;
    css::uno::Reference< css::util::XCloseable > LoadDocumentFromStorage_Impl();

    css::uno::Reference< css::util::XCloseable > LoadLink_Impl();

    css::uno::Reference< css::util::XCloseable > InitNewDocument_Impl();

    void StoreDocToStorage_Impl(
        const css::uno::Reference<css::embed::XStorage>& xStorage,
        const cpo::uno::Sequence<css::beans::PropertyValue>& rMediaArgs,
        const cpo::uno::Sequence<css::beans::PropertyValue>& rObjArgs,
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
                        const cpo::uno::Sequence< css::beans::PropertyValue >& aMedDescr );

    css::uno::Reference< css::util::XCloseable > CreateTempDocFromLink_Impl();

    OUString GetBaseURL_Impl() const;
    static OUString GetBaseURLFrom_Impl(
                    const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                    const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs );

    int ShowMsgDialog(TranslateId Msg, const OUString& sFileName);

    bool getAllowLinkUpdate() const;

protected:
    void SetInplaceActiveState();

public:
    OCommonEmbeddedObject(
        css::uno::Reference< css::uno::XComponentContext > xContext,
        const cpo::uno::Sequence< css::beans::NamedValue >& aObjectProps );

    // no persistence for linked objects, so the descriptors are provided in constructor
    OCommonEmbeddedObject(
        css::uno::Reference< css::uno::XComponentContext > xContext,
        const cpo::uno::Sequence< css::beans::NamedValue >& aObjectProps,
        const cpo::uno::Sequence< css::beans::PropertyValue >& aMediaDescr,
        const cpo::uno::Sequence< css::beans::PropertyValue >& aObjectDescr );

    virtual ~OCommonEmbeddedObject() override;

    void SaveObject_Impl();

    void requestPositioning( const css::awt::Rectangle& aRect );

    // not a real listener and should not be
    void PostEvent_Impl( const OUString& aEventName );

    OUString const & getContainerName() const { return m_aContainerName; }
// XInterface

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override ;

    virtual void acquire()
        noexcept override;

    virtual void release()
        noexcept override;

// XEmbeddedObject

    virtual void changeState( sal_Int32 nNewState ) override;

    virtual cpo::uno::Sequence< sal_Int32 > getReachableStates() override;

    virtual sal_Int32 getCurrentState() override;

    virtual void doVerb( sal_Int32 nVerbID ) override;

    virtual cpo::uno::Sequence< css::embed::VerbDescriptor > getSupportedVerbs() override;

    virtual void setClientSite(
                const css::uno::Reference< css::embed::XEmbeddedClient >& xClient ) override;

    virtual css::uno::Reference< css::embed::XEmbeddedClient > getClientSite() override;

    virtual void update() override;

    virtual void setUpdateMode( sal_Int32 nMode ) override;

    virtual sal_Int64 getStatus( sal_Int64 nAspect ) override;

    virtual void setContainerName( const OUString& sName ) override;

// EmbeddedUpdate

    virtual void SetOleState(bool bIsOleUpdate) override;


// XVisualObject

    virtual void setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize ) override;

    virtual css::awt::Size getVisualAreaSize( sal_Int64 nAspect ) override;

    virtual css::embed::VisualRepresentation getPreferredVisualRepresentation( ::sal_Int64 nAspect ) override;

    virtual sal_Int32 getMapUnit( sal_Int64 nAspect ) override;

// XEmbedPersist

    virtual void setPersistentEntry(
                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                    const OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                    const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) override;

    virtual void storeToEntry( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments, const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) override;

    virtual void storeAsEntry(
                const css::uno::Reference< css::embed::XStorage >& xStorage,
                const OUString& sEntName,
                const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) override;

    virtual void saveCompleted( bool bUseNew ) override;

    virtual bool hasEntry() override;

    virtual OUString getEntryName() override;

// XLinkageSupport

    virtual void breakLink( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                     const OUString& sEntName ) override;

    virtual bool isLink() override;

    virtual OUString getLinkURL() override;


// XCommonEmbedPersist

    virtual void storeOwn() override;

    virtual bool isReadonly() override;

    virtual void reload(
                const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) override;

// XEmbedPersist2

    virtual bool isStored() override;

// XInplaceObject

    virtual void setObjectRectangles( const css::awt::Rectangle& aPosRect,
                                          const css::awt::Rectangle& aClipRect ) override;

    virtual void enableModeless( bool bEnable ) override;

    virtual void translateAccelerators(
                    const cpo::uno::Sequence< css::awt::KeyEvent >& aKeys ) override;

// XClassifiedObject

    virtual cpo::uno::Sequence< sal_Int8 > getClassID() override;

    virtual OUString getClassName() override;

    virtual void setClassInfo(
                const cpo::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName ) override;


// XComponentSupplier

    virtual css::uno::Reference< css::util::XCloseable > getComponent() override;

// XStateChangeBroadcaster
    virtual void addStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) override;
    virtual void removeStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) override;

// XCloseable

    virtual void close( bool DeliverOwnership ) override;

    virtual void addCloseListener(
                const css::uno::Reference< css::util::XCloseListener >& Listener ) override;

    virtual void removeCloseListener(
                const css::uno::Reference< css::util::XCloseListener >& Listener ) override;

// XEventBroadcaster
    virtual void addEventListener(
                const css::uno::Reference< css::document::XEventListener >& Listener ) override;

    virtual void removeEventListener(
                const css::uno::Reference< css::document::XEventListener >& Listener ) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > getParent(  ) override;
    virtual void setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    // XDefaultSizeTransmitter
    //#i103460# charts do not necessarily have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    virtual void setDefaultSize( const css::awt::Size& rSize_100TH_MM ) override;

    // XServiceInfo
    OUString getImplementationName() override;
    bool supportsService( const OUString& ServiceName ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XInitialization
    void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    // XTypeProvider
    cpo::uno::Sequence<cpo::uno::Type> getTypes() override;
    cpo::uno::Sequence<sal_Int8> getImplementationId() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
