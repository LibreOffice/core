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

#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>

namespace framework
{

class ComboBoxControl;

class ComboboxToolbarController final : public ComplexToolbarController

{
    public:
        ComboboxToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                   const css::uno::Reference< css::frame::XFrame >& rFrame,
                                   ToolBox*     pToolBar,
                                   ToolBoxItemId nID,
                                   sal_Int32    nWidth,
                                   const OUString& aCommand );
        virtual ~ComboboxToolbarController() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // called from ComboBoxControl
        void Select();
        void Modify();
        void GetFocus();
        void LoseFocus();
        void Activate();

    private:
        virtual void executeControlCommand( const css::frame::ControlCommand& rControlCommand ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const override;

        VclPtr<ComboBoxControl>    m_pComboBox;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
