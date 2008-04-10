/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unodispatch.cxx,v $
 * $Revision: 1.23 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <vcl/svapp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <unodispatch.hxx>
#include <unoobj.hxx>
#include <view.hxx>
#include <cmdid.h>
#include "wrtsh.hxx"
#include "dbmgr.hxx"
#include <svx/dataaccessdescriptor.hxx>

using namespace ::com::sun::star;
using namespace rtl;
using namespace vos;

const char* cURLStart           = ".uno:DataSourceBrowser/";
const char* cURLFormLetter      = ".uno:DataSourceBrowser/FormLetter";
const char* cURLInsertContent   = ".uno:DataSourceBrowser/InsertContent";//data into fields
const char* cURLInsertColumns   = ".uno:DataSourceBrowser/InsertColumns";//data into text
const char* cURLDocumentDataSource  = ".uno:DataSourceBrowser/DocumentDataSource";//current data source of the document
const sal_Char* cInternalDBChangeNotification = ".uno::Writer/DataSourceChanged";
/*-- 07.11.00 13:25:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDispatchProviderInterceptor::SwXDispatchProviderInterceptor(SwView& rVw) :
    m_pView(&rVw)
{
    SfxFrame* pFrame = m_pView->GetViewFrame()->GetFrame();
    uno::Reference< frame::XFrame> xUnoFrame = pFrame->GetFrameInterface();
    m_xIntercepted = uno::Reference< frame::XDispatchProviderInterception>(xUnoFrame, uno::UNO_QUERY);
    if(m_xIntercepted.is())
    {
        m_refCount++;
        m_xIntercepted->registerDispatchProviderInterceptor((frame::XDispatchProviderInterceptor*)this);
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->addEventListener((lang::XEventListener*)this);
        m_refCount--;
    }
}
/*-- 07.11.00 13:25:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDispatchProviderInterceptor::~SwXDispatchProviderInterceptor()
{
}
/*-- 07.11.00 13:25:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< frame::XDispatch > SwXDispatchProviderInterceptor::queryDispatch(
    const util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
        throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    uno::Reference< frame::XDispatch> xResult;
    // create some dispatch ...
    if(m_pView && !aURL.Complete.compareToAscii(cURLStart, 23))
    {
        if(!aURL.Complete.compareToAscii(cURLFormLetter) ||
            !aURL.Complete.compareToAscii(cURLInsertContent) ||
                !aURL.Complete.compareToAscii(cURLInsertColumns)||
                    !aURL.Complete.compareToAscii(cURLDocumentDataSource))
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
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Reference< frame::XDispatch > > SwXDispatchProviderInterceptor::queryDispatches(
    const uno::Sequence< frame::DispatchDescriptor >& aDescripts ) throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    uno::Sequence< uno::Reference< frame::XDispatch> > aReturn(aDescripts.getLength());
    uno::Reference< frame::XDispatch>* pReturn = aReturn.getArray();
    const frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts)
    {
        *pReturn = queryDispatch(pDescripts->FeatureURL,
                pDescripts->FrameName, pDescripts->SearchFlags);
    }
    return aReturn;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< frame::XDispatchProvider > SwXDispatchProviderInterceptor::getSlaveDispatchProvider(  )
        throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    return m_xSlaveDispatcher;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatchProviderInterceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& xNewDispatchProvider ) throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    m_xSlaveDispatcher = xNewDispatchProvider;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< frame::XDispatchProvider > SwXDispatchProviderInterceptor::getMasterDispatchProvider(  )
        throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    return m_xMasterDispatcher;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatchProviderInterceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& xNewSupplier ) throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    m_xMasterDispatcher = xNewSupplier;
}
/*-- 07.11.00 13:25:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatchProviderInterceptor::disposing( const lang::EventObject& )
    throw(uno::RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    if (m_xIntercepted.is())
    {
        m_xIntercepted->releaseDispatchProviderInterceptor((frame::XDispatchProviderInterceptor*)this);
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener((lang::XEventListener*)this);
        m_xDispatch       = 0;
    }
    m_xIntercepted = NULL;
}
/* -----------------------------01.10.2001 14:31------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXDispatchProviderInterceptor::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------01.10.2001 14:31------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SwXDispatchProviderInterceptor::getSomething(
    const uno::Sequence< sal_Int8 >& aIdentifier )
        throw(uno::RuntimeException)
{
    if( aIdentifier.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        aIdentifier.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
    return 0;
}
/* -----------------------------01.10.2001 14:32------------------------------

 ---------------------------------------------------------------------------*/
