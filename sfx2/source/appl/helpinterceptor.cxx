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


#include "helpinterceptor.hxx"
#include "helpdispatch.hxx"
#include "newhelp.hxx"
#include <sfx2/sfxuno.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <vcl/window.hxx>
#include <limits.h>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;

HelpInterceptor_Impl::HelpInterceptor_Impl() :

    m_pWindow  ( nullptr ),
    m_nCurPos   ( 0 )

{
}


HelpInterceptor_Impl::~HelpInterceptor_Impl()
{
}


void HelpInterceptor_Impl::addURL( const OUString& rURL )
{
    size_t nCount = m_vHistoryUrls.size();
    if ( nCount && m_nCurPos < ( nCount - 1 ) )
    {
        for ( size_t i = nCount - 1; i > m_nCurPos; i-- )
        {
            m_vHistoryUrls.erase( m_vHistoryUrls.begin() + i );
        }
    }
    Reference<XFrame> xFrame(m_xIntercepted, UNO_QUERY);
    Reference<XController> xController;
    if(xFrame.is())
        xController = xFrame->getController();

    m_aCurrentURL = rURL;
    m_vHistoryUrls.emplace_back( rURL );
    m_nCurPos = m_vHistoryUrls.size() - 1;
// TODO ?
    if ( m_xListener.is() )
    {
        css::frame::FeatureStateEvent aEvent;
        URL aURL;
        aURL.Complete = rURL;
        aEvent.FeatureURL = aURL;
        aEvent.Source = static_cast<css::frame::XDispatch*>(this);
        m_xListener->statusChanged( aEvent );
    }

    m_pWindow->UpdateToolbox();
}


void HelpInterceptor_Impl::setInterception( const Reference< XFrame >& xFrame )
{
    m_xIntercepted.set( xFrame, UNO_QUERY );

    if ( m_xIntercepted.is() )
        m_xIntercepted->registerDispatchProviderInterceptor( static_cast<XDispatchProviderInterceptor*>(this) );
}


bool HelpInterceptor_Impl::HasHistoryPred() const
{
    return m_nCurPos > 0;
}

bool HelpInterceptor_Impl::HasHistorySucc() const
{
    return m_nCurPos < ( m_vHistoryUrls.size() - 1 );
}


// XDispatchProvider

Reference< XDispatch > SAL_CALL HelpInterceptor_Impl::queryDispatch(

    const URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )

{
    Reference< XDispatch > xResult;
    if ( m_xSlaveDispatcher.is() )
        xResult = m_xSlaveDispatcher->queryDispatch( aURL, aTargetFrameName, nSearchFlags );

    bool bHelpURL = aURL.Complete.toAsciiLowerCase().match("vnd.sun.star.help",0);

    if ( bHelpURL )
    {
        DBG_ASSERT( xResult.is(), "invalid dispatch" );
        HelpDispatch_Impl* pHelpDispatch = new HelpDispatch_Impl( *this, xResult );
        xResult.set( static_cast< ::cppu::OWeakObject* >(pHelpDispatch), UNO_QUERY );
    }

    return xResult;
}


Sequence < Reference < XDispatch > > SAL_CALL HelpInterceptor_Impl::queryDispatches(

    const Sequence< DispatchDescriptor >& aDescripts )

{
    Sequence< Reference< XDispatch > > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for ( sal_Int32 i = 0; i < aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}


// XDispatchProviderInterceptor

Reference< XDispatchProvider > SAL_CALL HelpInterceptor_Impl::getSlaveDispatchProvider()

{
    return m_xSlaveDispatcher;
}


void SAL_CALL HelpInterceptor_Impl::setSlaveDispatchProvider( const Reference< XDispatchProvider >& xNewSlave )

{
    m_xSlaveDispatcher = xNewSlave;
}


Reference< XDispatchProvider > SAL_CALL HelpInterceptor_Impl::getMasterDispatchProvider()

{
    return m_xMasterDispatcher;
}


void SAL_CALL HelpInterceptor_Impl::setMasterDispatchProvider( const Reference< XDispatchProvider >& xNewMaster )

{
    m_xMasterDispatcher = xNewMaster;
}


// XInterceptorInfo

Sequence< OUString > SAL_CALL HelpInterceptor_Impl::getInterceptedURLs()

{
    Sequence<OUString> aURLList { "vnd.sun.star.help://*" };
    return aURLList;
}


// XDispatch

void SAL_CALL HelpInterceptor_Impl::dispatch(
    const URL& aURL, const Sequence< css::beans::PropertyValue >& )
{
    bool bBack = aURL.Complete == ".uno:Backward";
    if ( !bBack && aURL.Complete != ".uno:Forward" )
        return;

    if ( m_vHistoryUrls.empty() )
        return;

    sal_uIntPtr nPos = ( bBack && m_nCurPos > 0 ) ? --m_nCurPos
                                            : ( !bBack && m_nCurPos < m_vHistoryUrls.size() - 1 )
                                            ? ++m_nCurPos
                                            : ULONG_MAX;

    if ( nPos < ULONG_MAX )
    {
        m_pWindow->loadHelpContent(m_vHistoryUrls[nPos], false); // false => don't add item to history again!
    }

    m_pWindow->UpdateToolbox();
}


void SAL_CALL HelpInterceptor_Impl::addStatusListener(
    const Reference< XStatusListener >& xControl, const URL& )
{
    DBG_ASSERT( !m_xListener.is(), "listener already exists" );
    m_xListener = xControl;
}


void SAL_CALL HelpInterceptor_Impl::removeStatusListener(
    const Reference< XStatusListener >&, const URL&)
{
    m_xListener = nullptr;
}

// HelpListener_Impl -----------------------------------------------------

HelpListener_Impl::HelpListener_Impl( HelpInterceptor_Impl* pInter )
{
    pInterceptor = pInter;
    pInterceptor->addStatusListener( this, css::util::URL() );
}


void SAL_CALL HelpListener_Impl::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    INetURLObject aObj( Event.FeatureURL.Complete );
    aFactory = aObj.GetHost();
    aChangeLink.Call( *this );
}


void SAL_CALL HelpListener_Impl::disposing( const css::lang::EventObject& )
{
    pInterceptor->removeStatusListener( this, css::util::URL() );
    pInterceptor = nullptr;
}

HelpStatusListener_Impl::HelpStatusListener_Impl(
        Reference < XDispatch > const & aDispatch, URL const & rURL)
{
    aDispatch->addStatusListener(this, rURL);
}

HelpStatusListener_Impl::~HelpStatusListener_Impl()
{
    if(xDispatch.is())
        xDispatch->removeStatusListener(this, css::util::URL());
}

void HelpStatusListener_Impl::statusChanged(
    const FeatureStateEvent& rEvent )
{
    aStateEvent = rEvent;
}

void HelpStatusListener_Impl::disposing( const EventObject& )
{
    xDispatch->removeStatusListener(this, css::util::URL());
    xDispatch = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
