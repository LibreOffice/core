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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_NEWMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_NEWMENUCONTROLLER_HXX

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
#include <framework/menuconfiguration.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <vcl/accel.hxx>
#include <vcl/menu.hxx>
#include <unordered_map>

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
            NewMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
            virtual ~NewMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

            // XMenuListener
            virtual void SAL_CALL itemSelected( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL itemActivated( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

            DECL_STATIC_LINK_TYPED( NewMenuController, ExecuteHdl_Impl, void*, void );

        private:
            virtual void impl_setPopupMenu() override;

            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            void retrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                     const ::com::sun::star::uno::Sequence< OUString >& rCommands,
                                                     std::vector< vcl::KeyCode >& aMenuShortCuts );
            void setAccelerators( PopupMenu* pPopupMenu );
            void determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const vcl::KeyCode& rKeyCode );
            void setMenuImages( PopupMenu* pPopupMenu, bool bSetImages );

        private:
            // members
            bool            m_bShowImages : 1,
                                m_bNewMenu    : 1,
                                m_bModuleIdentified : 1,
                                m_bAcceleratorCfg : 1;
            OUString       m_aTargetFrame;
            OUString       m_aModuleIdentifier;
            OUString       m_aEmptyDocURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xDocAcceleratorManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xModuleAcceleratorManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xGlobalAcceleratorManager;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_NEWMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
