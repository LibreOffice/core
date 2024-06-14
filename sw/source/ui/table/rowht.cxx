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

#include <svtools/unitconv.hxx>

#include <fmtfsize.hxx>
#include <swtypes.hxx>
#include <rowht.hxx>
#include <wrtsh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>

void SwTableHeightDlg::Apply()
{
    SwTwips nHeight = static_cast< SwTwips >(m_xHeightEdit->denormalize(m_xHeightEdit->get_value(FieldUnit::TWIP)));
    SwFormatFrameSize aSz(SwFrameSize::Fixed, 0, nHeight);

    SwFrameSize eFrameSize = m_xAutoHeightCB->get_active() ?  SwFrameSize::Minimum : SwFrameSize::Fixed;
    if(eFrameSize != aSz.GetHeightSizeType())
    {
        aSz.SetHeightSizeType(eFrameSize);
    }
    m_rSh.SetRowHeight(aSz);
}

SwTableHeightDlg::SwTableHeightDlg(weld::Window *pParent, SwWrtShell &rS)
    : GenericDialogController(pParent, u"modules/swriter/ui/rowheight.ui"_ustr, u"RowHeightDialog"_ustr)
    , m_rSh(rS)
    , m_xHeightEdit(m_xBuilder->weld_metric_spin_button(u"heightmf"_ustr, FieldUnit::CM))
    , m_xAutoHeightCB(m_xBuilder->weld_check_button(u"fit"_ustr))
{
    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( dynamic_cast< const SwWebDocShell*>(
                                m_rSh.GetView().GetDocShell() ) != nullptr  )->GetMetric();
    ::SetFieldUnit(*m_xHeightEdit, eFieldUnit);

    m_xHeightEdit->set_min(MINLAY, FieldUnit::TWIP);
    std::unique_ptr<SwFormatFrameSize> pSz = m_rSh.GetRowHeight();
    if (pSz)
    {
        auto nHeight = pSz->GetHeight();
        m_xAutoHeightCB->set_active(pSz->GetHeightSizeType() != SwFrameSize::Fixed);
        m_xHeightEdit->set_value(m_xHeightEdit->normalize(nHeight), FieldUnit::TWIP);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
