/*************************************************************************
 *
 *  $RCSfile: helpinterceptor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dv $ $Date: 2001-02-16 12:09:15 $
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

#include "helpinterceptor.hxx"
#include "sfxuno.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#include <limits.h>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

// class HelpInterceptor_Impl --------------------------------------------

HelpInterceptor_Impl::HelpInterceptor_Impl() :

    m_pHistory  ( NULL ),
    m_nCurPos   ( 0 )

{
}

// -----------------------------------------------------------------------

HelpInterceptor_Impl::~HelpInterceptor_Impl()
{
    for ( USHORT i = 0; m_pHistory && i < m_pHistory->Count(); ++i )
        delete m_pHistory->GetObject(i);
    delete m_pHistory;

    if ( m_xIntercepted.is() )
        m_xIntercepted->releaseDispatchProviderInterceptor( (XDispatchProviderInterceptor*)this );
}

// -----------------------------------------------------------------------

void HelpInterceptor_Impl::addURL( const String& rURL )
{
    if ( !m_pHistory )
        m_pHistory = new HelpHistoryList_Impl;
    ULONG nCount = m_pHistory->Count();
    if ( nCount && m_nCurPos < ( nCount - 1 ) )
    {
        for ( ULONG i = nCount - 1; i > m_nCurPos; i-- )
            delete m_pHistory->Remove(i);
    }

    m_pHistory->Insert( new HelpHistoryEntry_Impl( rURL ), LIST_APPEND );
    m_nCurPos = m_pHistory->Count() - 1;

    if ( m_xListener.is() )
    {
        ::com::sun::star::frame::FeatureStateEvent aEvent;
        URL aURL;
        aURL.Complete = rURL;
        aEvent.FeatureURL = aURL;
        aEvent.Source = (::com::sun::star::frame::XDispatch*)this;
        m_xListener->statusChanged( aEvent );
    }
}

// -----------------------------------------------------------------------

void HelpInterceptor_Impl::setInterception( Reference< XFrame > xFrame )
{
    m_xIntercepted = Reference< XDispatchProviderInterception>( xFrame, UNO_QUERY );

    if ( m_xIntercepted.is() )
        m_xIntercepted->registerDispatchProviderInterceptor( (XDispatchProviderInterceptor*)this );
}

// -----------------------------------------------------------------------

void HelpInterceptor_Impl::SetFactory( const String& rFactory )
{
    DBG_ASSERT( !m_pHistory, "invalid history" );
    if ( !m_pHistory )
    {
        m_pHistory = new HelpHistoryList_Impl;
        String aURL( DEFINE_CONST_UNICODE("vnd.sun.star.help://") );
        aURL += rFactory;
        aURL += String( DEFINE_CONST_UNICODE("/start") );
        m_pHistory->Insert( new HelpHistoryEntry_Impl( aURL ), ((ULONG)0x0) );
        m_nCurPos = m_pHistory->Count() - 1;
    }
}

// -----------------------------------------------------------------------
// XDispatchProvider

Reference< XDispatch > SAL_CALL HelpInterceptor_Impl::queryDispatch(

    const URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags )

    throw( RuntimeException )

{
    INetURLObject aObj( aURL.Complete );
    if ( aObj.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
        addURL( aURL.Complete );

    Reference< XDispatch > xResult;

    if ( !xResult.is() && m_xSlaveDispatcher.is() )
        xResult = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

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
    aURLList[0] = DEFINE_CONST_UNICODE("vnd.sun.star.help://");
    return aURLList;;
}

// -----------------------------------------------------------------------
// XDispatch

void SAL_CALL HelpInterceptor_Impl::dispatch(

    const URL& aURL, const Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw( RuntimeException )

{
    sal_Bool bBack = ( String( DEFINE_CONST_UNICODE(".uno:Backward") ) == String( aURL.Complete ) );
    if ( bBack || String( DEFINE_CONST_UNICODE(".uno:Forward") ) == String( aURL.Complete ) )
    {
        if ( m_pHistory )
        {
            ULONG nPos = ( bBack && m_nCurPos > 0 ) ? --m_nCurPos
                                                    : ( !bBack && m_nCurPos < m_pHistory->Count() - 1 )
                                                    ? ++m_nCurPos
                                                    : ULONG_MAX;

            if ( nPos < ULONG_MAX )
            {
                HelpHistoryEntry_Impl* pEntry = m_pHistory->GetObject( nPos );
                if ( pEntry )
                {
                    URL aURL;
                    aURL.Complete = pEntry->aURL;
                    Reference < XDispatch > xDisp = m_xSlaveDispatcher->queryDispatch( aURL, String(), 0 );
                    if ( xDisp.is() )
                        xDisp->dispatch( aURL, Sequence < PropertyValue >() );

                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void SAL_CALL HelpInterceptor_Impl::addStatusListener(

    const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
    DBG_ASSERT( !m_xListener.is(), "listener already exists" );
    m_xListener = xControl;
}

// -----------------------------------------------------------------------

void SAL_CALL HelpInterceptor_Impl::removeStatusListener(

    const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

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

void SAL_CALL HelpListener_Impl::disposing( const ::com::sun::star::lang::EventObject& obj )

    throw( ::com::sun::star::uno::RuntimeException )

{
    pInterceptor->removeStatusListener( this, ::com::sun::star::util::URL() );
    pInterceptor = NULL;
}

