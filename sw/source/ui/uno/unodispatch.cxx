/*************************************************************************
 *
 *  $RCSfile: unodispatch.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:23:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _UNODISPATCH_HXX
#include <unodispatch.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _SWVIEW_HXX
#include <view.hxx>
#endif
#include <cmdid.h>
#include "wrtsh.hxx"
#include "dbmgr.hxx"

#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
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
    Reference< XFrame> xUnoFrame = pFrame->GetFrameInterface();
    m_xIntercepted = Reference< XDispatchProviderInterception>(xUnoFrame, UNO_QUERY);
    if(m_xIntercepted.is())
    {
        m_refCount++;
        m_xIntercepted->registerDispatchProviderInterceptor((XDispatchProviderInterceptor*)this);
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
        Reference< XComponent> xInterceptedComponent(m_xIntercepted, UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->addEventListener((XEventListener*)this);
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
Reference< XDispatch > SwXDispatchProviderInterceptor::queryDispatch(
    const URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
        throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    Reference< XDispatch> xResult;
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
Sequence< Reference< XDispatch > > SwXDispatchProviderInterceptor::queryDispatches(
    const Sequence< DispatchDescriptor >& aDescripts ) throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    Sequence< Reference< XDispatch> > aReturn(aDescripts.getLength());
    Reference< XDispatch>* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts)
    {
        *pReturn = queryDispatch(pDescripts->FeatureURL,
                pDescripts->FrameName, pDescripts->SearchFlags);
    }
    return aReturn;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XDispatchProvider > SwXDispatchProviderInterceptor::getSlaveDispatchProvider(  )
        throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    return m_xSlaveDispatcher;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatchProviderInterceptor::setSlaveDispatchProvider(
    const Reference< XDispatchProvider >& xNewDispatchProvider ) throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    m_xSlaveDispatcher = xNewDispatchProvider;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XDispatchProvider > SwXDispatchProviderInterceptor::getMasterDispatchProvider(  )
        throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    return m_xMasterDispatcher;
}
/*-- 07.11.00 13:25:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatchProviderInterceptor::setMasterDispatchProvider(
    const Reference< XDispatchProvider >& xNewSupplier ) throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    m_xMasterDispatcher = xNewSupplier;
}
/*-- 07.11.00 13:25:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatchProviderInterceptor::disposing( const EventObject& Source )
    throw(RuntimeException)
{
    DispatchMutexLock_Impl aLock(*this);
    if (m_xIntercepted.is())
    {
        m_xIntercepted->releaseDispatchProviderInterceptor((XDispatchProviderInterceptor*)this);
        Reference< XComponent> xInterceptedComponent(m_xIntercepted, UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener((XEventListener*)this);
        m_xDispatch       = 0;
    }
    m_xIntercepted = NULL;
}
/* -----------------------------01.10.2001 14:31------------------------------

 ---------------------------------------------------------------------------*/
const Sequence< sal_Int8 > & SwXDispatchProviderInterceptor::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------01.10.2001 14:31------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SwXDispatchProviderInterceptor::getSomething(
    const Sequence< sal_Int8 >& aIdentifier )
        throw(RuntimeException)
{
    if( aIdentifier.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        aIdentifier.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
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
        m_xIntercepted->releaseDispatchProviderInterceptor((XDispatchProviderInterceptor*)this);
        Reference< XComponent> xInterceptedComponent(m_xIntercepted, UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener((XEventListener*)this);
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
        Reference<XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        Reference<XSelectionChangeListener> xThis = this;
        xSupplier->removeSelectionChangeListener(xThis);
    }
}
/*-- 07.11.00 14:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatch::dispatch(
    const URL& aURL, const Sequence< PropertyValue >& aArgs ) throw(RuntimeException)
{
    if(!m_pView)
        throw RuntimeException();
    SwWrtShell& rSh = m_pView->GetWrtShell();
    SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
    if(!aURL.Complete.compareToAscii(cURLInsertContent))
    {
        ::svx::ODataAccessDescriptor aDescriptor(aArgs);
        pNewDBMgr->MergeNew(DBMGR_MERGE, rSh, aDescriptor);
    }
    else if(!aURL.Complete.compareToAscii(cURLInsertColumns))
    {
        pNewDBMgr->InsertText(rSh, aArgs);
    }
    else if(!aURL.Complete.compareToAscii(cURLFormLetter))
    {
        pNewDBMgr->ExecuteFormLetter(rSh, aArgs);
    }
    else if(!aURL.Complete.compareToAscii(cURLDocumentDataSource))
    {
        OSL_ENSURE(sal_False, "SwXDispatch::dispatch: this URL is not to be dispatched!");
    }
    else if(!aURL.Complete.compareToAscii(cInternalDBChangeNotification))
    {
        FeatureStateEvent aEvent;
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
        throw RuntimeException();

}
/*-- 07.11.00 14:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatch::addStatusListener(
    const Reference< XStatusListener >& xControl, const URL& aURL ) throw(RuntimeException)
{
    if(!m_pView)
        throw RuntimeException();
    ShellModes eMode = m_pView->GetShellMode();
    sal_Bool bEnable = SEL_TEXT == eMode  ||
                       SEL_LIST_TEXT == eMode  ||
                       SEL_TABLE_TEXT == eMode  ||
                       SEL_TABLE_LIST_TEXT == eMode;

    m_bOldEnable = bEnable;
    FeatureStateEvent aEvent;
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
        Reference<XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        Reference<XSelectionChangeListener> xThis = this;
        xSupplier->addSelectionChangeListener(xThis);
        m_bListenerAdded = sal_True;
    }
}
/*-- 07.11.00 14:26:15---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDispatch::removeStatusListener(
    const Reference< XStatusListener >& xControl, const URL& aURL ) throw(RuntimeException)
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
        Reference<XSelectionSupplier> xSupplier = m_pView->GetUNOObject();
        Reference<XSelectionChangeListener> xThis = this;
        xSupplier->removeSelectionChangeListener(xThis);
        m_bListenerAdded = sal_False;
    }
}
/* -----------------------------07.03.01 10:27--------------------------------

 ---------------------------------------------------------------------------*/
void SwXDispatch::selectionChanged( const EventObject& aEvent ) throw(RuntimeException)
{
    ShellModes eMode = m_pView->GetShellMode();
    sal_Bool bEnable = SEL_TEXT == eMode  ||
                       SEL_LIST_TEXT == eMode  ||
                       SEL_TABLE_TEXT == eMode  ||
                       SEL_TABLE_LIST_TEXT == eMode;
    if(bEnable != m_bOldEnable)
    {
        m_bOldEnable = bEnable;
        FeatureStateEvent aEvent;
        aEvent.IsEnabled = bEnable;
        sal_Bool Requery = FALSE;
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
void SwXDispatch::disposing( const EventObject& rSource ) throw(RuntimeException)
{
    Reference<XSelectionSupplier> xSupplier(rSource.Source, UNO_QUERY);
    Reference<XSelectionChangeListener> xThis = this;
    xSupplier->removeSelectionChangeListener(xThis);
    m_bListenerAdded = sal_False;

    EventObject aObject;
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
                                                 SwXDispatchProviderInterceptor& rInterceptor) :
//    aGuard(rInterceptor.m_aMutex) #102295# solar mutex has to be used currently
    aGuard(Application::GetSolarMutex())
{
}
/* -----------------------------09.09.2002 08:48------------------------------

 ---------------------------------------------------------------------------*/
SwXDispatchProviderInterceptor::DispatchMutexLock_Impl::~DispatchMutexLock_Impl()
{
}

