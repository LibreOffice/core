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

#include "tp_Scale.hxx"
#include "tp_Scale.hrc"

#include "ResId.hxx"
#include "Strings.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "AxisHelper.hxx"

#include <svx/svxids.hrc>
#include <rtl/math.hxx>
// header for class SvxDoubleItem
#include <svx/chrtitem.hxx>
// header for class SfxBoolItem
#include <svl/eitem.hxx>
// header for SfxInt32Item
#include <svl/intitem.hxx>

// header for class WarningBox
#include <vcl/msgbox.hxx>

// header for class SvNumberformat
#include <svl/zformat.hxx>

#include <svtools/controldims.hrc>

#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

namespace chart
{

namespace
{

void lcl_placeControlAtX( Control& rControl, long nNewXPos )
{
    Point aPos( rControl.GetPosPixel() );
    aPos.X() = nNewXPos;
    rControl.SetPosPixel(aPos);
}

void lcl_placeControlAtY( Control& rControl, long nNewYPos )
{
    Point aPos( rControl.GetPosPixel() );
    aPos.Y() = nNewYPos;
    rControl.SetPosPixel(aPos);
}

void lcl_shiftControls( Control& rEdit, Control& rAuto, long nNewXPos )
{
    Point aPos( rEdit.GetPosPixel() );
    long nShift = nNewXPos - aPos.X();
    aPos.X() = nNewXPos;
    rEdit.SetPosPixel(aPos);

    aPos = rAuto.GetPosPixel();
    aPos.X() += nShift;
    rAuto.SetPosPixel(aPos);
}

long lcl_getLabelDistance( Control& rControl )
{
    return rControl.LogicToPixel( Size(RSC_SP_CTRL_DESC_X, 0), MapMode(MAP_APPFONT) ).Width();
}

void lcl_setValue( FormattedField& rFmtField, double fValue )
{
    rFmtField.SetValue( fValue );
    rFmtField.SetDefaultValue( fValue );
}

}

ScaleTabPage::ScaleTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_SCALE), rInAttrs),

    aFlScale(this, SchResId(FL_SCALE)),

    aCbxReverse(this, SchResId(CBX_REVERSE)),
    aCbxLogarithm(this, SchResId(CBX_LOGARITHM)),

    m_aTxt_AxisType(this, SchResId (TXT_AXIS_TYPE)),
    m_aLB_AxisType(this, SchResId(LB_AXIS_TYPE)),

    aTxtMin (this, SchResId (TXT_MIN)),
    aFmtFldMin(this, SchResId(EDT_MIN)),
    aCbxAutoMin(this, SchResId(CBX_AUTO_MIN)),

    aTxtMax(this, SchResId (TXT_MAX)),
    aFmtFldMax(this, SchResId(EDT_MAX)),
    aCbxAutoMax(this, SchResId(CBX_AUTO_MAX)),

    m_aTxt_TimeResolution(this, SchResId (TXT_TIME_RESOLUTION)),
    m_aLB_TimeResolution(this, SchResId(LB_TIME_RESOLUTION)),
    m_aCbx_AutoTimeResolution(this, SchResId(CBX_AUTO_TIME_RESOLUTION)),

    aTxtMain (this, SchResId (TXT_STEP_MAIN)),
    aFmtFldStepMain(this, SchResId(EDT_STEP_MAIN)),
    m_aMt_MainDateStep(this, SchResId(MT_MAIN_DATE_STEP)),
    m_aLB_MainTimeUnit(this, SchResId(LB_MAIN_TIME_UNIT)),
    aCbxAutoStepMain(this, SchResId(CBX_AUTO_STEP_MAIN)),

    aTxtHelpCount (this, SchResId (TXT_STEP_HELP_COUNT)),
    aTxtHelp (this, SchResId (TXT_STEP_HELP)),
    aMtStepHelp (this, SchResId (MT_STEPHELP)),
    m_aLB_HelpTimeUnit(this, SchResId(LB_HELP_TIME_UNIT)),
    aCbxAutoStepHelp(this, SchResId(CBX_AUTO_STEP_HELP)),

    aTxtOrigin (this, SchResId (TXT_ORIGIN)),
    aFmtFldOrigin(this, SchResId(EDT_ORIGIN)),
    aCbxAutoOrigin(this, SchResId(CBX_AUTO_ORIGIN)),

    fMin(0.0),
    fMax(0.0),
    fStepMain(0.0),
    nStepHelp(0),
    fOrigin(0.0),
    m_nTimeResolution(1),
    m_nMainTimeUnit(1),
    m_nHelpTimeUnit(1),
    m_nAxisType(chart2::AxisType::REALNUMBER),
    m_bAllowDateAxis(false),
    pNumFormatter(NULL),
    m_bShowAxisOrigin(false)
{
    FreeResource();
    SetExchangeSupport();

    aCbxAutoMin.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoMax.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoStepMain.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoStepHelp.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoOrigin.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    m_aCbx_AutoTimeResolution.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));

    m_aLB_AxisType.SetDropDownLineCount(3);
    m_aLB_AxisType.SetSelectHdl(LINK(this, ScaleTabPage, SelectAxisTypeHdl));

    m_aLB_TimeResolution.SetDropDownLineCount(3);
    m_aLB_MainTimeUnit.SetDropDownLineCount(3);
    m_aLB_HelpTimeUnit.SetDropDownLineCount(3);

    aFmtFldMin.SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));
    aFmtFldMax.SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));
    aFmtFldStepMain.SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));
    aFmtFldOrigin.SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));

    HideAllControls();
}

