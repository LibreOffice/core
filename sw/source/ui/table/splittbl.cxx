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

#include <wrtsh.hxx>
#include <splittbl.hxx>
#include <tblenum.hxx>

SwSplitTableDlg::SwSplitTableDlg(weld::Window* pParent, SwWrtShell& rSh)
    : GenericDialogController(pParent, "modules/swriter/ui/splittable.ui", "SplitTableDialog")
    , m_xContentCopyRB(m_xBuilder->weld_radio_button("copyheading"))
    , m_xBoxAttrCopyWithParaRB(m_xBuilder->weld_radio_button("customheadingapplystyle"))
    , m_xBoxAttrCopyNoParaRB(m_xBuilder->weld_radio_button("customheading"))
    , m_xBorderCopyRB(m_xBuilder->weld_radio_button("noheading"))
    , rShell(rSh)
    , m_nSplit(SplitTable_HeadlineOption::ContentCopy)
{
}

void SwSplitTableDlg::Apply()
{
    m_nSplit = SplitTable_HeadlineOption::ContentCopy;
    if (m_xBoxAttrCopyWithParaRB->get_active())
        m_nSplit = SplitTable_HeadlineOption::BoxAttrAllCopy;
    else if (m_xBoxAttrCopyNoParaRB->get_active())
        m_nSplit = SplitTable_HeadlineOption::BoxAttrCopy;
    else if (m_xBorderCopyRB->get_active())
        m_nSplit = SplitTable_HeadlineOption::BorderCopy;

    rShell.SplitTable(m_nSplit);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
