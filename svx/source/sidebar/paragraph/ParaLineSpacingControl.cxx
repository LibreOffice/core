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
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <svtools/unitconv.hxx>

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

#define DO_NOT_CUSTOM          0
#define USE_CUSTOM             1

namespace svx { namespace sidebar {


ParaLineSpacingControl::ParaLineSpacingControl(Window* pParent, svx::sidebar::ParaPropertyPanel& rPanel)
    : PopupControl( pParent,SVX_RES(RID_POPUPPANEL_PARAPAGE_LINESPACING))
    , mbUseLineSPCustom             (0)
    , mbLineSPDisable               (0)
    , mrParaPropertyPanel(rPanel)
    , mpBindings(NULL)
    , nMinFixDist(BEGIN_VALUE)
    , pActLineDistFld(&aLineDistAtPercentBox)
    , maLineSpacing(ValueSetWithTextControl::IMAGE_TEXT,this, SVX_RES( LINE_SPACING ) )
    , maCustomFT        ( this, SVX_RES( FT_CUSTOM ) )
    , maLSpacingFT      ( this, SVX_RES( FT_LINE_SPACING ) )
    , aLineDist( this, SVX_RES( LB_LINE_SPACING ))
    , maOfFT            ( this, SVX_RES( FT_OF ) )
    , aLineDistAtPercentBox   ( this, SVX_RES( ED_SBINDE_LINEDISTPERCENT ) )
    , aLineDistAtMetricBox    ( this, SVX_RES( ED_SBINDE_LINEDISTPOINT ) )
    , maSpacing1       (SVX_RES(IMG_SPACING1))
    , maSpacing115     (SVX_RES(IMG_SPACING115))
    , maSpacing15      (SVX_RES(IMG_SPACING15))
    , maSpacing2       (SVX_RES(IMG_SPACING2))
    , maSelSpacing1    (SVX_RES(IMG_SEL_SPACING1))
    , maSelSpacing115  (SVX_RES(IMG_SEL_SPACING115))
    , maSelSpacing15   (SVX_RES(IMG_SEL_SPACING15))
    , maSelSpacing2    (SVX_RES(IMG_SEL_SPACING2))
    , maImgCus         (SVX_RES(IMG_CUSTOM))
    , maImgCusGrey     (SVX_RES(IMG_CUSTOM_GRAY))
    , maStrCus         (SVX_RESSTR(STR_LCVALUE))
    , mpImg(NULL)
    , mpImgSel(NULL)
    , mpStr(NULL)
    , mpStrTip(NULL)
    , maLine(SVX_RESSTR(STR_LSPACING))
    , maOf(SVX_RESSTR(STR_LS_OF))
    , maValue( 0 )
    , maPos( 0 )
{
    initial();
    FreeResource();
    mpBindings = mrParaPropertyPanel.GetBindings();
//  m_eLNSpaceUnit = mrParaPropertyPanel.maLNSpaceControl.GetCoreMetric();
    m_eLNSpaceUnit = SFX_MAPUNIT_100TH_MM;
}
ParaLineSpacingControl::~ParaLineSpacingControl()
{
    delete[] mpImg;
    delete[] mpImgSel;
    delete[] mpStr;
    delete[] mpStrTip;
}

void ParaLineSpacingControl::initial()
{
    maLineSpacing.SetStyle( maLineSpacing.GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT  );

    maLineSpacing.SetControlBackground(
        GetSettings().GetStyleSettings().GetHighContrastMode()
        ? GetSettings().GetStyleSettings().GetMenuColor()
        : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
    maLineSpacing.SetColor(
        GetSettings().GetStyleSettings().GetHighContrastMode()
        ? GetSettings().GetStyleSettings().GetMenuColor()
        : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
    maLineSpacing.SetBackground(
        GetSettings().GetStyleSettings().GetHighContrastMode()
        ? GetSettings().GetStyleSettings().GetMenuColor()
        : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));

    mpImg = new Image[4];
    mpImg[0] = maSpacing1;
    mpImg[1] = maSpacing115;
    mpImg[2] = maSpacing15;
    mpImg[3] = maSpacing2;

    mpImgSel = new Image[4];
    mpImgSel[0] = maSelSpacing1;
    mpImgSel[1] = maSelSpacing115;
    mpImgSel[2] = maSelSpacing15;
    mpImgSel[3] = maSelSpacing2;

    mpStr = new OUString[4];
    mpStr[0] = SVX_RESSTR(STR_SPACING1);
    mpStr[1] = SVX_RESSTR(STR_SPACING115);
    mpStr[2] = SVX_RESSTR(STR_SPACING15);
    mpStr[3] = SVX_RESSTR(STR_SPACING2);

    mpStrTip = new OUString[4];
    mpStrTip[0] = SVX_RESSTR(STR_LS_SINGLE);
    mpStrTip[1] = SVX_RESSTR(STR_LS_115);
    mpStrTip[2] = SVX_RESSTR(STR_LS_15);
    mpStrTip[3] = SVX_RESSTR(STR_LS_DOUBLE);
    for (int i=0;i<4;i++)
        maLineSpacing.AddItem(mpImg[i], &mpImgSel[i],mpStr[i],&mpStrTip[i]);

    maLineSpacing.AddItem( maImgCus, 0, maStrCus, 0 );

    SetAllNoSel();
    Link aLink = LINK(this, ParaLineSpacingControl,VSSelHdl );
    maLineSpacing.SetSelectHdl(aLink);
    maLineSpacing.StartSelection();
    maLineSpacing.Show();

    aLink = LINK( this, ParaLineSpacingControl, LineSPDistHdl_Impl );
    aLineDist.SetSelectHdl(aLink);
    aLineDist.SelectEntryPos( LLINESPACE_1 ) ;
    aLink = LINK( this, ParaLineSpacingControl, LineSPDistAtHdl_Impl );
    aLineDistAtPercentBox.SetModifyHdl( aLink );
    aLineDistAtMetricBox.SetModifyHdl( aLink );
}


void ParaLineSpacingControl::PopupModeEndCallback()
{
    if( mbUseLineSPCustom )
    {
        //maLinePos = mpLineSPPage->maPos;
        //maLineValue = mpLineSPPage->maValue;

        SvtViewOptions aWinOpt( E_WINDOW, LSP_POS_GLOBAL_VALUE );
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq(1);
        aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("maLinePos") );
        aSeq[0].Value <<= ::rtl::OUString::number(maPos);
        aWinOpt.SetUserData( aSeq );

        SvtViewOptions aWinOpt2( E_WINDOW, LSP_LV_GLOBAL_VALUE );
        aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("maLineValue") );
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

        String aWinData( aTmp );
        maPos = (sal_uInt16)aWinData.ToInt32();
    }

