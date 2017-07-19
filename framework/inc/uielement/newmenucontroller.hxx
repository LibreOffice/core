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
    class NewMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            NewMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~NewMenuController() override;

            // XServiceInfo
            DECLARE_XSERVICEINFO_NOFACTORY
            /* Helper for registry */
            /// @throws css::uno::Exception
            static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
            static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

            // XInitialization
            virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

            // XMenuListener
            virtual void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) override;
            virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        private:
            virtual void impl_setPopupMenu() override;

            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );
            void retrieveShortcutsFromConfiguration( const css::uno::Reference< css::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                     const css::uno::Sequence< OUString >& rCommands,
                                                     std::vector< vcl::KeyCode >& aMenuShortCuts );
            void setAccelerators( PopupMenu* pPopupMenu );
            void determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const vcl::KeyCode& rKeyCode );
            void setMenuImages( PopupMenu* pPopupMenu, bool bSetImages );

        private:
            // members
            bool                                                      m_bShowImages : 1,
                                                                      m_bNewMenu    : 1,
                                                                      m_bModuleIdentified : 1,
                                                                      m_bAcceleratorCfg : 1;
            OUString                                                  m_aIconTheme;
            OUString                                                  m_aTargetFrame;
            OUString                                                  m_aModuleIdentifier;
            OUString                                                  m_aEmptyDocURL;
            css::uno::Reference< css::uno::XComponentContext >        m_xContext;
            css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xDocAcceleratorManager;
            css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xModuleAcceleratorManager;
            css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xGlobalAcceleratorManager;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_NEWMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
