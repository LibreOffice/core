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
#include <svx/dialogs.hrc>

SvxFormatCellsDialog::SvxFormatCellsDialog(weld::Window* pParent, const SfxItemSet* pAttr, const SdrModel& rModel)
    : SfxTabDialogController(pParent, "cui/ui/formatcellsdialog.ui", "FormatCellsDialog", pAttr)
    , mrOutAttrs(*pAttr)
    , mpColorTab(rModel.GetColorList())
    , mpGradientList(rModel.GetGradientList())
    , mpHatchingList(rModel.GetHatchList())
    , mpBitmapList(rModel.GetBitmapList())
    , mpPatternList(rModel.GetPatternList())
{
    AddTabPage("name", RID_SVXPAGE_CHAR_NAME);
    AddTabPage("effects", RID_SVXPAGE_CHAR_EFFECTS);
    AddTabPage("border", RID_SVXPAGE_BORDER );
    AddTabPage("area", RID_SVXPAGE_AREA);
}

void SvxFormatCellsDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
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
    else
        SfxTabDialogController::PageCreated(rId, rPage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
