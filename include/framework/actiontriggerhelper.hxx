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

#include <framework/fwkdllapi.h>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::awt { class XPopupMenu; }
namespace com::sun::star::container { class XIndexContainer; }

namespace framework
{
    class FWK_DLLPUBLIC ActionTriggerHelper
    {
        public:
            // Fills the submitted menu with the structure contained in the second
            // parameter rActionTriggerContainer
            // @param pNewMenu = must be a valid and empty menu
            // @param rActionTriggerContainer = must be an instance of service "com.sun.star.ui.ActionTriggerContainer"
            static void
                CreateMenuFromActionTriggerContainer(
                    const css::uno::Reference<css::awt::XPopupMenu>& rNewMenu,
                    const css::uno::Reference<css::container::XIndexContainer>& rActionTriggerContainer);

            // Creates a "css::ui::ActionTriggerContainer" with the structure of the menu
            // provided as a parameter. The implementation class stores the menu pointer
            // to optimize the time of creation of a menu from an actiontrigger structure.
            // IMPORTANT: The caller must ensure that the menu pointer is valid through the
            //            life time of the XIndexContainer object!!!
            // @param rMenu = Must be a valid menu. Please be aware that this implementation is based on
            //                the above mentioned restriction!!!

            static css::uno::Reference< css::container::XIndexContainer > CreateActionTriggerContainerFromMenu(
                const css::uno::Reference<css::awt::XPopupMenu>& rMenu, const OUString* pMenuIdentifier );

            // Fills the submitted rActionTriggerContainer with the structure of the menu
            // provided as the second parameter
            // @param rActionTriggerContainer = must be an instance of service "com.sun.star.ui.ActionTriggerContainer"
            // @param rMenu = must be a valid menu
            static void
                FillActionTriggerContainerFromMenu(
                    css::uno::Reference< css::container::XIndexContainer > const & rActionTriggerContainer,
                    const css::uno::Reference<css::awt::XPopupMenu>& rMenu);

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
