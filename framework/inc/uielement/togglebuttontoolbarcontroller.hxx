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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <tools/link.hxx>

class Menu;
class ToolBox;

namespace framework
{

class ToggleButtonToolbarController : public ComplexToolbarController

{
    public:
        enum Style
        {
            STYLE_DROPDOWNBUTTON,
            STYLE_TOGGLE_DROPDOWNBUTTON
        };

        ToggleButtonToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                       const css::uno::Reference< css::frame::XFrame >& rFrame,
                                       ToolBox* pToolBar,
                                       sal_uInt16 nID,
                                       Style eStyle,
                                       const OUString& aCommand );
        virtual ~ToggleButtonToolbarController() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XToolbarController
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;

    protected:
        virtual void executeControlCommand( const css::frame::ControlCommand& rControlCommand ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const override;

    private:
        DECL_LINK( MenuSelectHdl, Menu *, bool);

        Style                   m_eStyle;
        OUString                m_aCurrentSelection;
        std::vector< OUString > m_aDropdownMenuList;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
