/*************************************************************************
 *
 *  $RCSfile: dispuno.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-13 19:22:32 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <sfx2/viewfrm.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>

#include "dispuno.hxx"
#include "unoguard.hxx"
#include "tabvwsh.hxx"
#include "dbdocfun.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

const char* cURLInsertColumns = ".uno:DataSourceBrowser/InsertColumns"; //data into text

//------------------------------------------------------------------------

ScDispatchProviderInterceptor::ScDispatchProviderInterceptor(ScTabViewShell* pViewSh) :
    pViewShell( pViewSh )
{
    if ( pViewShell )
    {
        SfxFrame* pFrame = pViewShell->GetViewFrame()->GetFrame();
        uno::Reference<frame::XFrame> xUnoFrame = pFrame->GetFrameInterface();
        m_xIntercepted = uno::Reference<frame::XDispatchProviderInterception>(xUnoFrame, uno::UNO_QUERY);
        if (m_xIntercepted.is())
        {
            comphelper::increment( m_refCount );

            m_xIntercepted->registerDispatchProviderInterceptor(
                        static_cast<frame::XDispatchProviderInterceptor*>(this));
            // this should make us the top-level dispatch-provider for the component, via a call to our
            // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
            uno::Reference<lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
            if (xInterceptedComponent.is())
                xInterceptedComponent->addEventListener(static_cast<lang::XEventListener*>(this));

            comphelper::decrement( m_refCount );
        }
    }
}

ScDispatchProviderInterceptor::~ScDispatchProviderInterceptor()
{
}

// XDispatchProvider

uno::Reference<frame::XDispatch> SAL_CALL ScDispatchProviderInterceptor::queryDispatch(
                        const util::URL& aURL, const rtl::OUString& aTargetFrameName,
                        sal_Int32 nSearchFlags )
                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<frame::XDispatch> xResult;
    // create some dispatch ...
    if( !aURL.Complete.compareToAscii(cURLInsertColumns) )
    {
        if (!m_xMyDispatch.is())
            m_xMyDispatch = new ScDispatch( pViewShell );
        xResult = m_xMyDispatch;
    }

    // ask our slave provider
    if (!xResult.is() && m_xSlaveDispatcher.is())
        xResult = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    return xResult;
}

uno::Sequence< uno::Reference<frame::XDispatch> > SAL_CALL
                        ScDispatchProviderInterceptor::queryDispatches(
                        const uno::Sequence<frame::DispatchDescriptor>& aDescripts )
                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

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

// XDispatchProviderInterceptor

uno::Reference<frame::XDispatchProvider> SAL_CALL
                        ScDispatchProviderInterceptor::getSlaveDispatchProvider()
                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return m_xSlaveDispatcher;
}

void SAL_CALL ScDispatchProviderInterceptor::setSlaveDispatchProvider(
                        const uno::Reference<frame::XDispatchProvider>& xNewDispatchProvider )
                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    m_xSlaveDispatcher = xNewDispatchProvider;
}

uno::Reference<frame::XDispatchProvider> SAL_CALL
                        ScDispatchProviderInterceptor::getMasterDispatchProvider()
                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return m_xMasterDispatcher;
}

void SAL_CALL ScDispatchProviderInterceptor::setMasterDispatchProvider(
                        const uno::Reference<frame::XDispatchProvider>& xNewSupplier )
                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    m_xMasterDispatcher = xNewSupplier;
}

// XEventListener

void SAL_CALL ScDispatchProviderInterceptor::disposing( const lang::EventObject& Source )
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (m_xIntercepted.is())
    {
        m_xIntercepted->releaseDispatchProviderInterceptor(
                static_cast<frame::XDispatchProviderInterceptor*>(this));
        uno::Reference<lang::XComponent> xInterceptedComponent(m_xIntercepted, uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener(static_cast<lang::XEventListener*>(this));

        m_xMyDispatch = NULL;
    }
    m_xIntercepted = NULL;
}

//------------------------------------------------------------------------

ScDispatch::ScDispatch(ScTabViewShell* pViewSh) :
    pViewShell( pViewSh )
{
}

ScDispatch::~ScDispatch()
{
}

// XDispatch

void SAL_CALL ScDispatch::dispatch( const util::URL& aURL,
                                const uno::Sequence<beans::PropertyValue>& aArgs )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    BOOL bDone = FALSE;
    if ( pViewShell && !aURL.Complete.compareToAscii(cURLInsertColumns) )
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScAddress aPos( pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() );

        ScDBDocFunc aFunc( *pViewData->GetDocShell() );
        bDone = aFunc.DoImportUno( aPos, aArgs );
    }

    if (!bDone)
        throw uno::RuntimeException();
}

void SAL_CALL ScDispatch::addStatusListener(
                                const uno::Reference<frame::XStatusListener>& xControl,
                                const util::URL& aURL )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = sal_True;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    aEvent.FeatureURL = aURL;
    xControl->statusChanged( aEvent );

    //! ...
}

void SAL_CALL ScDispatch::removeStatusListener(
                                const uno::Reference<frame::XStatusListener>& xControl,
                                const util::URL& aURL )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! ...
}

