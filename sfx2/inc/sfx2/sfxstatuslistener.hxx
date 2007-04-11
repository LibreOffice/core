/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxstatuslistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:29:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SFXSTATUSLISTENER_HXX
#define _SFXSTATUSLISTENER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#include <sfx2/sfxuno.hxx>

// Interface for implementations which needs to get notifications about state changes
class SfxStatusListenerInterface
{
    public:
        virtual void StateChanged( USHORT nSlotId, SfxItemState eState, const SfxPoolItem* pState ) = 0;
};

class SFX2_DLLPUBLIC SfxStatusListener :
                          public ::com::sun::star::frame::XStatusListener   ,
                          public ::com::sun::star::lang::XTypeProvider      ,
                          public ::com::sun::star::lang::XComponent         ,
                          public ::cppu::OWeakObject
{
    public:
        SFX_DECL_XINTERFACE_XTYPEPROVIDER

        SfxStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider, USHORT nSlotId, const rtl::OUString& aCommand );
        virtual ~SfxStatusListener();

        // old methods from SfxControllerItem
        USHORT  GetId() const { return m_nSlotID; }
        void    Bind();
        void    Bind( USHORT nSlotID, const rtl::OUString& rNewCommand );
        void    UnBind();
        void    ReBind();

        virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

        // XComponent
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

    private:
        SfxStatusListener( const SfxStatusListener& );
        SfxStatusListener();
        SfxStatusListener& operator=( const SfxStatusListener& );

        USHORT                                                                          m_nSlotID;
        ::com::sun::star::util::URL                                                     m_aCommand;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  m_xDispatchProvider;
        ::com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >            m_xDispatch;
};

#endif // _SFXSTATUSLISTENER_HXX
