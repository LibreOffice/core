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

#ifndef _SFXSTATUSLISTENER_HXX
#define _SFXSTATUSLISTENER_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <svl/poolitem.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/conditn.hxx>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <sfx2/sfxuno.hxx>

// Interface for implementations which needs to get notifications about state changes
class SfxStatusListenerInterface
{
    public:
        virtual void StateChanged( sal_uInt16 nSlotId, SfxItemState eState, const SfxPoolItem* pState ) = 0;
};

class SFX2_DLLPUBLIC SfxStatusListener :
                          public ::com::sun::star::frame::XStatusListener   ,
                          public ::com::sun::star::lang::XTypeProvider      ,
                          public ::com::sun::star::lang::XComponent         ,
                          public ::cppu::OWeakObject
{
    public:
        SFX_DECL_XINTERFACE_XTYPEPROVIDER

        SfxStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const rtl::OUString& aCommand );
        virtual ~SfxStatusListener();

        // old methods from SfxControllerItem
        sal_uInt16  GetId() const { return m_nSlotID; }
        void    Bind();
        void    Bind( sal_uInt16 nSlotID, const rtl::OUString& rNewCommand );
        void    UnBind();
        void    ReBind();

        virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

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

        sal_uInt16                                                                          m_nSlotID;
        ::com::sun::star::util::URL                                                     m_aCommand;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  m_xDispatchProvider;
        ::com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >            m_xDispatch;
};

#endif // _SFXSTATUSLISTENER_HXX
