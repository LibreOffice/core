/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_

#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <vcl/accel.hxx>
#include <vcl/menu.hxx>
#include <boost/unordered_map.hpp>

namespace framework
{
    struct NewDocument
    {
        ::com::sun::star::util::URL                                                 aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   aArgSeq;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      xDispatch;
    };

    class NewMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            NewMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~NewMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XMenuListener
            virtual void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

            DECL_STATIC_LINK( NewMenuController, ExecuteHdl_Impl, NewDocument* );

        private:
            virtual void impl_setPopupMenu();
            struct AddInfo
            {
                rtl::OUString aTargetFrame;
                rtl::OUString aImageId;
            };

            typedef ::boost::unordered_map< int, AddInfo > AddInfoForId;

            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            void retrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                     const ::com::sun::star::uno::Sequence< rtl::OUString >& rCommands,
                                                     std::vector< KeyCode >& aMenuShortCuts );
            void setAccelerators( PopupMenu* pPopupMenu );
            void determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const KeyCode& rKeyCode );
            void setMenuImages( PopupMenu* pPopupMenu, sal_Bool bSetImages );

        private:
            // members
            sal_Bool            m_bShowImages : 1,
                                m_bNewMenu    : 1,
                                m_bModuleIdentified : 1,
                                m_bAcceleratorCfg : 1;
            AddInfoForId        m_aAddInfoForItem;
            rtl::OUString       m_aTargetFrame;
            rtl::OUString       m_aModuleIdentifier;
            rtl::OUString       m_aEmptyDocURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xDocAcceleratorManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xModuleAcceleratorManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xGlobalAcceleratorManager;
    };
}

#endif // __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
