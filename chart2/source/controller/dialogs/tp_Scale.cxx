/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "tp_Scale.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
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
#ifndef _ZFORMAT_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <svl/zformat.hxx>
#endif

#include <svtools/controldims.hrc>

#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

namespace
{

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

void lcl_placeControlsAtY( Control& rTop, Control& rBottom, long nNewYPos )
{
    Point aPos( rTop.GetPosPixel() );
    long nShift = nNewYPos - aPos.Y();
    aPos.Y() = nNewYPos;
    rTop.SetPosPixel(aPos);

    aPos = rBottom.GetPosPixel();
    aPos.Y() += nShift;
    rBottom.SetPosPixel(aPos);
}

}

ScaleTabPage::ScaleTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_SCALE), rInAttrs),

    aFlScale(this, SchResId(FL_SCALE)),
    aTxtMin (this, SchResId (TXT_MIN)),
    aFmtFldMin(this, SchResId(EDT_MIN)),
    aCbxAutoMin(this, SchResId(CBX_AUTO_MIN)),
    aTxtMax(this, SchResId (TXT_MAX)),
    aFmtFldMax(this, SchResId(EDT_MAX)),
    aCbxAutoMax(this, SchResId(CBX_AUTO_MAX)),
    aTxtMain (this, SchResId (TXT_STEP_MAIN)),
    aFmtFldStepMain(this, SchResId(EDT_STEP_MAIN)),
    aCbxAutoStepMain(this, SchResId(CBX_AUTO_STEP_MAIN)),
    aTxtHelp (this, SchResId (TXT_STEP_HELP)),
    aMtStepHelp (this, SchResId (MT_STEPHELP)),
    aCbxAutoStepHelp(this, SchResId(CBX_AUTO_STEP_HELP)),

    aTxtOrigin (this, SchResId (TXT_ORIGIN)),
    aFmtFldOrigin(this, SchResId(EDT_ORIGIN)),
    aCbxAutoOrigin(this, SchResId(CBX_AUTO_ORIGIN)),

    aCbxLogarithm(this, SchResId(CBX_LOGARITHM)),
    aCbxReverse(this, SchResId(CBX_REVERSE)),

    fMin(0.0),
    fMax(0.0),
    fStepMain(0.0),
    nStepHelp(0),
    fOrigin(0.0),
    nAxisType(chart2::AxisType::REALNUMBER),
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
    nLabelWidth = ::std::max( aTxtOrigin.CalcMinimumSize().Width(), nLabelWidth );
    nLabelWidth+=1;

    long nLabelDistance = aTxtMin.LogicToPixel( Size(RSC_SP_CTRL_DESC_X, 0), MapMode(MAP_APPFONT) ).Width();
    long nNewXPos = aTxtMin.GetPosPixel().X() + nLabelWidth + nLabelDistance;

    //ensure that the auto checkboxes are wide enough and have correct size for calculation
    aCbxAutoMin.SetSizePixel( aCbxAutoMin.CalcMinimumSize() );
    aCbxAutoMax.SetSizePixel( aCbxAutoMax.CalcMinimumSize() );
    aCbxAutoStepMain.SetSizePixel( aCbxAutoStepMain.CalcMinimumSize() );
    aCbxAutoStepHelp.SetSizePixel( aCbxAutoStepHelp.CalcMinimumSize() );
    aCbxAutoOrigin.SetSizePixel( aCbxAutoOrigin.CalcMinimumSize() );

    //ensure new pos is ok
    long nWidthOfOtherControls = aCbxAutoMin.GetPosPixel().X() + aCbxAutoMin.GetSizePixel().Width() - aFmtFldMin.GetPosPixel().X();
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
        aTxtOrigin.SetSizePixel(aSize);

        lcl_shiftControls( aFmtFldMin, aCbxAutoMin, nNewXPos );
        lcl_shiftControls( aFmtFldMax, aCbxAutoMax, nNewXPos );
        lcl_shiftControls( aFmtFldStepMain, aCbxAutoStepMain, nNewXPos );
        lcl_shiftControls( aMtStepHelp, aCbxAutoStepHelp, nNewXPos );
        lcl_shiftControls( aFmtFldOrigin, aCbxAutoOrigin, nNewXPos );
    }
}

