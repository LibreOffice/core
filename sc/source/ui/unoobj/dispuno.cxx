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

#include <sfx2/viewfrm.hxx>
#include <comphelper/uno3.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include "dispuno.hxx"
#include "tabvwsh.hxx"
#include "dbdocfun.hxx"
#include "dbdata.hxx"

using namespace com::sun::star;



static const char* cURLInsertColumns = ".uno:DataSourceBrowser/InsertColumns"; //data into text
static const char* cURLDocDataSource = ".uno:DataSourceBrowser/DocumentDataSource";



static uno::Reference<view::XSelectionSupplier> lcl_GetSelectionSupplier( SfxViewShell* pViewShell )
{
    if ( pViewShell )
    {
        SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
        if (pViewFrame)
        {
            return uno::Reference<view::XSelectionSupplier>( pViewFrame->GetFrame().GetController(), uno::UNO_QUERY );
        }
    }
    return uno::Reference<view::XSelectionSupplier>();
}




ScDispatchProviderInterceptor::ScDispatchProviderInterceptor(ScTabViewShell* pViewSh) :
    pViewShell( pViewSh )
{
    if ( pViewShell )
    {
        m_xIntercepted.set(uno::Reference<frame::XDispatchProviderInterception>(pViewShell->GetViewFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY));
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

void ScDispatchProviderInterceptor::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pViewShell = NULL;
}

// XDispatchProvider

uno::Reference<frame::XDispatch> SAL_CALL ScDispatchProviderInterceptor::queryDispatch(
                        const util::URL& aURL, const OUString& aTargetFrameName,
                        sal_Int32 nSearchFlags )
                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<frame::XDispatch> xResult;
    // create some dispatch ...
    if ( pViewShell && (
        aURL.Complete.equalsAscii(cURLInsertColumns) ||
        aURL.Complete.equalsAscii(cURLDocDataSource) ) )
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
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;
    return m_xSlaveDispatcher;
}

void SAL_CALL ScDispatchProviderInterceptor::setSlaveDispatchProvider(
                        const uno::Reference<frame::XDispatchProvider>& xNewDispatchProvider )
                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    m_xSlaveDispatcher.set(xNewDispatchProvider);
}

uno::Reference<frame::XDispatchProvider> SAL_CALL
                        ScDispatchProviderInterceptor::getMasterDispatchProvider()
                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return m_xMasterDispatcher;
}

void SAL_CALL ScDispatchProviderInterceptor::setMasterDispatchProvider(
                        const uno::Reference<frame::XDispatchProvider>& xNewSupplier )
                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    m_xMasterDispatcher.set(xNewSupplier);
}

// XEventListener

void SAL_CALL ScDispatchProviderInterceptor::disposing( const lang::EventObject& /* Source */ )
                                throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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



ScDispatch::ScDispatch(ScTabViewShell* pViewSh) :
    pViewShell( pViewSh ),
    bListeningToView( false )
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
        uno::Reference<view::XSelectionSupplier> xSupplier(lcl_GetSelectionSupplier( pViewShell ));
        if ( xSupplier.is() )
            xSupplier->removeSelectionChangeListener(this);
    }
}

void ScDispatch::Notify( SfxBroadcaster&, const SfxHint& rHint )
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
    SolarMutexGuard aGuard;

    sal_Bool bDone = false;
    if ( pViewShell && aURL.Complete.equalsAscii(cURLInsertColumns) )
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

static void lcl_FillDataSource( frame::FeatureStateEvent& rEvent, const ScImportParam& rParam )
{
    rEvent.IsEnabled = rParam.bImport;

    ::svx::ODataAccessDescriptor aDescriptor;
    if ( rParam.bImport )
    {
        sal_Int32 nType = rParam.bSql ? sdb::CommandType::COMMAND :
                    ( (rParam.nType == ScDbQuery) ? sdb::CommandType::QUERY :
                                                    sdb::CommandType::TABLE );

        aDescriptor.setDataSource(rParam.aDBName);
        aDescriptor[svx::daCommand]     <<= rParam.aStatement;
        aDescriptor[svx::daCommandType] <<= nType;
    }
    else
    {
        //  descriptor has to be complete anyway

        OUString aEmpty;
        aDescriptor[svx::daDataSource]  <<= aEmpty;
        aDescriptor[svx::daCommand]     <<= aEmpty;
        aDescriptor[svx::daCommandType] <<= (sal_Int32)sdb::CommandType::TABLE;
    }
    rEvent.State <<= aDescriptor.createPropertyValueSequence();
}

void SAL_CALL ScDispatch::addStatusListener(
    const uno::Reference<frame::XStatusListener>& xListener,
    const util::URL& aURL)
        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!pViewShell)
        throw uno::RuntimeException();

    //  initial state
    frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = sal_True;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    aEvent.FeatureURL = aURL;

    if ( aURL.Complete.equalsAscii(cURLDocDataSource) )
    {
        uno::Reference<frame::XStatusListener>* pObj =
                new uno::Reference<frame::XStatusListener>( xListener );
        aDataSourceListeners.push_back( pObj );

        if (!bListeningToView)
        {
            uno::Reference<view::XSelectionSupplier> xSupplier(lcl_GetSelectionSupplier( pViewShell ));
            if ( xSupplier.is() )
                xSupplier->addSelectionChangeListener(this);
            bListeningToView = true;
        }

        ScDBData* pDBData = pViewShell->GetDBData(false,SC_DB_OLD);
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
    SolarMutexGuard aGuard;

    if ( aURL.Complete.equalsAscii(cURLDocDataSource) )
    {
        sal_uInt16 nCount = aDataSourceListeners.size();
        for ( sal_uInt16 n=nCount; n--; )
        {
            uno::Reference<frame::XStatusListener>& rObj = aDataSourceListeners[n];
            if ( rObj == xListener )
            {
                aDataSourceListeners.erase( aDataSourceListeners.begin() + n );
                break;
            }
        }

        if ( aDataSourceListeners.empty() && pViewShell )
        {
            uno::Reference<view::XSelectionSupplier> xSupplier(lcl_GetSelectionSupplier( pViewShell ));
            if ( xSupplier.is() )
                xSupplier->removeSelectionChangeListener(this);
            bListeningToView = false;
        }
    }
}

// XSelectionChangeListener

void SAL_CALL ScDispatch::selectionChanged( const ::com::sun::star::lang::EventObject& /* aEvent */ )
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    //  currently only called for URL cURLDocDataSource

    if ( pViewShell )
    {
        ScImportParam aNewImport;
        ScDBData* pDBData = pViewShell->GetDBData(false,SC_DB_OLD);
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
            aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
            aEvent.FeatureURL.Complete = OUString::createFromAscii( cURLDocDataSource );

            lcl_FillDataSource( aEvent, aNewImport );       // modifies State, IsEnabled

            for ( sal_uInt16 n=0; n<aDataSourceListeners.size(); n++ )
                aDataSourceListeners[n]->statusChanged( aEvent );

            aLastImport = aNewImport;
        }
    }
}

// XEventListener

void SAL_CALL ScDispatch::disposing( const ::com::sun::star::lang::EventObject& rSource )
                                throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference<view::XSelectionSupplier> xSupplier(rSource.Source, uno::UNO_QUERY);
    xSupplier->removeSelectionChangeListener(this);
    bListeningToView = false;

    lang::EventObject aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    for ( sal_uInt16 n=0; n<aDataSourceListeners.size(); n++ )
        aDataSourceListeners[n]->disposing( aEvent );

    pViewShell = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