    SvtViewOptions aWinOpt2( E_WINDOW, LSP_LV_GLOBAL_VALUE );
    if ( aWinOpt2.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt2.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        String aWinData( aTmp );
        maValue = (sal_uInt16)aWinData.ToInt32();
    }
    String sHelpText;
    switch(maPos)
    {
        case LLINESPACE_1:
            sHelpText += mpStrTip[0];
            break;
        case LLINESPACE_15:
            sHelpText += mpStrTip[2];
            break;
        case LLINESPACE_2:
            sHelpText += mpStrTip[3];
            break;
        case LLINESPACE_PROP:
            sHelpText +=maLine;
            sHelpText.Append(String("Proportion: ", 12, RTL_TEXTENCODING_ASCII_US));
            sHelpText += maOf;
            sHelpText.Append( OUString::number( maValue ));
            break;
        case LLINESPACE_MIN:
            sHelpText += maLine;
            sHelpText.Append(String("At Least: ", 10, RTL_TEXTENCODING_ASCII_US));
            sHelpText += maOf;
            sHelpText.Append( OUString::number( maValue ));
            break;
        case LLINESPACE_DURCH:
            sHelpText += maLine;
            sHelpText.Append(String("Leading: ", 9, RTL_TEXTENCODING_ASCII_US));
            sHelpText += maOf;
            sHelpText.Append( OUString::number( maValue ));
            break;
        case LLINESPACE_FIX:
            sHelpText += maLine;
            sHelpText.Append(String("Fixed: ", 7, RTL_TEXTENCODING_ASCII_US));
            sHelpText += maOf;
            sHelpText.Append( OUString::number( maValue ));
            break;
    }
    if( !aWinOpt.Exists() && !aWinOpt2.Exists() )
        mbLineSPDisable = sal_True;
    else
        mbLineSPDisable = sal_False;

