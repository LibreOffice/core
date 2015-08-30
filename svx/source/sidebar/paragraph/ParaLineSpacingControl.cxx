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
#include <editeng/kernitem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/unitconv.hxx>

#define DEFAULT_LINE_SPACING  200
#define FIX_DIST_DEF          283
#define LINESPACE_1           100
#define LINESPACE_15          150
#define LINESPACE_2           200
#define LINESPACE_115         115

// values of the mpLineDist listbox
#define LLINESPACE_1          0
#define LLINESPACE_15         1
#define LLINESPACE_2          2
#define LLINESPACE_PROP       3
#define LLINESPACE_MIN        4
#define LLINESPACE_DURCH      5
#define LLINESPACE_FIX        6

// special case; should not conflict with the mpLinDist values
#define LLINESPACE_115        7

#define MIN_FIXED_DISTANCE    28

using namespace svx;

ParaLineSpacingControl::ParaLineSpacingControl(sal_uInt16 nId)
    : SfxPopupWindow(nId, "ParaLineSpacingControl", "svx/ui/paralinespacingcontrol.ui")
{
    mpSpacing1Button = get<PushButton>("spacing_1");
    mpSpacing115Button = get<PushButton>("spacing_115");
    mpSpacing15Button = get<PushButton>("spacing_15");
    mpSpacing2Button = get<PushButton>("spacing_2");

    mpLineDist = get<ListBox>("line_dist");

    mpLineDistLabel = get<FixedText>("value_label");
    mpLineDistAtPercentBox = get<MetricField>("percent_box");
    mpLineDistAtMetricBox = get<MetricField>("metric_box");

    mpActLineDistFld = mpLineDistAtPercentBox.get();

    meLNSpaceUnit = SFX_MAPUNIT_100TH_MM;

    Link<Button*,void> aLink = LINK(this, ParaLineSpacingControl, PredefinedValuesHandler);
    mpSpacing1Button->SetClickHdl(aLink);
    mpSpacing115Button->SetClickHdl(aLink);
    mpSpacing15Button->SetClickHdl(aLink);
    mpSpacing2Button->SetClickHdl(aLink);

    Link<> aLink2 = LINK( this, ParaLineSpacingControl, LineSPDistHdl_Impl );
    mpLineDist->SetSelectHdl(aLink2);
    SelectEntryPos(LLINESPACE_1);

    aLink2 = LINK( this, ParaLineSpacingControl, LineSPDistAtHdl_Impl );
    mpLineDistAtPercentBox->SetModifyHdl( aLink2 );
    mpLineDistAtMetricBox->SetModifyHdl( aLink2 );

    FieldUnit eUnit = FUNIT_INCH;
    const SfxPoolItem* pItem = NULL;
    if (SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_METRIC, pItem) >= SfxItemState::DEFAULT)
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
    else
        eUnit = SfxModule::GetCurrentFieldUnit();

    SetFieldUnit(*mpLineDistAtMetricBox, eUnit);

    Initialize();
}

ParaLineSpacingControl::~ParaLineSpacingControl()
{
    disposeOnce();
}

void ParaLineSpacingControl::dispose()
{
    mpActLineDistFld.clear();
    mpSpacing1Button.clear();
    mpSpacing115Button.clear();
    mpSpacing15Button.clear();
    mpSpacing2Button.clear();
    mpLineDist.clear();
    mpLineDistLabel.clear();
    mpLineDistAtPercentBox.clear();
    mpLineDistAtMetricBox.clear();
    SfxPopupWindow::dispose();
}

