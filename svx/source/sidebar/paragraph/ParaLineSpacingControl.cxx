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

#define _DEFAULT_LINE_SPACING  200
#define FIX_DIST_DEF           283
#define LINESPACE_1            100
#define LINESPACE_15           150
#define LINESPACE_2            200
#define LINESPACE_115          115

#define LLINESPACE_1          0
#define LLINESPACE_15         1
#define LLINESPACE_2          2
#define LLINESPACE_PROP       3
#define LLINESPACE_MIN        4
#define LLINESPACE_DURCH      5
#define LLINESPACE_FIX        6

#define DO_NOT_CUSTOM          false
#define USE_CUSTOM             true

#define LSP_POS_GLOBAL_VALUE   "Line_Spacing_Pos"
#define LSP_LV_GLOBAL_VALUE    "Line_Spacing_Lv"
#define BEGIN_VALUE            28

using namespace svx;
using namespace svx::sidebar;

ParaLineSpacingControl::ParaLineSpacingControl(sal_uInt16 nId)
    : SfxPopupWindow(nId, "ParaLineSpacingControl", "svx/ui/paralinespacingcontrol.ui")
    , mbUseLineSPCustom(false)
    , mbLineSPDisable(false)
    , nMinFixDist(BEGIN_VALUE)
//    , maLineSpacing(ValueSetWithTextControl::IMAGE_TEXT,this, SVX_RES( LINE_SPACING ) )
    , maValue( 0 )
    , maPos( 0 )
{
    mpSpacing1Button = get<PushButton>("spacing_1");
    mpSpacing115Button = get<PushButton>("spacing_115");
    mpSpacing15Button = get<PushButton>("spacing_15");
    mpSpacing2Button = get<PushButton>("spacing_2");
    mpSpacingLastButton = get<PushButton>("spacing_last");

    mpLineDist = get<ListBox>("line_dist");
    mpLineDistAtPercentBox = get<MetricField>("percent_box");
    mpLineDistAtMetricBox = get<MetricField>("metric_box");

    pActLineDistFld = mpLineDistAtPercentBox;

    initial();
    m_eLNSpaceUnit = SFX_MAPUNIT_100TH_MM;
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
    mpSpacingLastButton->SetClickHdl(aLink);

    aLink = LINK( this, ParaLineSpacingControl, LineSPDistHdl_Impl );
    mpLineDist->SetSelectHdl(aLink);
    mpLineDist->SelectEntryPos( LLINESPACE_1 ) ;

    aLink = LINK( this, ParaLineSpacingControl, LineSPDistAtHdl_Impl );
    mpLineDistAtPercentBox->SetModifyHdl( aLink );
    mpLineDistAtMetricBox->SetModifyHdl( aLink );
}

void ParaLineSpacingControl::PopupModeEndCallback()
{
    if( mbUseLineSPCustom )
    {
        //maLinePos = mpLineSPPage->maPos;
        //maLineValue = mpLineSPPage->maValue;

        SvtViewOptions aWinOpt( E_WINDOW, LSP_POS_GLOBAL_VALUE );
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq(1);
        aSeq[0].Name = "maLinePos";
        aSeq[0].Value <<= ::rtl::OUString::number(maPos);
        aWinOpt.SetUserData( aSeq );

        SvtViewOptions aWinOpt2( E_WINDOW, LSP_LV_GLOBAL_VALUE );
        aSeq[0].Name = "maLineValue";
        aSeq[0].Value <<= ::rtl::OUString::number(maValue);
        aWinOpt2.SetUserData( aSeq );
    }
}

