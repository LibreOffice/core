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

#include <sal/types.h>

#include <com/sun/star/task/theJobExecutor.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/frame/XGlobalEventBroadcaster.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Type.hxx>

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <comphelper/enumhelper.hxx>
#include <sfx2/app.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/eventcfg.hxx>
#include <eventsupplier.hxx>

#include <set>
#include <vector>

using namespace css;

namespace {

struct ModelCollectionMutexBase
{
public:
    osl::Mutex m_aLock;
};

typedef ::std::vector< css::uno::Reference< css::frame::XModel > > TModelList;


//TODO: remove support of obsolete document::XEventBroadcaster/Listener
class SfxGlobalEvents_Impl : public ModelCollectionMutexBase
                           , public ::cppu::WeakImplHelper< css::lang::XServiceInfo
                                                           , css::frame::XGlobalEventBroadcaster
                                                           , css::document::XEventBroadcaster
                                                           , css::document::XEventListener
                                                           , css::lang::XComponent
                                                            >
{
    css::uno::Reference< css::container::XNameReplace > m_xEvents;
    css::uno::Reference< css::document::XEventListener > m_xJobExecutorListener;
    ::comphelper::OInterfaceContainerHelper2 m_aLegacyListeners;
    ::comphelper::OInterfaceContainerHelper2 m_aDocumentListeners;
    std::multiset<css::uno::Reference<css::lang::XEventListener>> m_disposeListeners;
    TModelList m_lModels;
    bool m_disposed;

public:
    explicit SfxGlobalEvents_Impl(const css::uno::Reference < css::uno::XComponentContext >& rxContext);

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.sfx2.GlobalEventBroadcaster";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.GlobalEventBroadcaster" };
        return aSeq;
    }

    // css.document.XEventBroadcaster
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents() override;

    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::document::XEventListener >& xListener) override;

    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& xListener) override;

    // css.document.XDocumentEventBroadcaster
    virtual void SAL_CALL addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& Listener ) override;
    virtual void SAL_CALL removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& Listener ) override;
    virtual void SAL_CALL notifyDocumentEvent( const OUString& EventName, const css::uno::Reference< css::frame::XController2 >& ViewController, const css::uno::Any& Supplement ) override;

    // css.document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent) override;

    // css.document.XDocumentEventListener
    virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) override;

    // css.container.XSet
    virtual sal_Bool SAL_CALL has(const css::uno::Any& aElement) override;

    virtual void SAL_CALL insert(const css::uno::Any& aElement) override;

    virtual void SAL_CALL remove(const css::uno::Any& aElement) override;

    // css.container.XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // css.container.XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;

    virtual sal_Bool SAL_CALL hasElements() override;

    // css.lang.XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

    // css.lang.XComponent
    void SAL_CALL dispose() override;

    void SAL_CALL addEventListener(css::uno::Reference<css::lang::XEventListener> const & xListener)
        override;

    void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & aListener) override;

private:

    // threadsafe
    void implts_notifyJobExecution(const css::document::EventObject& aEvent);
    void implts_checkAndExecuteEventBindings(const css::document::DocumentEvent& aEvent);
    void implts_notifyListener(const css::document::DocumentEvent& aEvent);

    // not threadsafe
    TModelList::iterator impl_searchDoc(const css::uno::Reference< css::frame::XModel >& xModel);
};

SfxGlobalEvents_Impl::SfxGlobalEvents_Impl( const uno::Reference < uno::XComponentContext >& rxContext)
    : ModelCollectionMutexBase(       )
    , m_xJobExecutorListener( task::theJobExecutor::get( rxContext ), uno::UNO_QUERY_THROW )
    , m_aLegacyListeners      (m_aLock)
    , m_aDocumentListeners    (m_aLock)
    , m_disposed(false)
{
    osl_atomic_increment(&m_refCount);
    SfxApplication::GetOrCreate();
    m_xEvents = new GlobalEventConfig();
    osl_atomic_decrement(&m_refCount);
}

uno::Reference< container::XNameReplace > SAL_CALL SfxGlobalEvents_Impl::getEvents()
{
    // SAFE ->
    osl::MutexGuard aLock(m_aLock);
    if (m_disposed) {
        throw css::lang::DisposedException();
    }
    return m_xEvents;
    // <- SAFE
}


void SAL_CALL SfxGlobalEvents_Impl::addEventListener(const uno::Reference< document::XEventListener >& xListener)
{
    {
        osl::MutexGuard g(m_aLock);
        if (m_disposed) {
            throw css::lang::DisposedException();
        }
    }
    // container is threadsafe
    m_aLegacyListeners.addInterface(xListener);
    // Take care of an XComponent::dispose being processed in parallel with the above addInterface:
    {
        osl::MutexGuard g(m_aLock);
        if (!m_disposed) {
            return;
        }
    }
    m_aLegacyListeners.disposeAndClear({static_cast<OWeakObject *>(this)});
}


