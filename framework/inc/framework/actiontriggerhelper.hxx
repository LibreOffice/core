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

#ifndef __FRAMEWORK_HELPER_ACTIONTRIGGERHELPER_HXX_
#define __FRAMEWORK_HELPER_ACTIONTRIGGERHELPER_HXX_

#include <com/sun/star/container/XIndexContainer.hpp>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/menu.hxx>
#include <framework/fwedllapi.h>


namespace framework
{
    class FWE_DLLPUBLIC ActionTriggerHelper
    {
        public:
            // Fills the submitted menu with the structure contained in the second
            // parameter rActionTriggerContainer
            // @param pNewMenu = must be a valid and empty menu
            // @param rActionTriggerContainer = must be an instance of service "com.sun.star.ui.ActionTriggerContaienr"
            static void
                CreateMenuFromActionTriggerContainer(
                    Menu* pNewMenu,
                    const com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rActionTriggerContainer );

            // Creates a "com::sun::star::ui::ActionTriggerContainer" with the structure of the menu
            // provided as a parameter. The implementation class stores the menu pointer
            // to optimize the time of creation of a menu from a actiontrigger structure.
            // IMPORTANT: The caller must ensure that the menu pointer is valid through the
            //            life time of the XIndexContainer object!!!
            //
            // @param pNewMenu = Must be a valid menu. Please be aware that this implementation is based on
            //                   the above mentioned restriction!!!

            // #110897#
            static com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer > CreateActionTriggerContainerFromMenu(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
                const Menu* pMenu, const ::rtl::OUString* pMenuIdentifier );

            // Fills the submitted rActionTriggerContainer with the structure of the menu
            // provided as the second parameter
            // @param rActionTriggerContainer = must be an instance of service "com.sun.star.ui.ActionTriggerContainer"
            // @param pNewMenu = must be a valid menu
            static void
                FillActionTriggerContainerFromMenu(
                    com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rActionTriggerContainer,
                    const Menu* pMenu );

    };
}

#endif // __FRAMEWORK_HELPER_ACTIONTRIGGERHELPER_HXX_
