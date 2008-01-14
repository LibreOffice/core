/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_Scale.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:56:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
// header for class SvxDoubleItem
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// header for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

// header for class WarningBox
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

// header for class SvNumberformat
#ifndef _ZFORMAT_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <svtools/zformat.hxx>
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

void lcl_shiftControls( Control& rEdit, CheckBox& rAuto, long nNewXPos )
{
    Point aPos( rEdit.GetPosPixel() );
    long nShift = nNewXPos - aPos.X();
    aPos.X() = nNewXPos;
    rEdit.SetPosPixel(aPos);

    aPos = rAuto.GetPosPixel();
    aPos.X() += nShift;
    rAuto.SetPosPixel(aPos);
}

}

ScaleTabPage::ScaleTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_SCALE_Y), rInAttrs),

    aFlScale(this, SchResId(FL_SCALE_Y)),
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

    aFlTicks(this,SchResId(FL_TICKS)),
    aCbxTicksInner(this, SchResId(CBX_TICKS_INNER)),
    aCbxTicksOuter(this, SchResId(CBX_TICKS_OUTER)),

    aFlHelpTicks(this,SchResId(FL_HELPTICKS)),
    aCbxHelpTicksInner(this, SchResId(CBX_HELPTICKS_INNER)),
    aCbxHelpTicksOuter(this, SchResId(CBX_HELPTICKS_OUTER)),

    fMin(0.0),
    fMax(0.0),
    fStepMain(0.0),
    nStepHelp(0),
    fOrigin(0.0),
    nAxisType(chart2::AxisType::REALNUMBER),
    pNumFormatter(NULL)
{
    FreeResource();
    SetExchangeSupport();

    //optimize position of the controls
    {
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

            //tickmark controls
            long nCheckWidth = 1 + ::std::max( aCbxTicksInner.CalcMinimumSize().Width(), aCbxHelpTicksInner.CalcMinimumSize().Width() );
            aSize = aCbxTicksInner.GetSizePixel();
            aSize.Width() = nCheckWidth;

            long nCheckDistance = aCbxTicksInner.LogicToPixel( Size(RSC_SP_CTRL_X, 0), MapMode(MAP_APPFONT) ).Width();
            long nNewCheckXPos = aCbxTicksInner.GetPosPixel().X() + nCheckWidth + nCheckDistance;

            aCbxTicksOuter.SetSizePixel( aCbxTicksOuter.CalcMinimumSize() );
            aCbxHelpTicksOuter.SetSizePixel( aCbxHelpTicksOuter.CalcMinimumSize() );

            nWidthOfOtherControls = aCbxTicksOuter.GetSizePixel().Width();
            nLeftSpace = nDialogWidth - nNewCheckXPos - nWidthOfOtherControls;

            if(nLeftSpace>=0)
            {
                aCbxTicksInner.SetSizePixel(aSize);
                aCbxHelpTicksInner.SetSizePixel(aSize);

                if( nNewCheckXPos < nNewXPos && (nDialogWidth - nNewXPos - nWidthOfOtherControls)>=0 )
                    nNewCheckXPos = nNewXPos;//alignement looks nicer

                Point aPos( aCbxTicksOuter.GetPosPixel() );
                aPos.X() = nNewCheckXPos;
                aCbxTicksOuter.SetPosPixel(aPos);

                aPos = aCbxHelpTicksOuter.GetPosPixel();
                aPos.X() = nNewCheckXPos;
                aCbxHelpTicksOuter.SetPosPixel(aPos);
            }
        }
    }

    aCbxAutoMin.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoMax.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoStepMain.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoStepHelp.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    aCbxAutoOrigin.SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
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
    aTxtOrigin.Enable( bEnableForValueOrPercentAxis );
    aFmtFldOrigin.Enable( bEnableForValueOrPercentAxis );
    aCbxAutoOrigin.Enable( bEnableForValueOrPercentAxis );
    aCbxLogarithm.Enable( bEnableForValueOrPercentAxis );
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

    long nTicks=0;
    long nHelpTicks=0;

    if(aCbxHelpTicksInner.IsChecked())
        nHelpTicks|=CHAXIS_MARK_INNER;
    if(aCbxHelpTicksOuter.IsChecked())
        nHelpTicks|=CHAXIS_MARK_OUTER;
    if(aCbxTicksInner.IsChecked())
        nTicks|=CHAXIS_MARK_INNER;
    if(aCbxTicksOuter.IsChecked())
        nTicks|=CHAXIS_MARK_OUTER;

    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_TICKS,nTicks));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_HELPTICKS,nHelpTicks));

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

    long nTicks=0,nHelpTicks=0;
    if(rInAttrs.GetItemState(SCHATTR_AXIS_TICKS,TRUE, &pPoolItem)== SFX_ITEM_SET)
        nTicks=((const SfxInt32Item*)pPoolItem)->GetValue();
    if(rInAttrs.GetItemState(SCHATTR_AXIS_HELPTICKS,TRUE, &pPoolItem)== SFX_ITEM_SET)
        nHelpTicks=((const SfxInt32Item*)pPoolItem)->GetValue();

    aCbxHelpTicksInner.Check(BOOL(nHelpTicks&CHAXIS_MARK_INNER));
    aCbxHelpTicksOuter.Check(BOOL(nHelpTicks&CHAXIS_MARK_OUTER));
    aCbxTicksInner.Check(BOOL(nTicks&CHAXIS_MARK_INNER));
    aCbxTicksOuter.Check(BOOL(nTicks&CHAXIS_MARK_OUTER));

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
        DBG_ERROR( "No NumberFormatter available" );
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
    //user often forgets to switch visibility of help tickmarks on
    if( !aCbxAutoStepHelp.IsChecked() && aMtStepHelp.IsModified() && nStepHelp > 1
        && !aCbxHelpTicksInner.IsChecked() && !aCbxHelpTicksOuter.IsChecked() )
        //&& !aCbxHelpTicksInner.IsModified() && !aCbxHelpTicksOuter.IsModified() )
    {
        //check help ticks like main ticks
        if(aCbxTicksInner.IsChecked())
            aCbxHelpTicksInner.Check();
        if(aCbxTicksOuter.IsChecked())
            aCbxHelpTicksOuter.Check();
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

    // #101318#, #i6278# allow more decimal places than the output format.  As
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

        // for steps use standard format if date or time format is chosen
        short eType = pNumFormatter->GetType( nFmt );
        if( pNumFormatter &&
            ( eType == NUMBERFORMAT_DATE ||
              eType == NUMBERFORMAT_TIME ||
              eType == NUMBERFORMAT_DATETIME ) )
        {
            const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
            if( pFormat )
                nFmt = pNumFormatter->GetStandardFormat( pFormat->GetLanguage());
            else
                nFmt = pNumFormatter->GetStandardIndex();
        }

        aFmtFldStepMain.SetFormatKey( nFmt );
    }
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
