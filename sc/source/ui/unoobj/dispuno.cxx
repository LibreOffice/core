/*************************************************************************
 *
 *  $RCSfile: dispuno.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:33:06 $
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
#include <svx/dataaccessdescriptor.hxx>
#include <svtools/smplhint.hxx>

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include "dispuno.hxx"
#include "unoguard.hxx"
#include "tabvwsh.hxx"
#include "dbdocfun.hxx"
#include "dbcolect.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

const char* cURLInsertColumns = ".uno:DataSourceBrowser/InsertColumns"; //data into text
const char* cURLDocDataSource = ".uno:DataSourceBrowser/DocumentDataSource";

//------------------------------------------------------------------------

SV_IMPL_PTRARR( XStatusListenerArr_Impl, XStatusListenerPtr );

//------------------------------------------------------------------------

uno::Reference<view::XSelectionSupplier> lcl_GetSelectionSupplier( SfxViewShell* pViewShell )
{
    if ( pViewShell )
    {
        SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
        if (pViewFrame)
        {
            SfxFrame* pFrame = pViewFrame->GetFrame();
            if (pFrame)
                return uno::Reference<view::XSelectionSupplier>( pFrame->GetController(), uno::UNO_QUERY );
        }
    }
    return uno::Reference<view::XSelectionSupplier>();
}

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

        StartListening(*pViewShell);
    }
}

ScDispatchProviderInterceptor::~ScDispatchProviderInterceptor()
{
    if (pViewShell)
        EndListening(*pViewShell);
}

void ScDispatchProviderInterceptor::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pViewShell = NULL;
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
    if ( pViewShell && (
        !aURL.Complete.compareToAscii(cURLInsertColumns) ||
        !aURL.Complete.compareToAscii(cURLDocDataSource) ) )
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
    pViewShell( pViewSh ),
    bListeningToView( FALSE )
{
    if (pViewShell)
        StartListening(*pViewShell);
}

ScDispatch::~ScDispatch()
{
    if (pViewShell)
        EndListening(*pViewShell);

    if (bListeningToView && pViewShell)
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = lcl_GetSelectionSupplier( pViewShell );
        if ( xSupplier.is() )
        {
            uno::Reference<view::XSelectionChangeListener> xThis = this;
            xSupplier->removeSelectionChangeListener(xThis);
        }
    }
}

void ScDispatch::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pViewShell = NULL;
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
    // cURLDocDataSource is never dispatched

    if (!bDone)
        throw uno::RuntimeException();
}

void lcl_FillDataSource( frame::FeatureStateEvent& rEvent, const ScImportParam& rParam )
{
    rEvent.IsEnabled = rParam.bImport;

    ::svx::ODataAccessDescriptor aDescriptor;
    if ( rParam.bImport )
    {
        sal_Int32 nType = rParam.bSql ? sdb::CommandType::COMMAND :
                    ( (rParam.nType == ScDbQuery) ? sdb::CommandType::QUERY :
                                                    sdb::CommandType::TABLE );

        aDescriptor.setDataSource(rtl::OUString( rParam.aDBName ));
        aDescriptor[svx::daCommand]     <<= rtl::OUString( rParam.aStatement );
        aDescriptor[svx::daCommandType] <<= nType;
    }
    else
    {
        //  descriptor has to be complete anyway

        rtl::OUString aEmpty;
        aDescriptor[svx::daDataSource]  <<= aEmpty;
        aDescriptor[svx::daCommand]     <<= aEmpty;
        aDescriptor[svx::daCommandType] <<= (sal_Int32)sdb::CommandType::TABLE;
    }
    rEvent.State <<= aDescriptor.createPropertyValueSequence();
}

void SAL_CALL ScDispatch::addStatusListener(
                                const uno::Reference<frame::XStatusListener>& xListener,
                                const util::URL& aURL )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (!pViewShell)
        throw uno::RuntimeException();

    //  initial state
    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = sal_True;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    aEvent.FeatureURL = aURL;

    if ( !aURL.Complete.compareToAscii(cURLDocDataSource) )
    {
        uno::Reference<frame::XStatusListener>* pObj =
                new uno::Reference<frame::XStatusListener>( xListener );
        aDataSourceListeners.Insert( pObj, aDataSourceListeners.Count() );

        if (!bListeningToView)
        {
            uno::Reference<view::XSelectionSupplier> xSupplier = lcl_GetSelectionSupplier( pViewShell );
            if ( xSupplier.is() )
            {
                uno::Reference<view::XSelectionChangeListener> xThis = this;
                xSupplier->addSelectionChangeListener(xThis);
            }
            bListeningToView = sal_True;
        }

        ScDBData* pDBData = pViewShell->GetDBData(FALSE,SC_DB_OLD);
        if ( pDBData )
            pDBData->GetImportParam( aLastImport );
        lcl_FillDataSource( aEvent, aLastImport );          // modifies State, IsEnabled
    }
    //! else add to listener for "enabled" changes?

    xListener->statusChanged( aEvent );
}

void SAL_CALL ScDispatch::removeStatusListener(
                                const uno::Reference<frame::XStatusListener>& xListener,
                                const util::URL& aURL )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !aURL.Complete.compareToAscii(cURLDocDataSource) )
    {
        USHORT nCount = aDataSourceListeners.Count();
        for ( USHORT n=nCount; n--; )
        {
            uno::Reference<frame::XStatusListener> *pObj = aDataSourceListeners[n];
            if ( *pObj == xListener )
            {
                aDataSourceListeners.DeleteAndDestroy( n );
                break;
            }
        }

        if ( aDataSourceListeners.Count() == 0 && pViewShell )
        {
            uno::Reference<view::XSelectionSupplier> xSupplier = lcl_GetSelectionSupplier( pViewShell );
            if ( xSupplier.is() )
            {
                uno::Reference<view::XSelectionChangeListener> xThis = this;
                xSupplier->removeSelectionChangeListener(xThis);
            }
            bListeningToView = sal_False;
        }
    }
}

// XSelectionChangeListener

void SAL_CALL ScDispatch::selectionChanged( const ::com::sun::star::lang::EventObject& aEvent )
                                throw (::com::sun::star::uno::RuntimeException)
{
    //  currently only called for URL cURLDocDataSource

    if ( pViewShell )
    {
        ScImportParam aNewImport;
        ScDBData* pDBData = pViewShell->GetDBData(FALSE,SC_DB_OLD);
        if ( pDBData )
            pDBData->GetImportParam( aNewImport );

        //  notify listeners only if data source has changed
        if ( aNewImport.bImport    != aLastImport.bImport ||
             aNewImport.aDBName    != aLastImport.aDBName ||
             aNewImport.aStatement != aLastImport.aStatement ||
             aNewImport.bSql       != aLastImport.bSql ||
             aNewImport.nType      != aLastImport.nType )
        {
            frame::FeatureStateEvent aEvent;
            aEvent.Source = static_cast<cppu::OWeakObject*>(this);
            aEvent.FeatureURL.Complete = rtl::OUString::createFromAscii( cURLDocDataSource );

            lcl_FillDataSource( aEvent, aNewImport );       // modifies State, IsEnabled

            for ( USHORT n=0; n<aDataSourceListeners.Count(); n++ )
                (*aDataSourceListeners[n])->statusChanged( aEvent );

            aLastImport = aNewImport;
        }
    }
}

// XEventListener

void SAL_CALL ScDispatch::disposing( const ::com::sun::star::lang::EventObject& rSource )
                                throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference<view::XSelectionSupplier> xSupplier(rSource.Source, uno::UNO_QUERY);
    uno::Reference<view::XSelectionChangeListener> xThis = this;
    xSupplier->removeSelectionChangeListener(xThis);
    bListeningToView = sal_False;

    lang::EventObject aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    for ( USHORT n=0; n<aDataSourceListeners.Count(); n++ )
        (*aDataSourceListeners[n])->disposing( aEvent );

    pViewShell = NULL;
}