    if( mbLineSPDisable )
        maLineSpacing.ReplaceItemImages(5, maImgCusGrey,0);
    else
    {
        maLineSpacing.ReplaceItemImages(5, maImgCus,0);
        maLineSpacing.SetItemText(5,sHelpText);
    }

    SfxItemState eState = currSPState;

    SetFieldUnit( aLineDistAtMetricBox, currMetricUnit );

//  mpLineSPPage->SetAllNoSel();
    aLineDist.Enable();
    pActLineDistFld->Enable();
    pActLineDistFld->SetText( String() );
    //bool bValueSetFocus = sal_False;        //wj

    if( eState >= SFX_ITEM_AVAILABLE )
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
                        aLineDist.SelectEntryPos( LLINESPACE_1 );
                        pActLineDistFld->Disable();
                        pActLineDistFld->SetText( String() );
                        mbUseLineSPCustom = DO_NOT_CUSTOM;
                        if ( LINESPACE_1 == currSPItem->GetPropLineSpace() )
                        {
                            maLineSpacing.SelectItem(1);
                            //bValueSetFocus = sal_True;  //wj
                        }
                    }
                    break;

                case SVX_INTER_LINE_SPACE_PROP:
                    {
                        if ( LINESPACE_1 == currSPItem->GetPropLineSpace() )
                        {
                            aLineDist.SelectEntryPos( LLINESPACE_1 );
                            pActLineDistFld->Disable();
                            pActLineDistFld->SetText( String() );
                            mbUseLineSPCustom = DO_NOT_CUSTOM;
                            maLineSpacing.SelectItem(1);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }
                        if ( LINESPACE_15 == currSPItem->GetPropLineSpace() )
                        {
                            aLineDist.SelectEntryPos( LLINESPACE_15 );
                            pActLineDistFld->Disable();
                            pActLineDistFld->SetText( String() );

                            mbUseLineSPCustom = DO_NOT_CUSTOM;
                            maLineSpacing.SelectItem(3);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }
                        if ( LINESPACE_2 == currSPItem->GetPropLineSpace() )
                        {
                            aLineDist.SelectEntryPos( LLINESPACE_2 );
                            pActLineDistFld->Disable();
                            pActLineDistFld->SetText( String() );

                            mbUseLineSPCustom = DO_NOT_CUSTOM;
                            maLineSpacing.SelectItem(4);
                            //bValueSetFocus = sal_True;  //wj
                            break;
                        }

                        aLineDist.SelectEntryPos( LLINESPACE_PROP );
                        if(pActLineDistFld != &(aLineDistAtPercentBox))
                        {
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = &(aLineDistAtPercentBox);
                        }
                        else
                        {
                            pActLineDistFld = &(aLineDistAtMetricBox);
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = &(aLineDistAtPercentBox);
                        }
                        pActLineDistFld->Enable();
                        pActLineDistFld->Show();
                        aLineDistAtPercentBox.
                            SetValue( aLineDistAtPercentBox.Normalize(
                            currSPItem->GetPropLineSpace() ) );

                        if( currSPItem->GetPropLineSpace() == LINESPACE_115 )
                        {
                            mbUseLineSPCustom = DO_NOT_CUSTOM;
                            maLineSpacing.SelectItem(2);
                            //bValueSetFocus = sal_True;  //wj
                        }
                        else
                        {
                            mbUseLineSPCustom = USE_CUSTOM;
                            maLineSpacing.SetNoSelection();
                                                 maLineSpacing.SelectItem(0);
                        }
                    }
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    {
                        if(pActLineDistFld != &(aLineDistAtMetricBox))
                        {
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = &(aLineDistAtMetricBox);
                        }
                        else
                        {
                            pActLineDistFld = &(aLineDistAtPercentBox);
                            pActLineDistFld->Disable();
                            pActLineDistFld->Hide();
                            pActLineDistFld = &(aLineDistAtMetricBox);
                        }
                        pActLineDistFld->Enable();
                        pActLineDistFld->Show();
                        maLineSpacing.SetNoSelection();
                                          maLineSpacing.SelectItem(0);

                        SetMetricValue( aLineDistAtMetricBox,
                            currSPItem->GetInterLineSpace(), eUnit );
                        aLineDist.SelectEntryPos( LLINESPACE_DURCH );

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
                if(pActLineDistFld != &(aLineDistAtMetricBox))
                {
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = &(aLineDistAtMetricBox);
                }
                else
                {
                    pActLineDistFld = &(aLineDistAtPercentBox);
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = &(aLineDistAtMetricBox);
                }
                pActLineDistFld->Enable();
                pActLineDistFld->Show();
                maLineSpacing.SetNoSelection();
                            maLineSpacing.SelectItem(0);

                SetMetricValue(aLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
                aLineDist.SelectEntryPos( LLINESPACE_FIX );

                mbUseLineSPCustom = USE_CUSTOM;
            }
            break;

        case SVX_LINE_SPACE_MIN:
            {
                if(pActLineDistFld != &(aLineDistAtMetricBox))
                {
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = &(aLineDistAtMetricBox);
                }
                else
                {
                    pActLineDistFld = &(aLineDistAtPercentBox);
                    pActLineDistFld->Disable();
                    pActLineDistFld->Hide();
                    pActLineDistFld = &(aLineDistAtMetricBox);
                }
                pActLineDistFld->Enable();
                pActLineDistFld->Show();
                maLineSpacing.SetNoSelection();
                            maLineSpacing.SelectItem(0);

                SetMetricValue(aLineDistAtMetricBox, currSPItem->GetLineHeight(), eUnit);
                aLineDist.SelectEntryPos( LLINESPACE_MIN );
                mbUseLineSPCustom = USE_CUSTOM;
            }
            break;
        default:
            break;
        }
    }
    else if( eState == SFX_ITEM_DISABLED )
    {
        aLineDist.Disable();
        pActLineDistFld->Enable(sal_False);
        pActLineDistFld->SetText( String() );
        maLineSpacing.SetNoSelection();
              maLineSpacing.SelectItem(0);

        mbUseLineSPCustom = DO_NOT_CUSTOM;
    }
    else
    {
        pActLineDistFld->Enable(sal_False);
        pActLineDistFld->SetText( String() );
        aLineDist.SetNoSelection();
        maLineSpacing.SetNoSelection();
              maLineSpacing.SelectItem(0);
        mbUseLineSPCustom = DO_NOT_CUSTOM;
    }

    aLineDist.SaveValue();

    const sal_uInt16 uCount = aLineDist.GetEntryCount();
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
                aLineDist.RemoveEntry(LLINESPACE_FIX);
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
                aLineDist.InsertEntry(OUString("Fixed"), LLINESPACE_FIX);
            }
        }
    }
    maLineSpacing.Format();
    maLineSpacing.StartSelection();
}

