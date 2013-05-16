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
#ifndef SFX_SIDEBAR_CONTROLLER_FACTORY_HXX
#define SFX_SIDEBAR_CONTROLLER_FACTORY_HXX

#include "sfx2/dllapi.h"
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

class ToolBox;

namespace sfx2 { namespace sidebar {

/** Convenience class for the easy creation of toolbox controllers.
*/
class SFX2_DLLPUBLIC ControllerFactory
{
public:
    static cssu::Reference<css::frame::XToolbarController> CreateToolBoxController(
        ToolBox* pToolBox,
        const sal_uInt16 nItemId,
        const ::rtl::OUString& rsCommandName,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        const cssu::Reference<css::awt::XWindow>& rxParentWindow,
        const sal_Int32 nItemWidth);

private:
    static cssu::Reference<css::frame::XToolbarController> CreateToolBarController(
        ToolBox* pToolBox,
        const ::rtl::OUString& rsCommandName,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        const sal_Int32 nWidth);
};


} } // end of namespace sfx2::sidebar

#endif
