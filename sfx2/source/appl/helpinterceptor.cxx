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
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <vcl/window.hxx>
#include <limits.h>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;

extern void AppendConfigToken_Impl( OUString& rURL, sal_Bool bQuestionMark ); // sfxhelp.cxx

// class HelpInterceptor_Impl --------------------------------------------

HelpInterceptor_Impl::HelpInterceptor_Impl() :

    m_pHistory  ( NULL ),
    m_pWindow  ( NULL ),
    m_nCurPos   ( 0 )

{
}

// -----------------------------------------------------------------------

HelpInterceptor_Impl::~HelpInterceptor_Impl()
{
    if ( m_pHistory )
    {
        for ( size_t i = 0, n = m_pHistory->size(); i < n; ++i )
            delete m_pHistory->at( i );
        delete m_pHistory;
    }
}

// -----------------------------------------------------------------------

void HelpInterceptor_Impl::addURL( const OUString& rURL )
{
  if ( !m_pHistory )
        m_pHistory = new HelpHistoryList_Impl;

    size_t nCount = m_pHistory->size();
    if ( nCount && m_nCurPos < ( nCount - 1 ) )
    {
        for ( size_t i = nCount - 1; i > m_nCurPos; i-- )
        {
            delete m_pHistory->at( i );
            HelpHistoryList_Impl::iterator it = m_pHistory->begin();
            ::std::advance( it, i );
            m_pHistory->erase( it );
        }
    }
    Reference<XFrame> xFrame(m_xIntercepted, UNO_QUERY);
    Reference<XController> xController;
    if(xFrame.is())
        xController = xFrame->getController();
    Any aViewData;
    if(xController.is() && !m_pHistory->empty())
    {
        m_pHistory->at( m_nCurPos )->aViewData = xController->getViewData();
    }

    m_aCurrentURL = rURL;
    Any aEmptyViewData;
    m_pHistory->push_back( new HelpHistoryEntry_Impl( rURL, aEmptyViewData ) );
    m_nCurPos = m_pHistory->size() - 1;
// TODO ?
    if ( m_xListener.is() )
    {
        ::com::sun::star::frame::FeatureStateEvent aEvent;
        URL aURL;
        aURL.Complete = rURL;
        aEvent.FeatureURL = aURL;
        aEvent.Source = (::com::sun::star::frame::XDispatch*)this;
        m_xListener->statusChanged( aEvent );
    }

    m_pWindow->UpdateToolbox();
}

// -----------------------------------------------------------------------

void HelpInterceptor_Impl::setInterception( Reference< XFrame > xFrame )
{
    m_xIntercepted = Reference< XDispatchProviderInterception>( xFrame, UNO_QUERY );

    if ( m_xIntercepted.is() )
        m_xIntercepted->registerDispatchProviderInterceptor( (XDispatchProviderInterceptor*)this );
}

// -----------------------------------------------------------------------

sal_Bool HelpInterceptor_Impl::HasHistoryPred() const
{
    return m_pHistory && ( m_nCurPos > 0 );
}

sal_Bool HelpInterceptor_Impl::HasHistorySucc() const
{
    return m_pHistory && ( m_nCurPos < ( m_pHistory->size() - 1 ) );
}


// -----------------------------------------------------------------------
// XDispatchProvider

Reference< XDispatch > SAL_CALL HelpInterceptor_Impl::queryDispatch(

    const URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )

    throw( RuntimeException )

{
    Reference< XDispatch > xResult;
    if ( m_xSlaveDispatcher.is() )
        xResult = m_xSlaveDispatcher->queryDispatch( aURL, aTargetFrameName, nSearchFlags );

    sal_Bool bHelpURL = aURL.Complete.toAsciiLowerCase().match("vnd.sun.star.help",0);

    if ( bHelpURL )
    {
        DBG_ASSERT( xResult.is(), "invalid dispatch" );
        HelpDispatch_Impl* pHelpDispatch = new HelpDispatch_Impl( *this, xResult );
        xResult = Reference< XDispatch >( static_cast< ::cppu::OWeakObject* >(pHelpDispatch), UNO_QUERY );
    }

    return xResult;
}

// -----------------------------------------------------------------------

Sequence < Reference < XDispatch > > SAL_CALL HelpInterceptor_Impl::queryDispatches(

    const Sequence< DispatchDescriptor >& aDescripts )

    throw( RuntimeException )