void ParaLineSpacingControl::Rearrange(SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem,const ::sfx2::sidebar::EnumContext currentContext)
{
    SvtViewOptions aWinOpt( E_WINDOW, LSP_POS_GLOBAL_VALUE );
    if ( aWinOpt.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData( aTmp );
        maPos = (sal_uInt16)aWinData.toInt32();
    }

    SvtViewOptions aWinOpt2( E_WINDOW, LSP_LV_GLOBAL_VALUE );
    if ( aWinOpt2.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt2.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData( aTmp );
        maValue = (sal_uInt16)aWinData.toInt32();
    }
//    OUString sHelpText;
//    switch(maPos)
//    {
//        case LLINESPACE_1:
//            sHelpText += mpStrTip[0];
//            break;
//        case LLINESPACE_15:
//            sHelpText += mpStrTip[2];
//            break;
//        case LLINESPACE_2:
//            sHelpText += mpStrTip[3];
//            break;
//        case LLINESPACE_PROP:
//            sHelpText +=maLine;
//            sHelpText += "Proportion: ";
//            sHelpText += maOf;
//            sHelpText += OUString::number( maValue );
//            break;
//        case LLINESPACE_MIN:
//            sHelpText += maLine;
//            sHelpText += "At Least: ";
//            sHelpText += maOf;
//            sHelpText += OUString::number( maValue );
//            break;
//        case LLINESPACE_DURCH:
//            sHelpText += maLine;
//            sHelpText += "Leading: ";
//            sHelpText += maOf;
//            sHelpText += OUString::number( maValue );
//            break;
//        case LLINESPACE_FIX:
//            sHelpText += maLine;
//            sHelpText += "Fixed: ";
//            sHelpText += maOf;
//            sHelpText += OUString::number( maValue );
//            break;
//    }
    if( !aWinOpt.Exists() && !aWinOpt2.Exists() )
        mbLineSPDisable = true;
    else
        mbLineSPDisable = false;

//    if( mbLineSPDisable )
//        maLineSpacing.ReplaceItemImages(5, maImgCusGrey,0);
//    else
//    {
//        maLineSpacing.ReplaceItemImages(5, maImgCus,0);
//        maLineSpacing.SetItemText(5,sHelpText);
//    }

    SfxItemState eState = currSPState;

    SetFieldUnit(*mpLineDistAtMetricBox, currMetricUnit);

    mpLineDist->Enable();
    pActLineDistFld->Enable();
    pActLineDistFld->SetText( "" );
    //bool bValueSetFocus = sal_False;        //wj

    if( eState >= SfxItemState::DEFAULT )
    {
    //  SfxMapUnit eUnit = maLNSpaceControl.GetCoreMetric();
        SfxMapUnit eUnit = SFX_MAPUNIT_100TH_MM;
        m_eLNSpaceUnit = eUnit;

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
                        pActLineDistFld->Disable();
                        pActLineDistFld->SetText( "" );
                        mbUseLineSPCustom = DO_NOT_CUSTOM;
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
                            pActLineDistFld->Disable();
                            pActLineDistFld->SetText( "" );
                            mbUseLineSPCustom = DO_NOT_CUSTOM;
//                            maLineSpacing.SelectItem(1);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }
                        if ( LINESPACE_15 == currSPItem->GetPropLineSpace() )
                        {
                            mpLineDist->SelectEntryPos( LLINESPACE_15 );
                            pActLineDistFld->Disable();
                            pActLineDistFld->SetText( "" );

                            mbUseLineSPCustom = DO_NOT_CUSTOM;
//                            maLineSpacing.SelectItem(3);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }
                        if ( LINESPACE_2 == currSPItem->GetPropLineSpace() )
                        {
                            mpLineDist->SelectEntryPos( LLINESPACE_2 );
                            pActLineDistFld->Disable();
                            pActLineDistFld->SetText( "" );

                            mbUseLineSPCustom = DO_NOT_CUSTOM;
//                            maLineSpacing.SelectItem(4);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }

                        mpLineDist->SelectEntryPos( LLINESPACE_PROP );
                        if(pActLineDistFld != mpLineDistAtPercentBox)
                        {
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = mpLineDistAtPercentBox;
                        }
                        else
                        {
                            pActLineDistFld = mpLineDistAtMetricBox;
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = mpLineDistAtPercentBox;
                        }
                        pActLineDistFld->Enable();
                        pActLineDistFld->Show();
                        mpLineDistAtPercentBox->
                            SetValue( mpLineDistAtPercentBox->Normalize(
                            currSPItem->GetPropLineSpace() ) );

                        if( currSPItem->GetPropLineSpace() == LINESPACE_115 )
                        {
                            mbUseLineSPCustom = DO_NOT_CUSTOM;
//                            maLineSpacing.SelectItem(2);
                            //bValueSetFocus = sal_True;  //wj
                        }
                        else
                        {
                            mbUseLineSPCustom = USE_CUSTOM;
//                            maLineSpacing.SetNoSelection();
//                                                 maLineSpacing.SelectItem(0);
                        }
                    }
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    {
                        if(pActLineDistFld != mpLineDistAtMetricBox)
                        {
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = mpLineDistAtMetricBox;
                        }
                        else
                        {
                            pActLineDistFld = mpLineDistAtPercentBox;
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = mpLineDistAtMetricBox;
                        }
                        pActLineDistFld->Enable();
                        pActLineDistFld->Show();
//                        maLineSpacing.SetNoSelection();
//                                          maLineSpacing.SelectItem(0);

                        SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetInterLineSpace(), eUnit);
                        mpLineDist->SelectEntryPos( LLINESPACE_DURCH );

                        mbUseLineSPCustom = USE_CUSTOM;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SVX_LINE_SPACE_FIX:
            {
                if(pActLineDistFld != mpLineDistAtMetricBox)
                {
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = mpLineDistAtMetricBox;
                }
                else
                {
                    pActLineDistFld = mpLineDistAtPercentBox;
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = mpLineDistAtMetricBox;
                }
                pActLineDistFld->Enable();
                pActLineDistFld->Show();
//                maLineSpacing.SetNoSelection();
//                            maLineSpacing.SelectItem(0);

                SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
                mpLineDist->SelectEntryPos( LLINESPACE_FIX );
                mbUseLineSPCustom = USE_CUSTOM;
            }
            break;

        case SVX_LINE_SPACE_MIN:
            {
                if(pActLineDistFld != mpLineDistAtMetricBox)
                {
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = mpLineDistAtMetricBox;
                }
                else
                {
                    pActLineDistFld = mpLineDistAtPercentBox;
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = mpLineDistAtMetricBox;
                }
                pActLineDistFld->Enable();
                pActLineDistFld->Show();
//                maLineSpacing.SetNoSelection();
//                            maLineSpacing.SelectItem(0);

                SetMetricValue(*mpLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
                mpLineDist->SelectEntryPos( LLINESPACE_MIN );
                mbUseLineSPCustom = USE_CUSTOM;
            }
            break;
        default:
            break;
        }
    }
    else if( eState == SfxItemState::DISABLED )
    {
        mpLineDist->Disable();
        pActLineDistFld->Enable(false);
        pActLineDistFld->SetText( "" );
//        maLineSpacing.SetNoSelection();
//              maLineSpacing.SelectItem(0);

        mbUseLineSPCustom = DO_NOT_CUSTOM;
    }
    else
    {
        pActLineDistFld->Enable(false);
        pActLineDistFld->SetText( "" );
        mpLineDist->SetNoSelection();
//        maLineSpacing.SetNoSelection();
//              maLineSpacing.SelectItem(0);
        mbUseLineSPCustom = DO_NOT_CUSTOM;
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
            pActLineDistFld->Enable(false);
            pActLineDistFld->SetText( "" );
            break;

        case LLINESPACE_DURCH:
            mpLineDistAtPercentBox->Hide();
            pActLineDistFld = mpLineDistAtMetricBox;
            mpLineDistAtMetricBox->SetMin(0);


            if ( mpLineDistAtMetricBox->GetText().isEmpty() )
                mpLineDistAtMetricBox->SetValue(
                    mpLineDistAtMetricBox->Normalize( 0 ) );
            mpLineDistAtPercentBox->Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            break;

        case LLINESPACE_MIN:
            mpLineDistAtPercentBox->Hide();
            pActLineDistFld = mpLineDistAtMetricBox;
            mpLineDistAtMetricBox->SetMin(0);

            if ( mpLineDistAtMetricBox->GetText().isEmpty() )
                mpLineDistAtMetricBox->SetValue(
                    mpLineDistAtMetricBox->Normalize( 0 ), FUNIT_TWIP );
            mpLineDistAtPercentBox->Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            break;

        case LLINESPACE_PROP:
            mpLineDistAtMetricBox->Hide();
            pActLineDistFld = mpLineDistAtPercentBox;

            if ( mpLineDistAtPercentBox->GetText().isEmpty() )
                mpLineDistAtPercentBox->SetValue(
                    mpLineDistAtPercentBox->Normalize( 100 ), FUNIT_TWIP );
            mpLineDistAtMetricBox->Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            break;
        case LLINESPACE_FIX:
        {
            mpLineDistAtPercentBox->Hide();
            pActLineDistFld = mpLineDistAtMetricBox;
            sal_Int64 nTemp = mpLineDistAtMetricBox->GetValue();
            mpLineDistAtMetricBox->SetMin(mpLineDistAtMetricBox->Normalize(nMinFixDist), FUNIT_TWIP);

            if ( mpLineDistAtMetricBox->GetValue() != nTemp )
                SetMetricValue(*mpLineDistAtMetricBox, FIX_DIST_DEF, SFX_MAPUNIT_TWIP);

            mpLineDistAtPercentBox->Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
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

    SvxLineSpacingItem aSpacing(_DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
    sal_uInt16 nPos = mpLineDist->GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            {
                SetLineSpace( aSpacing, nPos );
                maPos = nPos;
            }
            break;

        case LLINESPACE_PROP:
            {
                SetLineSpace( aSpacing, nPos,
                    mpLineDistAtPercentBox->Denormalize(
                    (long)mpLineDistAtPercentBox->GetValue() ) );
                maPos = nPos;
                maValue =mpLineDistAtPercentBox->GetValue();
            }
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            {
                SetLineSpace(aSpacing, nPos, GetCoreValue(*mpLineDistAtMetricBox, m_eLNSpaceUnit));
                maPos = nPos;
                maValue = GetCoreValue(*mpLineDistAtMetricBox, m_eLNSpaceUnit);
            }
        break;

        default:
            OSL_ENSURE(false, "error!!");
            break;
    }

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);

    mbUseLineSPCustom = USE_CUSTOM;
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
        ExecuteLineSpacing(false, LLINESPACE_1);
    }
    else if (pControl == mpSpacing115Button)
    {
        ExecuteLineSpacing(false, LLINESPACE_PROP);
    }
    else if (pControl == mpSpacing15Button)
    {
        ExecuteLineSpacing(false, LLINESPACE_15);
    }
    else if (pControl == mpSpacing2Button)
    {
        ExecuteLineSpacing( false, 2 );
    }
    else
    {
        if(!(mbLineSPDisable))
        {
            mpLineDist->SelectEntryPos( maPos ) ;
            mpLineDist->SaveValue();

            SvxLineSpacingItem aSpacing(_DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
            switch(maPos)
            {
                case LLINESPACE_1:
                case LLINESPACE_15:
                case LLINESPACE_2:
                    SetLineSpace(aSpacing, maPos);
                    break;

                case LLINESPACE_PROP:
                    SetLineSpace(aSpacing, maPos, mpLineDistAtPercentBox->Denormalize((long)maValue));
                    break;

                case LLINESPACE_MIN:
                case LLINESPACE_DURCH:
                case LLINESPACE_FIX:
                    SetLineSpace(aSpacing, maPos, (long)maValue);
                    break;
            }

            SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
                    SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);

            ExecuteLineSpacing(USE_CUSTOM, 0);
        }
    }

    return 0;
}

void ParaLineSpacingControl::ExecuteLineSpacing( bool aIsCustom, sal_uInt16 aEntry )
{
    if( !aIsCustom )
    {
        mpLineDist->SelectEntryPos( aEntry ) ;
        mpLineDist->SaveValue();
        SvxLineSpacingItem aSpacing(_DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
        sal_uInt16 nPos = aEntry;
        if( aEntry == LLINESPACE_PROP )
            SetLineSpace( aSpacing, nPos, mpLineDistAtPercentBox->Denormalize( (long)115 ) );
        else
            SetLineSpace( aSpacing, nPos );

        SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SfxCallMode::RECORD, &aSpacing, 0L);
    }

    if( !aIsCustom )
    {
        mbUseLineSPCustom = DO_NOT_CUSTOM;
    }
//    maLineSpacing.SetNoSelection();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
