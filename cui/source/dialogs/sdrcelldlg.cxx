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
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel), RID_SVXPAGE_CHAR_NAME,
               RID_M + RID_TAB_FONT.sIconName);
    AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel), RID_SVXPAGE_CHAR_EFFECTS,
               RID_M + RID_TAB_FONTEFFECTS.sIconName);
    if (bStyle)
    {
        AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel), RID_SVXPAGE_CHAR_POSITION,
                   RID_M + RID_TAB_POSITION.sIconName);
        AddTabPage(u"highlight"_ustr, TabResId(RID_TAB_HIGHLIGHTING.aLabel), RID_SVXPAGE_BKG,
                   RID_M + RID_TAB_HIGHLIGHTING.sIconName);
        AddTabPage(u"indentspacing"_ustr, TabResId(RID_TAB_INDENTS.aLabel),
                   RID_SVXPAGE_STD_PARAGRAPH, RID_M + RID_TAB_INDENTS.sIconName);
        if (SvtCJKOptions::IsAsianTypographyEnabled())
            AddTabPage(u"asian"_ustr, TabResId(RID_TAB_ASIANTYPO.aLabel), RID_SVXPAGE_PARA_ASIAN,
                       RID_M + RID_TAB_ASIANTYPO.sIconName);
        AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_ALIGNMENT.aLabel),
                   SvxParaAlignTabPage::Create, SvxParaAlignTabPage::GetSdrRanges,
                   RID_M + RID_TAB_ALIGNMENT.sIconName);
    }
    AddTabPage(u"border"_ustr, TabResId(RID_TAB_BORDER.aLabel), RID_SVXPAGE_BORDER,
               RID_M + RID_TAB_BORDER.sIconName);
    AddTabPage(u"area"_ustr, TabResId(RID_TAB_BACKGROUND.aLabel), RID_SVXPAGE_AREA,
               RID_M + RID_TAB_BACKGROUND.sIconName);
    if (!bStyle)
        AddTabPage(u"shadow"_ustr, TabResId(RID_TAB_SHADOW.aLabel), SvxShadowTabPage::Create,
                   RID_M + RID_TAB_SHADOW.sIconName);
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