void SAL_CALL SfxGlobalEvents_Impl::removeEventListener(const uno::Reference< document::XEventListener >& xListener)
{
    // The below removeInterface will silently do nothing when m_disposed:
    // container is threadsafe
    m_aLegacyListeners.removeInterface(xListener);
}


void SAL_CALL SfxGlobalEvents_Impl::addDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& Listener )
{
    {
        osl::MutexGuard g(m_aLock);
        if (m_disposed) {
            throw css::lang::DisposedException();
        }
    }
    m_aDocumentListeners.addInterface( Listener );
    // Take care of an XComponent::dispose being processed in parallel with the above addInterface:
    {
        osl::MutexGuard g(m_aLock);
        if (!m_disposed) {
            return;
        }
    }
    m_aDocumentListeners.disposeAndClear({static_cast<OWeakObject *>(this)});
}


void SAL_CALL SfxGlobalEvents_Impl::removeDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& Listener )
{
    // The below removeInterface will silently do nothing when m_disposed:
    m_aDocumentListeners.removeInterface( Listener );
}


void SAL_CALL SfxGlobalEvents_Impl::notifyDocumentEvent( const OUString& /*_EventName*/,
        const uno::Reference< frame::XController2 >& /*_ViewController*/, const uno::Any& /*_Supplement*/ )
{
    // we're a multiplexer only, no chance to generate artificial events here
    throw lang::NoSupportException(OUString(), *this);
}


void SAL_CALL SfxGlobalEvents_Impl::notifyEvent(const document::EventObject& aEvent)
{
    // The below implts_* will silently do nothing when m_disposed:
    document::DocumentEvent aDocEvent(aEvent.Source, aEvent.EventName, nullptr, uno::Any());
    implts_notifyJobExecution(aEvent);
    implts_checkAndExecuteEventBindings(aDocEvent);
    implts_notifyListener(aDocEvent);
}


void SAL_CALL SfxGlobalEvents_Impl::documentEventOccured( const document::DocumentEvent& Event )
{
    // The below implts_* will silently do nothing when m_disposed:
    implts_notifyJobExecution(document::EventObject(Event.Source, Event.EventName));
    implts_checkAndExecuteEventBindings(Event);
    implts_notifyListener(Event);
}


void SAL_CALL SfxGlobalEvents_Impl::disposing(const lang::EventObject& aEvent)
{
    uno::Reference< frame::XModel > xDoc(aEvent.Source, uno::UNO_QUERY);

    // SAFE ->
    osl::MutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        m_lModels.erase(pIt);
    // <- SAFE
}

void SfxGlobalEvents_Impl::dispose() {
    std::multiset<css::uno::Reference<css::lang::XEventListener>> listeners;
    {
        osl::MutexGuard g(m_aLock);
        m_xEvents.clear();
        m_xJobExecutorListener.clear();
        m_disposeListeners.swap(listeners);
        m_lModels.clear();
        m_disposed = true;
    }
    m_aLegacyListeners.disposeAndClear({static_cast<OWeakObject *>(this)});
    m_aDocumentListeners.disposeAndClear({static_cast<OWeakObject *>(this)});
    for (auto const & i: listeners) {
        try {
            i->disposing({static_cast< cppu::OWeakObject * >(this)});
        } catch (css::lang::DisposedException &) {}
    }
}

void SfxGlobalEvents_Impl::addEventListener(
    css::uno::Reference<css::lang::XEventListener> const & xListener)
{
    if (!xListener.is()) {
        throw css::uno::RuntimeException("null listener");
    }
    {
        osl::MutexGuard g(m_aLock);
        if (!m_disposed) {
            m_disposeListeners.insert(xListener);
            return;
        }
    }
    try {
        xListener->disposing({static_cast< cppu::OWeakObject * >(this)});
    } catch (css::lang::DisposedException &) {}
}

void SfxGlobalEvents_Impl::removeEventListener(
    css::uno::Reference<css::lang::XEventListener> const & aListener)
{
    osl::MutexGuard g(m_aLock);
    auto const i = m_disposeListeners.find(aListener);
    if (i != m_disposeListeners.end()) {
        m_disposeListeners.erase(i);
    }
}

sal_Bool SAL_CALL SfxGlobalEvents_Impl::has(const uno::Any& aElement)
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;

    bool bHas = false;

    // SAFE ->
    osl::MutexGuard aLock(m_aLock);
    if (m_disposed) {
        throw css::lang::DisposedException();
    }
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        bHas = true;
    // <- SAFE

    return bHas;
}


