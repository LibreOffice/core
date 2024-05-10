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
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/enumhelper.hxx>
#include <rtl/ref.hxx>
#include <sfx2/app.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/eventcfg.hxx>
#include <eventsupplier.hxx>

#include <mutex>
#include <set>
#include <vector>

using namespace css;

namespace {

typedef ::std::vector< css::uno::Reference< css::frame::XModel > > TModelList;


//TODO: remove support of obsolete document::XEventBroadcaster/Listener
class SfxGlobalEvents_Impl : public ::cppu::WeakImplHelper< css::lang::XServiceInfo
                                                           , css::frame::XGlobalEventBroadcaster
                                                           , css::document::XEventBroadcaster
                                                           , css::document::XEventListener
                                                           , css::lang::XComponent
                                                            >
{
    std::mutex m_aLock;
    rtl::Reference< GlobalEventConfig > m_xEvents;
    css::uno::Reference< css::document::XEventListener > m_xJobExecutorListener;
    ::comphelper::OInterfaceContainerHelper4<document::XEventListener> m_aLegacyListeners;
    ::comphelper::OInterfaceContainerHelper4<document::XDocumentEventListener> m_aDocumentListeners;
    std::multiset<css::uno::Reference<css::lang::XEventListener>> m_disposeListeners;
    TModelList m_lModels;
    bool m_disposed;

public:
    explicit SfxGlobalEvents_Impl(const css::uno::Reference < css::uno::XComponentContext >& rxContext);

    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.sfx2.GlobalEventBroadcaster"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        css::uno::Sequence< OUString > aSeq { u"com.sun.star.frame.GlobalEventBroadcaster"_ustr };
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
    : m_xJobExecutorListener( task::theJobExecutor::get( rxContext ), uno::UNO_QUERY_THROW )
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
    std::scoped_lock aLock(m_aLock);
    if (m_disposed) {
        throw css::lang::DisposedException();
    }
    return m_xEvents;
    // <- SAFE
}


void SAL_CALL SfxGlobalEvents_Impl::addEventListener(const uno::Reference< document::XEventListener >& xListener)
{
    std::unique_lock g(m_aLock);
    if (m_disposed)
        throw css::lang::DisposedException();
    m_aLegacyListeners.addInterface(g, xListener);
}


void SAL_CALL SfxGlobalEvents_Impl::removeEventListener(const uno::Reference< document::XEventListener >& xListener)
{
    std::unique_lock g(m_aLock);
    // The below removeInterface will silently do nothing when m_disposed
    m_aLegacyListeners.removeInterface(g, xListener);
}


void SAL_CALL SfxGlobalEvents_Impl::addDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& Listener )
{
    std::unique_lock g(m_aLock);
    if (m_disposed)
        throw css::lang::DisposedException();
    m_aDocumentListeners.addInterface( g, Listener );
}


void SAL_CALL SfxGlobalEvents_Impl::removeDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& Listener )
{
    std::unique_lock g(m_aLock);
    // The below removeInterface will silently do nothing when m_disposed:
    m_aDocumentListeners.removeInterface( g, Listener );
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
    std::scoped_lock g(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        m_lModels.erase(pIt);
    // <- SAFE
}

void SfxGlobalEvents_Impl::dispose() {
    std::multiset<css::uno::Reference<css::lang::XEventListener>> listeners;
    {
        std::unique_lock g(m_aLock);
        if (m_disposed)
            return;
        m_disposed = true;
        auto tmpEvents = std::move(m_xEvents);
        auto tmpModels = std::move(m_lModels);
        m_xJobExecutorListener.clear();
        m_disposeListeners.swap(listeners);
        m_lModels.clear();
        g.unlock();
        // clear events&models outside lock because it will trigger a call back into us
        tmpEvents.clear();
        tmpModels.clear();
        g.lock();
        m_aLegacyListeners.disposeAndClear(g, {getXWeak()});
        m_aDocumentListeners.disposeAndClear(g, {getXWeak()});
    }
    for (auto const & i: listeners) {
        try {
            i->disposing({getXWeak()});
        } catch (css::lang::DisposedException &) {}
    }
}

void SfxGlobalEvents_Impl::addEventListener(
    css::uno::Reference<css::lang::XEventListener> const & xListener)
{
    if (!xListener.is()) {
        throw css::uno::RuntimeException(u"null listener"_ustr);
    }
    {
        std::scoped_lock g(m_aLock);
        if (!m_disposed) {
            m_disposeListeners.insert(xListener);
            return;
        }
    }
    try {
        xListener->disposing({getXWeak()});
    } catch (css::lang::DisposedException &) {}
}

void SfxGlobalEvents_Impl::removeEventListener(
    css::uno::Reference<css::lang::XEventListener> const & aListener)
{
    std::scoped_lock g(m_aLock);
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
    std::scoped_lock g(m_aLock);
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
                u"Can not locate at least the model parameter."_ustr,
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    {
        std::scoped_lock g(m_aLock);
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
                u"Can not locate at least the model parameter."_ustr,
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    {
        std::scoped_lock g(m_aLock);
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
    std::scoped_lock g(m_aLock);
    if (m_disposed) {
        throw css::lang::DisposedException();
    }
    uno::Sequence<uno::Any> models(m_lModels.size());
    auto modelsRange = asNonConstRange(models);
    for (size_t i = 0; i < m_lModels.size(); ++i)
    {
        modelsRange[i] <<= m_lModels[i];
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
    std::scoped_lock g(m_aLock);
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
        std::scoped_lock g(m_aLock);
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
    rtl::Reference<GlobalEventConfig> events;
    {
        std::scoped_lock g(m_aLock);
        events = m_xEvents;
    }
    if (!events.is()) {
        return;
    }
    try
    {
        if ( events->hasByName( aEvent.EventName ) )
        {
            uno::Sequence < beans::PropertyValue > aAny = events->getByName2(aEvent.EventName);
            SfxEvents_Impl::Execute(aAny, aEvent, nullptr);
        }
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
    std::unique_lock g(m_aLock);

    document::EventObject aLegacyEvent(aEvent.Source, aEvent.EventName);
    m_aLegacyListeners.forEach(g,
        [&aLegacyEvent](const css::uno::Reference<document::XEventListener>& xListener)
        {
            xListener->notifyEvent(aLegacyEvent);
        }
    );
    m_aDocumentListeners.forEach(g,
        [&aEvent](const css::uno::Reference<document::XDocumentEventListener>& xListener)
        {
            xListener->documentEventOccured(aEvent);
        }
    );
}


// not threadsafe ... must be locked from outside!
TModelList::iterator SfxGlobalEvents_Impl::impl_searchDoc(const uno::Reference< frame::XModel >& xModel)
{
    if (!xModel.is())
        return m_lModels.end();

    return std::find_if(m_lModels.begin(), m_lModels.end(),
        [&xModel](const uno::Reference< frame::XModel >& rxModel) {
            return rxModel == xModel;
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
