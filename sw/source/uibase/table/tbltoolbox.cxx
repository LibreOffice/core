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

#include <tbltoolbox.hxx>

#include <sfx2/tplpitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <vcl/combobox.hxx>
#include <vcl/toolbox.hxx>

SFX_IMPL_TOOLBOX_CONTROL( SwTableStyleToolBoxControl, SfxTemplateItem );

SwTableStyleToolBoxControl::SwTableStyleToolBoxControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :   SvxStyleToolBoxControl (nSlotId, nId, rTbx)
{
    nActFamily = 6; // table style
}

SwTableStyleToolBoxControl::~SwTableStyleToolBoxControl()
{ }

void SwTableStyleToolBoxControl::Update()
{
    SfxStyleSheetBasePool*  pPool     = nullptr;
    SfxObjectShell*         pDocShell = SfxObjectShell::Current();

    if (pDocShell)
        pPool = pDocShell->GetStyleSheetPool();

    const SfxTemplateItem* pItem = nullptr;

    if (nullptr == (pItem = pFamilyState[nActFamily-1])) // receive updates only for table styles
        return;

    if (pStyleSheetPool != pPool)
        pStyleSheetPool = pPool;

    FillStyleBox();

    if (pItem)
        SelectStyle(pItem->GetStyleName());
}

void SwTableStyleToolBoxControl::FillStyleBox()
{
    ComboBox* pBox = static_cast<ComboBox*>(GetToolBox().GetItemWindow( GetId() ));

    DBG_ASSERT( pStyleSheetPool, "StyleSheetPool not found!" ); //TODO convert to SAL_WARN_IF
    DBG_ASSERT( pBox,            "Control not found!" );

    if (pStyleSheetPool && pBox)
    {
        pStyleSheetPool->SetSearchMask(SfxStyleFamily::Table, SFXSTYLEBIT_ALL);

        pBox->SetUpdateMode( false );
        pBox->Clear();

        SfxStyleSheetBase* pStyle = pStyleSheetPool->First();
        while (pStyle)
        {
            pBox->InsertEntry( pStyle->GetName() );
            pStyle = pStyleSheetPool->Next();
        }

        pBox->SetUpdateMode( true );

        sal_uInt16 nLines = static_cast<sal_uInt16>(std::min( pBox->GetEntryCount(), static_cast<sal_Int32>(15)));
        pBox->SetDropDownLineCount( nLines );

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