void SAL_CALL SfxGlobalEvents_Impl::insert( const uno::Any& aElement )
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;
    if (!xDoc.is())
        throw lang::IllegalArgumentException(
                "Can not locate at least the model parameter.",
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    {
        osl::MutexGuard aLock(m_aLock);
        if (m_disposed) {
            throw css::lang::DisposedException();
        }
        TModelList::iterator pIt = impl_searchDoc(xDoc);
        if (pIt != m_lModels.end())
            throw container::ElementExistException(
                OUString(),
                static_cast<container::XSet*>(this));
        m_lModels.push_back(xDoc);
    }
    // <- SAFE

    uno::Reference< document::XDocumentEventBroadcaster > xDocBroadcaster(xDoc, uno::UNO_QUERY );
    if (xDocBroadcaster.is())
        xDocBroadcaster->addDocumentEventListener(this);
    else
    {
        // try the "legacy version" of XDocumentEventBroadcaster, which is XEventBroadcaster
        uno::Reference< document::XEventBroadcaster > xBroadcaster(xDoc, uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addEventListener(static_cast< document::XEventListener* >(this));
    }
}


void SAL_CALL SfxGlobalEvents_Impl::remove( const uno::Any& aElement )
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;
    if (!xDoc.is())
        throw lang::IllegalArgumentException(
                "Can not locate at least the model parameter.",
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    {
        osl::MutexGuard aLock(m_aLock);
        TModelList::iterator pIt = impl_searchDoc(xDoc);
        if (pIt == m_lModels.end())
            throw container::NoSuchElementException(
                OUString(),
                static_cast<container::XSet*>(this));
        m_lModels.erase(pIt);
    }
    // <- SAFE

    uno::Reference< document::XDocumentEventBroadcaster > xDocBroadcaster(xDoc, uno::UNO_QUERY );
    if (xDocBroadcaster.is())
        xDocBroadcaster->removeDocumentEventListener(this);
    else
    {
        // try the "legacy version" of XDocumentEventBroadcaster, which is XEventBroadcaster
        uno::Reference< document::XEventBroadcaster > xBroadcaster(xDoc, uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener(static_cast< document::XEventListener* >(this));
    }
}


uno::Reference< container::XEnumeration > SAL_CALL SfxGlobalEvents_Impl::createEnumeration()
{
    // SAFE ->
    osl::MutexGuard aLock(m_aLock);
    if (m_disposed) {
        throw css::lang::DisposedException();
    }
    uno::Sequence<uno::Any> models(m_lModels.size());
    for (size_t i = 0; i < m_lModels.size(); ++i)
    {
        models[i] <<= m_lModels[i];
    }
    uno::Reference<container::XEnumeration> xEnum(new ::comphelper::OAnyEnumeration(models));
    // <- SAFE

    return xEnum;
}


uno::Type SAL_CALL SfxGlobalEvents_Impl::getElementType()
{
    return cppu::UnoType<frame::XModel>::get();
}


sal_Bool SAL_CALL SfxGlobalEvents_Impl::hasElements()
{
    // SAFE ->
    osl::MutexGuard aLock(m_aLock);
    if (m_disposed) {
        throw css::lang::DisposedException();
    }
    return (!m_lModels.empty());
    // <- SAFE
}


void SfxGlobalEvents_Impl::implts_notifyJobExecution(const document::EventObject& aEvent)
{
    css::uno::Reference<css::document::XEventListener> listener;
    {
        osl::MutexGuard g(m_aLock);
        listener = m_xJobExecutorListener;
    }
    if (!listener.is()) {
        return;
    }
    try
    {
        listener->notifyEvent(aEvent);
    }
    catch(const uno::RuntimeException&)
        { throw; }
    catch(const uno::Exception&)
        {}
}


void SfxGlobalEvents_Impl::implts_checkAndExecuteEventBindings(const document::DocumentEvent& aEvent)
{
    css::uno::Reference<css::container::XNameReplace> events;
    {
        osl::MutexGuard g(m_aLock);
        events = m_xEvents;
    }
    if (!events.is()) {
        return;
    }
    try
    {
        uno::Any aAny;
        if ( events->hasByName( aEvent.EventName ) )
            aAny = events->getByName(aEvent.EventName);
        SfxEvents_Impl::Execute(aAny, aEvent, nullptr);
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
       DBG_UNHANDLED_EXCEPTION("sfx.notify");
    }
}


void SfxGlobalEvents_Impl::implts_notifyListener(const document::DocumentEvent& aEvent)
{
    // containers are threadsafe
    document::EventObject aLegacyEvent(aEvent.Source, aEvent.EventName);
    m_aLegacyListeners.notifyEach( &document::XEventListener::notifyEvent, aLegacyEvent );

    m_aDocumentListeners.notifyEach( &document::XDocumentEventListener::documentEventOccured, aEvent );
}


// not threadsafe ... must be locked from outside!
TModelList::iterator SfxGlobalEvents_Impl::impl_searchDoc(const uno::Reference< frame::XModel >& xModel)
{
    if (!xModel.is())
        return m_lModels.end();

    return std::find_if(m_lModels.begin(), m_lModels.end(),
        [&xModel](const TModelList::value_type& rxModel) {
            uno::Reference< frame::XModel > xContainerDoc(rxModel, uno::UNO_QUERY);
            return xContainerDoc == xModel;
        });
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SfxGlobalEvents_Impl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
