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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_INC_DUMMYOBJECT_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_INC_DUMMYOBJECT_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <cppuhelper/implbase.hxx>

namespace com { namespace sun { namespace star {
    namespace embed {
        class XStorage;
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

class ODummyEmbeddedObject : public ::cppu::WeakImplHelper
                        < css::embed::XEmbeddedObject
                        , css::embed::XEmbedPersist >
{
    ::osl::Mutex    m_aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;
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
    : m_pInterfaceContainer( NULL )
    , m_bDisposed( false )
    , m_nObjectState( -1 )
    , m_nCachedAspect( 0 )
    , m_bHasCachedSize( false )
    , m_bWaitSaveCompleted( false )
    {}

    virtual ~ODummyEmbeddedObject();

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


// XComponentSupplier

    virtual css::uno::Reference< css::util::XCloseable > SAL_CALL getComponent()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XStateChangeBroadcaster
    virtual void SAL_CALL addStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeStateChangeListener( const css::uno::Reference< css::embed::XStateChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
