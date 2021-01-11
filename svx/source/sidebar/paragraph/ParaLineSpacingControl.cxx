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

#include "ParaLineSpacingControl.hxx"

#include <editeng/editids.hrc>
#include <editeng/lspcitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <svtools/unitconv.hxx>

#include <svl/intitem.hxx>
#include <svl/itempool.hxx>

#include <svx/ParaLineSpacingPopup.hxx>

#define DEFAULT_LINE_SPACING  200
#define FIX_DIST_DEF          283
#define LINESPACE_1           100
#define LINESPACE_15          150
#define LINESPACE_2           200
#define LINESPACE_115         115

// values of the mxLineDist listbox
#define LLINESPACE_1          0
#define LLINESPACE_115        1
#define LLINESPACE_15         2
#define LLINESPACE_2          3
#define LLINESPACE_PROP       4
#define LLINESPACE_MIN        5
#define LLINESPACE_DURCH      6
#define LLINESPACE_FIX        7

#define MIN_FIXED_DISTANCE    28

using namespace svx;

ParaLineSpacingControl::ParaLineSpacingControl(SvxLineSpacingToolBoxControl* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/paralinespacingcontrol.ui", "ParaLineSpacingControl")
    , mxControl(pControl)
    , meLNSpaceUnit(MapUnit::Map100thMM)
    , mxSpacing1Button(m_xBuilder->weld_button("spacing_1"))
    , mxSpacing115Button(m_xBuilder->weld_button("spacing_115"))
    , mxSpacing15Button(m_xBuilder->weld_button("spacing_15"))
    , mxSpacing2Button(m_xBuilder->weld_button("spacing_2"))
    , mxLineDist(m_xBuilder->weld_combo_box("line_dist"))
    , mxLineDistLabel(m_xBuilder->weld_label("value_label"))
    , mxLineDistAtPercentBox(m_xBuilder->weld_metric_spin_button("percent_box", FieldUnit::PERCENT))
    , mxLineDistAtMetricBox(m_xBuilder->weld_metric_spin_button("metric_box", FieldUnit::CM))
    , mpActLineDistFld(mxLineDistAtPercentBox.get())
{
    Link<weld::Button&,void> aLink = LINK(this, ParaLineSpacingControl, PredefinedValuesHandler);
    mxSpacing1Button->connect_clicked(aLink);
    mxSpacing115Button->connect_clicked(aLink);
    mxSpacing15Button->connect_clicked(aLink);
    mxSpacing2Button->connect_clicked(aLink);

    Link<weld::ComboBox&,void> aLink3 = LINK( this, ParaLineSpacingControl, LineSPDistHdl_Impl );
    mxLineDist->connect_changed(aLink3);
    SelectEntryPos(LLINESPACE_1);

    Link<weld::MetricSpinButton&,void> aLink2 = LINK( this, ParaLineSpacingControl, LineSPDistAtHdl_Impl );
    mxLineDistAtPercentBox->connect_value_changed( aLink2 );
    mxLineDistAtMetricBox->connect_value_changed( aLink2 );

    FieldUnit eUnit = FieldUnit::INCH;
    const SfxPoolItem* pItem = nullptr;
    SfxViewFrame* pCurrent = SfxViewFrame::Current();
    if (pCurrent && pCurrent->GetBindings().GetDispatcher()->QueryState(SID_ATTR_METRIC, pItem) >= SfxItemState::DEFAULT)
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
    else
        eUnit = SfxModule::GetCurrentFieldUnit();

    SetFieldUnit(*mxLineDistAtMetricBox, eUnit);

    Initialize();
}

void ParaLineSpacingControl::GrabFocus()
{
    mxSpacing1Button->grab_focus();
}

ParaLineSpacingControl::~ParaLineSpacingControl()
{
}

void ParaLineSpacingControl::Initialize()
{
    const SfxPoolItem* pItem(nullptr);
    SfxViewFrame* pCurrent = SfxViewFrame::Current();
    SfxItemState eState = pCurrent ? pCurrent->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PARA_LINESPACE, pItem) : SfxItemState::UNKNOWN;

    const SvxLineSpacingItem* currSPItem = static_cast<const SvxLineSpacingItem*>(pItem);

    mxLineDist->set_sensitive(true);

    if( eState >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = pCurrent->GetPool().GetMetric(currSPItem->Which());
        meLNSpaceUnit = eUnit;

        switch( currSPItem->GetLineSpaceRule() )
        {
        case SvxLineSpaceRule::Auto:
            {
                SvxInterLineSpaceRule eInter = currSPItem->GetInterLineSpaceRule();

                switch( eInter )
                {
                case SvxInterLineSpaceRule::Off:
                    SelectEntryPos(LLINESPACE_1);
                    break;

                case SvxInterLineSpaceRule::Prop:
                    {
                        if ( LINESPACE_1 == currSPItem->GetPropLineSpace() )
                        {
                            SelectEntryPos(LLINESPACE_1);
                        }
                        else if ( LINESPACE_115 == currSPItem->GetPropLineSpace() )
                        {
                            SelectEntryPos(LLINESPACE_115);
                        }
                        else if ( LINESPACE_15 == currSPItem->GetPropLineSpace() )
                        {
                            SelectEntryPos(LLINESPACE_15);
                        }
                        else if ( LINESPACE_2 == currSPItem->GetPropLineSpace() )
                        {
                            SelectEntryPos(LLINESPACE_2);
                        }
                        else
                        {
                            SelectEntryPos(LLINESPACE_PROP);
                            mxLineDistAtPercentBox->set_value(mxLineDistAtPercentBox->normalize(currSPItem->GetPropLineSpace()), FieldUnit::PERCENT);
                        }
                    }
                    break;

                case SvxInterLineSpaceRule::Fix:
                    {
                        SelectEntryPos(LLINESPACE_DURCH);
                        SetMetricValue(*mxLineDistAtMetricBox, currSPItem->GetInterLineSpace(), eUnit);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SvxLineSpaceRule::Fix:
            {
                SelectEntryPos(LLINESPACE_FIX);
                SetMetricValue(*mxLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
            }
            break;

        case SvxLineSpaceRule::Min:
            {
                SelectEntryPos(LLINESPACE_MIN);
                SetMetricValue(*mxLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
            }
            break;
        default:
            break;
        }
    }
    else if( eState == SfxItemState::DISABLED )
    {
        mxLineDist->set_sensitive(false);
        mxLineDistLabel->set_sensitive(false);
        mpActLineDistFld->set_sensitive(false);
        mpActLineDistFld->set_text("");

    }
    else
    {
        mxLineDistLabel->set_sensitive(false);
        mpActLineDistFld->set_sensitive(false);
        mpActLineDistFld->set_text("");
        mxLineDist->set_active(-1);
    }

    mxLineDist->save_value();
}

void ParaLineSpacingControl::UpdateMetricFields()
{
    switch (mxLineDist->get_active())
    {
        case LLINESPACE_1:
        case LLINESPACE_115:
        case LLINESPACE_15:
        case LLINESPACE_2:
            if (mpActLineDistFld == mxLineDistAtPercentBox.get())
                mxLineDistAtMetricBox->hide();
            else
                mxLineDistAtPercentBox->hide();

            mxLineDistLabel->set_sensitive(false);
            mpActLineDistFld->show();
            mpActLineDistFld->set_sensitive(false);
            mpActLineDistFld->set_text("");
            break;

        case LLINESPACE_DURCH:
            mxLineDistAtPercentBox->hide();

            mpActLineDistFld = mxLineDistAtMetricBox.get();
            mxLineDistAtMetricBox->set_min(0, FieldUnit::NONE);

            if (mxLineDistAtMetricBox->get_text().isEmpty())
                mxLineDistAtMetricBox->set_value(mxLineDistAtMetricBox->normalize(0), FieldUnit::NONE);

            mxLineDistLabel->set_sensitive(true);
            mpActLineDistFld->show();
            mpActLineDistFld->set_sensitive(true);
            break;

        case LLINESPACE_MIN:
            mxLineDistAtPercentBox->hide();

            mpActLineDistFld = mxLineDistAtMetricBox.get();
            mxLineDistAtMetricBox->set_min(0, FieldUnit::NONE);

            if (mxLineDistAtMetricBox->get_text().isEmpty())
                mxLineDistAtMetricBox->set_value(mxLineDistAtMetricBox->normalize(0), FieldUnit::TWIP);

            mxLineDistLabel->set_sensitive(true);
            mpActLineDistFld->show();
            mpActLineDistFld->set_sensitive(true);
            break;

        case LLINESPACE_PROP:
            mxLineDistAtMetricBox->hide();

            mpActLineDistFld = mxLineDistAtPercentBox.get();

            if (mxLineDistAtPercentBox->get_text().isEmpty())
                mxLineDistAtPercentBox->set_value(mxLineDistAtPercentBox->normalize(100), FieldUnit::TWIP);

            mxLineDistLabel->set_sensitive(true);
            mpActLineDistFld->show();
            mpActLineDistFld->set_sensitive(true);
            break;

        case LLINESPACE_FIX:
            mxLineDistAtPercentBox->hide();

            mpActLineDistFld = mxLineDistAtMetricBox.get();
            sal_Int64 nTemp = mxLineDistAtMetricBox->get_value(FieldUnit::NONE);
            mxLineDistAtMetricBox->set_min(mxLineDistAtMetricBox->normalize(MIN_FIXED_DISTANCE), FieldUnit::TWIP);

            if (mxLineDistAtMetricBox->get_value(FieldUnit::NONE) != nTemp)
                SetMetricValue(*mxLineDistAtMetricBox, FIX_DIST_DEF, MapUnit::MapTwip);

            mxLineDistLabel->set_sensitive(true);
            mpActLineDistFld->show();
            mpActLineDistFld->set_sensitive(true);
            break;
    }
}

void ParaLineSpacingControl::SelectEntryPos(sal_Int32 nPos)
{
    mxLineDist->set_active(nPos);
    UpdateMetricFields();
}

IMPL_LINK_NOARG(ParaLineSpacingControl, LineSPDistHdl_Impl, weld::ComboBox&, void)
{
    UpdateMetricFields();
    ExecuteLineSpace();
}

IMPL_LINK_NOARG( ParaLineSpacingControl, LineSPDistAtHdl_Impl, weld::MetricSpinButton&, void )
{
    ExecuteLineSpace();
}

void ParaLineSpacingControl::ExecuteLineSpace()
{
    mxLineDist->save_value();

    SvxLineSpacingItem aSpacing(DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
    const sal_Int32 nPos = mxLineDist->get_active();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_115:
        case LLINESPACE_15:
        case LLINESPACE_2:
            SetLineSpace(aSpacing, nPos);
            break;

        case LLINESPACE_PROP:
            SetLineSpace(aSpacing, nPos, mxLineDistAtPercentBox->denormalize(static_cast<tools::Long>(mxLineDistAtPercentBox->get_value(FieldUnit::PERCENT))));
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            SetLineSpace(aSpacing, nPos, GetCoreValue(*mxLineDistAtMetricBox, meLNSpaceUnit));
            break;

        default:
            break;
    }

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, { &aSpacing });
}

void ParaLineSpacingControl::SetLineSpace(SvxLineSpacingItem& rLineSpace, sal_Int32 eSpace, tools::Long lValue)
{
    switch ( eSpace )
    {
        case LLINESPACE_1:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
            break;

        case LLINESPACE_115:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( LINESPACE_115 );
            break;

        case LLINESPACE_15:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( LINESPACE_15 );
            break;

        case LLINESPACE_2:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( LINESPACE_2 );
            break;

        case LLINESPACE_PROP:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( static_cast<sal_uInt16>(lValue) );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( static_cast<sal_uInt16>(lValue) );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
            break;

        case LLINESPACE_DURCH:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetInterLineSpace( static_cast<sal_uInt16>(lValue) );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight(static_cast<sal_uInt16>(lValue));
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Fix );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
        break;
    }
}

IMPL_LINK(ParaLineSpacingControl, PredefinedValuesHandler, weld::Button&, rControl, void)
{
    if (&rControl == mxSpacing1Button.get())
    {
        ExecuteLineSpacing(LLINESPACE_1);
    }
    else if (&rControl == mxSpacing115Button.get())
    {
        ExecuteLineSpacing(LLINESPACE_115);
    }
    else if (&rControl == mxSpacing15Button.get())
    {
        ExecuteLineSpacing(LLINESPACE_15);
    }
    else if (&rControl == mxSpacing2Button.get())
    {
        ExecuteLineSpacing(LLINESPACE_2);
    }
}

void ParaLineSpacingControl::ExecuteLineSpacing(sal_Int32 nEntry)
{
    SvxLineSpacingItem aSpacing(DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);

    SetLineSpace(aSpacing, nEntry);

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, { &aSpacing });

    // close when the user used the buttons
    mxControl->EndPopupMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
