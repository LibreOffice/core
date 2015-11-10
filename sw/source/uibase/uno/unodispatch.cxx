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

#include <vcl/svapp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <comphelper/servicehelper.hxx>
#include <unodispatch.hxx>
#include <unobaseclass.hxx>
#include <view.hxx>
#include <cmdid.h>
#include "wrtsh.hxx"
#include "dbmgr.hxx"

using namespace ::com::sun::star;

static const char* cURLFormLetter      = ".uno:DataSourceBrowser/FormLetter";
static const char* cURLInsertContent   = ".uno:DataSourceBrowser/InsertContent";//data into fields
static const char* cURLInsertColumns   = ".uno:DataSourceBrowser/InsertColumns";//data into text
static const char* cURLDocumentDataSource  = ".uno:DataSourceBrowser/DocumentDataSource";//current data source of the document
static const sal_Char* cInternalDBChangeNotification = ".uno::Writer/DataSourceChanged";

SwXDispatchProviderInterceptor::SwXDispatchProviderInterceptor(SwView& rVw) :
    m_pView(&rVw)
{
    uno::Reference< frame::XFrame> xUnoFrame = m_pView->GetViewFrame()->GetFrame().GetFrameInterface();
    m_xIntercepted.set(xUnoFrame, uno::UNO_QUERY);
    if(m_xIntercepted.is())
    {
        m_refCount++;
        m_xIntercepted->registerDispatchProviderInterceptor(static_cast<frame::XDispatchProviderInterceptor*>(this));
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fulfill
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->addEventListener(static_cast<lang::XEventListener*>(this));
        m_refCount--;
    }
}

SwXDispatchProviderInterceptor::~SwXDispatchProviderInterceptor()
{
}

uno::Reference< frame::XDispatch > SwXDispatchProviderInterceptor::queryDispatch(
    const util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
        throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
    uno::Reference< frame::XDispatch> xResult;
    // create some dispatch ...
    if(m_pView && aURL.Complete.startsWith(".uno:DataSourceBrowser/"))
    {
        if(aURL.Complete.equalsAscii(cURLFormLetter) ||
            aURL.Complete.equalsAscii(cURLInsertContent) ||
                aURL.Complete.equalsAscii(cURLInsertColumns)||
                    aURL.Complete.equalsAscii(cURLDocumentDataSource))
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

uno::Sequence< uno::Reference< frame::XDispatch > > SwXDispatchProviderInterceptor::queryDispatches(
    const uno::Sequence< frame::DispatchDescriptor >& aDescripts ) throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
    uno::Sequence< uno::Reference< frame::XDispatch> > aReturn(aDescripts.getLength());
    uno::Reference< frame::XDispatch>* pReturn = aReturn.getArray();
    const frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_Int32 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts)
    {
        *pReturn = queryDispatch(pDescripts->FeatureURL,
                pDescripts->FrameName, pDescripts->SearchFlags);
    }
    return aReturn;
}

uno::Reference< frame::XDispatchProvider > SwXDispatchProviderInterceptor::getSlaveDispatchProvider(  )
        throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
    return m_xSlaveDispatcher;
}

void SwXDispatchProviderInterceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& xNewDispatchProvider ) throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
    m_xSlaveDispatcher = xNewDispatchProvider;
}

uno::Reference< frame::XDispatchProvider > SwXDispatchProviderInterceptor::getMasterDispatchProvider(  )
        throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
    return m_xMasterDispatcher;
}

void SwXDispatchProviderInterceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& xNewSupplier ) throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
    m_xMasterDispatcher = xNewSupplier;
}