void ScaleTabPage::EnableControls()
{
    bool bEnableForValueOrPercentAxis = chart2::AxisType::REALNUMBER == nAxisType || chart2::AxisType::PERCENT == nAxisType;
    aFlScale.Enable( bEnableForValueOrPercentAxis );
    aTxtMin.Enable( bEnableForValueOrPercentAxis );
    aFmtFldMin.Enable( bEnableForValueOrPercentAxis );
    aCbxAutoMin.Enable( bEnableForValueOrPercentAxis );
    aTxtMax.Enable( bEnableForValueOrPercentAxis );
    aFmtFldMax.Enable( bEnableForValueOrPercentAxis );
    aCbxAutoMax.Enable( bEnableForValueOrPercentAxis );
    aTxtMain.Enable( bEnableForValueOrPercentAxis );
    aFmtFldStepMain.Enable( bEnableForValueOrPercentAxis );
    aCbxAutoStepMain.Enable( bEnableForValueOrPercentAxis );
    aTxtHelp.Enable( bEnableForValueOrPercentAxis );
    aMtStepHelp.Enable( bEnableForValueOrPercentAxis );
    aCbxAutoStepHelp.Enable( bEnableForValueOrPercentAxis );
    aCbxLogarithm.Enable( bEnableForValueOrPercentAxis );

    aTxtOrigin.Show( m_bShowAxisOrigin && bEnableForValueOrPercentAxis );
    aFmtFldOrigin.Show( m_bShowAxisOrigin && bEnableForValueOrPercentAxis );
    aCbxAutoOrigin.Show( m_bShowAxisOrigin && bEnableForValueOrPercentAxis );

    long nNewYPos = aTxtOrigin.GetPosPixel().Y();
    if( m_bShowAxisOrigin )
        nNewYPos += ( aTxtOrigin.GetPosPixel().Y() - aTxtHelp.GetPosPixel().Y() );
    lcl_placeControlsAtY( aCbxLogarithm, aCbxReverse, nNewYPos );
}



IMPL_LINK( ScaleTabPage, EnableValueHdl, CheckBox *, pCbx )
{
    if (pCbx == &aCbxAutoMin)
    {
        aFmtFldMin.Enable(!aCbxAutoMin.IsChecked());
    }
    else if (pCbx == &aCbxAutoMax)
    {
        aFmtFldMax.Enable(!aCbxAutoMax.IsChecked());
    }
    else if (pCbx == &aCbxAutoStepMain)
    {
        aFmtFldStepMain.Enable(!aCbxAutoStepMain.IsChecked());
    }
    else if (pCbx == &aCbxAutoStepHelp)
    {
        aMtStepHelp.Show ();
        aMtStepHelp.Enable( ! aCbxAutoStepHelp.IsChecked() );
    }
    else if (pCbx == &aCbxAutoOrigin)
    {
        aFmtFldOrigin.Enable(!aCbxAutoOrigin.IsChecked());
    }
    return 0;
}

SfxTabPage* ScaleTabPage::Create(Window* pWindow,const SfxItemSet& rOutAttrs)
{
    return new ScaleTabPage(pWindow, rOutAttrs);
}

BOOL ScaleTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    DBG_ASSERT( pNumFormatter, "No NumberFormatter available" );

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MIN      ,aCbxAutoMin.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MAX      ,aCbxAutoMax.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP,aCbxAutoStepHelp.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN   ,aCbxAutoOrigin.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM     ,aCbxLogarithm.IsChecked()));
    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_REVERSE       ,aCbxReverse.IsChecked()));
    rOutAttrs.Put(SvxDoubleItem(fMax     , SCHATTR_AXIS_MAX));
    rOutAttrs.Put(SvxDoubleItem(fMin     , SCHATTR_AXIS_MIN));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_STEP_HELP, nStepHelp));
    rOutAttrs.Put(SvxDoubleItem(fOrigin  , SCHATTR_AXIS_ORIGIN));

    rOutAttrs.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN,aCbxAutoStepMain.IsChecked()));
    rOutAttrs.Put(SvxDoubleItem(fStepMain,SCHATTR_AXIS_STEP_MAIN));

    return TRUE;
}

