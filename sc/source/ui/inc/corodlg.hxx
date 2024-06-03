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

class ScColRowLabelDlg : public weld::GenericDialogController
{
public:
    ScColRowLabelDlg(weld::Window* pParent, bool bCol, bool bRow)
        : GenericDialogController(pParent, u"modules/scalc/ui/changesourcedialog.ui"_ustr,
                                  u"ChangeSourceDialog"_ustr)
        , m_xBtnRow(m_xBuilder->weld_check_button(u"row"_ustr))
        , m_xBtnCol(m_xBuilder->weld_check_button(u"col"_ustr))
    {
        m_xBtnCol->set_active(bCol);
        m_xBtnRow->set_active(bRow);
    }

    bool IsCol() const { return m_xBtnCol->get_active(); }
    bool IsRow() const { return m_xBtnRow->get_active(); }

private:
    std::unique_ptr<weld::CheckButton> m_xBtnRow;
    std::unique_ptr<weld::CheckButton> m_xBtnCol;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
