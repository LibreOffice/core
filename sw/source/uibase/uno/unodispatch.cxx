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

#include <config_features.h>
#include <config_fuzzers.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <comphelper/servicehelper.hxx>
#include <osl/diagnose.h>
#include <unodispatch.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <wrtsh.hxx>
#include <dbmgr.hxx>

using namespace ::com::sun::star;

const char cURLFormLetter[] = ".uno:DataSourceBrowser/FormLetter";
const char cURLInsertContent[] = ".uno:DataSourceBrowser/InsertContent";//data into fields
const char cURLInsertColumns[] = ".uno:DataSourceBrowser/InsertColumns";//data into text
const char cURLDocumentDataSource[] = ".uno:DataSourceBrowser/DocumentDataSource";//current data source of the document
const char cInternalDBChangeNotification[] = ".uno::Writer/DataSourceChanged";

SwXDispatchProviderInterceptor::SwXDispatchProviderInterceptor(SwView& rVw) :
    m_pView(&rVw)
{
    uno::Reference< frame::XFrame> xUnoFrame = m_pView->GetViewFrame().GetFrame().GetFrameInterface();
    m_xIntercepted.set(xUnoFrame, uno::UNO_QUERY);
    if(m_xIntercepted.is())
    {
        osl_atomic_increment(&m_refCount);
        m_xIntercepted->registerDispatchProviderInterceptor(static_cast<frame::XDispatchProviderInterceptor*>(this));
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got a fallback for requests we (i.e. our master) cannot fulfill
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->addEventListener(static_cast<lang::XEventListener*>(this));
        osl_atomic_decrement(&m_refCount);
    }
}

SwXDispatchProviderInterceptor::~SwXDispatchProviderInterceptor()
{
}

uno::Reference< frame::XDispatch > SwXDispatchProviderInterceptor::queryDispatch(
    const util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
{
    DispatchMutexLock_Impl aLock;
    uno::Reference< frame::XDispatch> xResult;
    // create some dispatch ...
    if(m_pView && aURL.Complete.startsWith(".uno:DataSourceBrowser/"))
    {
        if(aURL.Complete == cURLFormLetter ||
            aURL.Complete == cURLInsertContent ||
                aURL.Complete == cURLInsertColumns ||
                    aURL.Complete == cURLDocumentDataSource)
        {
            if(!m_xDispatch.is())
                m_xDispatch = new SwXDispatch(*m_pView);
            xResult = m_xDispatch;
        }
    }

    // ask our slave provider
    if (!xResult.is() && m_xSlaveDispatcher.is())
        xResult = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    return xResult;
}

uno::Sequence<OUString> SAL_CALL SwXDispatchProviderInterceptor::getInterceptedURLs()
{
    uno::Sequence<OUString> aRet =
    {
         u".uno:DataSourceBrowser/*"_ustr
    };

    return aRet;
}

uno::Sequence< uno::Reference< frame::XDispatch > > SwXDispatchProviderInterceptor::queryDispatches(
    const uno::Sequence< frame::DispatchDescriptor >& aDescripts )
{
    DispatchMutexLock_Impl aLock;
    uno::Sequence< uno::Reference< frame::XDispatch> > aReturn(aDescripts.getLength());
    std::transform(aDescripts.begin(), aDescripts.end(), aReturn.getArray(),
        [this](const frame::DispatchDescriptor& rDescr) -> uno::Reference<frame::XDispatch> {
            return queryDispatch(rDescr.FeatureURL, rDescr.FrameName, rDescr.SearchFlags); });
    return aReturn;
}

uno::Reference< frame::XDispatchProvider > SwXDispatchProviderInterceptor::getSlaveDispatchProvider(  )
{
    DispatchMutexLock_Impl aLock;
    return m_xSlaveDispatcher;
}

void SwXDispatchProviderInterceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& xNewDispatchProvider )
{
    DispatchMutexLock_Impl aLock;
    m_xSlaveDispatcher = xNewDispatchProvider;
}

uno::Reference< frame::XDispatchProvider > SwXDispatchProviderInterceptor::getMasterDispatchProvider(  )
{
    DispatchMutexLock_Impl aLock;
    return m_xMasterDispatcher;
}

void SwXDispatchProviderInterceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& xNewSupplier )
{
    DispatchMutexLock_Impl aLock;
    m_xMasterDispatcher = xNewSupplier;
}

void SwXDispatchProviderInterceptor::disposing( const lang::EventObject& )
{
    DispatchMutexLock_Impl aLock;
    if (m_xIntercepted.is())
    {
        m_xIntercepted->releaseDispatchProviderInterceptor(static_cast<frame::XDispatchProviderInterceptor*>(this));
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener(static_cast<lang::XEventListener*>(this));
        m_xDispatch       = nullptr;
    }
    m_xIntercepted = nullptr;
}

void    SwXDispatchProviderInterceptor::Invalidate()
{
    DispatchMutexLock_Impl aLock;
    if (m_xIntercepted.is())
    {
        m_xIntercepted->releaseDispatchProviderInterceptor(static_cast<frame::XDispatchProviderInterceptor*>(this));
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener(static_cast<lang::XEventListener*>(this));
        m_xDispatch       = nullptr;
    }
    m_xIntercepted = nullptr;
    m_pView = nullptr;
}

SwXDispatch::SwXDispatch(SwView& rVw) :
    m_pView(&rVw),
    m_bOldEnable(false),
    m_bListenerAdded(false)
{
}

SwXDispatch::~SwXDispatch()
{
    if(m_bListenerAdded && m_pView)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        uno::Reference<view::XSelectionChangeListener> xThis = this;
        xSupplier->removeSelectionChangeListener(xThis);
    }
}

void SwXDispatch::dispatch(const util::URL& aURL,
    const uno::Sequence< beans::PropertyValue >& aArgs)
{
    if(!m_pView)
        throw uno::RuntimeException();
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
    (void) aArgs;
    if (false)
    {
    }
#else
    SwWrtShell& rSh = m_pView->GetWrtShell();
    SwDBManager* pDBManager = rSh.GetDBManager();
    if(aURL.Complete == cURLInsertContent)
    {
        svx::ODataAccessDescriptor aDescriptor(aArgs);
        SwMergeDescriptor aMergeDesc( DBMGR_MERGE, rSh, aDescriptor );
        pDBManager->Merge(aMergeDesc);
    }
    else if(aURL.Complete == cURLInsertColumns)
    {
        SwDBManager::InsertText(rSh, aArgs);
    }
    else if(aURL.Complete == cURLFormLetter)
    {
        SfxUnoAnyItem aDBProperties(FN_PARAM_DATABASE_PROPERTIES, uno::Any(aArgs));
        m_pView->GetViewFrame().GetDispatcher()->ExecuteList(
            FN_MAILMERGE_WIZARD,
            SfxCallMode::ASYNCHRON,
            { &aDBProperties });
    }
#endif
    else if(aURL.Complete == cURLDocumentDataSource)
    {
        OSL_FAIL("SwXDispatch::dispatch: this URL is not to be dispatched!");
    }
    else if(aURL.Complete == cInternalDBChangeNotification)
    {
        frame::FeatureStateEvent aEvent;
        aEvent.Source = getXWeak();

        const SwDBData& rData = m_pView->GetWrtShell().GetDBData();
        svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(rData.sDataSource);
        aDescriptor[svx::DataAccessDescriptorProperty::Command]       <<= rData.sCommand;
        aDescriptor[svx::DataAccessDescriptorProperty::CommandType]   <<= rData.nCommandType;

        aEvent.State <<= aDescriptor.createPropertyValueSequence();
        aEvent.IsEnabled = !rData.sDataSource.isEmpty();

        // calls to statusChanged may call addStatusListener or removeStatusListener
        // so copy m_aStatusListenerVector on stack
        auto copyStatusListenerVector = m_aStatusListenerVector;
        for (auto & status : copyStatusListenerVector)
        {
            if(status.aURL.Complete == cURLDocumentDataSource)
            {
                aEvent.FeatureURL = status.aURL;
                status.xListener->statusChanged( aEvent );
            }
        }
    }
    else
        throw uno::RuntimeException();

}

