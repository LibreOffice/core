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
#include "ParaPropertyPanel.hrc"
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <editeng/kernitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/unitconv.hxx>
#include <vcl/settings.hxx>

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
using namespace svx::sidebar;

ParaLineSpacingControl::ParaLineSpacingControl(sal_uInt16 nId)
    : SfxPopupWindow(nId, "ParaLineSpacingControl", "svx/ui/paralinespacingcontrol.ui")
//    , maLineSpacing(ValueSetWithTextControl::IMAGE_TEXT,this, SVX_RES( LINE_SPACING ) )
{
    mpSpacing1Button = get<PushButton>("spacing_1");
    mpSpacing115Button = get<PushButton>("spacing_115");
    mpSpacing15Button = get<PushButton>("spacing_15");
    mpSpacing2Button = get<PushButton>("spacing_2");

    mpLineDist = get<ListBox>("line_dist");
    mpLineDistAtPercentBox = get<MetricField>("percent_box");
    mpLineDistAtMetricBox = get<MetricField>("metric_box");

    mpActLineDistFld = mpLineDistAtPercentBox;

    initial();
    meLNSpaceUnit = SFX_MAPUNIT_100TH_MM;
}

ParaLineSpacingControl::~ParaLineSpacingControl()
{
}

void ParaLineSpacingControl::initial()
{
    Link aLink = LINK(this, ParaLineSpacingControl, PredefinedValuesHandler);
    mpSpacing1Button->SetClickHdl(aLink);
    mpSpacing115Button->SetClickHdl(aLink);
    mpSpacing15Button->SetClickHdl(aLink);
    mpSpacing2Button->SetClickHdl(aLink);

    aLink = LINK( this, ParaLineSpacingControl, LineSPDistHdl_Impl );
    mpLineDist->SetSelectHdl(aLink);
    mpLineDist->SelectEntryPos( LLINESPACE_1 ) ;

    aLink = LINK( this, ParaLineSpacingControl, LineSPDistAtHdl_Impl );
    mpLineDistAtPercentBox->SetModifyHdl( aLink );
    mpLineDistAtMetricBox->SetModifyHdl( aLink );
}