void ParaLineSpacingControl::SetAllNoSel()
{
    maLineSpacing.SelectItem(1);
    maLineSpacing.SetNoSelection();
}

IMPL_LINK( ParaLineSpacingControl, LineSPDistHdl_Impl, ListBox*, pBox )
{
    maLineSpacing.SetNoSelection();
       maLineSpacing.SelectItem(0);
    maLineSpacing.Format();
    maLineSpacing.StartSelection();

    switch( pBox->GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            pActLineDistFld->Enable(sal_False);
            pActLineDistFld->SetText( String() );
            break;

        case LLINESPACE_DURCH:
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);


            if ( aLineDistAtMetricBox.GetText().isEmpty() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 0 ) );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            break;

        case LLINESPACE_MIN:
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);

            if ( aLineDistAtMetricBox.GetText().isEmpty() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 0 ), FUNIT_TWIP );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            break;

        case LLINESPACE_PROP:
            aLineDistAtMetricBox.Hide();
            pActLineDistFld = &aLineDistAtPercentBox;

            if ( aLineDistAtPercentBox.GetText().isEmpty() )
                aLineDistAtPercentBox.SetValue(
                    aLineDistAtPercentBox.Normalize( 100 ), FUNIT_TWIP );
            aLineDistAtMetricBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            break;
        case LLINESPACE_FIX:
        {
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            sal_Int64 nTemp = aLineDistAtMetricBox.GetValue();
            aLineDistAtMetricBox.SetMin(aLineDistAtMetricBox.Normalize(nMinFixDist), FUNIT_TWIP);

            if ( aLineDistAtMetricBox.GetValue() != nTemp )
                SetMetricValue( aLineDistAtMetricBox,
                                    FIX_DIST_DEF, SFX_MAPUNIT_TWIP );
            aLineDistAtPercentBox.Hide();
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
    aLineDist.SaveValue();
    maLineSpacing.SetNoSelection();

    SvxLineSpacingItem aSpacing(_DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
    sal_uInt16 nPos = aLineDist.GetSelectEntryPos();

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
                    aLineDistAtPercentBox.Denormalize(
                    (long)aLineDistAtPercentBox.GetValue() ) );
                maPos = nPos;
                maValue =aLineDistAtPercentBox.GetValue();
            }
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            {
                SetLineSpace( aSpacing, nPos,
                    GetCoreValue( aLineDistAtMetricBox, m_eLNSpaceUnit ) );
                maPos = nPos;
                maValue = GetCoreValue( aLineDistAtMetricBox, m_eLNSpaceUnit );
            }
        break;

        default:
            OSL_ENSURE(false, "error!!");
            break;
    }

    mpBindings->GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SFX_CALLMODE_RECORD, &aSpacing, 0L);

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