void SwXDispatchProviderInterceptor::disposing( const lang::EventObject& )
    throw(uno::RuntimeException, std::exception)
{
    DispatchMutexLock_Impl aLock(*this);
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

namespace
{
    class theSwXDispatchProviderInterceptorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXDispatchProviderInterceptorUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXDispatchProviderInterceptor::getUnoTunnelId()
{
    return theSwXDispatchProviderInterceptorUnoTunnelId::get().getSeq();
}

sal_Int64 SwXDispatchProviderInterceptor::getSomething(
    const uno::Sequence< sal_Int8 >& aIdentifier )
        throw(uno::RuntimeException, std::exception)
{
    if( aIdentifier.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        aIdentifier.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
    return 0;
}

void    SwXDispatchProviderInterceptor::Invalidate()
{
    DispatchMutexLock_Impl aLock(*this);
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
        throw (uno::RuntimeException, std::exception)
{
    if(!m_pView)
        throw uno::RuntimeException();
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) aArgs;
    if (false)
    {
    }
#else
    SwWrtShell& rSh = m_pView->GetWrtShell();
    SwDBManager* pDBManager = rSh.GetDBManager();
    if(aURL.Complete.equalsAscii(cURLInsertContent))
    {
        svx::ODataAccessDescriptor aDescriptor(aArgs);
        SwMergeDescriptor aMergeDesc( DBMGR_MERGE, rSh, aDescriptor );
        pDBManager->MergeNew(aMergeDesc);
    }
    else if(aURL.Complete.equalsAscii(cURLInsertColumns))
    {
        SwDBManager::InsertText(rSh, aArgs);
    }
    else if(aURL.Complete.equalsAscii(cURLFormLetter))
    {
        SfxUsrAnyItem aDBProperties(FN_PARAM_DATABASE_PROPERTIES, uno::makeAny(aArgs));
        m_pView->GetViewFrame()->GetDispatcher()->Execute(
            FN_MAILMERGE_WIZARD,
            SfxCallMode::ASYNCHRON,
            &aDBProperties, 0L);
    }
#endif
    else if(aURL.Complete.equalsAscii(cURLDocumentDataSource))
    {
        OSL_FAIL("SwXDispatch::dispatch: this URL is not to be dispatched!");
    }
    else if(aURL.Complete.equalsAscii(cInternalDBChangeNotification))
    {
        frame::FeatureStateEvent aEvent;
        aEvent.IsEnabled = sal_True;
        aEvent.Source = *static_cast<cppu::OWeakObject*>(this);

        const SwDBData& rData = m_pView->GetWrtShell().GetDBDesc();
        svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(rData.sDataSource);
        aDescriptor[svx::daCommand]       <<= rData.sCommand;
        aDescriptor[svx::daCommandType]   <<= rData.nCommandType;

        aEvent.State <<= aDescriptor.createPropertyValueSequence();
        aEvent.IsEnabled = !rData.sDataSource.isEmpty();

        StatusListenerList::iterator aListIter = m_aListenerList.begin();
        for(aListIter = m_aListenerList.begin(); aListIter != m_aListenerList.end(); ++aListIter)
        {
            StatusStruct_Impl aStatus = *aListIter;
            if(aStatus.aURL.Complete.equalsAscii(cURLDocumentDataSource))
            {
                aEvent.FeatureURL = aStatus.aURL;
                aStatus.xListener->statusChanged( aEvent );
            }
        }
    }
    else
        throw uno::RuntimeException();

}

void SwXDispatch::addStatusListener(
    const uno::Reference< frame::XStatusListener >& xControl, const util::URL& aURL ) throw(uno::RuntimeException, std::exception)
{
    if(!m_pView)
        throw uno::RuntimeException();
    ShellModes eMode = m_pView->GetShellMode();
    bool bEnable = SHELL_MODE_TEXT == eMode  ||
                       SHELL_MODE_LIST_TEXT == eMode  ||
                       SHELL_MODE_TABLE_TEXT == eMode  ||
                       SHELL_MODE_TABLE_LIST_TEXT == eMode;

    m_bOldEnable = bEnable;
    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = bEnable;
    aEvent.Source = *static_cast<cppu::OWeakObject*>(this);
    aEvent.FeatureURL = aURL;

    // one of the URLs requires a special state ....
    if (aURL.Complete.equalsAscii(cURLDocumentDataSource))
    {
        const SwDBData& rData = m_pView->GetWrtShell().GetDBDesc();

        svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(rData.sDataSource);
        aDescriptor[svx::daCommand]       <<= rData.sCommand;
        aDescriptor[svx::daCommandType]   <<= rData.nCommandType;

        aEvent.State <<= aDescriptor.createPropertyValueSequence();
        aEvent.IsEnabled = !rData.sDataSource.isEmpty();
    }

    xControl->statusChanged( aEvent );

    StatusListenerList::iterator aListIter = m_aListenerList.begin();
    StatusStruct_Impl aStatus;
    aStatus.xListener = xControl;
    aStatus.aURL = aURL;
    m_aListenerList.insert(aListIter, aStatus);

    if(!m_bListenerAdded)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        uno::Reference<view::XSelectionChangeListener> xThis = this;
        xSupplier->addSelectionChangeListener(xThis);
        m_bListenerAdded = true;
    }
}

void SwXDispatch::removeStatusListener(
    const uno::Reference< frame::XStatusListener >& xControl, const util::URL&  ) throw(uno::RuntimeException, std::exception)
{
    StatusListenerList::iterator aListIter = m_aListenerList.begin();
    for(aListIter = m_aListenerList.begin(); aListIter != m_aListenerList.end(); ++aListIter)
    {
        StatusStruct_Impl aStatus = *aListIter;
        if(aStatus.xListener.get() == xControl.get())
        {
            m_aListenerList.erase(aListIter);
            break;
        }
    }
    if(m_aListenerList.empty() && m_pView)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        uno::Reference<view::XSelectionChangeListener> xThis = this;
        xSupplier->removeSelectionChangeListener(xThis);
        m_bListenerAdded = false;
    }
}

void SwXDispatch::selectionChanged( const lang::EventObject&  ) throw(uno::RuntimeException, std::exception)
{
    ShellModes eMode = m_pView->GetShellMode();
    bool bEnable = SHELL_MODE_TEXT == eMode  ||
                       SHELL_MODE_LIST_TEXT == eMode  ||
                       SHELL_MODE_TABLE_TEXT == eMode  ||
                       SHELL_MODE_TABLE_LIST_TEXT == eMode;
    if(bEnable != m_bOldEnable)
    {
        m_bOldEnable = bEnable;
        frame::FeatureStateEvent aEvent;
        aEvent.IsEnabled = bEnable;
        aEvent.Source = *static_cast<cppu::OWeakObject*>(this);

        StatusListenerList::iterator aListIter = m_aListenerList.begin();
        for(aListIter = m_aListenerList.begin(); aListIter != m_aListenerList.end(); ++aListIter)
        {
            StatusStruct_Impl aStatus = *aListIter;
            aEvent.FeatureURL = aStatus.aURL;
            if (!aStatus.aURL.Complete.equalsAscii(cURLDocumentDataSource))
                // the document's data source does not depend on the selection, so it's state does not change here
                aStatus.xListener->statusChanged( aEvent );
        }
    }
}

void SwXDispatch::disposing( const lang::EventObject& rSource ) throw(uno::RuntimeException, std::exception)
{
    uno::Reference<view::XSelectionSupplier> xSupplier(rSource.Source, uno::UNO_QUERY);
    uno::Reference<view::XSelectionChangeListener> xThis = this;
    xSupplier->removeSelectionChangeListener(xThis);
    m_bListenerAdded = false;

    lang::EventObject aObject;
    aObject.Source = static_cast<cppu::OWeakObject*>(this);
    StatusListenerList::iterator aListIter = m_aListenerList.begin();
    for(; aListIter != m_aListenerList.end(); ++aListIter)
    {
        StatusStruct_Impl aStatus = *aListIter;
        aStatus.xListener->disposing(aObject);
    }
    m_pView = nullptr;
}

const sal_Char* SwXDispatch::GetDBChangeURL()
{
    return cInternalDBChangeNotification;
}

SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::DispatchMutexLock_Impl(
                                                 SwXDispatchProviderInterceptor& )
{
}

SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::~DispatchMutexLock_Impl()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
