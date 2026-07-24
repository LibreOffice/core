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

#include <sal/config.h>

#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/multicontainer2.hxx>

namespace com::sun::star {
    namespace embed {
        class XStorage;
    }
}

/**
 * Represents an OLE object that has native data (next to the replacement
 * image), but we don't understand that data.
 */
class ODummyEmbeddedObject : public ::cppu::WeakImplHelper
                        < css::embed::XEmbeddedObject
                        , css::embed::XEmbedPersist
                        , css::lang::XServiceInfo >
{
    ::osl::Mutex    m_aMutex;
    std::unique_ptr<comphelper::OMultiTypeInterfaceContainerHelper2>
        m_pInterfaceContainer;
    bool m_bDisposed;

    OUString m_aEntryName;
    css::uno::Reference< css::embed::XStorage > m_xParentStorage;
    sal_Int32 m_nObjectState;

    css::uno::Reference< css::embed::XEmbeddedClient > m_xClientSite;

    sal_Int64 m_nCachedAspect;
    css::awt::Size m_aCachedSize;
    bool m_bHasCachedSize;

    // following information will be used between SaveAs and SaveCompleted
    bool m_bWaitSaveCompleted;
    OUString m_aNewEntryName;
    css::uno::Reference< css::embed::XStorage > m_xNewParentStorage;

protected:
    void CheckInit_WrongState();    //throw WrongStateException on m_nObjectState == -1
    void CheckInit_Runtime();       //throw RuntimeException on m_nObjectState == -1
    void PostEvent_Impl( const OUString& aEventName );

public:

    ODummyEmbeddedObject()
    : m_bDisposed( false )
    , m_nObjectState( -1 )
    , m_nCachedAspect( 0 )
    , m_bHasCachedSize( false )
    , m_bWaitSaveCompleted( false )
    {}

    virtual ~ODummyEmbeddedObject() override;

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


// XCommonEmbedPersist

    virtual void storeOwn() override;

    virtual bool isReadonly() override;

    virtual void reload(
                const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) override;


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

    // XServiceInfo
    OUString getImplementationName() override;
    bool supportsService( const OUString& ServiceName ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
