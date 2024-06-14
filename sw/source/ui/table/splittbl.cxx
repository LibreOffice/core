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

SplitTable_HeadlineOption SwSplitTableDlg::m_eRememberedSplitOption
    = SplitTable_HeadlineOption::ContentCopy;

SwSplitTableDlg::SwSplitTableDlg(weld::Window* pParent, SwWrtShell& rSh)
    : GenericDialogController(pParent, u"modules/swriter/ui/splittable.ui"_ustr,
                              u"SplitTableDialog"_ustr)
    , m_xBoxAttrCopyWithParaRB(m_xBuilder->weld_radio_button(u"customheadingapplystyle"_ustr))
    , m_xBoxAttrCopyNoParaRB(m_xBuilder->weld_radio_button(u"customheading"_ustr))
    , m_xBorderCopyRB(m_xBuilder->weld_radio_button(u"noheading"_ustr))
    , m_rShell(rSh)
    , m_nSplit(SplitTable_HeadlineOption::ContentCopy)
{
    // tdf#131759 - remember last used option in split table dialog
    m_nSplit = m_eRememberedSplitOption;
    switch (m_nSplit)
    {
        case SplitTable_HeadlineOption::BoxAttrAllCopy:
            m_xBoxAttrCopyWithParaRB->set_active(true);
            break;
        case SplitTable_HeadlineOption::BoxAttrCopy:
            m_xBoxAttrCopyNoParaRB->set_active(true);
            break;
        case SplitTable_HeadlineOption::BorderCopy:
            m_xBorderCopyRB->set_active(true);
            break;
        case SplitTable_HeadlineOption::NONE:
        case SplitTable_HeadlineOption::ContentCopy:
        default:
            // Use the default value in case of an invalid option
            m_nSplit = SplitTable_HeadlineOption::ContentCopy;
    }
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

    // tdf#131759 - remember last used option in split table dialog
    m_eRememberedSplitOption = m_nSplit;

    m_rShell.SplitTable(m_nSplit);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
