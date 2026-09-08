/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <framework/fwkdllapi.h>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <vcl/toolboxid.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace cpo::uno { template <typename > class Reference; }
namespace svt { class StatusbarController; }
namespace svt { class ToolboxController; }

class StatusBar;
class ToolBox;

typedef rtl::Reference<svt::ToolboxController> ( *pfunc_setToolBoxControllerCreator)(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    ToolBox* pToolbox,
    ToolBoxItemId nID,
    const OUString& aCommandURL );

typedef rtl::Reference<svt::StatusbarController> ( *pfunc_setStatusBarControllerCreator)(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    StatusBar* pStatusBar,
    unsigned short nID,
    const OUString& aCommandURL );

typedef void ( *pfunc_getRefreshToolbars)(
    cpo::uno::Reference< css::frame::XFrame > const & rFrame );

typedef void ( *pfunc_createDockingWindow)(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    std::u16string_view rResourceURL );

typedef bool ( *pfunc_isDockingWindowVisible)(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    std::u16string_view rResourceURL );


namespace framework
{
FWK_DLLPUBLIC pfunc_setToolBoxControllerCreator SetToolBoxControllerCreator( pfunc_setToolBoxControllerCreator pSetToolBoxControllerCreator );
FWK_DLLPUBLIC rtl::Reference<svt::ToolboxController> CreateToolBoxController(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    ToolBox* pToolbox,
    ToolBoxItemId nID,
    const OUString& aCommandURL );

FWK_DLLPUBLIC pfunc_setStatusBarControllerCreator SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator );
FWK_DLLPUBLIC rtl::Reference<svt::StatusbarController> CreateStatusBarController(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    StatusBar* pStatusBar,
    unsigned short nID,
     const OUString& aCommandURL );

FWK_DLLPUBLIC pfunc_getRefreshToolbars SetRefreshToolbars( pfunc_getRefreshToolbars pRefreshToolbarsFunc );
FWK_DLLPUBLIC void RefreshToolbars(
     cpo::uno::Reference< css::frame::XFrame > const & rFrame );

FWK_DLLPUBLIC pfunc_createDockingWindow SetDockingWindowCreator( pfunc_createDockingWindow pCreateDockingWindow );
FWK_DLLPUBLIC void CreateDockingWindow(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    std::u16string_view rResourceURL );

FWK_DLLPUBLIC pfunc_isDockingWindowVisible SetIsDockingWindowVisible( pfunc_isDockingWindowVisible pIsDockingWindowVisible );
FWK_DLLPUBLIC bool IsDockingWindowVisible(
    const cpo::uno::Reference< css::frame::XFrame >& rFrame,
    std::u16string_view rResourceURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
