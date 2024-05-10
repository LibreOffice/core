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

#include <svx/dialog/gotodlg.hxx>

using namespace com::sun::star;

namespace svx
{
GotoPageDlg::GotoPageDlg(weld::Window* pParent, const OUString& title, const OUString& label,
                         sal_uInt16 curPagePos, sal_uInt16 pageCnt)
    : GenericDialogController(pParent, u"svx/ui/gotopagedialog.ui"_ustr, u"GotoPageDialog"_ustr)
    , mnMaxPageCnt(1)
    , mxMtrPageCtrl(m_xBuilder->weld_spin_button(u"page"_ustr))
    , mxPageNumberLbl(m_xBuilder->weld_label(u"page_count"_ustr))
    , mxPageLbl(m_xBuilder->weld_label(u"page_label"_ustr))
{
    set_title(title);
    mxPageLbl->set_label(label);

    mxMtrPageCtrl->set_value(curPagePos);

    sal_uInt16 nTotalPage = pageCnt;
    OUString sStr = mxPageNumberLbl->get_label();
    mxPageNumberLbl->set_label(sStr.replaceFirst("$1", OUString::number(nTotalPage)));
    mnMaxPageCnt = nTotalPage;

    mxMtrPageCtrl->connect_changed(LINK(this, GotoPageDlg, PageModifiedHdl));
    mxMtrPageCtrl->set_position(-1);
    mxMtrPageCtrl->select_region(0, -1);
}

IMPL_LINK_NOARG(GotoPageDlg, PageModifiedHdl, weld::Entry&, void)
{
    if (mxMtrPageCtrl->get_text().isEmpty())
        return;

    auto page_value = mxMtrPageCtrl->get_text().toInt32();

    if (page_value <= 0)
        mxMtrPageCtrl->set_value(1);
    else if (page_value > mnMaxPageCnt)
        mxMtrPageCtrl->set_value(mnMaxPageCnt);
    else
        mxMtrPageCtrl->set_value(page_value);

    mxMtrPageCtrl->set_position(-1);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
