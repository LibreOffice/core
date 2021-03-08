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
#pragma once

#include <sfx2/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/toolbox.hxx>

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::frame { class XController; }
namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::frame { class XToolbarController; }

class ToolBox;

namespace weld {
    class Builder;
    class Toolbar;
}

namespace sfx2::sidebar {

/** Convenience class for the easy creation of toolbox controllers.
*/
class ControllerFactory
{
public:
    static css::uno::Reference<css::frame::XToolbarController> CreateToolBoxController(
        ToolBox* pToolBox,
        const ToolBoxItemId nItemId,
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const sal_Int32 nItemWidth, bool bSideBar);

    static css::uno::Reference<css::frame::XToolbarController> CreateToolBoxController(
        weld::Toolbar& rToolbar,
        weld::Builder& rBuilder,
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        const css::uno::Reference<css::frame::XController>& rxController,
        bool bSideBar);

private:
    static css::uno::Reference<css::frame::XToolbarController> CreateToolBarController(
        const css::uno::Reference<css::awt::XWindow>& rToolbar,
        const OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        const css::uno::Reference<css::frame::XController>& rxController,
        const sal_Int32 nWidth, bool bSideBar);
};


} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
