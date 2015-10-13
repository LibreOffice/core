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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX

#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <unotools/intlwrapper.hxx>

#include <vector>

namespace framework
{
    class ToolbarsMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            ToolbarsMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~ToolbarsMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const css::uno::Reference< css::awt::XPopupMenu >& PopupMenu ) throw (css::uno::RuntimeException, std::exception) override;

            // XInitialization
            virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

            // XMenuListener
            virtual void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

            struct ExecuteInfo
            {
                css::uno::Reference< css::frame::XDispatch >     xDispatch;
                css::util::URL                                                aTargetURL;
                css::uno::Sequence< css::beans::PropertyValue >  aArgs;
            };

            DECL_STATIC_LINK_TYPED( ToolbarsMenuController, ExecuteHdl_Impl, void*, void );

        private:
            void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu >& rPopupMenu );
            css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > getLayoutManagerToolbars( const css::uno::Reference< css::frame::XLayoutManager >& rLayoutManager );
            OUString getUINameFromCommand( const OUString& rCommandURL );
            css::uno::Reference< css::frame::XDispatch > getDispatchFromCommandURL( const OUString& rCommandURL );
            void addCommand( css::uno::Reference< css::awt::XPopupMenu >& rPopupMenu, const OUString& rCommandURL, const OUString& aLabel );
            bool isContextSensitiveToolbarNonVisible() { return m_bResetActive;}

            css::uno::Reference< css::uno::XComponentContext >        m_xContext;
            css::uno::Reference< css::container::XNameAccess >        m_xPersistentWindowState;
            css::uno::Reference< css::container::XNameAccess >        m_xUICommandDescription;
            css::uno::Reference< css::ui::XUIConfigurationManager >   m_xModuleCfgMgr;
            css::uno::Reference< css::ui::XUIConfigurationManager >   m_xDocCfgMgr;
            OUString                                                  m_aModuleIdentifier;
            OUString                                                  m_aPropUIName;
            OUString                                                  m_aPropResourceURL;
            bool                                                      m_bModuleIdentified;
            bool                                                      m_bResetActive;
            std::vector< OUString >                                   m_aCommandVector;
            IntlWrapper                                               m_aIntlWrapper;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
