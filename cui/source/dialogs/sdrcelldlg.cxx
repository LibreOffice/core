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

#include <svl/cjkoptions.hxx>
#include <svx/flagsdef.hxx>
#include "cuires.hrc"
#include "sdrcelldlg.hxx"
#include "dialmgr.hxx"
#include "cuitabarea.hxx"
#include "svx/svdmodel.hxx"
#include "border.hxx"
#include <svx/dialogs.hrc>

SvxFormatCellsDialog::SvxFormatCellsDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel )
    : SfxTabDialog(pParent, "FormatCellsDialog", "cui/ui/formatcellsdialog.ui", pAttr)
    , mrOutAttrs(*pAttr)
    , mpColorTab(pModel->GetColorList())
    , mpGradientList(pModel->GetGradientList())
    , mpHatchingList(pModel->GetHatchList())
    , mpBitmapList(pModel->GetBitmapList())
    , mpPatternList(pModel->GetPatternList())
    , m_nAreaPageId(0)
{
    AddTabPage("name", RID_SVXPAGE_CHAR_NAME);
    AddTabPage("effects", RID_SVXPAGE_CHAR_EFFECTS);
    m_nBorderPageId = AddTabPage("border", RID_SVXPAGE_BORDER );
    m_nAreaPageId = AddTabPage("area", RID_SVXPAGE_AREA);
}

void SvxFormatCellsDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nAreaPageId)
    {
        SvxAreaTabPage& rAreaPage = static_cast<SvxAreaTabPage&>(rPage);
        rAreaPage.SetColorList( mpColorTab );
        rAreaPage.SetGradientList( mpGradientList );
        rAreaPage.SetHatchingList( mpHatchingList );
        rAreaPage.SetBitmapList( mpBitmapList );
        rAreaPage.SetPatternList( mpPatternList );;
        rAreaPage.ActivatePage( mrOutAttrs );
    }
    else if (nId == m_nBorderPageId)
    {
        SvxBorderTabPage& rBorderPage = static_cast<SvxBorderTabPage&>(rPage);
        rBorderPage.SetTableMode();
    }
    else
        SfxTabDialog::PageCreated( nId, rPage );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