void    SwXDispatchProviderInterceptor::Invalidate()
{
    DispatchMutexLock_Impl aLock(*this);
    if (m_xIntercepted.is())
    {
        m_xIntercepted->releaseDispatchProviderInterceptor((frame::XDispatchProviderInterceptor*)this);
        uno::Reference< lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener((lang::XEventListener*)this);
        m_xDispatch       = 0;
    }
    m_xIntercepted = NULL;
    m_pView = 0;
}
/* -----------------------------07.11.00 14:26--------------------------------

 ---------------------------------------------------------------------------*/
SwXDispatch::SwXDispatch(SwView& rVw) :
    m_pView(&rVw),
    m_bOldEnable(sal_False),
    m_bListenerAdded(sal_False)
{
}
/*-- 07.11.00 14:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDispatch::~SwXDispatch()
{
    if(m_bListenerAdded && m_pView)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        uno::Reference<view::XSelectionChangeListener> xThis = this;
        xSupplier->removeSelectionChangeListener(xThis);
    }
}
/*-- 07.11.00 14:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatch::dispatch(
    const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs ) throw(uno::RuntimeException)
{
    if(!m_pView)
        throw uno::RuntimeException();
    SwWrtShell& rSh = m_pView->GetWrtShell();
    SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
    if(!aURL.Complete.compareToAscii(cURLInsertContent))
    {
        ::svx::ODataAccessDescriptor aDescriptor(aArgs);
        SwMergeDescriptor aMergeDesc( DBMGR_MERGE, rSh, aDescriptor );
        pNewDBMgr->MergeNew(aMergeDesc);
    }
    else if(!aURL.Complete.compareToAscii(cURLInsertColumns))
    {
        pNewDBMgr->InsertText(rSh, aArgs);
    }
    else if(!aURL.Complete.compareToAscii(cURLFormLetter))
    {
        SfxUsrAnyItem aDBProperties(FN_PARAM_DATABASE_PROPERTIES, uno::makeAny(aArgs));
        m_pView->GetViewFrame()->GetDispatcher()->Execute(
            FN_MAILMERGE_WIZARD,
            SFX_CALLMODE_ASYNCHRON,
            &aDBProperties, 0L);
//      pNewDBMgr->ExecuteFormLetter(rSh, aArgs);
    }
    else if(!aURL.Complete.compareToAscii(cURLDocumentDataSource))
    {
        OSL_ENSURE(sal_False, "SwXDispatch::dispatch: this URL is not to be dispatched!");
    }
    else if(!aURL.Complete.compareToAscii(cInternalDBChangeNotification))
    {
        frame::FeatureStateEvent aEvent;
        aEvent.IsEnabled = sal_True;
        aEvent.Source = *(cppu::OWeakObject*)this;

        const SwDBData& rData = m_pView->GetWrtShell().GetDBDesc();
        ::svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(rData.sDataSource);
        aDescriptor[::svx::daCommand]       <<= rData.sCommand;
        aDescriptor[::svx::daCommandType]   <<= rData.nCommandType;

        aEvent.State <<= aDescriptor.createPropertyValueSequence();
        aEvent.IsEnabled = rData.sDataSource.getLength() > 0;

        StatusListenerList::iterator aListIter = m_aListenerList.begin();
        for(aListIter = m_aListenerList.begin(); aListIter != m_aListenerList.end(); ++aListIter)
        {
            StatusStruct_Impl aStatus = *aListIter;
            if(!aStatus.aURL.Complete.compareToAscii(cURLDocumentDataSource))
            {
                aEvent.FeatureURL = aStatus.aURL;
                aStatus.xListener->statusChanged( aEvent );
            }
        }
    }
    else
        throw uno::RuntimeException();

}
/*-- 07.11.00 14:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatch::addStatusListener(
    const uno::Reference< frame::XStatusListener >& xControl, const util::URL& aURL ) throw(uno::RuntimeException)
{
    if(!m_pView)
        throw uno::RuntimeException();
    ShellModes eMode = m_pView->GetShellMode();
    sal_Bool bEnable = SHELL_MODE_TEXT == eMode  ||
                       SHELL_MODE_LIST_TEXT == eMode  ||
                       SHELL_MODE_TABLE_TEXT == eMode  ||
                       SHELL_MODE_TABLE_LIST_TEXT == eMode;

    m_bOldEnable = bEnable;
    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = bEnable;
    aEvent.Source = *(cppu::OWeakObject*)this;
    aEvent.FeatureURL = aURL;

    // one of the URLs requires a special state ....
    if (!aURL.Complete.compareToAscii(cURLDocumentDataSource))
    {
        const SwDBData& rData = m_pView->GetWrtShell().GetDBDesc();

        ::svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(rData.sDataSource);
        aDescriptor[::svx::daCommand]       <<= rData.sCommand;
        aDescriptor[::svx::daCommandType]   <<= rData.nCommandType;

        aEvent.State <<= aDescriptor.createPropertyValueSequence();
        aEvent.IsEnabled = rData.sDataSource.getLength() > 0;
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
        m_bListenerAdded = sal_True;
    }
}
/*-- 07.11.00 14:26:15---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatch::removeStatusListener(
    const uno::Reference< frame::XStatusListener >& xControl, const util::URL&  ) throw(uno::RuntimeException)
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
        m_bListenerAdded = sal_False;
    }
}
/* -----------------------------07.03.01 10:27--------------------------------

 ---------------------------------------------------------------------------*/