void ParaLineSpacingControl::Rearrange(SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem,const ::sfx2::sidebar::EnumContext currentContext)
{
    SfxItemState eState = currSPState;

    SetFieldUnit(*mpLineDistAtMetricBox, currMetricUnit);

    mpLineDist->Enable();
    mpActLineDistFld->Enable();
    mpActLineDistFld->SetText( "" );
    //bool bValueSetFocus = sal_False;        //wj

    if( eState >= SfxItemState::DEFAULT )
    {
    //  SfxMapUnit eUnit = maLNSpaceControl.GetCoreMetric();
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
                    {
                        mpLineDist->SelectEntryPos( LLINESPACE_1 );
                        mpActLineDistFld->Disable();
                        mpActLineDistFld->SetText( "" );
                        if ( LINESPACE_1 == currSPItem->GetPropLineSpace() )
                        {
//                            maLineSpacing.SelectItem(1);
                            //bValueSetFocus = sal_True;  //wj
                        }
                    }
                    break;

                case SVX_INTER_LINE_SPACE_PROP:
                    {
                        if ( LINESPACE_1 == currSPItem->GetPropLineSpace() )
                        {
                            mpLineDist->SelectEntryPos( LLINESPACE_1 );
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->SetText( "" );
//                            maLineSpacing.SelectItem(1);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }
                        if ( LINESPACE_15 == currSPItem->GetPropLineSpace() )
                        {
                            mpLineDist->SelectEntryPos( LLINESPACE_15 );
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->SetText( "" );

//                            maLineSpacing.SelectItem(3);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }
                        if ( LINESPACE_2 == currSPItem->GetPropLineSpace() )
                        {
                            mpLineDist->SelectEntryPos( LLINESPACE_2 );
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->SetText( "" );

//                            maLineSpacing.SelectItem(4);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }

                        mpLineDist->SelectEntryPos( LLINESPACE_PROP );
                        if(mpActLineDistFld != mpLineDistAtPercentBox)
                        {
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->Hide();
                            mpActLineDistFld = mpLineDistAtPercentBox;
                        }
                        else
                        {
                            mpActLineDistFld = mpLineDistAtMetricBox;
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->Hide();
                            mpActLineDistFld = mpLineDistAtPercentBox;
                        }
                        mpActLineDistFld->Enable();
                        mpActLineDistFld->Show();
                        mpLineDistAtPercentBox->
                            SetValue( mpLineDistAtPercentBox->Normalize(
                            currSPItem->GetPropLineSpace() ) );

                        if( currSPItem->GetPropLineSpace() == LINESPACE_115 )
                        {
//                            maLineSpacing.SelectItem(2);
                            //bValueSetFocus = sal_True;  //wj
                        }
                        else
                        {
//                            maLineSpacing.SetNoSelection();
//                                                 maLineSpacing.SelectItem(0);
                        }
                    }
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    {
                        if(mpActLineDistFld != mpLineDistAtMetricBox)
                        {
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->Hide();
                            mpActLineDistFld = mpLineDistAtMetricBox;
                        }
                        else
                        {
                            mpActLineDistFld = mpLineDistAtPercentBox;
                            mpActLineDistFld->Disable();
                            mpActLineDistFld->Hide();
                            mpActLineDistFld = mpLineDistAtMetricBox;
                        }
                        mpActLineDistFld->Enable();
                        mpActLineDistFld->Show();
//                        maLineSpacing.SetNoSelection();
//                                          maLineSpacing.SelectItem(0);

                        SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetInterLineSpace(), eUnit);
                        mpLineDist->SelectEntryPos( LLINESPACE_DURCH );

                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SVX_LINE_SPACE_FIX:
            {
                if(mpActLineDistFld != mpLineDistAtMetricBox)
                {
                    mpActLineDistFld->Disable();
                    mpActLineDistFld->Hide();
                    mpActLineDistFld = mpLineDistAtMetricBox;
                }
                else
                {
                    mpActLineDistFld = mpLineDistAtPercentBox;
                    mpActLineDistFld->Disable();
                    mpActLineDistFld->Hide();
                    mpActLineDistFld = mpLineDistAtMetricBox;
                }
                mpActLineDistFld->Enable();
                mpActLineDistFld->Show();
//                maLineSpacing.SetNoSelection();
//                            maLineSpacing.SelectItem(0);

                SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
                mpLineDist->SelectEntryPos( LLINESPACE_FIX );
            }
            break;

        case SVX_LINE_SPACE_MIN:
            {
                if(mpActLineDistFld != mpLineDistAtMetricBox)
                {
                    mpActLineDistFld->Disable();
                    mpActLineDistFld->Hide();
                    mpActLineDistFld = mpLineDistAtMetricBox;
                }
                else
                {
                    mpActLineDistFld = mpLineDistAtPercentBox;
                    mpActLineDistFld->Disable();
                    mpActLineDistFld->Hide();
                    mpActLineDistFld = mpLineDistAtMetricBox;
                }
                mpActLineDistFld->Enable();
                mpActLineDistFld->Show();
//                maLineSpacing.SetNoSelection();
//                            maLineSpacing.SelectItem(0);

                SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
                mpLineDist->SelectEntryPos( LLINESPACE_MIN );
            }
            break;
        default:
            break;
        }
    }
    else if( eState == SfxItemState::DISABLED )
    {
        mpLineDist->Disable();
        mpActLineDistFld->Enable(false);
        mpActLineDistFld->SetText( "" );
//        maLineSpacing.SetNoSelection();
//              maLineSpacing.SelectItem(0);

    }
    else
    {
        mpActLineDistFld->Enable(false);
        mpActLineDistFld->SetText( "" );
        mpLineDist->SetNoSelection();
//        maLineSpacing.SetNoSelection();
//              maLineSpacing.SelectItem(0);
    }

    mpLineDist->SaveValue();

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
//    maLineSpacing.Format();
//    maLineSpacing.StartSelection();
}

IMPL_LINK( ParaLineSpacingControl, LineSPDistHdl_Impl, ListBox*, pBox )
{
//    maLineSpacing.SetNoSelection();
//       maLineSpacing.SelectItem(0);
//    maLineSpacing.Format();
//    maLineSpacing.StartSelection();

    switch( pBox->GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            mpActLineDistFld->Enable(false);
            mpActLineDistFld->SetText( "" );
            break;

        case LLINESPACE_DURCH:
            mpLineDistAtPercentBox->Hide();
            mpActLineDistFld = mpLineDistAtMetricBox;
            mpLineDistAtMetricBox->SetMin(0);


            if ( mpLineDistAtMetricBox->GetText().isEmpty() )
                mpLineDistAtMetricBox->SetValue(
                    mpLineDistAtMetricBox->Normalize( 0 ) );
            mpLineDistAtPercentBox->Hide();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;

        case LLINESPACE_MIN:
            mpLineDistAtPercentBox->Hide();
            mpActLineDistFld = mpLineDistAtMetricBox;
            mpLineDistAtMetricBox->SetMin(0);

            if ( mpLineDistAtMetricBox->GetText().isEmpty() )
                mpLineDistAtMetricBox->SetValue(
                    mpLineDistAtMetricBox->Normalize( 0 ), FUNIT_TWIP );
            mpLineDistAtPercentBox->Hide();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;

        case LLINESPACE_PROP:
            mpLineDistAtMetricBox->Hide();
            mpActLineDistFld = mpLineDistAtPercentBox;

            if ( mpLineDistAtPercentBox->GetText().isEmpty() )
                mpLineDistAtPercentBox->SetValue(
                    mpLineDistAtPercentBox->Normalize( 100 ), FUNIT_TWIP );
            mpLineDistAtMetricBox->Hide();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
            break;
        case LLINESPACE_FIX:
        {
            mpLineDistAtPercentBox->Hide();
            mpActLineDistFld = mpLineDistAtMetricBox;
            sal_Int64 nTemp = mpLineDistAtMetricBox->GetValue();
            mpLineDistAtMetricBox->SetMin(mpLineDistAtMetricBox->Normalize(MIN_FIXED_DISTANCE), FUNIT_TWIP);

            if ( mpLineDistAtMetricBox->GetValue() != nTemp )
                SetMetricValue(*mpLineDistAtMetricBox, FIX_DIST_DEF, SFX_MAPUNIT_TWIP);

            mpLineDistAtPercentBox->Hide();
            mpActLineDistFld->Show();
            mpActLineDistFld->Enable();
        }
        break;
    }
    ExecuteLineSpace();
    return 0;
}

IMPL_LINK_NOARG( ParaLineSpacingControl, LineSPDistAtHdl_Impl )
{
    ExecuteLineSpace();
    return (0L);
}

void ParaLineSpacingControl::ExecuteLineSpace()
{
    mpLineDist->SaveValue();
//    maLineSpacing.SetNoSelection();

    SvxLineSpacingItem aSpacing(DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
    sal_uInt16 nPos = mpLineDist->GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            SetLineSpace( aSpacing, nPos );
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
            OSL_ENSURE(false, "error!!");
            break;
    }

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);
}

void ParaLineSpacingControl::SetLineSpace( SvxLineSpacingItem& rLineSpace,
                        int eSpace, long lValue )
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

IMPL_LINK(ParaLineSpacingControl, PredefinedValuesHandler, void *, pControl)
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

    return 0;
}

void ParaLineSpacingControl::ExecuteLineSpacing(sal_uInt16 nEntry)
{
    mpLineDist->SelectEntryPos(nEntry) ;
    mpLineDist->SaveValue();
    SvxLineSpacingItem aSpacing(DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);

    // special-case the 1.15 line spacing
    if (nEntry == LLINESPACE_115)
        SetLineSpace(aSpacing, LLINESPACE_PROP, mpLineDistAtPercentBox->Denormalize(115L));
    else
        SetLineSpace(aSpacing, nEntry);

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);

    // close when the user used the buttons
    EndPopupMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
