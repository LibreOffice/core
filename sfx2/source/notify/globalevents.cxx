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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Type.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <comphelper/enumhelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/evntconf.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/eventcfg.hxx>
#include <eventsupplier.hxx>

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
                                                           , css::document::XEventListener
                                                            >
{
    css::uno::Reference< css::container::XNameReplace > m_xEvents;
    css::uno::Reference< css::document::XEventListener > m_xJobExecutorListener;
    ::cppu::OInterfaceContainerHelper m_aLegacyListeners;
    ::cppu::OInterfaceContainerHelper m_aDocumentListeners;
    TModelList m_lModels;
    GlobalEventConfig* pImp;

public:
    explicit SfxGlobalEvents_Impl(const css::uno::Reference < css::uno::XComponentContext >& rxContext);
    virtual ~SfxGlobalEvents_Impl();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.sfx2.GlobalEventBroadcaster");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.frame.GlobalEventBroadcaster";
        return aSeq;
    }

    // css.document.XEventBroadcaster
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents()
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::document::XEventListener >& xListener)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& xListener)
        throw(css::uno::RuntimeException, std::exception) override;

    // css.document.XDocumentEventBroadcaster
    virtual void SAL_CALL addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyDocumentEvent( const OUString& _EventName, const css::uno::Reference< css::frame::XController2 >& _ViewController, const css::uno::Any& _Supplement ) throw (css::lang::IllegalArgumentException, css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // css.document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    // css.document.XDocumentEventListener
    virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // css.container.XSet
    virtual sal_Bool SAL_CALL has(const css::uno::Any& aElement)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL insert(const css::uno::Any& aElement)
        throw(css::lang::IllegalArgumentException  ,
              css::container::ElementExistException,
              css::uno::RuntimeException, std::exception           ) override;

    virtual void SAL_CALL remove(const css::uno::Any& aElement)
        throw(css::lang::IllegalArgumentException   ,
              css::container::NoSuchElementException,
              css::uno::RuntimeException, std::exception            ) override;

    // css.container.XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration()
        throw(css::uno::RuntimeException, std::exception) override;

    // css.container.XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasElements()
        throw(css::uno::RuntimeException, std::exception) override;

    // css.lang.XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
        throw(css::uno::RuntimeException, std::exception) override;

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
    , pImp                    (0      )
{
    m_refCount++;
    SfxGetpApp();
    pImp                   = new GlobalEventConfig();
    m_xEvents              = pImp;
    m_refCount--;
}


SfxGlobalEvents_Impl::~SfxGlobalEvents_Impl()
{
}


uno::Reference< container::XNameReplace > SAL_CALL SfxGlobalEvents_Impl::getEvents()
    throw(uno::RuntimeException, std::exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return m_xEvents;
    // <- SAFE
}


void SAL_CALL SfxGlobalEvents_Impl::addEventListener(const uno::Reference< document::XEventListener >& xListener)
    throw(uno::RuntimeException, std::exception)
{
    // container is threadsafe
    m_aLegacyListeners.addInterface(xListener);
}


void SAL_CALL SfxGlobalEvents_Impl::removeEventListener(const uno::Reference< document::XEventListener >& xListener)
    throw(uno::RuntimeException, std::exception)
{
    // container is threadsafe
    m_aLegacyListeners.removeInterface(xListener);
}


void SAL_CALL SfxGlobalEvents_Impl::addDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& _Listener )
    throw(uno::RuntimeException, std::exception)
{
    m_aDocumentListeners.addInterface( _Listener );
}


void SAL_CALL SfxGlobalEvents_Impl::removeDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& _Listener )
    throw(uno::RuntimeException, std::exception)
{
    m_aDocumentListeners.removeInterface( _Listener );
}


void SAL_CALL SfxGlobalEvents_Impl::notifyDocumentEvent( const OUString& /*_EventName*/,
        const uno::Reference< frame::XController2 >& /*_ViewController*/, const uno::Any& /*_Supplement*/ )
        throw (lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException, std::exception)
{
    // we're a multiplexer only, no chance to generate artificial events here
    throw lang::NoSupportException(OUString(), *this);
}


