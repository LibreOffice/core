/*************************************************************************
 *
 *  $RCSfile: querystatus.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:34:08 $
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

#ifndef _SFXQUERYSTATUS_HXX
#include <querystatus.hxx>
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
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATUS_HPP_
#include <com/sun/star/frame/status/ItemStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATE_HPP_
#include <com/sun/star/frame/status/ItemState.hpp>
#endif

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SFX_IMPL_XINTERFACE_2( SfxQueryStatus, OWeakObject, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )
SFX_IMPL_XTYPEPROVIDER_2( SfxQueryStatus, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )

SfxQueryStatus::SfxQueryStatus( const Reference< XDispatchProvider >& rDispatchProvider, USHORT nSlotId, const OUString& rCommand ) :
    cppu::OWeakObject(),
    m_eState( SFX_ITEM_DISABLED ),
    m_pItem( 0 ),
    m_bQueryInProgress( sal_False ),
    m_nSlotID( nSlotId )
{
    m_aCommand.Complete = rCommand;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                            rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( m_aCommand );
    if ( rDispatchProvider.is() )
        m_xDispatch = rDispatchProvider->queryDispatch( m_aCommand, rtl::OUString(), 0 );
    m_aCondition.reset();
}

SfxQueryStatus::~SfxQueryStatus()
{
}

void SAL_CALL SfxQueryStatus::disposing( const EventObject& Source )
throw( RuntimeException )
{
    m_xDispatch.clear();
}

void SAL_CALL SfxQueryStatus::statusChanged( const FeatureStateEvent& rEvent)
throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    m_pItem  = NULL;
    m_eState = SFX_ITEM_DISABLED;

    if ( rEvent.IsEnabled )
    {
        m_eState = SFX_ITEM_AVAILABLE;
        ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

        if ( pType == ::getBooleanCppuType() )
        {
            sal_Bool bTemp ;
            rEvent.State >>= bTemp ;
            m_pItem = new SfxBoolItem( m_nSlotID, bTemp );
        }
        else if ( pType == ::getCppuType((const sal_uInt16*)0) )
        {
            sal_uInt16 nTemp ;
            rEvent.State >>= nTemp ;
            m_pItem = new SfxUInt16Item( m_nSlotID, nTemp );
        }
        else if ( pType == ::getCppuType((const sal_uInt32*)0) )
        {
            sal_uInt32 nTemp ;
            rEvent.State >>= nTemp ;
            m_pItem = new SfxUInt32Item( m_nSlotID, nTemp );
        }
        else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
        {
            ::rtl::OUString sTemp ;
            rEvent.State >>= sTemp ;
            m_pItem = new SfxStringItem( m_nSlotID, sTemp );
        }
        else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
        {
            ItemStatus aItemStatus;
            rEvent.State >>= aItemStatus;
            m_eState = aItemStatus.State;
            m_pItem = new SfxVoidItem( m_nSlotID );
        }
        else
        {
            m_eState = SFX_ITEM_UNKNOWN;
            m_pItem  = new SfxVoidItem( m_nSlotID );
        }
    }

    DeleteItemOnIdle( m_pItem );

    try
    {
        m_aCondition.set();
        m_xDispatch->removeStatusListener( Reference< XStatusListener >( static_cast< cppu::OWeakObject* >( this ), UNO_QUERY ),
                                           m_aCommand );
    }
    catch ( Exception& )
    {
    }
}

// Query method
SfxItemState SfxQueryStatus::QueryState( SfxPoolItem*& rpPoolItem )
{
    if ( !m_bQueryInProgress )
    {
        m_pItem  = NULL;
        m_eState = SFX_ITEM_DISABLED;

        if ( m_xDispatch.is() )
        {
            try
            {
                m_aCondition.reset();
                m_bQueryInProgress = sal_True;
                m_xDispatch->addStatusListener( Reference< XStatusListener >( static_cast< cppu::OWeakObject* >( this ), UNO_QUERY ),
                                                m_aCommand );
            }
            catch ( Exception& )
            {
                m_aCondition.set();
            }
        }
        else
            m_aCondition.set();
    }

    m_aCondition.wait();

    m_bQueryInProgress = sal_False;
    rpPoolItem = m_pItem;
    return m_eState;
}
