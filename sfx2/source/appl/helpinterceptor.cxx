/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sfx2.hxx"

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

extern void AppendConfigToken_Impl( String& rURL, sal_Bool bQuestionMark ); // sfxhelp.cxx

// class HelpInterceptor_Impl --------------------------------------------

HelpInterceptor_Impl::HelpInterceptor_Impl() :

    m_pHistory  ( NULL ),
    m_nCurPos   ( 0 )

{
}

// -----------------------------------------------------------------------

HelpInterceptor_Impl::~HelpInterceptor_Impl()
{
    for ( sal_uInt16 i = 0; m_pHistory && i < m_pHistory->Count(); ++i )
        delete m_pHistory->GetObject(i);
    delete m_pHistory;
}

// -----------------------------------------------------------------------

void HelpInterceptor_Impl::addURL( const String& rURL )
{
  if ( !m_pHistory )
        m_pHistory = new HelpHistoryList_Impl;
    sal_uIntPtr nCount = m_pHistory->Count();
    if ( nCount && m_nCurPos < ( nCount - 1 ) )
    {
        for ( sal_uIntPtr i = nCount - 1; i > m_nCurPos; i-- )
            delete m_pHistory->Remove(i);
    }
    Reference<XFrame> xFrame(m_xIntercepted, UNO_QUERY);
    Reference<XController> xController;
    if(xFrame.is())
        xController = xFrame->getController();
    Any aViewData;
    if(xController.is() && m_pHistory->Count())
    {
        m_pHistory->GetObject(m_nCurPos)->aViewData = xController->getViewData();
    }

    m_aCurrentURL = rURL;
    Any aEmptyViewData;
    m_pHistory->Insert( new HelpHistoryEntry_Impl( rURL, aEmptyViewData ), LIST_APPEND );
    m_nCurPos = m_pHistory->Count() - 1;
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

void HelpInterceptor_Impl::SetStartURL( const String& rURL )
{
    DBG_ASSERT( !m_pHistory, "invalid history" );
    if ( !m_pHistory )
    {
        m_pHistory = new HelpHistoryList_Impl;
        Any aEmptyViewData;
        m_pHistory->Insert( new HelpHistoryEntry_Impl( rURL, aEmptyViewData ), ((sal_uIntPtr)0x0) );
        m_nCurPos = m_pHistory->Count() - 1;

        m_pWindow->UpdateToolbox();
    }
    m_aCurrentURL = rURL;
}

sal_Bool HelpInterceptor_Impl::HasHistoryPred() const
{
    return m_pHistory && ( m_nCurPos > 0 );
}

sal_Bool HelpInterceptor_Impl::HasHistorySucc() const
{
    return m_pHistory && ( m_nCurPos < ( m_pHistory->Count() - 1 ) );
}


// -----------------------------------------------------------------------
// XDispatchProvider

Reference< XDispatch > SAL_CALL HelpInterceptor_Impl::queryDispatch(

    const URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags )

    throw( RuntimeException )

{
    Reference< XDispatch > xResult;
    if ( m_xSlaveDispatcher.is() )
        xResult = m_xSlaveDispatcher->queryDispatch( aURL, aTargetFrameName, nSearchFlags );

    // INetURLObject aObj( aURL.Complete );
    // sal_Bool bHelpURL = ( aObj.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP );
    sal_Bool bHelpURL = aURL.Complete.toAsciiLowerCase().match(rtl::OUString::createFromAscii("vnd.sun.star.help"),0);

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

Sequence< ::rtl::OUString > SAL_CALL HelpInterceptor_Impl::getInterceptedURLs()

    throw( RuntimeException )

{
    Sequence< ::rtl::OUString > aURLList( 1 );
    aURLList[0] = DEFINE_CONST_UNICODE("vnd.sun.star.help://*");
    return aURLList;;
}

// -----------------------------------------------------------------------
// XDispatch

void SAL_CALL HelpInterceptor_Impl::dispatch(
    const URL& aURL, const Sequence< ::com::sun::star::beans::PropertyValue >& ) throw( RuntimeException )
{
    sal_Bool bBack = ( String( DEFINE_CONST_UNICODE(".uno:Backward") ) == String( aURL.Complete ) );
    if ( bBack || String( DEFINE_CONST_UNICODE(".uno:Forward") ) == String( aURL.Complete ) )
    {
        if ( m_pHistory )
        {
            if(m_pHistory->Count() > m_nCurPos)
            {
                Reference<XFrame> xFrame(m_xIntercepted, UNO_QUERY);
                Reference<XController> xController;
                if(xFrame.is())
                    xController = xFrame->getController();
                if(xController.is())
                {
                    m_pHistory->GetObject(m_nCurPos)->aViewData = xController->getViewData();
                }
            }

            sal_uIntPtr nPos = ( bBack && m_nCurPos > 0 ) ? --m_nCurPos
                                                    : ( !bBack && m_nCurPos < m_pHistory->Count() - 1 )
                                                    ? ++m_nCurPos
                                                    : ULONG_MAX;

            if ( nPos < ULONG_MAX )
            {
                HelpHistoryEntry_Impl* pEntry = m_pHistory->GetObject( nPos );
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
/*-- 05.09.2002 12:17:59---------------------------------------------------

  -----------------------------------------------------------------------*/
HelpStatusListener_Impl::HelpStatusListener_Impl(
        Reference < XDispatch > aDispatch, URL& rURL)
{
    aDispatch->addStatusListener(this, rURL);
}
/*-- 05.09.2002 12:17:59---------------------------------------------------

  -----------------------------------------------------------------------*/
HelpStatusListener_Impl::~HelpStatusListener_Impl()
{
    if(xDispatch.is())
        xDispatch->removeStatusListener(this, com::sun::star::util::URL());
}
/*-- 05.09.2002 12:17:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void HelpStatusListener_Impl::statusChanged(
    const FeatureStateEvent& rEvent ) throw( RuntimeException )
{
    aStateEvent = rEvent;
}
/*-- 05.09.2002 12:18:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void HelpStatusListener_Impl::disposing( const EventObject& ) throw( RuntimeException )
{
    xDispatch->removeStatusListener(this, com::sun::star::util::URL());
    xDispatch = 0;
}