IMPL_LINK( ScaleTabPage, FmtFieldModifiedHdl, FormattedField*, pFmtFied )
{
    if( pFmtFied )
        pFmtFied->SetDefaultValue( pFmtFied->GetValue() );
    return 0;
}

void ScaleTabPage::StateChanged( StateChangedType nType )
{
    TabPage::StateChanged( nType );

    if( nType == STATE_CHANGE_INITSHOW )
        AdjustControlPositions();
}

void ScaleTabPage::AdjustControlPositions()
{
    //optimize position of the controls
    long nLabelWidth = ::std::max( aTxtMin.CalcMinimumSize().Width(), aTxtMax.CalcMinimumSize().Width() );
    nLabelWidth = ::std::max( aTxtMain.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth = ::std::max( aTxtHelp.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth = ::std::max( aTxtHelpCount.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth = ::std::max( aTxtOrigin.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth = ::std::max( m_aTxt_TimeResolution.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth = ::std::max( m_aTxt_AxisType.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth+=1;

    long nLabelDistance = lcl_getLabelDistance(aTxtMin);
    long nNewXPos = aTxtMin.GetPosPixel().X() + nLabelWidth + nLabelDistance;

    //ensure that the auto checkboxes are wide enough and have correct size for calculation
    aCbxAutoMin.SetSizePixel( aCbxAutoMin.CalcMinimumSize() );
    aCbxAutoMax.SetSizePixel( aCbxAutoMax.CalcMinimumSize() );
    aCbxAutoStepMain.SetSizePixel( aCbxAutoStepMain.CalcMinimumSize() );
    aCbxAutoStepHelp.SetSizePixel( aCbxAutoStepHelp.CalcMinimumSize() );
    aCbxAutoOrigin.SetSizePixel( aCbxAutoOrigin.CalcMinimumSize() );
    m_aCbx_AutoTimeResolution.SetSizePixel( m_aCbx_AutoTimeResolution.CalcMinimumSize() );

    //ensure new pos is ok
    long nWidthOfOtherControls = m_aLB_MainTimeUnit.GetPosPixel().X() + m_aLB_MainTimeUnit.GetSizePixel().Width() - aFmtFldMin.GetPosPixel().X();
    long nDialogWidth = GetSizePixel().Width();

    long nLeftSpace = nDialogWidth - nNewXPos - nWidthOfOtherControls;
    if(nLeftSpace>=0)
    {
        Size aSize( aTxtMin.GetSizePixel() );
        aSize.Width() = nLabelWidth;
        aTxtMin.SetSizePixel(aSize);
        aTxtMax.SetSizePixel(aSize);
        aTxtMain.SetSizePixel(aSize);
        aTxtHelp.SetSizePixel(aSize);
        aTxtHelpCount.SetSizePixel(aSize);
        aTxtOrigin.SetSizePixel(aSize);
        m_aTxt_TimeResolution.SetSizePixel(aSize);
        m_aTxt_AxisType.SetSizePixel(aSize);

        long nOrgAutoCheckX = aCbxAutoMin.GetPosPixel().X();
        lcl_placeControlAtX( aCbxAutoStepMain, nOrgAutoCheckX );
        lcl_placeControlAtX( aCbxAutoStepHelp, nOrgAutoCheckX );

        lcl_shiftControls( aFmtFldMin, aCbxAutoMin, nNewXPos );
        lcl_shiftControls( aFmtFldMax, aCbxAutoMax, nNewXPos );
        lcl_shiftControls( aFmtFldStepMain, aCbxAutoStepMain, nNewXPos );
        lcl_placeControlAtX( m_aMt_MainDateStep, aFmtFldStepMain.GetPosPixel().X() );
        lcl_shiftControls( aMtStepHelp, aCbxAutoStepHelp, nNewXPos );
        lcl_shiftControls( aFmtFldOrigin, aCbxAutoOrigin, nNewXPos );
        lcl_shiftControls( m_aLB_TimeResolution, m_aCbx_AutoTimeResolution, nNewXPos );
        lcl_placeControlAtX( m_aLB_AxisType, nNewXPos );

        nNewXPos = aCbxAutoStepMain.GetPosPixel().X() + aCbxAutoStepMain.GetSizePixel().Width() + nLabelDistance;
        lcl_placeControlAtX( m_aLB_MainTimeUnit, nNewXPos );
        lcl_placeControlAtX( m_aLB_HelpTimeUnit, nNewXPos );
    }
    PlaceIntervalControlsAccordingToAxisType();
}

void ScaleTabPage::PlaceIntervalControlsAccordingToAxisType()
{
    long nMinX = std::min( aCbxAutoStepMain.GetPosPixel().X(), m_aLB_MainTimeUnit.GetPosPixel().X() );
    long nLabelDistance = lcl_getLabelDistance(aTxtMin);
    long nListWidth = m_aLB_MainTimeUnit.GetSizePixel().Width();

    if( chart2::AxisType::DATE == m_nAxisType )
    {
        lcl_placeControlAtX( m_aLB_MainTimeUnit, nMinX );
        lcl_placeControlAtX( m_aLB_HelpTimeUnit, nMinX );
        long nSecondX = nMinX + nListWidth + nLabelDistance;
        lcl_placeControlAtX( aCbxAutoStepMain, nSecondX );
        lcl_placeControlAtX( aCbxAutoStepHelp, nSecondX );

        long nOne = m_aMt_MainDateStep.LogicToPixel( Size(0, 1), MapMode(MAP_APPFONT) ).Height();

        long nYMajor = m_aMt_MainDateStep.GetPosPixel().Y();
        lcl_placeControlAtY( aCbxAutoStepMain , nYMajor+(3*nOne));
        lcl_placeControlAtY( aTxtMain , nYMajor+nOne+nOne);

        long nYMinor = m_aLB_HelpTimeUnit.GetPosPixel().Y();
        lcl_placeControlAtY( aMtStepHelp , nYMinor );
        lcl_placeControlAtY( aCbxAutoStepHelp , nYMinor+(3*nOne));
    }
    else
    {
        lcl_placeControlAtX( aCbxAutoStepMain, nMinX );
        lcl_placeControlAtX( aCbxAutoStepHelp, nMinX );
        long nSecondX = nMinX + aCbxAutoStepMain.GetSizePixel().Width() + nLabelDistance;
        long nSecondXMax = GetSizePixel().Width() - nListWidth;
        if( nSecondX > nSecondXMax )
            nSecondX = nSecondXMax;
        lcl_placeControlAtX( m_aLB_MainTimeUnit, nSecondX );
        lcl_placeControlAtX( m_aLB_HelpTimeUnit, nSecondX );
    }
}

void ScaleTabPage::EnableControls()
{
    bool bValueAxis = chart2::AxisType::REALNUMBER == m_nAxisType || chart2::AxisType::PERCENT == m_nAxisType || chart2::AxisType::DATE == m_nAxisType;
    bool bDateAxis = chart2::AxisType::DATE == m_nAxisType;

    m_aTxt_AxisType.Show(m_bAllowDateAxis);
    m_aLB_AxisType.Show(m_bAllowDateAxis);

    aCbxLogarithm.Show( bValueAxis && !bDateAxis );
    aTxtMin.Show( bValueAxis );
    aFmtFldMin.Show( bValueAxis );
    aCbxAutoMin.Show( bValueAxis );
    aTxtMax.Show( bValueAxis );
    aFmtFldMax.Show( bValueAxis );
    aCbxAutoMax.Show( bValueAxis );
    aTxtMain.Show( bValueAxis );
    aFmtFldStepMain.Show( bValueAxis );
    aCbxAutoStepMain.Show( bValueAxis );
    aTxtHelp.Show( bValueAxis );
    aTxtHelpCount.Show( bValueAxis );
    aMtStepHelp.Show( bValueAxis );
    aCbxAutoStepHelp.Show( bValueAxis );

    aTxtOrigin.Show( m_bShowAxisOrigin && bValueAxis );
    aFmtFldOrigin.Show( m_bShowAxisOrigin && bValueAxis );
    aCbxAutoOrigin.Show( m_bShowAxisOrigin && bValueAxis );

    aTxtHelpCount.Show( bValueAxis && !bDateAxis );
    aTxtHelp.Show( bDateAxis );

    m_aTxt_TimeResolution.Show( bDateAxis );
    m_aLB_TimeResolution.Show( bDateAxis );
    m_aCbx_AutoTimeResolution.Show( bDateAxis );

    bool bWasDateAxis = m_aMt_MainDateStep.IsVisible();
    if( bWasDateAxis != bDateAxis )
    {
        //transport value from one to other control
        if( bWasDateAxis )
            lcl_setValue( aFmtFldStepMain, m_aMt_MainDateStep.GetValue() );
        else
            m_aMt_MainDateStep.SetValue( static_cast<sal_Int32>(aFmtFldStepMain.GetValue()) );
    }
    aFmtFldStepMain.Show( bValueAxis && !bDateAxis );
    m_aMt_MainDateStep.Show( bDateAxis );

    m_aLB_MainTimeUnit.Show( bDateAxis );
    m_aLB_HelpTimeUnit.Show( bDateAxis );

    EnableValueHdl(&aCbxAutoMin);
    EnableValueHdl(&aCbxAutoMax);
    EnableValueHdl(&aCbxAutoStepMain);
    EnableValueHdl(&aCbxAutoStepHelp);
    EnableValueHdl(&aCbxAutoOrigin);
    EnableValueHdl(&m_aCbx_AutoTimeResolution);
}

IMPL_LINK( ScaleTabPage, EnableValueHdl, CheckBox *, pCbx )
{
    bool bEnable = pCbx && !pCbx->IsChecked() && pCbx->IsEnabled();
    if (pCbx == &aCbxAutoMin)
    {
        aFmtFldMin.Enable( bEnable );
    }
    else if (pCbx == &aCbxAutoMax)
    {
        aFmtFldMax.Enable( bEnable );
    }
    else if (pCbx == &aCbxAutoStepMain)
    {
        aFmtFldStepMain.Enable( bEnable );
        m_aMt_MainDateStep.Enable( bEnable );
        m_aLB_MainTimeUnit.Enable( bEnable );
    }
    else if (pCbx == &aCbxAutoStepHelp)
    {
        aMtStepHelp.Enable( bEnable );
        m_aLB_HelpTimeUnit.Enable( bEnable );
    }
    else if (pCbx == &m_aCbx_AutoTimeResolution)
    {
        m_aLB_TimeResolution.Enable( bEnable );
    }
    else if (pCbx == &aCbxAutoOrigin)
    {
        aFmtFldOrigin.Enable( bEnable );
    }
    return 0;
}

enum AxisTypeListBoxEntry
{
    TYPE_AUTO=0,
    TYPE_TEXT=1,
    TYPE_DATE=2
};

IMPL_LINK_NOARG(ScaleTabPage, SelectAxisTypeHdl)
{
    sal_uInt16 nPos = m_aLB_AxisType.GetSelectEntryPos();
    if( nPos==TYPE_DATE )
        m_nAxisType = chart2::AxisType::DATE;
    else
        m_nAxisType = chart2::AxisType::CATEGORY;
    if( chart2::AxisType::DATE == m_nAxisType )
        aCbxLogarithm.Check(false);
    EnableControls();
    PlaceIntervalControlsAccordingToAxisType();
    SetNumFormat();
    return 0;
}

SfxTabPage* ScaleTabPage::Create(Window* pWindow,const SfxItemSet& rOutAttrs)
{
    return new ScaleTabPage(pWindow, rOutAttrs);
}

sal_Bool ScaleTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    OSL_PRECOND( pNumFormatter, "No NumberFormatter available" );

    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXISTYPE, m_nAxisType));
    if(m_bAllowDateAxis)
        rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_DATEAXIS, TYPE_AUTO==m_aLB_AxisType.GetSelectEntryPos()));

    bool bAutoScale = false;
    if( m_nAxisType==chart2::AxisType::CATEGORY )
        bAutoScale = true;//reset scaling for category charts

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MIN      ,bAutoScale || aCbxAutoMin.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MAX      ,bAutoScale || aCbxAutoMax.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP,bAutoScale || aCbxAutoStepHelp.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN   ,bAutoScale || aCbxAutoOrigin.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM     ,aCbxLogarithm.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_REVERSE       ,aCbxReverse.IsChecked()));
    rOutAttrs.Put(SvxDoubleItem(fMax     , SCHATTR_AXIS_MAX));
    rOutAttrs.Put(SvxDoubleItem(fMin     , SCHATTR_AXIS_MIN));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_STEP_HELP, nStepHelp));
    rOutAttrs.Put(SvxDoubleItem(fOrigin  , SCHATTR_AXIS_ORIGIN));

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN,bAutoScale || aCbxAutoStepMain.IsChecked()));
    rOutAttrs.Put(SvxDoubleItem(fStepMain,SCHATTR_AXIS_STEP_MAIN));

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_TIME_RESOLUTION,bAutoScale || m_aCbx_AutoTimeResolution.IsChecked()));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_TIME_RESOLUTION,m_nTimeResolution));

    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_MAIN_TIME_UNIT,m_nMainTimeUnit));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_HELP_TIME_UNIT,m_nHelpTimeUnit));

    return sal_True;
}

