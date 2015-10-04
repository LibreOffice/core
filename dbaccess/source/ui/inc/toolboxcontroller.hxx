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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include "apitools.hxx"
#include "moduledbu.hxx"

#include <map>
#include <memory>

class PopupMenu;
namespace dbaui
{
    typedef ::cppu::ImplHelper <   css::lang::XServiceInfo> TToolboxController_BASE;

    class OToolboxController : public ::svt::ToolboxController
                              ,public TToolboxController_BASE
    {
        typedef std::map<OUString, sal_Bool> TCommandState;
        OModuleClient   m_aModuleClient;
        TCommandState   m_aStates;
        sal_uInt16      m_nToolBoxId;

        ::std::unique_ptr<PopupMenu> getMenu();
    public:
        OToolboxController(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;
        // XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
        // XToolbarController
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
    };
} // dbaui
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