void ParaLineSpacingControl::Initialize()
{
    const SfxPoolItem* pItem;
    SfxItemState eState = SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PARA_LINESPACE, pItem);

    const SvxLineSpacingItem* currSPItem = static_cast<const SvxLineSpacingItem*>(pItem);

    mpLineDist->Enable();

    if( eState >= SfxItemState::DEFAULT )
    {
        SfxMapUnit eUnit = SFX_MAPUNIT_100TH_MM;
        meLNSpaceUnit = eUnit;

        switch( currSPItem->GetLineSpaceRule() )
        {
        case SVX_LINE_SPACE_AUTO:
            {
                SvxInterLineSpace eInter = currSPItem->GetInterLineSpaceRule();

                switch( eInter )
                {
                case SVX_INTER_LINE_SPACE_OFF:
                    SelectEntryPos(LLINESPACE_1);
                    break;

                case SVX_INTER_LINE_SPACE_PROP:
                    {
                        if ( LINESPACE_1 == currSPItem->GetPropLineSpace() )
                        {
                            SelectEntryPos(LLINESPACE_1);
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
                            mpLineDistAtPercentBox->SetValue(mpLineDistAtPercentBox->Normalize(currSPItem->GetPropLineSpace()));
                        }
                    }
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    {
                        SelectEntryPos(LLINESPACE_DURCH);
                        SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetInterLineSpace(), eUnit);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SVX_LINE_SPACE_FIX:
            {
                SelectEntryPos(LLINESPACE_FIX);
                SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
            }
            break;

        case SVX_LINE_SPACE_MIN:
            {
                SelectEntryPos(LLINESPACE_MIN);
                SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
            }
            break;
        default:
            break;
        }
    }
    else if( eState == SfxItemState::DISABLED )
    {
        mpLineDist->Disable();
        mpLineDistLabel->Disable();
        mpActLineDistFld->Disable();
        mpActLineDistFld->SetText("");

    }
    else
    {
        mpLineDistLabel->Disable();
        mpActLineDistFld->Disable();
        mpActLineDistFld->SetText("");
        mpLineDist->SetNoSelection();
    }

    mpLineDist->SaveValue();

    /* TODO
    const sal_uInt16 uCount = mpLineDist->GetEntryCount();
    if( uCount == LLINESPACE_FIX + 1 )
    {
        switch (currentContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
        case CombinedEnumContext(Application_Calc, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            {
                mpLineDist->RemoveEntry(LLINESPACE_FIX);
            }
        }
    }
    else if( uCount == LLINESPACE_FIX)
    {
        switch (currentContext.GetCombinedContext_DI())
        {
            case CombinedEnumContext(Application_WriterVariants, Context_Default):
            case CombinedEnumContext(Application_WriterVariants, Context_Text):
            case CombinedEnumContext(Application_WriterVariants, Context_Table):
            {
                mpLineDist->InsertEntry(OUString("Fixed"), LLINESPACE_FIX);
            }
        }
    }
    */
}

void ParaLineSpacingControl::UpdateMetricFields()
{
    switch (mpLineDist->GetSelectEntryPos())
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            if (mpActLineDistFld == mpLineDistAtPercentBox)
                mpLineDistAtMetricBox->Hide();
            else
                mpLineDistAtPercentBox->Hide();

            mpLineDistLabel->Disable();
            mpActLineDistFld->Show();
            mpActLineDistFld->Disable();
            mpActLineDistFld->SetText("");
            break;

        case LLINESPACE_DURCH:
            mpLineDistAtPercentBox->Hide();

            mpActLineDistFld = mpLineDistAtMetricBox.get();
            mpLineDistAtMetricBox->SetMin(0);

            if (mpLineDistAtMetricBox->GetText().isEmpty())
                mpLineDistAtMetricBox->SetValue(mpLineDistAtMetricBox->Normalize(0));

            mpLineDistLabel->Enable();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;

        case LLINESPACE_MIN:
            mpLineDistAtPercentBox->Hide();

            mpActLineDistFld = mpLineDistAtMetricBox.get();
            mpLineDistAtMetricBox->SetMin(0);

            if (mpLineDistAtMetricBox->GetText().isEmpty())
                mpLineDistAtMetricBox->SetValue(mpLineDistAtMetricBox->Normalize(0), FUNIT_TWIP);

            mpLineDistLabel->Enable();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;

        case LLINESPACE_PROP:
            mpLineDistAtMetricBox->Hide();

            mpActLineDistFld = mpLineDistAtPercentBox.get();

            if (mpLineDistAtPercentBox->GetText().isEmpty())
                mpLineDistAtPercentBox->SetValue(mpLineDistAtPercentBox->Normalize(100), FUNIT_TWIP);

            mpLineDistLabel->Enable();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;

        case LLINESPACE_FIX:
            mpLineDistAtPercentBox->Hide();

            mpActLineDistFld = mpLineDistAtMetricBox.get();
            sal_Int64 nTemp = mpLineDistAtMetricBox->GetValue();
            mpLineDistAtMetricBox->SetMin(mpLineDistAtMetricBox->Normalize(MIN_FIXED_DISTANCE), FUNIT_TWIP);

            if (mpLineDistAtMetricBox->GetValue() != nTemp)
                SetMetricValue(*mpLineDistAtMetricBox, FIX_DIST_DEF, SFX_MAPUNIT_TWIP);

            mpLineDistLabel->Enable();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;
    }
}

