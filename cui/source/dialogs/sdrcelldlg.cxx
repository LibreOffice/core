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
    AddTabPage(u"name"_ustr, RID_SVXPAGE_CHAR_NAME);
    AddTabPage(u"effects"_ustr, RID_SVXPAGE_CHAR_EFFECTS);
    AddTabPage(u"border"_ustr, RID_SVXPAGE_BORDER );
    AddTabPage(u"area"_ustr, RID_SVXPAGE_AREA);

    if (bStyle)
    {
        AddTabPage(u"position"_ustr, RID_SVXPAGE_CHAR_POSITION);
        AddTabPage(u"highlight"_ustr, RID_SVXPAGE_BKG);
        AddTabPage(u"indentspacing"_ustr, RID_SVXPAGE_STD_PARAGRAPH);
        AddTabPage(u"alignment"_ustr, SvxParaAlignTabPage::Create, SvxParaAlignTabPage::GetSdrRanges);
        RemoveTabPage(u"shadow"_ustr);
    }
    else
    {
        RemoveTabPage(u"position"_ustr);
        RemoveTabPage(u"highlight"_ustr);
        RemoveTabPage(u"indentspacing"_ustr);
        RemoveTabPage(u"alignment"_ustr);
        AddTabPage(u"shadow"_ustr, SvxShadowTabPage::Create, nullptr);
        RemoveStandardButton();
    }

    if (bStyle && SvtCJKOptions::IsAsianTypographyEnabled())
        AddTabPage(u"asian"_ustr, RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage(u"asian"_ustr);
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
