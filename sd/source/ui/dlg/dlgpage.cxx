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

#include <svl/intitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <i18nutil/paper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdlg.hxx>

#include <dlgpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>

#include <svl/eitem.hxx>
#include <svx/flagsdef.hxx>

/**
 * Constructor of tab dialog: appends pages to the dialog
 */
SdPageDlg::SdPageDlg(SfxObjectShell const* pDocSh, weld::Window* pParent, const SfxItemSet* pAttr,
                     bool bAreaPage, bool bIsImpressDoc)
    : SfxTabDialogController(pParent, u"modules/sdraw/ui/drawpagedialog.ui"_ustr,
                             u"DrawPageDialog"_ustr, pAttr)
    , mbIsImpressDoc(bIsImpressDoc)
{
    SvxColorListItem const* pColorListItem = pDocSh->GetItem(SID_COLOR_TABLE);
    SvxGradientListItem const* pGradientListItem = pDocSh->GetItem(SID_GRADIENT_LIST);
    SvxBitmapListItem const* pBitmapListItem = pDocSh->GetItem(SID_BITMAP_LIST);
    SvxPatternListItem const* pPatternListItem = pDocSh->GetItem(SID_PATTERN_LIST);
    SvxHatchListItem const* pHatchListItem = pDocSh->GetItem(SID_HATCH_LIST);

    mpColorList = pColorListItem->GetColorList();
    mpGradientList = pGradientListItem->GetGradientList();
    mpHatchingList = pHatchListItem->GetHatchList();
    mpBitmapList = pBitmapListItem->GetBitmapList();
    mpPatternList = pPatternListItem->GetPatternList();

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    AddTabPage(u"RID_SVXPAGE_PAGE"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PAGE), nullptr);
    AddTabPage(u"RID_SVXPAGE_AREA"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_AREA), nullptr);
    AddTabPage(u"RID_SVXPAGE_TRANSPARENCE"_ustr,
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TRANSPARENCE), nullptr);

    if (!bAreaPage) // I have to add the page before I remove it !
    {
        RemoveTabPage(u"RID_SVXPAGE_AREA"_ustr);
        RemoveTabPage(u"RID_SVXPAGE_TRANSPARENCE"_ustr);
    }

    if (mbIsImpressDoc)
    {
        set_title(SdResId(STR_SLIDE_SETUP_TITLE));
        m_xTabCtrl->set_tab_label_text(u"RID_SVXPAGE_PAGE"_ustr, SdResId(STR_SLIDE_NAME));
    }
}

void SdPageDlg::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "RID_SVXPAGE_PAGE")
    {
        aSet.Put(SfxUInt16Item(SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_PRESENTATION));
        aSet.Put(SfxUInt16Item(SID_PAPER_START, PAPER_A0));
        aSet.Put(SfxUInt16Item(SID_PAPER_END, PAPER_E));

        if (mbIsImpressDoc)
            aSet.Put(SfxBoolItem(SID_IMPRESS_DOC, true));

        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_AREA")
    {
        aSet.Put(SvxColorListItem(mpColorList, SID_COLOR_TABLE));
        aSet.Put(SvxGradientListItem(mpGradientList, SID_GRADIENT_LIST));
        aSet.Put(SvxHatchListItem(mpHatchingList, SID_HATCH_LIST));
        aSet.Put(SvxBitmapListItem(mpBitmapList, SID_BITMAP_LIST));
        aSet.Put(SvxPatternListItem(mpPatternList, SID_PATTERN_LIST));
        aSet.Put(SfxUInt16Item(SID_PAGE_TYPE, 0));
        aSet.Put(SfxUInt16Item(SID_DLG_TYPE, 1));
        aSet.Put(SfxUInt16Item(SID_TABPAGE_POS, 0));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_TRANSPARENCE")
    {
        aSet.Put(SfxUInt16Item(SID_PAGE_TYPE, 0));
        aSet.Put(SfxUInt16Item(SID_DLG_TYPE, 1));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
