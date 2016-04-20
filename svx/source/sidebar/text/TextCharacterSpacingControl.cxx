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
#include <sfx2/bindings.hxx>
#include "TextCharacterSpacingControl.hxx"
#include "TextPropertyPanel.hrc"
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/settings.hxx>
#include <vcl/lstbox.hxx>
#include <svl/itempool.hxx>

#define SPACING_VERY_TIGHT  -30
#define SPACING_TIGHT       -15
#define SPACING_NORMAL        0
#define SPACING_LOOSE        30
#define SPACING_VERY_LOOSE   60

namespace svx {

TextCharacterSpacingControl::TextCharacterSpacingControl(sal_uInt16 nId)
    : SfxPopupWindow(nId, "TextCharacterSpacingControl", "svx/ui/textcharacterspacingcontrol.ui")
,   mnId(nId)
,   mnCustomKern(0)
,   mnLastCus(SPACING_NOCUSTOM)
{
    get(maEditKerning, "kerning");
    get(maLBKerning, "kerninglb");
    get(maFTBy, "change_by_label");
    get(maFTSpacing, "spacing_label");
    maEditKerning->Disable();
    maFTBy->Disable();

    get(maNormal, "normal");
    get(maVeryTight, "very_tight");
    get(maTight, "tight");
    get(maVeryLoose, "very_loose");
    get(maLoose, "loose");
    get(maLastCustom, "last_custom");

    maLBKerning->SetSelectHdl(LINK(this, TextCharacterSpacingControl, KerningSelectHdl));
    maLBKerning->SetHelpId(HID_SPACING_CB_KERN);
    maEditKerning->SetModifyHdl(LINK(this, TextCharacterSpacingControl, KerningModifyHdl));
    maEditKerning->SetHelpId(HID_SPACING_MB_KERN);

    Link<Button*,void> aLink = LINK(this, TextCharacterSpacingControl, PredefinedValuesHdl);
    maNormal->SetClickHdl(aLink);
    maVeryTight->SetClickHdl(aLink);
    maTight->SetClickHdl(aLink);
    maVeryLoose->SetClickHdl(aLink);
    maLoose->SetClickHdl(aLink);
    maLastCustom->SetClickHdl(aLink);

    Initialize();
}

TextCharacterSpacingControl::~TextCharacterSpacingControl()
{
    disposeOnce();
}

void TextCharacterSpacingControl::dispose()
{
    if (GetLastCustomState() == SPACING_CLOSE_BY_CUS_EDIT)
    {
        SvtViewOptions aWinOpt(E_WINDOW, SIDEBAR_SPACING_GLOBAL_VALUE);
        css::uno::Sequence<css::beans::NamedValue> aSeq
            { { "Spacing", css::uno::makeAny(OUString::number(GetLastCustomValue())) } };
        aWinOpt.SetUserData(aSeq);
    }

    maFTBy.clear();
    maFTSpacing.clear();
    maLBKerning.clear();
    maEditKerning.clear();

    maNormal.clear();
    maVeryTight.clear();
    maTight.clear();
    maVeryLoose.clear();
    maLoose.clear();
    maLastCustom.clear();

    SfxPopupWindow::dispose();
}

void TextCharacterSpacingControl::Initialize()
{
    const SfxPoolItem* pItem;
    SfxItemState eState = SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_CHAR_KERNING, pItem);

    const SvxKerningItem* pKerningItem = static_cast<const SvxKerningItem*>(pItem);
    long nKerning = 0;

    if(pKerningItem)
        nKerning = pKerningItem->GetValue();

