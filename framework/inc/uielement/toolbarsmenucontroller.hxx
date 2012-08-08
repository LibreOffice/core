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

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX_

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
            ToolbarsMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~ToolbarsMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& PopupMenu ) throw (::com::sun::star::uno::RuntimeException);

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XMenuListener
            virtual void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

            struct ExecuteInfo
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
                ::com::sun::star::util::URL                                                aTargetURL;
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
            };

            DECL_STATIC_LINK( ToolbarsMenuController, ExecuteHdl_Impl, ExecuteInfo* );

        private:
            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > > getLayoutManagerToolbars( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& rLayoutManager );
            rtl::OUString getUINameFromCommand( const rtl::OUString& rCommandURL );
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > getDispatchFromCommandURL( const rtl::OUString& rCommandURL );
            void addCommand( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu, const rtl::OUString& rCommandURL, const rtl::OUString& aLabel );
            sal_Bool isContextSensitiveToolbarNonVisible();

            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >                m_xPersistentWindowState;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >                m_xUICommandDescription;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >   m_xModuleCfgMgr;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >   m_xDocCfgMgr;
            rtl::OUString                                                                               m_aModuleIdentifier;
            rtl::OUString                                                                               m_aPropUIName;
            rtl::OUString                                                                               m_aPropResourceURL;
            sal_Bool                                                                                    m_bModuleIdentified;
            sal_Bool                                                                                    m_bResetActive;
            std::vector< rtl::OUString >                                                                m_aCommandVector;
            IntlWrapper                                                                                 m_aIntlWrapper;
    };
}

#endif // __FRAMEWORK_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