void ParaLineSpacingControl::SelectEntryPos(sal_Int32 nPos)
{
    mpLineDist->SelectEntryPos(nPos);
    UpdateMetricFields();
}

IMPL_LINK(ParaLineSpacingControl, LineSPDistHdl_Impl, ListBox*, /*pBox*/)
{
    UpdateMetricFields();
    ExecuteLineSpace();
    return 0;
}

IMPL_LINK_NOARG( ParaLineSpacingControl, LineSPDistAtHdl_Impl )
{
    ExecuteLineSpace();
    return 0L;
}

void ParaLineSpacingControl::ExecuteLineSpace()
{
    mpLineDist->SaveValue();

    SvxLineSpacingItem aSpacing(DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
    const sal_Int32 nPos = mpLineDist->GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            SetLineSpace(aSpacing, nPos);
            break;

        case LLINESPACE_PROP:
            SetLineSpace(aSpacing, nPos, mpLineDistAtPercentBox->Denormalize((long)mpLineDistAtPercentBox->GetValue()));
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            SetLineSpace(aSpacing, nPos, GetCoreValue(*mpLineDistAtMetricBox, meLNSpaceUnit));
            break;

        default:
            break;
    }

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);
}

void ParaLineSpacingControl::SetLineSpace(SvxLineSpacingItem& rLineSpace, sal_Int32 eSpace, long lValue)
{
    switch ( eSpace )
    {
        case LLINESPACE_1:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_15:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( LINESPACE_15 );
            break;

        case LLINESPACE_2:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( LINESPACE_2 );
            break;

        case LLINESPACE_PROP:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( (sal_uInt8)lValue );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( (sal_uInt16)lValue );
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_DURCH:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetInterLineSpace( (sal_uInt16)lValue );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight((sal_uInt16)lValue);
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_FIX;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        break;
    }
}

IMPL_LINK_TYPED(ParaLineSpacingControl, PredefinedValuesHandler, Button*, pControl, void)
{
    if (pControl == mpSpacing1Button)
    {
        ExecuteLineSpacing(LLINESPACE_1);
    }
    else if (pControl == mpSpacing115Button)
    {
        ExecuteLineSpacing(LLINESPACE_115);
    }
    else if (pControl == mpSpacing15Button)
    {
        ExecuteLineSpacing(LLINESPACE_15);
    }
    else if (pControl == mpSpacing2Button)
    {
        ExecuteLineSpacing(LLINESPACE_2);
    }
}

void ParaLineSpacingControl::ExecuteLineSpacing(sal_Int32 nEntry)
{
    SvxLineSpacingItem aSpacing(DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);

    // special-case the 1.15 line spacing
    if (nEntry == LLINESPACE_115)
        SetLineSpace(aSpacing, LLINESPACE_PROP, mpLineDistAtPercentBox->Denormalize(LINESPACE_115));
    else
        SetLineSpace(aSpacing, nEntry);

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);

    // close when the user used the buttons
    EndPopupMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
