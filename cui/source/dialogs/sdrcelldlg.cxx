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

#include <sdrcelldlg.hxx>
#include <cuitabarea.hxx>
#include <svx/svdmodel.hxx>
#include <border.hxx>
#include <paragrph.hxx>
#include <svl/intitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svx/flagsdef.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#include <vcl/tabs.hrc>

SvxFormatCellsDialog::SvxFormatCellsDialog(weld::Window* pParent, const SfxItemSet& rAttr, const SdrModel& rModel, bool bStyle)
    : SfxTabDialogController(pParent, u"cui/ui/formatcellsdialog.ui"_ustr, u"FormatCellsDialog"_ustr, &rAttr, bStyle)
    , mrOutAttrs(rAttr)
    , mpColorTab(rModel.GetColorList())
    , mnColorTabState ( ChangeType::NONE )
    , mpGradientList(rModel.GetGradientList())
    , mpHatchingList(rModel.GetHatchList())
    , mpBitmapList(rModel.GetBitmapList())
    , mpPatternList(rModel.GetPatternList())
{
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_CUICELLS_FONT), RID_SVXPAGE_CHAR_NAME,
               RID_M + RID_TAB_FONT_IMG);
    AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_CUICELLS_FONTEFFECTS), RID_SVXPAGE_CHAR_EFFECTS,
               RID_M + RID_TAB_FONTEFFECTS_IMG);
    if (bStyle)
    {
        AddTabPage(u"position"_ustr, TabResId(RID_TAB_CUICELLS_POSITION), RID_SVXPAGE_CHAR_POSITION,
                   RID_M + RID_TAB_POSITION_IMG);
        AddTabPage(u"highlight"_ustr, TabResId(RID_TAB_CUICELLS_HIGHLIGHTING), RID_SVXPAGE_BKG,
                   RID_M + RID_TAB_HIGHLIGHTING_IMG);
        AddTabPage(u"indentspacing"_ustr, TabResId(RID_TAB_CUICELLS_INDENTS),
                   RID_SVXPAGE_STD_PARAGRAPH, RID_M + RID_TAB_INDENTS_IMG);
        if (SvtCJKOptions::IsAsianTypographyEnabled())
            AddTabPage(u"asian"_ustr, TabResId(RID_TAB_CUICELLS_ASIANTYPO), RID_SVXPAGE_PARA_ASIAN,
                       RID_M + RID_TAB_ASIANTYPO_IMG);
        AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_CUICELLS_ALIGNMENT),
                   SvxParaAlignTabPage::Create, SvxParaAlignTabPage::GetSdrRanges,
                   RID_M + RID_TAB_ALIGNMENT_IMG);
    }
    AddTabPage(u"border"_ustr, TabResId(RID_TAB_CUICELLS_BORDER), RID_SVXPAGE_BORDER,
               RID_M + RID_TAB_BORDER_IMG);
    AddTabPage(u"area"_ustr, TabResId(RID_TAB_CUICELLS_BACKGROUND), RID_SVXPAGE_AREA,
               RID_M + RID_TAB_BACKGROUND_IMG);
    if (!bStyle)
        AddTabPage(u"shadow"_ustr, TabResId(RID_TAB_CUICELLS_SHADOW), SvxShadowTabPage::Create,
                   RID_M + RID_TAB_SHADOW_IMG);
}

void SvxFormatCellsDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (rId == "area")
    {
        SvxAreaTabPage& rAreaPage = static_cast<SvxAreaTabPage&>(rPage);
        rAreaPage.SetColorList( mpColorTab );
        rAreaPage.SetGradientList( mpGradientList );
        rAreaPage.SetHatchingList( mpHatchingList );
        rAreaPage.SetBitmapList( mpBitmapList );
        rAreaPage.SetPatternList( mpPatternList );
        rAreaPage.ActivatePage( mrOutAttrs );
    }
    else if (rId == "border")
    {
        SvxBorderTabPage& rBorderPage = static_cast<SvxBorderTabPage&>(rPage);
        rBorderPage.SetTableMode();
    }
    else if (rId == "shadow")
    {
        static_cast<SvxShadowTabPage&>(rPage).SetColorList( mpColorTab );
        static_cast<SvxShadowTabPage&>(rPage).SetColorChgd( &mnColorTabState );
    }
    else if (rId == "alignment")
    {
        static_cast<SvxParaAlignTabPage&>(rPage).EnableSdrVertAlign();
    }
    else if (rId == "highlight")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
    else
        SfxTabDialogController::PageCreated(rId, rPage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
