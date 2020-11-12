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

#include <vcl/weld.hxx>
#include "breakpoint.hxx"

namespace basctl
{
class BreakPointDialog final : public weld::GenericDialogController
{
    BreakPointList& m_rOriginalBreakPointList;
    BreakPointList m_aModifiedBreakPointList;

    std::unique_ptr<weld::EntryTreeView> m_xComboBox;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xNewButton;
    std::unique_ptr<weld::Button> m_xDelButton;
    std::unique_ptr<weld::CheckButton> m_xCheckBox;
    std::unique_ptr<weld::SpinButton> m_xNumericField;

    void CheckButtons();
    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);
    DECL_LINK(EditModifyHdl, weld::ComboBox&, void);
    DECL_LINK(FieldModifyHdl, weld::SpinButton&, void);
    DECL_LINK(ButtonHdl, weld::Button&, void);
    DECL_LINK(TreeModifyHdl, weld::TreeView&, bool);
    void UpdateFields(BreakPoint const& rBrk);
    BreakPoint* GetSelectedBreakPoint();

public:
    BreakPointDialog(weld::Window* pParent, BreakPointList& rBrkList);
    virtual ~BreakPointDialog() override;

    void SetCurrentBreakPoint(BreakPoint const& rBrk);
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
