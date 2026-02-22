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

#include <sfx2/objsh.hxx>
#include <svx/svxids.hrc>

#include <svx/xtable.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>

#include <vcl/tabs.hrc>
#include <vcl/weld/Dialog.hxx>

SvxAreaTabDialog::SvxAreaTabDialog
(
    weld::Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    bool bShadow,
    bool bSlideBackground
)
    : SfxTabDialogController(pParent, u"cui/ui/areadialog.ui"_ustr, u"AreaDialog"_ustr, pAttr)
    , mpDrawModel          ( pModel ),
    mpColorList          ( pModel->GetColorList() ),
    mpNewColorList       ( pModel->GetColorList() ),
    mpGradientList       ( pModel->GetGradientList() ),
    mpNewGradientList    ( pModel->GetGradientList() ),
    mpHatchingList       ( pModel->GetHatchList() ),
    mpNewHatchingList    ( pModel->GetHatchList() ),
    mpBitmapList         ( pModel->GetBitmapList() ),
    mpNewBitmapList      ( pModel->GetBitmapList() ),
    mpPatternList        ( pModel->GetPatternList() ),
    mpNewPatternList     ( pModel->GetPatternList() ),
    mnColorListState(ChangeType::NONE)
{
    if (bSlideBackground)
        AddTabPage(u"RID_SVXPAGE_AREA"_ustr, TabResId(RID_TAB_AREA.aLabel),
                   SvxAreaTabPage::CreateWithSlideBackground, RID_L + RID_TAB_AREA.sIconName);
    else
        AddTabPage(u"RID_SVXPAGE_AREA"_ustr, TabResId(RID_TAB_AREA.aLabel), SvxAreaTabPage::Create,
                   RID_L + RID_TAB_AREA.sIconName);

    if (bShadow)
    {
        AddTabPage(u"RID_SVXPAGE_SHADOW"_ustr, TabResId(RID_TAB_SHADOW.aLabel),
                   SvxShadowTabPage::Create, RID_L + RID_TAB_SHADOW.sIconName);
    }
    AddTabPage(u"RID_SVXPAGE_TRANSPARENCE"_ustr, TabResId(RID_TAB_TRANSPARENCE.aLabel),
               SvxTransparenceTabPage::Create, RID_L + RID_TAB_TRANSPARENCE.sIconName);

    weld::Button& rBtnCancel = GetCancelButton();
    rBtnCancel.connect_clicked(LINK(this, SvxAreaTabDialog, CancelHdlImpl));
}

void SvxAreaTabDialog::SavePalettes()
{
    SfxObjectShell* pShell(SfxObjectShell::Current());
    if (!pShell)
    {
        SAL_WARN("cui.dialogs", "SvxAreaTabDialog: No SfxObjectShell!");
        return;
    }

    if( mpNewColorList != mpDrawModel->GetColorList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewColorList.get()) );
        SvxColorListItem aColorListItem( mpNewColorList, SID_COLOR_TABLE );
        pShell->PutItem( aColorListItem );
        mpColorList = mpDrawModel->GetColorList();
    }
    if( mpNewGradientList != mpDrawModel->GetGradientList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewGradientList.get()) );
        SvxGradientListItem aItem( mpNewGradientList, SID_GRADIENT_LIST );
        pShell->PutItem( aItem );
        mpGradientList = mpDrawModel->GetGradientList();
    }
    if( mpNewHatchingList != mpDrawModel->GetHatchList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewHatchingList.get()) );
        SvxHatchListItem aItem( mpNewHatchingList, SID_HATCH_LIST );
        pShell->PutItem( aItem );
        mpHatchingList = mpDrawModel->GetHatchList();
    }
    if( mpNewBitmapList != mpDrawModel->GetBitmapList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewBitmapList.get()) );
        SvxBitmapListItem aItem( mpNewBitmapList, SID_BITMAP_LIST );
        pShell->PutItem( aItem );
        mpBitmapList = mpDrawModel->GetBitmapList();
    }
    if( mpNewPatternList != mpDrawModel->GetPatternList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewPatternList.get()) );
        SvxPatternListItem aItem( mpNewPatternList, SID_PATTERN_LIST );
        pShell->PutItem( aItem );
        mpPatternList = mpDrawModel->GetPatternList();
    }

    // save the tables when they have been changed
    if (mnColorListState & ChangeType::MODIFIED && mpColorList.is())
    {
        SvxColorListItem aItem( mpColorList, SID_COLOR_TABLE );
        // ToolBoxControls are informed:
        pShell->PutItem( aItem );
    }
}

short SvxAreaTabDialog::Ok()
{
    SavePalettes();
    // RET_OK is returned, if at least one
    // TabPage returns sal_True in FillItemSet().
    // This happens by default at the moment.
    return SfxTabDialogController::Ok();
}

IMPL_LINK_NOARG(SvxAreaTabDialog, CancelHdlImpl, weld::Button&, void)
{
    SavePalettes();
    m_xDialog->response(RET_CANCEL);
}

void SvxAreaTabDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (rId == "RID_SVXPAGE_AREA")
    {
        static_cast<SvxAreaTabPage&>(rPage).SetColorList( mpColorList );
        static_cast<SvxAreaTabPage&>(rPage).SetGradientList( mpGradientList );
        static_cast<SvxAreaTabPage&>(rPage).SetHatchingList( mpHatchingList );
        static_cast<SvxAreaTabPage&>(rPage).SetBitmapList( mpBitmapList );
        static_cast<SvxAreaTabPage&>(rPage).SetPatternList( mpPatternList );
        static_cast<SvxAreaTabPage&>(rPage).SetColorChgd( &mnColorListState );
    }
    else if (rId == "RID_SVXPAGE_SHADOW")
    {
        static_cast<SvxShadowTabPage&>(rPage).SetColorList( mpColorList );
        static_cast<SvxShadowTabPage&>(rPage).SetColorChgd( &mnColorListState );
    }
    else if (rId == "RID_SVXPAGE_TRANSPARENCE")
    {
        static_cast<SvxTransparenceTabPage&>(rPage).SetPageType( PageType::Area );
        static_cast<SvxTransparenceTabPage&>(rPage).SetDlgType( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
