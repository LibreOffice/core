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

#include <CustomScaleDialog.hxx>
#include <strings.hrc>
#include <sdresid.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>

namespace sd
{
SdCustomScaleDialog::SdCustomScaleDialog(Window* pWindow, sal_uInt16 scaleX, sal_uInt16 scaleY,
                                         bool bIsUniformlyScaled)
    : ModalDialog(pWindow, "CustomScaleDialog", "modules/simpress/ui/customscalemenu.ui")
{
    get(m_CBUniformScale, "UniformScale");
    get(m_pHorizontalScale, "HorizontalScale");
    get(m_pVerticalScale, "VerticalScale");
    get(pOk_btn, "ok_btn");
    get(pCancel_btn, "cancel_btn");

    pOk_btn->SetClickHdl(LINK(this, SdCustomScaleDialog, OkHdl));
    pCancel_btn->SetClickHdl(LINK(this, SdCustomScaleDialog, CancelHdl));
    m_CBUniformScale->SetClickHdl(LINK(this, SdCustomScaleDialog, UniformHdl));

    m_pHorizontalScale->SetValue(scaleX);
    m_pHorizontalScale->SetMin(-1000);
    m_pHorizontalScale->SetMax(1000);

    m_pVerticalScale->SetMin(-1000);
    m_pVerticalScale->SetMax(1000);

    // Check if the previously selected option with to scale in both directions.
    if (bIsUniformlyScaled)
    {
        m_CBUniformScale->Check(true);
        m_pVerticalScale->Disable();
    }
    else
    {
        m_pVerticalScale->SetValue(scaleY);
    }
}

SdCustomScaleDialog::~SdCustomScaleDialog() { disposeOnce(); }

void SdCustomScaleDialog::dispose()
{
    m_pHorizontalScale.clear();
    m_pVerticalScale.clear();
    m_CBUniformScale.clear();
    pOk_btn.clear();
    pCancel_btn.clear();
    ModalDialog::dispose();
}

bool SdCustomScaleDialog::IsScaledUniformly() const { return m_CBUniformScale->IsChecked(); }

sal_uInt16 SdCustomScaleDialog::getHorizontalScale() const
{
    return m_pHorizontalScale->GetValue();
}

sal_uInt16 SdCustomScaleDialog::getVerticalScale() const { return m_pVerticalScale->GetValue(); }

IMPL_LINK_NOARG(SdCustomScaleDialog, OkHdl, Button*, void)
{
    m_CBUniformScale->SaveValue();
    if (m_CBUniformScale->IsChecked())
    {
        m_pVerticalScale->SaveValue();
    }
    else
    {
        m_pVerticalScale->SaveValue();
        m_pVerticalScale->SaveValue();
    }
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SdCustomScaleDialog, CancelHdl, Button*, void)
{
    Close();
    EndDialog(RET_CANCEL);
}

IMPL_LINK_NOARG(SdCustomScaleDialog, UniformHdl, Button*, void)
{
    if (m_CBUniformScale->IsChecked())
    {
        // if uniform scaling is selected, disable one SpinButton,
        // (for now Vertical) only take input from one SpinButton
        m_pVerticalScale->SetEmptyFieldValue();
        m_pVerticalScale->Disable();
    }
    else
        m_pVerticalScale->Enable();
}

} // end of namespace

/* vim: set shiftwidth=4 softtabstop=4 expandtab: */
