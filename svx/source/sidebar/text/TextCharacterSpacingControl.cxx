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
#include <unotools/viewoptions.hxx>
#include <editeng/editids.hrc>
#include <editeng/kernitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/TextCharacterSpacingPopup.hxx>
#include <svl/itempool.hxx>
#include <helpids.h>

#include <com/sun/star/beans/NamedValue.hpp>

#define SPACING_VERY_TIGHT  -30
#define SPACING_TIGHT       -15
#define SPACING_NORMAL        0
#define SPACING_LOOSE        30
#define SPACING_VERY_LOOSE   60

namespace svx {

TextCharacterSpacingControl::TextCharacterSpacingControl(TextCharacterSpacingPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/textcharacterspacingcontrol.ui", "TextCharacterSpacingControl")
    , mnCustomKern(0)
    , mnLastCus(SPACING_NOCUSTOM)
    , mxEditKerning(m_xBuilder->weld_metric_spin_button("kerning", FieldUnit::POINT))
    , mxTight(m_xBuilder->weld_button("tight"))
    , mxVeryTight(m_xBuilder->weld_button("very_tight"))
    , mxNormal(m_xBuilder->weld_button("normal"))
    , mxLoose(m_xBuilder->weld_button("loose"))
    , mxVeryLoose(m_xBuilder->weld_button("very_loose"))
    , mxLastCustom(m_xBuilder->weld_button("last_custom"))
    , mxControl(pControl)
{
    mxEditKerning->connect_value_changed(LINK(this, TextCharacterSpacingControl, KerningModifyHdl));
    mxEditKerning->set_help_id(HID_SPACING_MB_KERN);

    Link<weld::Button&,void> aLink = LINK(this, TextCharacterSpacingControl, PredefinedValuesHdl);
    mxNormal->connect_clicked(aLink);
    mxVeryTight->connect_clicked(aLink);
    mxTight->connect_clicked(aLink);
    mxVeryLoose->connect_clicked(aLink);
    mxLoose->connect_clicked(aLink);
    mxLastCustom->connect_clicked(aLink);

    Initialize();
}

void TextCharacterSpacingControl::GrabFocus()
{
    mxVeryTight->grab_focus();
}

TextCharacterSpacingControl::~TextCharacterSpacingControl()
{
    if (mnLastCus == SPACING_CLOSE_BY_CUS_EDIT)
    {
        SvtViewOptions aWinOpt(EViewType::Window, SIDEBAR_SPACING_GLOBAL_VALUE);
        css::uno::Sequence<css::beans::NamedValue> aSeq
            { { "Spacing", css::uno::makeAny(OUString::number(mnCustomKern)) } };
        aWinOpt.SetUserData(aSeq);
    }
}

void TextCharacterSpacingControl::Initialize()
{
    const SfxPoolItem* pItem;
    SfxItemState eState = SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_CHAR_KERNING, pItem);

    const SvxKerningItem* pKerningItem = static_cast<const SvxKerningItem*>(pItem);
    tools::Long nKerning = 0;

    if(pKerningItem)
        nKerning = pKerningItem->GetValue();

    SvtViewOptions aWinOpt(EViewType::Window, SIDEBAR_SPACING_GLOBAL_VALUE);
    if(aWinOpt.Exists())
    {
        css::uno::Sequence<css::beans::NamedValue> aSeq = aWinOpt.GetUserData();
        OUString aTmp;
        if(aSeq.hasElements())
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
        MapUnit eUnit = GetCoreMetric();
        MapUnit eOrgUnit = eUnit;
        tools::Long nBig = mxEditKerning->normalize(nKerning);
        nKerning = OutputDevice::LogicToLogic(nBig, eOrgUnit, MapUnit::MapPoint);
        mxEditKerning->set_value(nKerning, FieldUnit::NONE);
    }
    else if(SfxItemState::DISABLED == eState)
    {
        mxEditKerning->set_text(OUString());
        mxEditKerning->set_sensitive(false);
    }
    else
    {
        mxEditKerning->set_text(OUString());
        mxEditKerning->set_sensitive(false);
    }
}

void TextCharacterSpacingControl::ExecuteCharacterSpacing(tools::Long nValue, bool bClose)
{
    MapUnit eUnit = GetCoreMetric();

    tools::Long nSign = (nValue < 0) ? -1 : 1;
    nValue = nValue * nSign;

    tools::Long nVal = OutputDevice::LogicToLogic(nValue, MapUnit::MapPoint, eUnit);
    short nKern = (nValue == 0) ? 0 : static_cast<short>(mxEditKerning->denormalize(nVal));

    SvxKerningItem aKernItem(nSign * nKern, SID_ATTR_CHAR_KERNING);

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList(SID_ATTR_CHAR_KERNING,
        SfxCallMode::RECORD, { &aKernItem });

    if (bClose)
        mxControl->EndPopupMode();
}

IMPL_LINK(TextCharacterSpacingControl, PredefinedValuesHdl, weld::Button&, rControl, void)
{
    mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;

    if (&rControl == mxNormal.get())
    {
        ExecuteCharacterSpacing(SPACING_NORMAL);
    }
    else if (&rControl == mxVeryTight.get())
    {
        ExecuteCharacterSpacing(SPACING_VERY_TIGHT);
    }
    else if (&rControl == mxTight.get())
    {
        ExecuteCharacterSpacing(SPACING_TIGHT);
    }
    else if (&rControl == mxVeryLoose.get())
    {
        ExecuteCharacterSpacing(SPACING_VERY_LOOSE);
    }
    else if (&rControl == mxLoose.get())
    {
        ExecuteCharacterSpacing(SPACING_LOOSE);
    }
    else if (&rControl == mxLastCustom.get())
    {
        ExecuteCharacterSpacing(mnCustomKern);
    }
}

IMPL_LINK_NOARG(TextCharacterSpacingControl, KerningModifyHdl, weld::MetricSpinButton&, void)
{
    mnLastCus = SPACING_CLOSE_BY_CUS_EDIT;
    mnCustomKern = mxEditKerning->get_value(FieldUnit::NONE);

    ExecuteCharacterSpacing(mnCustomKern, false);
}

MapUnit TextCharacterSpacingControl::GetCoreMetric()
{
    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich(SID_ATTR_CHAR_KERNING);
    return rPool.GetMetric(nWhich);
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
