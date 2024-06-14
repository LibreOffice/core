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

#include <svx/svxids.hrc>
#include <svx/numinf.hxx>
#include <tblnumfm.hxx>

#include <svx/dialogs.hrc>
#include <svl/itemset.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/tabdlg.hxx>

SwNumFormatDlg::SwNumFormatDlg(weld::Widget* pParent, const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, &rSet, u"cui/ui/formatnumberdialog.ui"_ustr, u"FormatNumberDialog"_ustr)
    , m_xContent( m_xBuilder->weld_container(u"content"_ustr) )
{
    // Create TabPage
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc(RID_SVXPAGE_NUMBERFORMAT);
    if ( fnCreatePage )
    {
        std::unique_ptr<SfxTabPage> xNewPage = (*fnCreatePage)(m_xContent.get(), this, &rSet);
        SfxAllItemSet aSet(*(rSet.GetPool()));
        aSet.Put(xNewPage->GetItemSet().Get( SID_ATTR_NUMBERFORMAT_INFO));
        xNewPage->PageCreated(aSet);
        SetTabPage(std::move(xNewPage));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
