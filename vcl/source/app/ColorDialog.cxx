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

#include <vcl/ColorDialog.hxx>
#include <vcl/weld.hxx>

ColorDialog::ColorDialog(weld::Window* pParent, vcl::ColorPickerMode eMode)
{
    VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
    assert(pFact);
    m_pDialog = pFact->CreateColorPickerDialog(pParent, COL_BLACK, static_cast<sal_Int16>(eMode));
    assert(m_pDialog);
}

ColorDialog::~ColorDialog() {}

void ColorDialog::SetColor(const Color& rColor) { m_pDialog->SetColor(rColor); }

Color ColorDialog::GetColor() const { return m_pDialog->GetColor(); }

short ColorDialog::Execute() { return m_pDialog->Execute(); }

void ColorDialog::ExecuteAsync(const std::function<void(sal_Int32)>& func)
{
    m_aResultFunc = func;
    m_pDialog->StartExecuteAsync(m_aResultFunc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