    SvtViewOptions aWinOpt(E_WINDOW, SIDEBAR_SPACING_GLOBAL_VALUE);
    if(aWinOpt.Exists())
    {
        css::uno::Sequence<css::beans::NamedValue> aSeq = aWinOpt.GetUserData();
        ::rtl::OUString aTmp;
        if(aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData(aTmp);
        mnCustomKern = aWinData.toInt32();
        mnLastCus = SPACING_CLOSE_BY_CUS_EDIT;
    }
    else
    {
        mnLastCus = SPACING_NOCUSTOM;
    }

    if(eState >= SfxItemState::DEFAULT)
    {
        maLBKerning->Enable();
        maFTSpacing->Enable();

        SfxMapUnit eUnit = GetCoreMetric();
        MapUnit eOrgUnit = (MapUnit)eUnit;
        MapUnit ePntUnit(MAP_POINT);
        long nBig = maEditKerning->Normalize(nKerning);
        nKerning = LogicToLogic(nBig, eOrgUnit, ePntUnit);

        if(nKerning > 0)
        {
            maFTBy->Enable();
            maEditKerning->Enable();
            maEditKerning->SetMax(9999);
            maEditKerning->SetLast(9999);
            maEditKerning->SetValue(nKerning);
            maLBKerning->SelectEntryPos(SIDEBAR_SPACE_EXPAND);
        }
        else if(nKerning < 0)
        {
            maFTBy->Enable();
            maEditKerning->Enable();
            maEditKerning->SetValue(-nKerning);
            maLBKerning->SelectEntryPos(SIDEBAR_SPACE_CONDENSED);
            long nMax = GetSelFontSize()/6;
            maEditKerning->SetMax(maEditKerning->Normalize(nMax), FUNIT_POINT);
            maEditKerning->SetLast(maEditKerning->GetMax(maEditKerning->GetUnit()));
        }
        else
        {
            maLBKerning->SelectEntryPos(SIDEBAR_SPACE_NORMAL);
            maFTBy->Disable();
            maEditKerning->Disable();
            maEditKerning->SetValue(0);
            maEditKerning->SetMax(9999);
            maEditKerning->SetLast(9999);
        }
    }
    else if(SfxItemState::DISABLED == eState)
    {
        maEditKerning->SetText(OUString());
        maLBKerning->SetNoSelection();
        maLBKerning->Disable();
        maFTSpacing->Disable();
        maEditKerning->Disable();
        maFTBy->Disable();
    }
    else
    {
        maLBKerning->Enable();
        maFTSpacing->Enable();
        maLBKerning->SetNoSelection();
        maEditKerning->SetText(OUString());
        maEditKerning->Disable();
        maFTBy->Disable();
    }
}

void TextCharacterSpacingControl::ExecuteCharacterSpacing(long nValue, bool bClose)
{
    SfxMapUnit eUnit = GetCoreMetric();

    long nSign = (nValue < 0) ? -1 : 1;
    nValue = nValue * nSign;

    long nVal = LogicToLogic(nValue, MAP_POINT, (MapUnit)eUnit);
    short nKern = (nValue == 0) ? 0 : (short)maEditKerning->Denormalize(nVal);

    SvxKerningItem aKernItem(nSign * nKern, SID_ATTR_CHAR_KERNING);

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList(SID_ATTR_CHAR_KERNING,
        SfxCallMode::RECORD, { &aKernItem });

    if(bClose)
        EndPopupMode();
}

IMPL_LINK_TYPED(TextCharacterSpacingControl, PredefinedValuesHdl, Button*, pControl, void)
{
    mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;

    if(pControl == maNormal)
    {
        ExecuteCharacterSpacing(SPACING_NORMAL);
    }
    else if(pControl == maVeryTight)
    {
        ExecuteCharacterSpacing(SPACING_VERY_TIGHT);
    }
    else if(pControl == maTight)
    {
        ExecuteCharacterSpacing(SPACING_TIGHT);
    }
    else if(pControl == maVeryLoose)
    {
        ExecuteCharacterSpacing(SPACING_VERY_LOOSE);
    }
    else if(pControl == maLoose)
    {
        ExecuteCharacterSpacing(SPACING_LOOSE);
    }
    else if(pControl == maLastCustom)
    {
        ExecuteCharacterSpacing(mnCustomKern);
    }
}

IMPL_LINK_NOARG_TYPED(TextCharacterSpacingControl, KerningSelectHdl, ListBox&, void)
{
    if(maLBKerning->GetSelectEntryPos() > 0)
    {
        maFTBy->Enable();
        maEditKerning->Enable();
    }
    else
    {
        maEditKerning->SetValue(0);
        maFTBy->Disable();
        maEditKerning->Disable();
    }

    KerningModifyHdl(*maEditKerning);
}

IMPL_LINK_NOARG_TYPED(TextCharacterSpacingControl, KerningModifyHdl, Edit&, void)
{
    const sal_Int32 nPos = maLBKerning->GetSelectEntryPos();

    mnLastCus = SPACING_CLOSE_BY_CUS_EDIT;
    if(nPos == SIDEBAR_SPACE_EXPAND || nPos == SIDEBAR_SPACE_CONDENSED)
    {
        long nTmp = static_cast<long>(maEditKerning->GetValue());
        if(nPos == SIDEBAR_SPACE_CONDENSED)
        {
            long nMax = GetSelFontSize()/6;
            maEditKerning->SetMax(maEditKerning->Normalize(nMax), FUNIT_TWIP);
            maEditKerning->SetLast(maEditKerning->GetMax(maEditKerning->GetUnit()));
            if(nTmp > maEditKerning->GetMax())
                nTmp = maEditKerning->GetMax();
            mnCustomKern = -nTmp;
        }
        else
        {
            maEditKerning->SetMax(9999);
            maEditKerning->SetLast(9999);
            if(nTmp > maEditKerning->GetMax(FUNIT_TWIP))
                nTmp = maEditKerning->GetMax(FUNIT_TWIP);
            mnCustomKern = nTmp;
        }
    }
    else
    {
        mnCustomKern = 0;
    }

    ExecuteCharacterSpacing(mnCustomKern, false);
}

SfxMapUnit TextCharacterSpacingControl::GetCoreMetric() const
{
    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich(mnId);
    return rPool.GetMetric(nWhich);
}

long TextCharacterSpacingControl::GetSelFontSize() const
{
    const SfxPoolItem* pItem;
    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_CHAR_FONTHEIGHT, pItem);

    const SvxFontHeightItem* pHeightItem = static_cast<const SvxFontHeightItem*>(pItem);

    long nH = 240;
    SfxMapUnit eUnit = GetCoreMetric();
    if(pHeightItem)
        nH = LogicToLogic(pHeightItem->GetHeight(), (MapUnit)eUnit, MAP_TWIP);
    return nH;
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
