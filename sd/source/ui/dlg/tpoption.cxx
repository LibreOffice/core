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

#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/strarray.hxx>
#include <svx/dlgutil.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "tpoption.hrc"
#include "tpoption.hxx"
#include "strings.hrc"
#include "app.hrc"
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SdTpOptionsSnap::SdTpOptionsSnap( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SvxGridTabPage(pParent, rInAttrs)
{
    aGrpSnap.Show();
    aCbxSnapHelplines.Show();
    aCbxSnapBorder.Show();
    aCbxSnapFrame.Show();
    aCbxSnapPoints.Show();
    aFtSnapArea.Show();
    aMtrFldSnapArea.Show();
    aGrpOrtho.Show();
    aCbxOrtho.Show();
    aCbxBigOrtho.Show();
    aCbxRotate.Show();
    aMtrFldAngle.Show();
    aFtBezAngle.Show();
    aMtrFldBezAngle.Show();
    aSeparatorFL.Show();
}

// -----------------------------------------------------------------------

SdTpOptionsSnap::~SdTpOptionsSnap()
{
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsSnap::FillItemSet( SfxItemSet& rAttrs )
{
    SvxGridTabPage::FillItemSet(rAttrs);
    SdOptionsSnapItem aOptsItem( ATTR_OPTIONS_SNAP );

    aOptsItem.GetOptionsSnap().SetSnapHelplines( aCbxSnapHelplines.IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapBorder( aCbxSnapBorder.IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapFrame( aCbxSnapFrame.IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapPoints( aCbxSnapPoints.IsChecked() );
    aOptsItem.GetOptionsSnap().SetOrtho( aCbxOrtho.IsChecked() );
    aOptsItem.GetOptionsSnap().SetBigOrtho( aCbxBigOrtho.IsChecked() );
    aOptsItem.GetOptionsSnap().SetRotate( aCbxRotate.IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapArea( (sal_Int16) aMtrFldSnapArea.GetValue() );
    aOptsItem.GetOptionsSnap().SetAngle( (sal_Int16) aMtrFldAngle.GetValue() );
    aOptsItem.GetOptionsSnap().SetEliminatePolyPointLimitAngle( (sal_Int16) aMtrFldBezAngle.GetValue() );

    rAttrs.Put( aOptsItem );

    // we get a possible existing GridItem, this ensures that we do net set
    // some default values by accident
    return( sal_True );
}

// -----------------------------------------------------------------------

void SdTpOptionsSnap::Reset( const SfxItemSet& rAttrs )
{
    SvxGridTabPage::Reset(rAttrs);

    SdOptionsSnapItem aOptsItem( (const SdOptionsSnapItem&) rAttrs.
                        Get( ATTR_OPTIONS_SNAP ) );

    aCbxSnapHelplines.Check( aOptsItem.GetOptionsSnap().IsSnapHelplines() );
    aCbxSnapBorder.Check( aOptsItem.GetOptionsSnap().IsSnapBorder() );
    aCbxSnapFrame.Check( aOptsItem.GetOptionsSnap().IsSnapFrame() );
    aCbxSnapPoints.Check( aOptsItem.GetOptionsSnap().IsSnapPoints() );
    aCbxOrtho.Check( aOptsItem.GetOptionsSnap().IsOrtho() );
    aCbxBigOrtho.Check( aOptsItem.GetOptionsSnap().IsBigOrtho() );
    aCbxRotate.Check( aOptsItem.GetOptionsSnap().IsRotate() );
    aMtrFldSnapArea.SetValue( aOptsItem.GetOptionsSnap().GetSnapArea() );
    aMtrFldAngle.SetValue( aOptsItem.GetOptionsSnap().GetAngle() );
    aMtrFldBezAngle.SetValue( aOptsItem.GetOptionsSnap().GetEliminatePolyPointLimitAngle() );

    aCbxRotate.GetClickHdl().Call(0);
}

// -----------------------------------------------------------------------

SfxTabPage* SdTpOptionsSnap::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsSnap( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*  TabPage to adjust the content options
|*
\************************************************************************/

SdTpOptionsContents::SdTpOptionsContents( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SfxTabPage          ( pParent, SdResId( TP_OPTIONS_CONTENTS ), rInAttrs ),
        aGrpDisplay         ( this, SdResId( GRP_DISPLAY ) ),
        aCbxRuler           ( this, SdResId( CBX_RULER ) ),
        aCbxDragStripes     ( this, SdResId( CBX_HELPLINES ) ),
        aCbxHandlesBezier   ( this, SdResId( CBX_HANDLES_BEZIER ) ),
        aCbxMoveOutline     ( this, SdResId( CBX_MOVE_OUTLINE ) )
{
    FreeResource();
}

// -----------------------------------------------------------------------

SdTpOptionsContents::~SdTpOptionsContents()
{
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsContents::FillItemSet( SfxItemSet& rAttrs )
{
    sal_Bool bModified = sal_False;

    if( aCbxRuler.GetSavedValue()           != aCbxRuler.IsChecked() ||
        aCbxMoveOutline.GetSavedValue()     != aCbxMoveOutline.IsChecked() ||
        aCbxDragStripes.GetSavedValue()     != aCbxDragStripes.IsChecked() ||
        aCbxHandlesBezier.GetSavedValue()   != aCbxHandlesBezier.IsChecked() )
    {
        SdOptionsLayoutItem aOptsItem( ATTR_OPTIONS_LAYOUT );

        aOptsItem.GetOptionsLayout().SetRulerVisible( aCbxRuler.IsChecked() );
        aOptsItem.GetOptionsLayout().SetMoveOutline( aCbxMoveOutline.IsChecked() );
        aOptsItem.GetOptionsLayout().SetDragStripes( aCbxDragStripes.IsChecked() );
        aOptsItem.GetOptionsLayout().SetHandlesBezier( aCbxHandlesBezier.IsChecked() );

        rAttrs.Put( aOptsItem );
        bModified = sal_True;
    }
    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsContents::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsContentsItem aOptsItem( (const SdOptionsContentsItem&) rAttrs.
                        Get( ATTR_OPTIONS_CONTENTS ) );

    SdOptionsLayoutItem aLayoutItem( (const SdOptionsLayoutItem&) rAttrs.
                        Get( ATTR_OPTIONS_LAYOUT ) );

    aCbxRuler.Check( aLayoutItem.GetOptionsLayout().IsRulerVisible() );
    aCbxMoveOutline.Check( aLayoutItem.GetOptionsLayout().IsMoveOutline() );
    aCbxDragStripes.Check( aLayoutItem.GetOptionsLayout().IsDragStripes() );
    aCbxHandlesBezier.Check( aLayoutItem.GetOptionsLayout().IsHandlesBezier() );

    aCbxRuler.SaveValue();
    aCbxMoveOutline.SaveValue();
    aCbxDragStripes.SaveValue();
    aCbxHandlesBezier.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* SdTpOptionsContents::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsContents( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*  TabPage to adjust the misc options
|*
\************************************************************************/
#define TABLE_COUNT 12
#define TOKEN (sal_Unicode(':'))

SdTpOptionsMisc::SdTpOptionsMisc( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SfxTabPage          ( pParent, SdResId( TP_OPTIONS_MISC ), rInAttrs ),
    aGrpText                    ( this, SdResId( GRP_TEXT ) ),
    aCbxQuickEdit               ( this, SdResId( CBX_QUICKEDIT ) ),
    aCbxPickThrough             ( this, SdResId( CBX_PICKTHROUGH ) ),

    // At the moment, template & layout are not running synchronized
    aGrpProgramStart            ( this, SdResId( GRP_PROGRAMSTART ) ),
    aCbxStartWithTemplate       ( this, SdResId( CBX_START_WITH_TEMPLATE ) ),

    aGrpSettings                ( this, SdResId( GRP_SETTINGS ) ),
    aCbxMasterPageCache         ( this, SdResId( CBX_MASTERPAGE_CACHE ) ),
    aCbxCopy                    ( this, SdResId( CBX_COPY ) ),
    aCbxMarkedHitMovesAlways    ( this, SdResId( CBX_MARKED_HIT_MOVES_ALWAYS ) ),
    aCbxCrookNoContortion       ( this, SdResId( CBX_CROOK_NO_CONTORTION ) ),

    aTxtMetric                  ( this, SdResId( FT_METRIC ) ),
    aLbMetric                   ( this, SdResId( LB_METRIC ) ),
    aTxtTabstop                 ( this, SdResId( FT_TABSTOP ) ),
    aMtrFldTabstop              ( this, SdResId( MTR_FLD_TABSTOP ) ),

    aCbxStartWithActualPage     ( this, SdResId( CBX_START_WITH_ACTUAL_PAGE ) ),
    aGrpStartWithActualPage     ( this, SdResId( GRP_START_WITH_ACTUAL_PAGE ) ),
    aCbxEnableSdremote          ( this, SdResId( CBX_ENABLE_SDREMOTE ) ),
    aTxtCompatibility           ( this, SdResId( FT_COMPATIBILITY ) ),
    aCbxUsePrinterMetrics       ( this, SdResId( CB_USE_PRINTER_METRICS ) ),
    aCbxCompatibility           ( this, SdResId( CB_MERGE_PARA_DIST ) ),
    aGrpScale                   ( this, SdResId( GRP_SCALE ) ),
    aFtScale                    ( this, SdResId( FT_SCALE ) ),
    aCbScale                    ( this, SdResId( CB_SCALE ) ),
    aFtOriginal                 ( this, SdResId( FT_ORIGINAL ) ),
    aFtEquivalent               ( this, SdResId( FT_EQUIVALENT ) ),
    aFtPageWidth                ( this, SdResId( FT_PAGEWIDTH ) ),
    aFiInfo1                    ( this, SdResId( FI_INFO_1 ) ),
    aMtrFldOriginalWidth        ( this, SdResId( MTR_FLD_ORIGINAL_WIDTH ) ),
    aFtPageHeight               ( this, SdResId( FT_PAGEHEIGHT ) ),
    aFiInfo2                    ( this, SdResId( FI_INFO_2 ) ),
    aMtrFldOriginalHeight       ( this, SdResId( MTR_FLD_ORIGINAL_HEIGHT ) ),
    aMtrFldInfo1                ( this, WinBits( WB_HIDE ) ),
    aMtrFldInfo2                ( this, WinBits( WB_HIDE ) )
{
    FreeResource();
    SetExchangeSupport();

    // set metric
    FieldUnit eFUnit;

    sal_uInt16 nWhich = GetWhich( SID_ATTR_METRIC );
    if ( rInAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rInAttrs.Get( nWhich );
        eFUnit = (FieldUnit)rItem.GetValue();
    }
    else
        eFUnit = SfxModule::GetCurrentFieldUnit();

    SetFieldUnit( aMtrFldTabstop, eFUnit );

    // fill ListBox with metrics
    SvxStringArray aMetricArr( RID_SVXSTR_FIELDUNIT_TABLE );
    sal_uInt16 i;

    for ( i = 0; i < aMetricArr.Count(); ++i )
    {
        String sMetric = aMetricArr.GetStringByPos( i );
        long nFieldUnit = aMetricArr.GetValue( i );
        sal_uInt16 nPos = aLbMetric.InsertEntry( sMetric );
        aLbMetric.SetEntryData( nPos, (void*)nFieldUnit );
    }
    aLbMetric.SetSelectHdl( LINK( this, SdTpOptionsMisc, SelectMetricHdl_Impl ) );

    SetFieldUnit( aMtrFldOriginalWidth, eFUnit );
    SetFieldUnit( aMtrFldOriginalHeight, eFUnit );
    aMtrFldOriginalWidth.SetLast( 999999999 );
    aMtrFldOriginalWidth.SetMax( 999999999 );
    aMtrFldOriginalHeight.SetLast( 999999999 );
    aMtrFldOriginalHeight.SetMax( 999999999 );

    // temporary fields for info texts (for formatting/calculation)
    aMtrFldInfo1.SetUnit( eFUnit );
    aMtrFldInfo1.SetMax( 999999999 );
    aMtrFldInfo1.SetDecimalDigits( 2 );
    aMtrFldInfo2.SetUnit( eFUnit );
    aMtrFldInfo2.SetMax( 999999999 );
    aMtrFldInfo2.SetDecimalDigits( 2 );

    // determine PoolUnit
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Fuellen der CB
    sal_uInt16 aTable[ TABLE_COUNT ] =
        { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for( i = 0; i < TABLE_COUNT; i++ )
        aCbScale.InsertEntry( GetScale( 1, aTable[i] ) );
    for( i = 1; i < TABLE_COUNT; i++ )
        aCbScale.InsertEntry( GetScale(  aTable[i], 1 ) );
}

// -----------------------------------------------------------------------

SdTpOptionsMisc::~SdTpOptionsMisc()
{
}
// -----------------------------------------------------------------------
void SdTpOptionsMisc::ActivatePage( const SfxItemSet& rSet )
{
    // We have to call SaveValue again since it can happen that the value
    // has no effect on other TabPages
    aLbMetric.SaveValue();
    // change metric if necessary (since TabPage is in the Dialog where
    // the metric is set)
    const SfxPoolItem* pAttr = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SfxUInt16Item* pItem = (SfxUInt16Item*) pAttr;

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != aMtrFldOriginalWidth.GetUnit() )
        {
            // set metrics
            sal_Int64 nVal = aMtrFldOriginalWidth.Denormalize( aMtrFldOriginalWidth.GetValue( FUNIT_TWIP ) );
            SetFieldUnit( aMtrFldOriginalWidth, eFUnit, sal_True );
            aMtrFldOriginalWidth.SetValue( aMtrFldOriginalWidth.Normalize( nVal ), FUNIT_TWIP );

            nVal = aMtrFldOriginalHeight.Denormalize( aMtrFldOriginalHeight.GetValue( FUNIT_TWIP ) );
            SetFieldUnit( aMtrFldOriginalHeight, eFUnit, sal_True );
            aMtrFldOriginalHeight.SetValue( aMtrFldOriginalHeight.Normalize( nVal ), FUNIT_TWIP );


            if( nWidth != 0 && nHeight != 0 )
            {
                aMtrFldInfo1.SetUnit( eFUnit );
                aMtrFldInfo2.SetUnit( eFUnit );

                SetMetricValue( aMtrFldInfo1, nWidth, ePoolUnit );
                aInfo1 = aMtrFldInfo1.GetText();
                aFiInfo1.SetText( aInfo1 );

                SetMetricValue( aMtrFldInfo2, nHeight, ePoolUnit );
                aInfo2 = aMtrFldInfo2.GetText();
                aFiInfo2.SetText( aInfo2 );
            }
        }
    }
}

// -----------------------------------------------------------------------

int SdTpOptionsMisc::DeactivatePage( SfxItemSet* pActiveSet )
{
    // check parser
    sal_Int32 nX, nY;
    if( SetScale( aCbScale.GetText(), nX, nY ) )
    {
        if( pActiveSet )
            FillItemSet( *pActiveSet );
        return( LEAVE_PAGE );
    }
    WarningBox aWarnBox( GetParent(), WB_YES_NO, String( SdResId( STR_WARN_SCALE_FAIL ) ) );
    short nReturn = aWarnBox.Execute();

    if( nReturn == RET_YES )
        return( KEEP_PAGE );

    if( pActiveSet )
        FillItemSet( *pActiveSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsMisc::FillItemSet( SfxItemSet& rAttrs )
{
    sal_Bool bModified = sal_False;

    if( aCbxStartWithTemplate.GetSavedValue()   != aCbxStartWithTemplate.IsChecked() ||
        aCbxMarkedHitMovesAlways.GetSavedValue()!= aCbxMarkedHitMovesAlways.IsChecked() ||
        aCbxCrookNoContortion.GetSavedValue()   != aCbxCrookNoContortion.IsChecked() ||
        aCbxQuickEdit.GetSavedValue()           != aCbxQuickEdit.IsChecked() ||
        aCbxPickThrough.GetSavedValue()         != aCbxPickThrough.IsChecked() ||
        aCbxMasterPageCache.GetSavedValue()     != aCbxMasterPageCache.IsChecked() ||
        aCbxCopy.GetSavedValue()                != aCbxCopy.IsChecked() ||
        aCbxStartWithActualPage.GetSavedValue() != aCbxStartWithActualPage.IsChecked() ||
        aCbxEnableSdremote.GetSavedValue()      != aCbxEnableSdremote.IsChecked() ||
        aCbxCompatibility.GetSavedValue()       != aCbxCompatibility.IsChecked() ||
        aCbxUsePrinterMetrics.GetSavedValue()   != aCbxUsePrinterMetrics.IsChecked() )
    {
        SdOptionsMiscItem aOptsItem( ATTR_OPTIONS_MISC );

        aOptsItem.GetOptionsMisc().SetStartWithTemplate( aCbxStartWithTemplate.IsChecked() );
        aOptsItem.GetOptionsMisc().SetMarkedHitMovesAlways( aCbxMarkedHitMovesAlways.IsChecked() );
        aOptsItem.GetOptionsMisc().SetCrookNoContortion( aCbxCrookNoContortion.IsChecked() );
        aOptsItem.GetOptionsMisc().SetQuickEdit( aCbxQuickEdit.IsChecked() );
        aOptsItem.GetOptionsMisc().SetPickThrough( aCbxPickThrough.IsChecked() );
        aOptsItem.GetOptionsMisc().SetMasterPagePaintCaching( aCbxMasterPageCache.IsChecked() );
        aOptsItem.GetOptionsMisc().SetDragWithCopy( aCbxCopy.IsChecked() );
        aOptsItem.GetOptionsMisc().SetStartWithActualPage( aCbxStartWithActualPage.IsChecked() );
        aOptsItem.GetOptionsMisc().SetEnableSdremote( aCbxEnableSdremote.IsChecked() );
        aOptsItem.GetOptionsMisc().SetSummationOfParagraphs( aCbxCompatibility.IsChecked() );
        aOptsItem.GetOptionsMisc().SetPrinterIndependentLayout (
            aCbxUsePrinterMetrics.IsChecked()
            ? ::com::sun::star::document::PrinterIndependentLayout::DISABLED
            : ::com::sun::star::document::PrinterIndependentLayout::ENABLED);
        rAttrs.Put( aOptsItem );

        bModified = sal_True;
    }

    // metric
    const sal_uInt16 nMPos = aLbMetric.GetSelectEntryPos();
    if ( nMPos != aLbMetric.GetSavedValue() )
    {
        sal_uInt16 nFieldUnit = (sal_uInt16)(long)aLbMetric.GetEntryData( nMPos );
        rAttrs.Put( SfxUInt16Item( GetWhich( SID_ATTR_METRIC ),
                                     (sal_uInt16)nFieldUnit ) );
        bModified |= sal_True;
    }

    // tabulator space
    if( aMtrFldTabstop.GetText() != aMtrFldTabstop.GetSavedValue() )
    {
        sal_uInt16 nWh = GetWhich( SID_ATTR_DEFTABSTOP );
        SfxMapUnit eUnit = rAttrs.GetPool()->GetMetric( nWh );
        SfxUInt16Item aDef( nWh,(sal_uInt16)GetCoreValue( aMtrFldTabstop, eUnit ) );
        rAttrs.Put( aDef );
        bModified |= sal_True;
    }

    sal_Int32 nX, nY;
    if( SetScale( aCbScale.GetText(), nX, nY ) )
    {
        rAttrs.Put( SfxInt32Item( ATTR_OPTIONS_SCALE_X, nX ) );
        rAttrs.Put( SfxInt32Item( ATTR_OPTIONS_SCALE_Y, nY ) );

        bModified = sal_True;
    }

    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsMisc::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsMiscItem aOptsItem( (const SdOptionsMiscItem&) rAttrs.
                        Get( ATTR_OPTIONS_MISC ) );

    aCbxStartWithTemplate.Check( aOptsItem.GetOptionsMisc().IsStartWithTemplate() );
    aCbxMarkedHitMovesAlways.Check( aOptsItem.GetOptionsMisc().IsMarkedHitMovesAlways() );
    aCbxCrookNoContortion.Check( aOptsItem.GetOptionsMisc().IsCrookNoContortion() );
    aCbxQuickEdit.Check( aOptsItem.GetOptionsMisc().IsQuickEdit() );
    aCbxPickThrough.Check( aOptsItem.GetOptionsMisc().IsPickThrough() );
    aCbxMasterPageCache.Check( aOptsItem.GetOptionsMisc().IsMasterPagePaintCaching() );
    aCbxCopy.Check( aOptsItem.GetOptionsMisc().IsDragWithCopy() );
    aCbxStartWithActualPage.Check( aOptsItem.GetOptionsMisc().IsStartWithActualPage() );
    aCbxEnableSdremote.Check( aOptsItem.GetOptionsMisc().IsEnableSdremote() );
    aCbxCompatibility.Check( aOptsItem.GetOptionsMisc().IsSummationOfParagraphs() );
    aCbxUsePrinterMetrics.Check( aOptsItem.GetOptionsMisc().GetPrinterIndependentLayout()==1 );
    aCbxStartWithTemplate.SaveValue();
    aCbxMarkedHitMovesAlways.SaveValue();
    aCbxCrookNoContortion.SaveValue();
    aCbxQuickEdit.SaveValue();
    aCbxPickThrough.SaveValue();

    aCbxMasterPageCache.SaveValue();
    aCbxCopy.SaveValue();
    aCbxEnableSdremote.SaveValue();
    aCbxCompatibility.SaveValue();
    aCbxUsePrinterMetrics.SaveValue();

    // metric
    sal_uInt16 nWhich = GetWhich( SID_ATTR_METRIC );
    aLbMetric.SetNoSelection();

    if ( rAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rAttrs.Get( nWhich );
        long nFieldUnit = (long)rItem.GetValue();

        for ( sal_uInt16 i = 0; i < aLbMetric.GetEntryCount(); ++i )
        {
            if ( (long)aLbMetric.GetEntryData( i ) == nFieldUnit )
            {
                aLbMetric.SelectEntryPos( i );
                break;
            }
        }
    }

    // tabulator space
    nWhich = GetWhich( SID_ATTR_DEFTABSTOP );
    if( rAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = rAttrs.GetPool()->GetMetric( nWhich );
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rAttrs.Get( nWhich );
        SetMetricValue( aMtrFldTabstop, rItem.GetValue(), eUnit );
    }
    aLbMetric.SaveValue();
    aMtrFldTabstop.SaveValue();
    //Scale
    sal_Int32 nX = ( (const SfxInt32Item&) rAttrs.
                 Get( ATTR_OPTIONS_SCALE_X ) ).GetValue();
    sal_Int32 nY = ( (const SfxInt32Item&) rAttrs.
                 Get( ATTR_OPTIONS_SCALE_Y ) ).GetValue();
    nWidth = ( (const SfxUInt32Item&) rAttrs.
                    Get( ATTR_OPTIONS_SCALE_WIDTH ) ).GetValue();
    nHeight = ( (const SfxUInt32Item&) rAttrs.
                    Get( ATTR_OPTIONS_SCALE_HEIGHT ) ).GetValue();

    aCbScale.SetText( GetScale( nX, nY ) );

    aFtOriginal.Hide();
    aFtEquivalent.Hide();
    aMtrFldOriginalWidth.Hide();
    aMtrFldOriginalWidth.SetText( aInfo1 ); // empty
    aMtrFldOriginalHeight.Hide();
    aMtrFldOriginalHeight.SetText( aInfo2 ); //empty
    aFtPageWidth.Hide();
    aFtPageHeight.Hide();
    aFiInfo1.Hide();
    aFiInfo2.Hide();

    UpdateCompatibilityControls ();
}

// -----------------------------------------------------------------------

SfxTabPage* SdTpOptionsMisc::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsMisc( pWindow, rAttrs ) );
}
//------------------------------------------------------------------------

IMPL_LINK_NOARG(SdTpOptionsMisc, SelectMetricHdl_Impl)
{
    sal_uInt16 nPos = aLbMetric.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        FieldUnit eUnit = (FieldUnit)(long)aLbMetric.GetEntryData( nPos );
        sal_Int64 nVal =
            aMtrFldTabstop.Denormalize( aMtrFldTabstop.GetValue( FUNIT_TWIP ) );
        SetFieldUnit( aMtrFldTabstop, eUnit );
        aMtrFldTabstop.SetValue( aMtrFldTabstop.Normalize( nVal ), FUNIT_TWIP );
    }
    return 0;
}


namespace {
void lcl_MoveWin( Window& rWin, long nYDiff)
{
    Point aPos(rWin.GetPosPixel());
    aPos.Y() += nYDiff;
    rWin.SetPosPixel (aPos);
}

void lcl_MoveWin( Window& rWin, long nXdiff, long nYdiff)
{
    Point aPos(rWin.GetPosPixel());
    aPos.X() += nXdiff;
    aPos.Y() += nYdiff;
    rWin.SetPosPixel(aPos);
}
}

void SdTpOptionsMisc::SetImpressMode (void)
{
    long nDialogWidth = GetSizePixel().Width();
    long nLineHeight = aCbxPickThrough.GetPosPixel().Y()
        - aCbxQuickEdit.GetPosPixel().Y();

    // Put both "Text objects" check boxes side by side.
    lcl_MoveWin (aCbxPickThrough,
        nDialogWidth/2 - aCbxPickThrough.GetPosPixel().X(),
        -nLineHeight);

    // Move the other controls up one line.
    lcl_MoveWin (aGrpProgramStart, -nLineHeight);
    lcl_MoveWin (aCbxStartWithTemplate, -nLineHeight);
    lcl_MoveWin (aGrpSettings, -nLineHeight);
    lcl_MoveWin (aCbxMasterPageCache, -nLineHeight);
    lcl_MoveWin (aCbxCopy, -nLineHeight);
    lcl_MoveWin (aCbxMarkedHitMovesAlways, -nLineHeight);
    lcl_MoveWin (aCbxCrookNoContortion, -nLineHeight);
    lcl_MoveWin (aTxtMetric, -nLineHeight);
    lcl_MoveWin (aLbMetric, -nLineHeight);
    lcl_MoveWin (aTxtTabstop, -nLineHeight);
    lcl_MoveWin (aMtrFldTabstop, -nLineHeight);
    lcl_MoveWin (aGrpStartWithActualPage, -nLineHeight);
    lcl_MoveWin (aCbxStartWithActualPage, -nLineHeight);
    lcl_MoveWin (aCbxEnableSdremote, -nLineHeight);
    lcl_MoveWin (aTxtCompatibility, -nLineHeight);

    // Move the printer-independent-metrics check box up two lines to change
    // places with spacing-between-paragraphs check box.
    lcl_MoveWin (aCbxUsePrinterMetrics, -2*nLineHeight);
#ifndef ENABLE_SDREMOTE_BLUETOOTH
    aCbxEnableSdremote.Hide();
#endif
}

void    SdTpOptionsMisc::SetDrawMode()
{
    aCbxStartWithTemplate.Hide();
    aGrpProgramStart.Hide();
    aCbxStartWithActualPage.Hide();
    aCbxEnableSdremote.Hide();
    aCbxCompatibility.Hide();
    aGrpStartWithActualPage.Hide();
    aCbxCrookNoContortion.Show();

    aGrpScale.Show();
    aFtScale.Show();
    aCbScale.Show();

    aFtOriginal.Show();
    aFtEquivalent.Show();

    aFtPageWidth.Show();
    aFiInfo1.Show();
    aMtrFldOriginalWidth.Show();

    aFtPageHeight.Show();
    aFiInfo2.Show();
    aMtrFldOriginalHeight.Show();

    long nDiff = aGrpSettings.GetPosPixel().Y() - aGrpProgramStart.GetPosPixel().Y();
    lcl_MoveWin( aGrpSettings, -nDiff );
    lcl_MoveWin( aCbxMasterPageCache, -nDiff );
    lcl_MoveWin( aCbxCopy, -nDiff );
    lcl_MoveWin( aCbxMarkedHitMovesAlways, -nDiff );
    lcl_MoveWin( aCbxCrookNoContortion, -nDiff );
    nDiff -= aCbxCrookNoContortion.GetPosPixel().Y() - aCbxMarkedHitMovesAlways.GetPosPixel().Y();
    lcl_MoveWin( aTxtMetric, -nDiff );
    lcl_MoveWin( aLbMetric, -nDiff );
    lcl_MoveWin( aTxtTabstop, -nDiff );
    lcl_MoveWin( aMtrFldTabstop, -nDiff );

    // Move the scale controls so that they are visually centered between the
    // group controls above and below.
    lcl_MoveWin (aFtScale, -17);
    lcl_MoveWin (aCbScale, -17);

    // Move the printer-independent-metrics check box in the place that the
    // spacing-between-paragraphs check box normally is in.
    aCbxUsePrinterMetrics.SetPosPixel (aCbxCompatibility.GetPosPixel());
}

// -----------------------------------------------------------------------

String SdTpOptionsMisc::GetScale( sal_Int32 nX, sal_Int32 nY )
{
    String aScale( OUString::valueOf( nX ) );
    aScale.Append( TOKEN );
    aScale.Append( OUString::valueOf( nY ) );

    return( aScale );
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsMisc::SetScale( const String& aScale, sal_Int32& rX, sal_Int32& rY )
{
    if( comphelper::string::getTokenCount(aScale, TOKEN) != 2 )
        return( sal_False );

    OUString aTmp(aScale.GetToken( 0, TOKEN ));
    if (!comphelper::string::isdigitAsciiString(aTmp))
        return sal_False;

    rX = (long) aTmp.toInt32();
    if( rX == 0 )
        return( sal_False );

    aTmp = aScale.GetToken( 1, TOKEN );
    if (!comphelper::string::isdigitAsciiString(aTmp))
        return sal_False;

    rY = (long) aTmp.toInt32();
    if( rY == 0 )
        return sal_False;

    return sal_True;
}

void SdTpOptionsMisc::UpdateCompatibilityControls (void)
{
    // Disable the compatibility controls by default.  Enable them only when
    // there is at least one open document.
    sal_Bool bIsEnabled = sal_False;

    try
    {
        // Get a component enumeration from the desktop and search it for documents.
        Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext());
        do
        {
            Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);

            Reference<container::XEnumerationAccess> xComponents (
                xDesktop->getComponents(), UNO_QUERY);
            if ( ! xComponents.is())
                break;

            Reference<container::XEnumeration> xEnumeration (
                xComponents->createEnumeration());
            if ( ! xEnumeration.is())
                break;

            while (xEnumeration->hasMoreElements())
            {
                Reference<frame::XModel> xModel (xEnumeration->nextElement(), UNO_QUERY);
                if (xModel.is())
                {
                    // There is at leas one model/document: Enable the compatibility controls.
                    bIsEnabled = sal_True;
                    break;
                }
            }

        }
        while (false); // One 'loop'.
    }
    catch (const uno::Exception&)
    {
        // When there is an exception then simply use the default value of
        // bIsEnabled and disable the controls.
    }

    aTxtCompatibility.Enable (bIsEnabled);
    aCbxCompatibility.Enable(bIsEnabled);
    aCbxUsePrinterMetrics.Enable (bIsEnabled);
}

void SdTpOptionsMisc::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_SDMODE_FLAG,sal_False);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SD_DRAW_MODE ) == SD_DRAW_MODE )
            SetDrawMode();
        if ( ( nFlags & SD_IMPRESS_MODE ) == SD_IMPRESS_MODE )
            SetImpressMode();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
