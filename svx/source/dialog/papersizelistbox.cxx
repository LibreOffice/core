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

#include <svx/dialmgr.hxx>
#include <svx/papersizelistbox.hxx>
#include "page.hrc"

SvxPaperSizeListBox::SvxPaperSizeListBox(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
{
    m_xControl->set_size_request(150, -1);
}

void SvxPaperSizeListBox::FillPaperSizeEntries( PaperSizeApp eApp )
{
    const std::pair<TranslateId, int>* pPaperAry = eApp == PaperSizeApp::Std ?
        RID_SVXSTRARY_PAPERSIZE_STD : RID_SVXSTRARY_PAPERSIZE_DRAW;
    sal_uInt32 nCnt = eApp == PaperSizeApp::Std ?
        SAL_N_ELEMENTS(RID_SVXSTRARY_PAPERSIZE_STD) : SAL_N_ELEMENTS(RID_SVXSTRARY_PAPERSIZE_DRAW);

    for ( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        OUString aStr = SvxResId(pPaperAry[i].first);
        Paper eSize = static_cast<Paper>(pPaperAry[i].second);
        m_xControl->append(OUString::number(static_cast<sal_Int32>(eSize)), aStr);
    }
}

void SvxPaperSizeListBox::set_active_id( Paper ePreselectPaper )
{
    int nEntryCount = m_xControl->get_count();
    int nSelPos = -1;
    int nUserPos = -1;

    for (int i = 0; i < nEntryCount; ++i)
    {
        Paper eTmp = static_cast<Paper>(m_xControl->get_id(i).toInt32());
        if (eTmp == ePreselectPaper)
        {
            nSelPos = i;
            break;
        }

        if (eTmp == PAPER_USER)
           nUserPos = i;
    }

    // preselect current paper format - #115915#: ePaper might not be in aPaperSizeBox so use PAPER_USER instead
    m_xControl->set_active((nSelPos != -1) ? nSelPos : nUserPos);
}

Paper SvxPaperSizeListBox::get_active_id() const
{
    return static_cast<Paper>(m_xControl->get_active_id().toInt32());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

