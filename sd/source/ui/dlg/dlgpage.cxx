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
#include <svx/drawitem.hxx>

#include "sdresid.hxx"
#include "dlgpage.hxx"

#include "DrawDocShell.hxx"
#include <svl/aeitem.hxx>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/svxenum.hxx>

/**
 * Constructor of tab dialog: appends pages to the dialog
 */
SdPageDlg::SdPageDlg( SfxObjectShell* pDocSh, Window* pParent, const SfxItemSet* pAttr, sal_Bool bAreaPage ) :
        SfxTabDialog ( pParent, SdResId( TAB_PAGE ), pAttr ),
        mpDocShell          ( pDocSh )
{
    SvxColorListItem aColorListItem(*( (const SvxColorListItem*)
        ( mpDocShell->GetItem( SID_COLOR_TABLE ) ) ) );
    SvxGradientListItem aGradientListItem(*( (const SvxGradientListItem*)
        ( mpDocShell->GetItem( SID_GRADIENT_LIST ) ) ) );
    SvxBitmapListItem aBitmapListItem(*( (const SvxBitmapListItem*)
        ( mpDocShell->GetItem( SID_BITMAP_LIST ) ) ) );
    SvxHatchListItem aHatchListItem(*( (const SvxHatchListItem*)
        ( mpDocShell->GetItem( SID_HATCH_LIST ) ) ) );

    mpColorList = aColorListItem.GetColorList();
    mpGradientList = aGradientListItem.GetGradientList();
    mpHatchingList = aHatchListItem.GetHatchList();
    mpBitmapList = aBitmapListItem.GetBitmapList();

    FreeResource();

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");

    AddTabPage( RID_SVXPAGE_PAGE, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), 0 );
    AddTabPage( RID_SVXPAGE_AREA, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), 0 );

    if(!bAreaPage)  // I have to add the page before I remove it !
        RemoveTabPage( RID_SVXPAGE_AREA );
}


void SdPageDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch(nId)
    {
    case RID_SVXPAGE_PAGE:
        aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_PRESENTATION));
        aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_PAPER_START, PAPER_A0));
        aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_PAPER_END, PAPER_E));
        rPage.PageCreated(aSet);
        break;
    case RID_SVXPAGE_AREA:
            aSet.Put (SvxColorListItem(mpColorList,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(mpGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(mpHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(mpBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,0));
            rPage.PageCreated(aSet);
        break;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