void ScaleTabPage::Reset(const SfxItemSet& rInAttrs)
{
    DBG_ASSERT( pNumFormatter, "No NumberFormatter available" );
    if(!pNumFormatter)
        return;

    const SfxPoolItem *pPoolItem = NULL;
    nAxisType=chart2::AxisType::REALNUMBER;
    if (rInAttrs.GetItemState(SCHATTR_AXISTYPE, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nAxisType = (int) ((const SfxInt32Item*)pPoolItem)->GetValue();
        EnableControls();
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_MIN,TRUE,&pPoolItem) == SFX_ITEM_SET)
        aCbxAutoMin.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fMin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldMin.SetValue( fMin );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_MAX,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoMax.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_MAX,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fMax = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldMax.SetValue( fMax );
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_STEP_MAIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoStepMain.Check(((const SfxBoolItem*)pPoolItem)->GetValue());

    if (rInAttrs.GetItemState(SCHATTR_AXIS_STEP_MAIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fStepMain = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldStepMain.SetValue( fStepMain );
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_STEP_HELP,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoStepHelp.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_LOGARITHM,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxLogarithm.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_REVERSE,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxReverse.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_STEP_HELP,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nStepHelp = ((const SfxInt32Item*)pPoolItem)->GetValue();
        aMtStepHelp.SetValue( nStepHelp );
    }
    if (rInAttrs.GetItemState(SCHATTR_AXIS_AUTO_ORIGIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
        aCbxAutoOrigin.Check(((const SfxBoolItem*)pPoolItem)->GetValue());
    if (rInAttrs.GetItemState(SCHATTR_AXIS_ORIGIN,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        fOrigin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
        aFmtFldOrigin.SetValue( fOrigin );
    }

    EnableValueHdl(&aCbxAutoMin);
    EnableValueHdl(&aCbxAutoMax);
    EnableValueHdl(&aCbxAutoStepMain);
    EnableValueHdl(&aCbxAutoStepHelp);
    EnableValueHdl(&aCbxAutoOrigin);
}

int ScaleTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( !pNumFormatter )
    {
        OSL_FAIL( "No NumberFormatter available" );
        return LEAVE_PAGE;
    }

    sal_uInt32 nIndex = pNumFormatter->GetStandardIndex(LANGUAGE_SYSTEM);
    const SfxPoolItem *pPoolItem = NULL;
    if( GetItemSet().GetItemState( SID_ATTR_NUMBERFORMAT_VALUE, TRUE, &pPoolItem ) == SFX_ITEM_SET )
        nIndex = static_cast< sal_uInt32 >( static_cast< const SfxInt32Item* >(pPoolItem)->GetValue());
    else
    {
        OSL_ENSURE( false, "Using Standard Language" );
    }

    Edit* pEdit = NULL;
    USHORT nErrStrId = 0;
    double fDummy;

    fMax = aFmtFldMax.GetValue();
    fMin = aFmtFldMin.GetValue();
    fOrigin = aFmtFldOrigin.GetValue();
    fStepMain = aFmtFldStepMain.GetValue();
    nStepHelp = static_cast< sal_Int32 >( aMtStepHelp.GetValue());

    //do some reasonable automatic correction of user input if necessary
    if (!aCbxAutoMax.IsChecked() && !aCbxAutoMin.IsChecked() &&
             fMin >= fMax)
    {
        pEdit = &aFmtFldMin;
        nErrStrId = STR_MIN_GREATER_MAX;
    }
    // check for entries in invalid ranges
    if ( aCbxLogarithm.IsChecked() &&
            ( ( !aCbxAutoMin.IsChecked() && fMin <= 0.0 )
             || ( !aCbxAutoMax.IsChecked() && fMax <= 0.0 ) ) )
    {
        pEdit = &aFmtFldMin;
        nErrStrId = STR_BAD_LOGARITHM;
    }
    if (!aCbxAutoStepMain.IsChecked() && fStepMain <= 0)
    {
        pEdit = &aFmtFldStepMain;
        nErrStrId = STR_STEP_GT_ZERO;
    }

    //check wich entries need user action

    // check for entries that cannot be parsed for the current number format
    if ( aFmtFldMin.IsModified()
        && !aCbxAutoMin.IsChecked()
        && !pNumFormatter->IsNumberFormat(aFmtFldMin.GetText(), nIndex, fDummy))
    {
        pEdit = &aFmtFldMin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (aFmtFldMax.IsModified() && !aCbxAutoMax.IsChecked() &&
             !pNumFormatter->IsNumberFormat(aFmtFldMax.GetText(),
                                            nIndex, fDummy))
    {
        pEdit = &aFmtFldMax;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (aFmtFldStepMain.IsModified() && !aCbxAutoStepMain.IsChecked() &&
             !pNumFormatter->IsNumberFormat(aFmtFldStepMain.GetText(),
                                            nIndex, fDummy))
    {
        pEdit = &aFmtFldStepMain;
        nErrStrId = STR_STEP_GT_ZERO;
    }
    else if (aFmtFldOrigin.IsModified() && !aCbxAutoOrigin.IsChecked() &&
             !pNumFormatter->IsNumberFormat(aFmtFldOrigin.GetText(),
                                            nIndex, fDummy))
    {
        pEdit = &aFmtFldOrigin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (!aCbxAutoStepMain.IsChecked() && fStepMain <= 0.0)
    {
        pEdit = &aFmtFldStepMain;
        nErrStrId = STR_STEP_GT_ZERO;
    }

    if( ShowWarning( nErrStrId, pEdit ) )
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

    if( GetItemSet().GetItemState( SID_ATTR_NUMBERFORMAT_VALUE, TRUE, &pPoolItem ) == SFX_ITEM_SET )
    {
        ULONG nFmt = (ULONG)((const SfxInt32Item*)pPoolItem)->GetValue();

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

bool ScaleTabPage::ShowWarning( USHORT nResIdMessage, Edit * pControl /* = NULL */ )
{
    if( nResIdMessage == 0 )
        return false;

    WarningBox( this, WinBits( WB_OK ), String( SchResId( nResIdMessage ))).Execute();
    if( pControl )
    {
        pControl->GrabFocus();
        pControl->SetSelection( Selection( 0, SELECTION_MAX ));
    }
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
