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

#include <mergetbl.hxx>

SwMergeTableDlg::SwMergeTableDlg(weld::Window* pParent, bool& rWithPrev)
    : GenericDialogController(pParent, u"modules/swriter/ui/mergetabledialog.ui"_ustr,
                              u"MergeTableDialog"_ustr)
    , m_rMergePrev(rWithPrev)
    , m_xMergePrevRB(m_xBuilder->weld_radio_button(u"prev"_ustr))
{
    m_xMergePrevRB->set_active(true);
}

void SwMergeTableDlg::Apply() { m_rMergePrev = m_xMergePrevRB->get_active(); }

short SwMergeTableDlg::run()
{
    int nRet = GenericDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