IMPL_LINK(ParaLineSpacingControl, VSSelHdl, void *, pControl)
{
    maLineSpacing.SetNoSelection();
    bool bClosePop = true;
    if(pControl == &maLineSpacing)
    {
        sal_uInt16 iPos = maLineSpacing.GetSelectItemId();
        switch ( iPos )
        {
            case 1:
                ExecuteLineSpacing( 0, 0 );
                break;
            case 2:
                ExecuteLineSpacing( 0, 3 );
                break;
            case 3:
                ExecuteLineSpacing( 0, 1 );
                break;
            case 4:
                ExecuteLineSpacing( 0, 2 );
                break;
            case 5:
                {
                    if(!(mbLineSPDisable))
                    {
                        //maPos = mrParaPropertyPanel.maLinePos;
                        aLineDist.SelectEntryPos( maPos ) ;
                        aLineDist.SaveValue();
                        //maValue = mrParaPropertyPanel.maLineValue;

                        SvxLineSpacingItem aSpacing(_DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
                        switch(maPos)
                        {
                        case LLINESPACE_1:
                        case LLINESPACE_15:
                        case LLINESPACE_2:
                            SetLineSpace( aSpacing, maPos );
                            break;

                        case LLINESPACE_PROP:
                            SetLineSpace( aSpacing, maPos,
                                aLineDistAtPercentBox.Denormalize( (long)maValue ) );
                            break;

                        case LLINESPACE_MIN:
                        case LLINESPACE_DURCH:
                        case LLINESPACE_FIX:
                            SetLineSpace( aSpacing, maPos, (long)maValue );
                            break;
                        }

                        mpBindings->GetDispatcher()->Execute(
                            SID_ATTR_PARA_LINESPACE, SFX_CALLMODE_RECORD, &aSpacing, 0L);

                        ExecuteLineSpacing( USE_CUSTOM, 0 );
                    }
                    else
                        bClosePop = sal_False;
                }
                break;
        }
    }
    if(bClosePop)
        mrParaPropertyPanel.EndSpacingPopupMode();
    return 0;
}

void ParaLineSpacingControl::ExecuteLineSpacing( sal_uInt16 aIsCustom, sal_uInt16 aEntry )
{
    if( !aIsCustom )
    {
        aLineDist.SelectEntryPos( aEntry ) ;
        aLineDist.SaveValue();
        SvxLineSpacingItem aSpacing(_DEFAULT_LINE_SPACING, SID_ATTR_PARA_LINESPACE);
        sal_uInt16 nPos = aEntry;
        if( aEntry == LLINESPACE_PROP )
            SetLineSpace( aSpacing, nPos, aLineDistAtPercentBox.Denormalize( (long)115 ) );
        else
            SetLineSpace( aSpacing, nPos );

        mpBindings->GetDispatcher()->Execute(
            SID_ATTR_PARA_LINESPACE, SFX_CALLMODE_RECORD, &aSpacing, 0L);
    }

    if( !aIsCustom )
    {
        mbUseLineSPCustom = DO_NOT_CUSTOM;
        mrParaPropertyPanel.EndSpacingPopupMode();
    }
    maLineSpacing.SetNoSelection();
}

}} // end of namespace sidebar

