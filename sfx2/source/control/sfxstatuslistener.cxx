/*************************************************************************
 *
 *  $RCSfile: sfxstatuslistener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:34:22 $
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

#ifndef _SFXSTATUSLISTENER_HXX
#include <sfxstatuslistener.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#include <svtools/intitem.hxx>
#include <svtools/itemset.hxx>
#include <svtools/itemdel.hxx>

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#include <comphelper/processfactory.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATUS_HPP_
#include <com/sun/star/frame/status/ItemStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATE_HPP_
#include <com/sun/star/frame/status/ItemState.hpp>
#endif

#include "viewfrm.hxx"
#include "dispatch.hxx"
#include "unoctitm.hxx"
#include "msgpool.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SFX_IMPL_XINTERFACE_3( SfxStatusListener, OWeakObject, ::com::sun::star::lang::XComponent, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )
SFX_IMPL_XTYPEPROVIDER_3( SfxStatusListener, ::com::sun::star::lang::XComponent, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )

SfxStatusListener::SfxStatusListener( const Reference< XDispatchProvider >& rDispatchProvider, USHORT nSlotId, const OUString& rCommand ) :
    cppu::OWeakObject(),
    m_nSlotID( nSlotId ),
    m_xDispatchProvider( rDispatchProvider )
{
    m_aCommand.Complete = rCommand;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                            rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( m_aCommand );
    if ( rDispatchProvider.is() )
        m_xDispatch = rDispatchProvider->queryDispatch( m_aCommand, rtl::OUString(), 0 );
}

SfxStatusListener::~SfxStatusListener()
{
}

// old sfx controller item C++ API
void SfxStatusListener::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    // must be implemented by sub class
}

void SfxStatusListener::Bind()
{
    if ( !m_xDispatch.is() && m_xDispatchProvider.is() )
    {
        m_xDispatch = m_xDispatchProvider->queryDispatch( m_aCommand, rtl::OUString(), 0 );
        try
        {
            Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
            m_xDispatch->addStatusListener( aStatusListener, m_aCommand );
        }
        catch( Exception& )
        {
        }
    }
}

void SfxStatusListener::Bind( USHORT nSlotId, const rtl::OUString& rNewCommand )
{
    // first remove old listener, if we have a dispatch object
    Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    if ( m_xDispatch.is() )
        m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
    if ( m_xDispatchProvider.is() )
    {
        // Store new command data and query for new dispatch
        m_nSlotID = nSlotId;
        m_aCommand.Complete = rNewCommand;
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                                rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( m_aCommand );

        m_xDispatch = m_xDispatchProvider->queryDispatch( m_aCommand, rtl::OUString(), 0 );

        try
        {
            m_xDispatch->addStatusListener( aStatusListener, m_aCommand );
        }
        catch( Exception& )
        {
        }
    }
}

void SfxStatusListener::UnBind()
{
    if ( m_xDispatch.is() )
    {
        Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
        m_xDispatch.clear();
    }
}

void SfxStatusListener::ReBind()
{
    Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    if ( m_xDispatch.is() )
        m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
    if ( m_xDispatchProvider.is() )
    {
        try
        {
            m_xDispatch = m_xDispatchProvider->queryDispatch( m_aCommand, rtl::OUString(), 0 );
            if ( m_xDispatch.is() )
                m_xDispatch->addStatusListener( aStatusListener, m_aCommand );
        }
        catch( Exception& )
        {
        }
    }
}

// new UNO API
void SAL_CALL SfxStatusListener::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    if ( m_xDispatch.is() && m_aCommand.Complete.getLength() > 0 )
    {
        try
        {
            Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
            m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
        }
        catch ( Exception& )
        {
        }
    }

    m_xDispatch.clear();
    m_xDispatchProvider.clear();
}

void SAL_CALL SfxStatusListener::addEventListener( const Reference< XEventListener >& xListener )
throw ( RuntimeException )
{
    // do nothing - this is a wrapper class which does not support listeners
}

void SAL_CALL SfxStatusListener::removeEventListener( const Reference< XEventListener >& aListener )
throw ( RuntimeException )
{
    // do nothing - this is a wrapper class which does not support listeners
}

void SAL_CALL SfxStatusListener::disposing( const EventObject& Source )
throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( Source.Source == Reference< XInterface >( m_xDispatch, UNO_QUERY ))
        m_xDispatch.clear();
    else if ( Source.Source == Reference< XInterface >( m_xDispatchProvider, UNO_QUERY ))
        m_xDispatchProvider.clear();
}

void SAL_CALL SfxStatusListener::statusChanged( const FeatureStateEvent& rEvent)
throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxViewFrame* pViewFrame = NULL;
    if ( m_xDispatch.is() )
    {
        Reference< XUnoTunnel > xTunnel( m_xDispatch, UNO_QUERY );
        SfxOfficeDispatch* pDisp = NULL;
        if ( xTunnel.is() )
        {
            sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
            pDisp = (SfxOfficeDispatch*)(nImplementation);
        }

        if ( pDisp )
            pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
    }

    SfxSlotPool& rPool = SFX_APP()->GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetSlot( m_nSlotID );

    SfxItemState eState = SFX_ITEM_DISABLED;
    SfxPoolItem* pItem = NULL;
    if ( rEvent.IsEnabled )
    {
        eState = SFX_ITEM_AVAILABLE;
        ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

        if ( pType == ::getVoidCppuType() )
        {
            pItem = new SfxVoidItem( m_nSlotID );
            eState = SFX_ITEM_UNKNOWN;
        }
        if ( pType == ::getBooleanCppuType() )
        {
            sal_Bool bTemp ;
            rEvent.State >>= bTemp ;
            pItem = new SfxBoolItem( m_nSlotID, bTemp );
        }
        else if ( pType == ::getCppuType((const sal_uInt16*)0) )
        {
            sal_uInt16 nTemp ;
            rEvent.State >>= nTemp ;
            pItem = new SfxUInt16Item( m_nSlotID, nTemp );
        }
        else if ( pType == ::getCppuType((const sal_uInt32*)0) )
        {
            sal_uInt32 nTemp ;
            rEvent.State >>= nTemp ;
            pItem = new SfxUInt32Item( m_nSlotID, nTemp );
        }
        else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
        {
            ::rtl::OUString sTemp ;
            rEvent.State >>= sTemp ;
            pItem = new SfxStringItem( m_nSlotID, sTemp );
        }
        else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
        {
            ItemStatus aItemStatus;
            rEvent.State >>= aItemStatus;
            eState = aItemStatus.State;
            pItem = new SfxVoidItem( m_nSlotID );
        }
        else
        {
            if ( pSlot )
                pItem = pSlot->GetType()->CreateItem();
            if ( pItem )
            {
                pItem->SetWhich( m_nSlotID );
                pItem->PutValue( rEvent.State );
            }
            else
                pItem = new SfxVoidItem( m_nSlotID );
        }
    }

    StateChanged( m_nSlotID, eState, pItem );
    delete pItem;
}