void SAL_CALL SfxGlobalEvents_Impl::notifyEvent(const document::EventObject& aEvent)
    throw(uno::RuntimeException, std::exception)
{
    document::DocumentEvent aDocEvent(aEvent.Source, aEvent.EventName, NULL, uno::Any());
    implts_notifyJobExecution(aEvent);
    implts_checkAndExecuteEventBindings(aDocEvent);
    implts_notifyListener(aDocEvent);
}


void SAL_CALL SfxGlobalEvents_Impl::documentEventOccured( const document::DocumentEvent& _Event )
    throw (uno::RuntimeException, std::exception)
{
    implts_notifyJobExecution(document::EventObject(_Event.Source, _Event.EventName));
    implts_checkAndExecuteEventBindings(_Event);
    implts_notifyListener(_Event);
}


void SAL_CALL SfxGlobalEvents_Impl::disposing(const lang::EventObject& aEvent)
    throw(uno::RuntimeException, std::exception)
{
    uno::Reference< frame::XModel > xDoc(aEvent.Source, uno::UNO_QUERY);

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        m_lModels.erase(pIt);
    aLock.clear();
    // <- SAFE
}


sal_Bool SAL_CALL SfxGlobalEvents_Impl::has(const uno::Any& aElement)
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;

    bool bHas = false;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        bHas = true;
    aLock.clear();
    // <- SAFE

    return bHas;
}


void SAL_CALL SfxGlobalEvents_Impl::insert( const uno::Any& aElement )
    throw (lang::IllegalArgumentException  ,
           container::ElementExistException,
           uno::RuntimeException, std::exception           )
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;
    if (!xDoc.is())
        throw lang::IllegalArgumentException(
                OUString("Can not locate at least the model parameter."),
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        throw container::ElementExistException(
                OUString(),
                static_cast< container::XSet* >(this));
    m_lModels.push_back(xDoc);
    aLock.clear();
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
    throw (lang::IllegalArgumentException   ,
           container::NoSuchElementException,
           uno::RuntimeException, std::exception            )
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;
    if (!xDoc.is())
        throw lang::IllegalArgumentException(
                OUString("Can not locate at least the model parameter."),
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt == m_lModels.end())
        throw container::NoSuchElementException(
                OUString(),
                static_cast< container::XSet* >(this));
    m_lModels.erase(pIt);
    aLock.clear();
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
    throw (uno::RuntimeException, std::exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    uno::Sequence<uno::Any> models(m_lModels.size());
    for (size_t i = 0; i < m_lModels.size(); ++i)
    {
        models[i] = uno::makeAny(m_lModels[i]);
    }
    uno::Reference< container::XEnumeration > xEnum(
        static_cast<container::XEnumeration*>(
            new ::comphelper::OAnyEnumeration(models)));
    aLock.clear();
    // <- SAFE

    return xEnum;
}


uno::Type SAL_CALL SfxGlobalEvents_Impl::getElementType()
    throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<frame::XModel>::get();
}


sal_Bool SAL_CALL SfxGlobalEvents_Impl::hasElements()
    throw (uno::RuntimeException, std::exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return (m_lModels.size()>0);
    // <- SAFE
}


void SfxGlobalEvents_Impl::implts_notifyJobExecution(const document::EventObject& aEvent)
{
    try
    {
        m_xJobExecutorListener->notifyEvent(aEvent);
    }
    catch(const uno::RuntimeException&)
        { throw; }
    catch(const uno::Exception&)
        {}
}


void SfxGlobalEvents_Impl::implts_checkAndExecuteEventBindings(const document::DocumentEvent& aEvent)
{
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);
        uno::Reference< container::XNameReplace > xEvents = m_xEvents;
        aLock.clear();
        // <- SAFE

        uno::Any aAny;
        if ( xEvents.is() && xEvents->hasByName( aEvent.EventName ) )
            aAny = xEvents->getByName(aEvent.EventName);
        SfxEvents_Impl::Execute(aAny, aEvent, 0);
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
       DBG_UNHANDLED_EXCEPTION();
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

    TModelList::iterator pIt;
    for (  pIt  = m_lModels.begin();
           pIt != m_lModels.end()  ;
         ++pIt                     )
    {
        uno::Reference< frame::XModel > xContainerDoc(*pIt, uno::UNO_QUERY);
        if (xContainerDoc == xModel)
            break;
    }

    return pIt;
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(
            static_cast<cppu::OWeakObject *>(new SfxGlobalEvents_Impl(context)))
    {}

    rtl::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
