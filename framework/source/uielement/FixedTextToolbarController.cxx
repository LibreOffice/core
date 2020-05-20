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

#include <uielement/FixedTextToolbarController.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace framework
{
FixedTextToolbarController::FixedTextToolbarController(
    const Reference<XComponentContext>& rxContext, const Reference<XFrame>& rFrame,
    ToolBox* pToolbar, sal_uInt16 nID, const OUString& aCommand)
    : ComplexToolbarController(rxContext, rFrame, pToolbar, nID, aCommand)
{
    m_pFixedTextControl = VclPtr<FixedText>::Create(m_xToolbar, WB_NOMULTILINE | WB_VCENTER
                                                                    | WB_LEFT | WB_NOPOINTERFOCUS);
    m_xToolbar->SetItemWindow(m_nID, m_pFixedTextControl);
    m_xToolbar->SetItemBits(m_nID, ToolBoxItemBits::AUTOSIZE | m_xToolbar->GetItemBits(m_nID));
}

void SAL_CALL FixedTextToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    m_xToolbar->SetItemWindow(m_nID, nullptr);
    m_pFixedTextControl.disposeAndClear();
    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> FixedTextToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs(2);
    const OUString aSelectedText = m_pFixedTextControl->GetText();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void FixedTextToolbarController::executeControlCommand(
    const css::frame::ControlCommand& rControlCommand)
{
    SolarMutexGuard aSolarMutexGuard;

    if (rControlCommand.Command != "SetText")
        return;

    for (const NamedValue& rArg : rControlCommand.Arguments)
    {
        if (rArg.Name == "Text")
        {
            OUString aText;
            rArg.Value >>= aText;
            m_pFixedTextControl->SetText(aText);
            m_pFixedTextControl->SetSizePixel(m_pFixedTextControl->GetOptimalSize());

            // send notification
            notifyTextChanged(aText);
            break;
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