void SwXDispatch::selectionChanged( const lang::EventObject&  ) throw(uno::RuntimeException)
{
    ShellModes eMode = m_pView->GetShellMode();
    sal_Bool bEnable = SHELL_MODE_TEXT == eMode  ||
                       SHELL_MODE_LIST_TEXT == eMode  ||
                       SHELL_MODE_TABLE_TEXT == eMode  ||
                       SHELL_MODE_TABLE_LIST_TEXT == eMode;
    if(bEnable != m_bOldEnable)
    {
        m_bOldEnable = bEnable;
        frame::FeatureStateEvent aEvent;
        aEvent.IsEnabled = bEnable;
        aEvent.Source = *(cppu::OWeakObject*)this;

        StatusListenerList::iterator aListIter = m_aListenerList.begin();
        for(aListIter = m_aListenerList.begin(); aListIter != m_aListenerList.end(); ++aListIter)
        {
            StatusStruct_Impl aStatus = *aListIter;
            aEvent.FeatureURL = aStatus.aURL;
            if (0 != aStatus.aURL.Complete.compareToAscii(cURLDocumentDataSource))
                // the document's data source does not depend on the selection, so it's state does not change here
                aStatus.xListener->statusChanged( aEvent );
        }
    }
}
/* -----------------------------07.03.01 10:46--------------------------------

 ---------------------------------------------------------------------------*/
void SwXDispatch::disposing( const lang::EventObject& rSource ) throw(uno::RuntimeException)
{
    uno::Reference<view::XSelectionSupplier> xSupplier(rSource.Source, uno::UNO_QUERY);
    uno::Reference<view::XSelectionChangeListener> xThis = this;
    xSupplier->removeSelectionChangeListener(xThis);
    m_bListenerAdded = sal_False;

    lang::EventObject aObject;
    aObject.Source = (cppu::OWeakObject*)this;
    StatusListenerList::iterator aListIter = m_aListenerList.begin();
    for(; aListIter != m_aListenerList.end(); ++aListIter)
    {
        StatusStruct_Impl aStatus = *aListIter;
        aStatus.xListener->disposing(aObject);
    }
    m_pView = 0;
}
/* -----------------------------12.07.01 13:30--------------------------------

 ---------------------------------------------------------------------------*/
const sal_Char* SwXDispatch::GetDBChangeURL()
{
    return cInternalDBChangeNotification;
}
/* -----------------------------09.09.2002 08:48------------------------------

 ---------------------------------------------------------------------------*/
SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::DispatchMutexLock_Impl(
                                                 SwXDispatchProviderInterceptor& ) :
//    aGuard(rInterceptor.m_aMutex) #102295# solar mutex has to be used currently
    aGuard(Application::GetSolarMutex())
{
}
/* -----------------------------09.09.2002 08:48------------------------------

 ---------------------------------------------------------------------------*/
SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::~DispatchMutexLock_Impl()
{
}

