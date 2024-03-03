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

#include <global.hxx>

class ScInsertCellDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::RadioButton> m_xBtnCellsDown;
    std::unique_ptr<weld::RadioButton> m_xBtnCellsRight;
    std::unique_ptr<weld::RadioButton> m_xBtnInsRow;
    std::unique_ptr<weld::RadioButton> m_xBtnInsCol;
    std::unique_ptr<weld::SpinButton> m_xNumberOfRows;
    std::unique_ptr<weld::SpinButton> m_xNumberOfCols;

    // These are arbitrarily chosen.
    const size_t MAX_INS_ROWS = 4000;
    const size_t MAX_INS_COLS = 4000;

public:
    ScInsertCellDlg(weld::Window* pParent, bool bDisallowCellMove);
    virtual ~ScInsertCellDlg() override;

    InsCellCmd GetInsCellCmd() const;
    size_t GetCount() const;
    DECL_LINK(RadioButtonsHdl, weld::Toggleable&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
