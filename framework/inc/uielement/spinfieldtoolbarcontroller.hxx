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
#include <vcl/vclptr.hxx>

class ToolBox;

namespace framework
{

class SpinfieldControl;

class SpinfieldToolbarController final : public ComplexToolbarController

{
    public:
        SpinfieldToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                    const css::uno::Reference< css::frame::XFrame >& rFrame,
                                    ToolBox* pToolBar,
                                    ToolBoxItemId nID,
                                    sal_Int32 nWidth,
                                    const OUString& aCommand );
        virtual ~SpinfieldToolbarController() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // called from SpinfieldControl
        void Modify();
        void GetFocus();
        void LoseFocus();
        void Activate();

        OUString FormatOutputString(double fValue);
    private:
        virtual void executeControlCommand( const css::frame::ControlCommand& rControlCommand ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const override;

        static bool impl_getValue( const css::uno::Any& rAny, sal_Int32& nValue, double& fValue, bool& bFloat );

        bool              m_bFloat;
        double            m_nMax;
        double            m_nMin;
        double            m_nValue;
        double            m_nStep;
        VclPtr<SpinfieldControl> m_pSpinfieldControl;
        OUString     m_aOutFormat;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