void SwXDispatch::addStatusListener(
    const uno::Reference< frame::XStatusListener >& xControl, const util::URL& aURL )
{
    if(!m_pView)
        throw uno::RuntimeException();
    ShellMode eMode = m_pView->GetShellMode();
    bool bEnable = ShellMode::Text == eMode  ||
                       ShellMode::ListText == eMode  ||
                       ShellMode::TableText == eMode  ||
                       ShellMode::TableListText == eMode;

    m_bOldEnable = bEnable;
    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = bEnable;
    aEvent.Source = getXWeak();
    aEvent.FeatureURL = aURL;

    // one of the URLs requires a special state...
    if (aURL.Complete == cURLDocumentDataSource)
    {
        const SwDBData& rData = m_pView->GetWrtShell().GetDBData();

        svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(rData.sDataSource);
        aDescriptor[svx::DataAccessDescriptorProperty::Command]       <<= rData.sCommand;
        aDescriptor[svx::DataAccessDescriptorProperty::CommandType]   <<= rData.nCommandType;

        aEvent.State <<= aDescriptor.createPropertyValueSequence();
        aEvent.IsEnabled = !rData.sDataSource.isEmpty();
    }

    xControl->statusChanged( aEvent );

    StatusStruct_Impl aStatus;
    aStatus.xListener = xControl;
    aStatus.aURL = aURL;
    m_aStatusListenerVector.emplace_back(aStatus);

    if(!m_bListenerAdded)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        uno::Reference<view::XSelectionChangeListener> xThis = this;
        xSupplier->addSelectionChangeListener(xThis);
        m_bListenerAdded = true;
    }
}

void SwXDispatch::removeStatusListener(
    const uno::Reference< frame::XStatusListener >& xControl, const util::URL&  )
{
    std::erase_if(
            m_aStatusListenerVector,
            [&](const StatusStruct_Impl& status) { return status.xListener.get() == xControl.get(); });
    if(m_aStatusListenerVector.empty() && m_pView)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        uno::Reference<view::XSelectionChangeListener> xThis = this;
        xSupplier->removeSelectionChangeListener(xThis);
        m_bListenerAdded = false;
    }
}

void SwXDispatch::selectionChanged( const lang::EventObject&  )
{
    ShellMode eMode = m_pView->GetShellMode();
    bool bEnable = ShellMode::Text == eMode  ||
                       ShellMode::ListText == eMode  ||
                       ShellMode::TableText == eMode  ||
                       ShellMode::TableListText == eMode;
    if(bEnable == m_bOldEnable)
        return;

    m_bOldEnable = bEnable;
    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = bEnable;
    aEvent.Source = getXWeak();

    // calls to statusChanged may call addStatusListener or removeStatusListener
    // so copy m_aStatusListenerVector on stack
    auto copyStatusListenerVector = m_aStatusListenerVector;
    for (auto & status : copyStatusListenerVector)
    {
        aEvent.FeatureURL = status.aURL;
        if (status.aURL.Complete != cURLDocumentDataSource)
            // the document's data source does not depend on the selection, so it's state does not change here
            status.xListener->statusChanged( aEvent );
    }
}

void SwXDispatch::disposing( const lang::EventObject& rSource )
{
    uno::Reference<view::XSelectionSupplier> xSupplier(rSource.Source, uno::UNO_QUERY);
    uno::Reference<view::XSelectionChangeListener> xThis = this;
    xSupplier->removeSelectionChangeListener(xThis);
    m_bListenerAdded = false;

    lang::EventObject aObject;
    aObject.Source = getXWeak();
    // calls to statusChanged may call addStatusListener or removeStatusListener
    // so copy m_aStatusListenerVector on stack
    auto copyStatusListenerVector = m_aStatusListenerVector;
    for (auto & status : copyStatusListenerVector)
    {
        status.xListener->disposing(aObject);
    }
    m_pView = nullptr;
}

const char* SwXDispatch::GetDBChangeURL()
{
    return cInternalDBChangeNotification;
}

SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::DispatchMutexLock_Impl()
{
}

SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::~DispatchMutexLock_Impl()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