{
    Sequence< Reference< XDispatch > > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for ( sal_Int16 i = 0; i < aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}

// -----------------------------------------------------------------------
// XDispatchProviderInterceptor

Reference< XDispatchProvider > SAL_CALL HelpInterceptor_Impl::getSlaveDispatchProvider()

    throw( RuntimeException )

{
    return m_xSlaveDispatcher;
}

// -----------------------------------------------------------------------

void SAL_CALL HelpInterceptor_Impl::setSlaveDispatchProvider( const Reference< XDispatchProvider >& xNewSlave )

    throw( RuntimeException )

{
    m_xSlaveDispatcher = xNewSlave;
}

// -----------------------------------------------------------------------

Reference< XDispatchProvider > SAL_CALL HelpInterceptor_Impl::getMasterDispatchProvider()

    throw( RuntimeException )

{
    return m_xMasterDispatcher;
}

// -----------------------------------------------------------------------

void SAL_CALL HelpInterceptor_Impl::setMasterDispatchProvider( const Reference< XDispatchProvider >& xNewMaster )

    throw( RuntimeException )

{
    m_xMasterDispatcher = xNewMaster;
}

// -----------------------------------------------------------------------
// XInterceptorInfo

Sequence< OUString > SAL_CALL HelpInterceptor_Impl::getInterceptedURLs()

    throw( RuntimeException )

{
    Sequence< OUString > aURLList( 1 );
    aURLList[0] = "vnd.sun.star.help://*";
    return aURLList;
}

// -----------------------------------------------------------------------
// XDispatch

void SAL_CALL HelpInterceptor_Impl::dispatch(
    const URL& aURL, const Sequence< ::com::sun::star::beans::PropertyValue >& ) throw( RuntimeException )
{
    sal_Bool bBack = ( OUString( ".uno:Backward" ) == OUString( aURL.Complete ) );
    if ( bBack || OUString( ".uno:Forward" ) == OUString( aURL.Complete ) )
    {
        if ( m_pHistory )
        {
            if(m_pHistory->size() > m_nCurPos)
            {
                Reference<XFrame> xFrame(m_xIntercepted, UNO_QUERY);
                Reference<XController> xController;
                if(xFrame.is())
                    xController = xFrame->getController();
                if(xController.is())
                {
                    m_pHistory->at( m_nCurPos )->aViewData = xController->getViewData();
                }
            }

            sal_uIntPtr nPos = ( bBack && m_nCurPos > 0 ) ? --m_nCurPos
                                                    : ( !bBack && m_nCurPos < m_pHistory->size() - 1 )
                                                    ? ++m_nCurPos
                                                    : ULONG_MAX;

            if ( nPos < ULONG_MAX )
            {
                HelpHistoryEntry_Impl* pEntry = m_pHistory->at( nPos );
                if ( pEntry )
                    m_pWindow->loadHelpContent(pEntry->aURL, sal_False); // false => dont add item to history again!
            }

            m_pWindow->UpdateToolbox();
        }
    }
}

// -----------------------------------------------------------------------

void SAL_CALL HelpInterceptor_Impl::addStatusListener(
    const Reference< XStatusListener >& xControl, const URL& ) throw( RuntimeException )
{
    DBG_ASSERT( !m_xListener.is(), "listener already exists" );
    m_xListener = xControl;
}

// -----------------------------------------------------------------------

void SAL_CALL HelpInterceptor_Impl::removeStatusListener(
    const Reference< XStatusListener >&, const URL&) throw( RuntimeException )
{
    m_xListener = 0;
}

// HelpListener_Impl -----------------------------------------------------

HelpListener_Impl::HelpListener_Impl( HelpInterceptor_Impl* pInter )
{
    pInterceptor = pInter;
    pInterceptor->addStatusListener( this, ::com::sun::star::util::URL() );
}

// -----------------------------------------------------------------------

void SAL_CALL HelpListener_Impl::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )

    throw( ::com::sun::star::uno::RuntimeException )

{
    INetURLObject aObj( Event.FeatureURL.Complete );
    aFactory = aObj.GetHost();
    aChangeLink.Call( this );
}

// -----------------------------------------------------------------------

void SAL_CALL HelpListener_Impl::disposing( const ::com::sun::star::lang::EventObject& )

    throw( ::com::sun::star::uno::RuntimeException )

{
    pInterceptor->removeStatusListener( this, ::com::sun::star::util::URL() );
    pInterceptor = NULL;
}

HelpStatusListener_Impl::HelpStatusListener_Impl(
        Reference < XDispatch > aDispatch, URL& rURL)
{
    aDispatch->addStatusListener(this, rURL);
}

HelpStatusListener_Impl::~HelpStatusListener_Impl()
{
    if(xDispatch.is())
        xDispatch->removeStatusListener(this, com::sun::star::util::URL());
}

void HelpStatusListener_Impl::statusChanged(
    const FeatureStateEvent& rEvent ) throw( RuntimeException )
{
    aStateEvent = rEvent;
}

void HelpStatusListener_Impl::disposing( const EventObject& ) throw( RuntimeException )
{
    xDispatch->removeStatusListener(this, com::sun::star::util::URL());
    xDispatch = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