void ScaleTabPage::Reset(const SfxItemSet& rInAttrs)
{
    OSL_PRECOND( pNumFormatter, "No NumberFormatter available" );
    if(!pNumFormatter)
        return;

    const SfxPoolItem *pPoolItem = NULL;
    if (rInAttrs.GetItemState(SCHATTR_AXIS_ALLOW_DATEAXIS, sal_True, &pPoolItem) == SFX_ITEM_SET)
        m_bAllowDateAxis = (bool) ((const SfxBoolItem*)pPoolItem)->GetValue();
    m_nAxisType=chart2::AxisType::REALNUMBER;
    if (rInAttrs.GetItemState(SCHATTR_AXISTYPE, sal_True, &pPoolItem) == SFX_ITEM_SET)
        m_nAxisType = (int) ((const SfxInt32Item*)pPoolItem)->GetValue();
    if( m_nAxisType==chart2::AxisType::DATE && !m_bAllowDateAxis )
        m_nAxisType=chart2::AxisType::CATEGORY;
    if( m_bAllowDateAxis )
    {
        bool bAutoDateAxis = false;
        if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_DATEAXIS, sal_True, &pPoolItem) == SFX_ITEM_SET)
            bAutoDateAxis = (bool) ((const SfxBoolItem*)pPoolItem)->GetValue();

        sal_uInt16 nPos = 0;
        if( m_nAxisType==chart2::AxisType::DATE )
            nPos=TYPE_DATE;
        else if( bAutoDateAxis )
            nPos=TYPE_AUTO;
        else
            nPos=TYPE_TEXT;
        m_aLB_AxisType.SelectEntryPos( nPos );
    }

    if( m_bAllowDateAxis )
        aCbxReverse.SetHelpId("chart2:CheckBox:TP_SCALE:CBX_REVERSE:MayBeDateAxis");
    else if( m_nAxisType==chart2::AxisType::CATEGORY || m_nAxisType==chart2::AxisType::SERIES )
        aCbxReverse.SetHelpId("chart2:CheckBox:TP_SCALE:CBX_REVERSE:Category");

    PlaceIntervalControlsAccordingToAxisType();

    aCbxAutoMin.Check( true );
    aCbxAutoMax.Check( true );
    aCbxAutoStepMain.Check( true );
    aCbxAutoStepHelp.Check( true );
    aCbxAutoOrigin.Check( true );
    m_aCbx_AutoTimeResolution.Check( true );

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_MIN,sal_True,&pPoolItem) == SFX_ITEM_SET)
        aCbxAutoMin.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MIN,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        fMin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        lcl_setValue( aFmtFldMin, fMin );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_MAX,sal_True, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoMax.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MAX,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        fMax = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        lcl_setValue( aFmtFldMax, fMax );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_STEP_MAIN,sal_True, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoStepMain.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_STEP_MAIN,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        fStepMain = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        lcl_setValue( aFmtFldStepMain, fStepMain );
        m_aMt_MainDateStep.SetValue( static_cast<sal_Int32>(fStepMain) );
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_STEP_HELP,sal_True, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoStepHelp.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_LOGARITHM,sal_True, &pPoolItem) == SFX_ITEM_SET)
        aCbxLogarithm.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_REVERSE,sal_True, &pPoolItem) == SFX_ITEM_SET)
        aCbxReverse.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_STEP_HELP,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        nStepHelp = ((const SfxInt32Item*)pPoolItem)->GetValue();
        aMtStepHelp.SetValue( nStepHelp );
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_ORIGIN,sal_True, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoOrigin.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_ORIGIN,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        fOrigin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        lcl_setValue( aFmtFldOrigin, fOrigin );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_TIME_RESOLUTION,sal_True, &pPoolItem) == SFX_ITEM_SET)
        m_aCbx_AutoTimeResolution.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_TIME_RESOLUTION,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        m_nTimeResolution = ((const SfxInt32Item*)pPoolItem)->GetValue();
        m_aLB_TimeResolution.SelectEntryPos( m_nTimeResolution );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MAIN_TIME_UNIT,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        m_nMainTimeUnit = ((const SfxInt32Item*)pPoolItem)->GetValue();
        m_aLB_MainTimeUnit.SelectEntryPos( m_nMainTimeUnit );
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_HELP_TIME_UNIT,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        m_nHelpTimeUnit = ((const SfxInt32Item*)pPoolItem)->GetValue();
        m_aLB_HelpTimeUnit.SelectEntryPos( m_nHelpTimeUnit );
    }

    EnableControls();
    SetNumFormat();
}

int ScaleTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( !pNumFormatter )
    {
        OSL_FAIL( "No NumberFormatter available" );
        return LEAVE_PAGE;
    }

    bool bDateAxis = chart2::AxisType::DATE == m_nAxisType;

    sal_uInt32 nMinMaxOriginFmt = aFmtFldMax.GetFormatKey();
    if ((pNumFormatter->GetType(nMinMaxOriginFmt) &~ NUMBERFORMAT_DEFINED) == NUMBERFORMAT_TEXT)
        nMinMaxOriginFmt = 0;
    // numberformat_text cause numbers to fail being numbers...  Shouldn't happen, but can.
    sal_uInt32 nStepFmt = aFmtFldStepMain.GetFormatKey();
    if ((pNumFormatter->GetType(nStepFmt) &~NUMBERFORMAT_DEFINED) == NUMBERFORMAT_TEXT)
        nStepFmt = 0;

    Control* pControl = NULL;
    sal_uInt16 nErrStrId = 0;
    double fDummy;

    fMax = aFmtFldMax.GetValue();
    fMin = aFmtFldMin.GetValue();
    fOrigin = aFmtFldOrigin.GetValue();
    fStepMain = bDateAxis ? m_aMt_MainDateStep.GetValue() : aFmtFldStepMain.GetValue();
    nStepHelp = static_cast< sal_Int32 >( aMtStepHelp.GetValue());
    m_nTimeResolution = m_aLB_TimeResolution.GetSelectEntryPos();
    m_nMainTimeUnit = m_aLB_MainTimeUnit.GetSelectEntryPos();
    m_nHelpTimeUnit = m_aLB_HelpTimeUnit.GetSelectEntryPos();

    if( chart2::AxisType::REALNUMBER != m_nAxisType )
        aCbxLogarithm.Show( false );

    //check which entries need user action

    if ( aCbxLogarithm.IsChecked() &&
            ( ( !aCbxAutoMin.IsChecked() && fMin <= 0.0 )
             || ( !aCbxAutoMax.IsChecked() && fMax <= 0.0 ) ) )
    {
        pControl = &aFmtFldMin;
        nErrStrId = STR_BAD_LOGARITHM;
    }
    // check for entries that cannot be parsed for the current number format
    else if ( aFmtFldMin.IsModified()
              && !aCbxAutoMin.IsChecked()
              && !pNumFormatter->IsNumberFormat( aFmtFldMin.GetText(), nMinMaxOriginFmt, fDummy))
    {
        pControl = &aFmtFldMin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if ( aFmtFldMax.IsModified()
              && !aCbxAutoMax.IsChecked()
              && !pNumFormatter->IsNumberFormat( aFmtFldMax.GetText(), nMinMaxOriginFmt, fDummy))
    {
        pControl = &aFmtFldMax;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if ( !bDateAxis && aFmtFldStepMain.IsModified()
              && !aCbxAutoStepMain.IsChecked()
              && !pNumFormatter->IsNumberFormat( aFmtFldStepMain.GetText(), nStepFmt, fDummy))
    {
        pControl = &aFmtFldStepMain;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (aFmtFldOrigin.IsModified() && !aCbxAutoOrigin.IsChecked() &&
             !pNumFormatter->IsNumberFormat( aFmtFldOrigin.GetText(), nMinMaxOriginFmt, fDummy))
    {
        pControl = &aFmtFldOrigin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (!aCbxAutoStepMain.IsChecked() && fStepMain <= 0.0)
    {
        pControl = &aFmtFldStepMain;
        nErrStrId = STR_STEP_GT_ZERO;
    }
    else if (!aCbxAutoMax.IsChecked() && !aCbxAutoMin.IsChecked() &&
             fMin >= fMax)
    {
        pControl = &aFmtFldMin;
        nErrStrId = STR_MIN_GREATER_MAX;
    }
    else if( bDateAxis )
    {
        if( !aCbxAutoStepMain.IsChecked() && !aCbxAutoStepHelp.IsChecked() )
        {
            if( m_nHelpTimeUnit > m_nMainTimeUnit )
            {
                pControl = &m_aLB_MainTimeUnit;
                nErrStrId = STR_INVALID_INTERVALS;
            }
            else if( m_nHelpTimeUnit == m_nMainTimeUnit && nStepHelp > fStepMain )
            {
                pControl = &m_aLB_MainTimeUnit;
                nErrStrId = STR_INVALID_INTERVALS;
            }
        }
        if( !nErrStrId && !m_aCbx_AutoTimeResolution.IsChecked() )
        {
            if( (!aCbxAutoStepMain.IsChecked() && m_nTimeResolution > m_nMainTimeUnit )
                ||
                (!aCbxAutoStepHelp.IsChecked() && m_nTimeResolution > m_nHelpTimeUnit )
                )
            {
                pControl = &m_aLB_TimeResolution;
                nErrStrId = STR_INVALID_TIME_UNIT;
            }
        }
    }

    if( ShowWarning( nErrStrId, pControl ) )
        return KEEP_PAGE;

    if( pItemSet )
        FillItemSet( *pItemSet );

    return LEAVE_PAGE;
}

void ScaleTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    pNumFormatter = pFormatter;
    aFmtFldMax.SetFormatter( pNumFormatter );
    aFmtFldMin.SetFormatter( pNumFormatter );
    aFmtFldStepMain.SetFormatter( pNumFormatter );
    aFmtFldOrigin.SetFormatter( pNumFormatter );

    // #i6278# allow more decimal places than the output format.  As
    // the numbers shown in the edit fields are used for input, it makes more
    // sense to display the values in the input format rather than the output
    // format.
    aFmtFldMax.UseInputStringForFormatting();
    aFmtFldMin.UseInputStringForFormatting();
    aFmtFldStepMain.UseInputStringForFormatting();
    aFmtFldOrigin.UseInputStringForFormatting();

    SetNumFormat();
}

void ScaleTabPage::SetNumFormat()
{
    const SfxPoolItem *pPoolItem = NULL;

    if( GetItemSet().GetItemState( SID_ATTR_NUMBERFORMAT_VALUE, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_uLong nFmt = (sal_uLong)((const SfxInt32Item*)pPoolItem)->GetValue();

        aFmtFldMax.SetFormatKey( nFmt );
        aFmtFldMin.SetFormatKey( nFmt );
        aFmtFldOrigin.SetFormatKey( nFmt );

        if( pNumFormatter )
        {
            short eType = pNumFormatter->GetType( nFmt );
            if( eType == NUMBERFORMAT_DATE )
            {
                // for intervals use standard format for dates (so you can enter a number of days)
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardIndex( pFormat->GetLanguage());
                else
                    nFmt = pNumFormatter->GetStandardIndex();
            }
            else if( eType == NUMBERFORMAT_DATETIME )
            {
                // for intervals use time format for date times
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardFormat( NUMBERFORMAT_TIME, pFormat->GetLanguage() );
                else
                    nFmt = pNumFormatter->GetStandardFormat( NUMBERFORMAT_TIME );
            }

            if( chart2::AxisType::DATE == m_nAxisType && ( eType != NUMBERFORMAT_DATE && eType != NUMBERFORMAT_DATETIME) )
            {
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardFormat( NUMBERFORMAT_DATE, pFormat->GetLanguage() );
                else
                    nFmt = pNumFormatter->GetStandardFormat( NUMBERFORMAT_DATE );

                aFmtFldMax.SetFormatKey( nFmt );
                aFmtFldMin.SetFormatKey( nFmt );
                aFmtFldOrigin.SetFormatKey( nFmt );
            }
        }

        aFmtFldStepMain.SetFormatKey( nFmt );
    }
}

void ScaleTabPage::ShowAxisOrigin( bool bShowOrigin )
{
    m_bShowAxisOrigin = bShowOrigin;
    if( !AxisHelper::isAxisPositioningEnabled() )
        m_bShowAxisOrigin = true;
}

bool ScaleTabPage::ShowWarning( sal_uInt16 nResIdMessage, Control* pControl /* = NULL */ )
{
    if( nResIdMessage == 0 )
        return false;

    WarningBox( this, WinBits( WB_OK ), SCH_RESSTR( nResIdMessage ) ).Execute();
    if( pControl )
    {
        pControl->GrabFocus();
        Edit* pEdit = dynamic_cast<Edit*>(pControl);
        if(pEdit)
            pEdit->SetSelection( Selection( 0, SELECTION_MAX ));
    }
    return true;
}

void ScaleTabPage::HideAllControls()
{
    // We need to set these controls invisible when the class is instantiated
    // since some code in EnableControls() depends on that logic. The real
    // visibility of these controls depend on axis data type, and are
    // set in EnableControls().

    m_aTxt_AxisType.Hide();
    m_aLB_AxisType.Hide();

    aCbxLogarithm.Hide();
    aTxtMin.Hide();
    aFmtFldMin.Hide();
    aCbxAutoMin.Hide();
    aTxtMax.Hide();
    aFmtFldMax.Hide();
    aCbxAutoMax.Hide();
    aTxtMain.Hide();
    aFmtFldStepMain.Hide();
    aCbxAutoStepMain.Hide();
    aTxtHelp.Hide();
    aTxtHelpCount.Hide();
    aMtStepHelp.Hide();
    aCbxAutoStepHelp.Hide();

    aTxtOrigin.Hide();
    aFmtFldOrigin.Hide();
    aCbxAutoOrigin.Hide();

    aTxtHelpCount.Hide();
    aTxtHelp.Hide();

    m_aTxt_TimeResolution.Hide();
    m_aLB_TimeResolution.Hide();
    m_aCbx_AutoTimeResolution.Hide();

    aFmtFldStepMain.Hide();
    m_aMt_MainDateStep.Hide();

    m_aLB_MainTimeUnit.Hide();
    m_aLB_HelpTimeUnit.Hide();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
