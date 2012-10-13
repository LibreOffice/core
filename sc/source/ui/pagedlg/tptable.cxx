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


#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "scitems.hxx"

#include "tptable.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "pagedlg.hrc"

// =======================================================================

void EmptyNumericField::Modify()
{
    if( GetText().Len() )
        NumericField::Modify();
    else
        SetEmptyFieldValue();
}

void EmptyNumericField::SetValue( sal_Int64 nValue )
{
    if( nValue == 0 )
        SetEmptyFieldValue();
    else
        NumericField::SetValue( nValue );
}

sal_Int64 EmptyNumericField::GetValue() const
{
    return IsEmptyFieldValue() ? 0 : NumericField::GetValue();
}

// =======================================================================

// STATIC DATA -----------------------------------------------------------

static sal_uInt16 pPageTableRanges[] =
{
    ATTR_PAGE_NOTES, ATTR_PAGE_FIRSTPAGENO,
    0
};

static sal_Bool lcl_PutVObjModeItem( sal_uInt16            nWhich,
                          SfxItemSet&       rCoreSet,
                          const SfxItemSet& rOldSet,
                          const CheckBox&   rBtn );

static sal_Bool lcl_PutScaleItem( sal_uInt16               nWhich,
                       SfxItemSet&          rCoreSet,
                       const SfxItemSet&    rOldSet,
                       const ListBox&       rListBox,
                       sal_uInt16               nLBEntry,
                       const SpinField&     rEd,
                       sal_uInt16               nValue );

static sal_Bool lcl_PutScaleItem2( sal_uInt16               nWhich,
                       SfxItemSet&          rCoreSet,
                       const SfxItemSet&    rOldSet,
                       const ListBox&       rListBox,
                       sal_uInt16               nLBEntry,
                       const NumericField&  rEd1,
                       const NumericField&  rEd2 );

static sal_Bool lcl_PutBoolItem( sal_uInt16            nWhich,
                      SfxItemSet&       rCoreSet,
                      const SfxItemSet& rOldSet,
                      sal_Bool              bIsChecked,
                      sal_Bool              bSavedValue );

//------------------------------------------------------------------------

#define PAGENO_HDL          LINK(this,ScTablePage,PageNoHdl)
#define PAGEDIR_HDL         LINK(this,ScTablePage,PageDirHdl)
#define SCALE_HDL           LINK(this,ScTablePage,ScaleHdl)

#define WAS_DEFAULT(w,s)    (SFX_ITEM_DEFAULT==(s).GetItemState((w),sal_True))
#define GET_BOOL(sid,set)   ((const SfxBoolItem&)((set).Get(GetWhich((sid))))).GetValue()
#define GET_USHORT(sid,set) (sal_uInt16)((const SfxUInt16Item&)((set).Get(GetWhich((sid))))).GetValue()
#define GET_SHOW(sid,set)   ( ScVObjMode( ((const ScViewObjectModeItem&)((set).Get(GetWhich((sid))))).GetValue() ) \
                              == VOBJ_MODE_SHOW )

//========================================================================

ScTablePage::ScTablePage( Window* pParent, const SfxItemSet& rCoreAttrs ) :

        SfxTabPage( pParent, ScResId( RID_SCPAGE_TABLE ), rCoreAttrs ),

        aFlPageDir          ( this, ScResId( FL_PAGEDIR ) ),
        aBtnTopDown         ( this, ScResId( BTN_TOPDOWN ) ),
        aBtnLeftRight       ( this, ScResId( BTN_LEFTRIGHT ) ),
        aBmpPageDir         ( this, ScResId( BMP_PAGEDIR ) ),
        aImgLeftRight       (       ScResId( IMG_LEFTRIGHT ) ),
        aImgTopDown         (       ScResId( IMG_TOPDOWN ) ),
        aBtnPageNo          ( this, ScResId( BTN_PAGENO ) ),
        aEdPageNo           ( this, ScResId( ED_PAGENO ) ),
        aFlPrint            ( this, ScResId( FL_PRINT ) ),
        aBtnHeaders         ( this, ScResId( BTN_HEADER ) ),
        aBtnGrid            ( this, ScResId( BTN_GRID ) ),
        aBtnNotes           ( this, ScResId( BTN_NOTES ) ),
        aBtnObjects         ( this, ScResId( BTN_OBJECTS ) ),
        aBtnCharts          ( this, ScResId( BTN_CHARTS ) ),
        aBtnDrawings        ( this, ScResId( BTN_DRAWINGS ) ),
        aBtnFormulas        ( this, ScResId( BTN_FORMULAS ) ),
        aBtnNullVals        ( this, ScResId( BTN_NULLVALS ) ),
        aFlScale            ( this, ScResId( FL_SCALE ) ),
        aFtScaleMode        ( this, ScResId( FT_SCALEMODE ) ),
        aLbScaleMode        ( this, ScResId( LB_SCALEMODE ) ),
        aFtScaleAll         ( this, ScResId( FT_SCALEFACTOR ) ),
        aEdScaleAll         ( this, ScResId( ED_SCALEALL ) ),
        aFtScalePageWidth   ( this, ScResId( FT_SCALEPAGEWIDTH ) ),
        aEdScalePageWidth   ( this, ScResId( ED_SCALEPAGEWIDTH ) ),
        aFtScalePageHeight  ( this, ScResId( FT_SCALEPAGEHEIGHT ) ),
        aEdScalePageHeight  ( this, ScResId( ED_SCALEPAGEHEIGHT ) ),
        aFtScalePageNum     ( this, ScResId( FT_SCALEPAGENUM ) ),
        aEdScalePageNum     ( this, ScResId( ED_SCALEPAGENUM ) )
{
    SetExchangeSupport();
    aBtnPageNo.SetClickHdl( PAGENO_HDL );
    aBtnTopDown.SetClickHdl( PAGEDIR_HDL );
    aBtnLeftRight.SetClickHdl( PAGEDIR_HDL );
    aLbScaleMode.SetSelectHdl( SCALE_HDL );

    Size aBmpSize = Image( ScResId( IMG_LEFTRIGHT ) ).GetSizePixel();
    aBmpPageDir.SetOutputSizePixel( aBmpSize );

    FreeResource();

    aEdPageNo.SetAccessibleName(aBtnPageNo.GetText());
    aEdPageNo.SetAccessibleRelationLabeledBy(&aBtnPageNo);
}

// -----------------------------------------------------------------------

void ScTablePage::ShowImage()
{
    bool bLeftRight = aBtnLeftRight.IsChecked();
    aBmpPageDir.SetImage( (bLeftRight ? aImgLeftRight : aImgTopDown) );
}

// -----------------------------------------------------------------------

ScTablePage::~ScTablePage()
{
}

//------------------------------------------------------------------------

sal_uInt16* ScTablePage::GetRanges()
{
    return pPageTableRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* ScTablePage::Create( Window* pParent, const SfxItemSet& rCoreSet )
{
    return ( new ScTablePage( pParent, rCoreSet ) );
}

// -----------------------------------------------------------------------

void ScTablePage::Reset( const SfxItemSet& rCoreSet )
{
    sal_Bool    bTopDown = GET_BOOL( SID_SCATTR_PAGE_TOPDOWN, rCoreSet );
    sal_uInt16  nWhich   = 0;

    //-----------
    // sal_Bool-Flags
    //-----------
    aBtnNotes       .Check( GET_BOOL(SID_SCATTR_PAGE_NOTES,rCoreSet) );
    aBtnGrid        .Check( GET_BOOL(SID_SCATTR_PAGE_GRID,rCoreSet) );
    aBtnHeaders     .Check( GET_BOOL(SID_SCATTR_PAGE_HEADERS,rCoreSet) );
    aBtnFormulas    .Check( GET_BOOL(SID_SCATTR_PAGE_FORMULAS,rCoreSet) );
    aBtnNullVals    .Check( GET_BOOL(SID_SCATTR_PAGE_NULLVALS,rCoreSet) );
    aBtnTopDown     .Check( bTopDown );
    aBtnLeftRight   .Check( !bTopDown );

    //------------------
    // Erste Druckseite:
    //------------------
    sal_uInt16 nPage = GET_USHORT(SID_SCATTR_PAGE_FIRSTPAGENO,rCoreSet);
    aBtnPageNo.Check( nPage != 0 );
    aEdPageNo.SetValue( (nPage != 0) ? nPage : 1 );
    PageNoHdl( NULL );

    //-------------------
    // Objektdarstellung:
    //-------------------
    aBtnCharts      .Check( GET_SHOW( SID_SCATTR_PAGE_CHARTS, rCoreSet ) );
    aBtnObjects     .Check( GET_SHOW( SID_SCATTR_PAGE_OBJECTS, rCoreSet ) );
    aBtnDrawings    .Check( GET_SHOW( SID_SCATTR_PAGE_DRAWINGS, rCoreSet ) );

    //------------
    // Skalierung:
    //------------

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALE);
    if ( rCoreSet.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nScale = ((const SfxUInt16Item&)rCoreSet.Get(nWhich)).GetValue();
        if( nScale > 0 )
            aLbScaleMode.SelectEntryPos( SC_TPTABLE_SCALE_PERCENT );
        aEdScaleAll.SetValue( (nScale > 0) ? nScale : 100 );
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETO);
    if ( rCoreSet.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        const ScPageScaleToItem& rItem = static_cast< const ScPageScaleToItem& >( rCoreSet.Get( nWhich ) );
        sal_uInt16 nWidth = rItem.GetWidth();
        sal_uInt16 nHeight = rItem.GetHeight();

        /*  width==0 and height==0 is invalid state, used as "not selected".
            Dialog shows width=height=1 then. */
        bool bValid = nWidth || nHeight;
        if( bValid )
            aLbScaleMode.SelectEntryPos( SC_TPTABLE_SCALE_TO );
        aEdScalePageWidth.SetValue( bValid ? nWidth : 1 );
        aEdScalePageHeight.SetValue( bValid ? nHeight : 1 );
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETOPAGES);
    if ( rCoreSet.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nPages = ((const SfxUInt16Item&)rCoreSet.Get(nWhich)).GetValue();
        if( nPages > 0 )
            aLbScaleMode.SelectEntryPos( SC_TPTABLE_SCALE_TO_PAGES );
        aEdScalePageNum.SetValue( (nPages > 0) ? nPages : 1 );
    }

    if( aLbScaleMode.GetSelectEntryCount() == 0 )
    {
        // fall back to 100%
        OSL_FAIL( "ScTablePage::Reset - missing scaling item" );
        aLbScaleMode.SelectEntryPos( SC_TPTABLE_SCALE_PERCENT );
        aEdScaleAll.SetValue( 100 );
    }

    PageDirHdl( NULL );
    ScaleHdl( NULL );

    // merken fuer FillItemSet
    aBtnFormulas    .SaveValue();
    aBtnNullVals    .SaveValue();
    aBtnNotes       .SaveValue();
    aBtnGrid        .SaveValue();
    aBtnHeaders     .SaveValue();
    aBtnTopDown     .SaveValue();
    aBtnLeftRight   .SaveValue();
    aLbScaleMode    .SaveValue();
    aBtnCharts      .SaveValue();
    aBtnObjects     .SaveValue();
    aBtnDrawings    .SaveValue();
    aBtnPageNo      .SaveValue();
    aEdPageNo       .SaveValue();
    aEdScaleAll     .SaveValue();
    aEdScalePageWidth.SaveValue();
    aEdScalePageHeight.SaveValue();
    aEdScalePageNum .SaveValue();
}

// -----------------------------------------------------------------------

sal_Bool ScTablePage::FillItemSet( SfxItemSet& rCoreSet )
{
    const SfxItemSet&   rOldSet      = GetItemSet();
    sal_uInt16              nWhichPageNo = GetWhich(SID_SCATTR_PAGE_FIRSTPAGENO);
    sal_Bool                bDataChanged = false;

    //-----------
    // sal_Bool-Flags
    //-----------

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NOTES),
                                     rCoreSet, rOldSet,
                                     aBtnNotes.IsChecked(),
                                     aBtnNotes.GetSavedValue() != STATE_NOCHECK );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_GRID),
                                     rCoreSet, rOldSet,
                                     aBtnGrid.IsChecked(),
                                     aBtnGrid.GetSavedValue() != STATE_NOCHECK );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_HEADERS),
                                     rCoreSet, rOldSet,
                                     aBtnHeaders.IsChecked(),
                                     aBtnHeaders.GetSavedValue() != STATE_NOCHECK );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_TOPDOWN),
                                     rCoreSet, rOldSet,
                                     aBtnTopDown.IsChecked(),
                                     aBtnTopDown.GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_FORMULAS),
                                     rCoreSet, rOldSet,
                                     aBtnFormulas.IsChecked(),
                                     aBtnFormulas.GetSavedValue() != STATE_NOCHECK );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NULLVALS),
                                     rCoreSet, rOldSet,
                                     aBtnNullVals.IsChecked(),
                                     aBtnNullVals.GetSavedValue() != STATE_NOCHECK );

    //------------------
    // Erste Druckseite:
    //------------------
    sal_Bool bUseValue = aBtnPageNo.IsChecked();

    if (   WAS_DEFAULT(nWhichPageNo,rOldSet)
        && (    (!bUseValue && bUseValue == aBtnPageNo.GetSavedValue())
            || (   bUseValue && bUseValue == aBtnPageNo.GetSavedValue()
                && aEdPageNo.GetText() == aEdPageNo.GetSavedValue() ) ) )
    {
            rCoreSet.ClearItem( nWhichPageNo );
    }
    else
    {
        sal_uInt16 nPage = (sal_uInt16)( aBtnPageNo.IsChecked()
                                    ? aEdPageNo.GetValue()
                                    : 0 );

        rCoreSet.Put( SfxUInt16Item( nWhichPageNo, nPage ) );
        bDataChanged = sal_True;
    }

    //-------------------
    // Objektdarstellung:
    //-------------------

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_CHARTS),
                                         rCoreSet, rOldSet, aBtnCharts );

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_OBJECTS),
                                         rCoreSet, rOldSet, aBtnObjects );

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_DRAWINGS),
                                         rCoreSet, rOldSet, aBtnDrawings );

    //------------
    // Skalierung:
    //------------

    if( !aEdScalePageWidth.GetValue() && !aEdScalePageHeight.GetValue() )
    {
        aLbScaleMode.SelectEntryPos( SC_TPTABLE_SCALE_PERCENT );
        aEdScaleAll.SetValue( 100 );
    }

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALE),
                                      rCoreSet, rOldSet,
                                      aLbScaleMode, SC_TPTABLE_SCALE_PERCENT,
                                      aEdScaleAll, (sal_uInt16)aEdScaleAll.GetValue() );

    bDataChanged |= lcl_PutScaleItem2( GetWhich(SID_SCATTR_PAGE_SCALETO),
                                      rCoreSet, rOldSet,
                                      aLbScaleMode, SC_TPTABLE_SCALE_TO,
                                      aEdScalePageWidth, aEdScalePageHeight );

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALETOPAGES),
                                      rCoreSet, rOldSet,
                                      aLbScaleMode, SC_TPTABLE_SCALE_TO_PAGES,
                                      aEdScalePageNum, (sal_uInt16)aEdScalePageNum.GetValue() );

    return bDataChanged;
}

//------------------------------------------------------------------------

int ScTablePage::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( *pSetP );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

void ScTablePage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        ShowImage();
    SfxTabPage::DataChanged( rDCEvt );
}

//------------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScTablePage, PageDirHdl)
{
    ShowImage();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScTablePage, PageNoHdl, CheckBox*, pBtn )
{
    if ( aBtnPageNo.IsChecked() )
    {
        aEdPageNo.Enable();
        if ( pBtn )
            aEdPageNo.GrabFocus();
    }
    else
        aEdPageNo.Disable();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScTablePage, ScaleHdl)
{
    // controls for "Reduce/enlarge"
    bool bPercent = (aLbScaleMode.GetSelectEntryPos() == SC_TPTABLE_SCALE_PERCENT);
    aFtScaleAll.Show( bPercent );
    aEdScaleAll.Show( bPercent );

    // controls for "Scale to width/height"
    bool bScaleTo = (aLbScaleMode.GetSelectEntryPos() == SC_TPTABLE_SCALE_TO);
    aFtScalePageWidth.Show( bScaleTo );
    aEdScalePageWidth.Show( bScaleTo );
    aFtScalePageHeight.Show( bScaleTo );
    aEdScalePageHeight.Show( bScaleTo );

    // controls for "Scale to pages"
    bool bScalePages = (aLbScaleMode.GetSelectEntryPos() == SC_TPTABLE_SCALE_TO_PAGES);
    aFtScalePageNum.Show( bScalePages );
    aEdScalePageNum.Show( bScalePages );

    return 0;
}

//========================================================================
// Hilfsfunktionen fuer FillItemSet:
//========================================================================

static sal_Bool lcl_PutBoolItem( sal_uInt16            nWhich,
                     SfxItemSet&        rCoreSet,
                     const SfxItemSet&  rOldSet,
                     sal_Bool               bIsChecked,
                     sal_Bool               bSavedValue )
{
    sal_Bool bDataChanged = (   bSavedValue == bIsChecked
                         && WAS_DEFAULT(nWhich,rOldSet) );

    if ( bDataChanged )
        rCoreSet.ClearItem(nWhich);
    else
        rCoreSet.Put( SfxBoolItem( nWhich, bIsChecked ) );

    return bDataChanged;
}

//------------------------------------------------------------------------

static sal_Bool lcl_PutVObjModeItem( sal_uInt16            nWhich,
                         SfxItemSet&        rCoreSet,
                         const SfxItemSet&  rOldSet,
                         const CheckBox&    rBtn )
{
    sal_Bool bIsChecked   = rBtn.IsChecked();
    sal_Bool bDataChanged = (   rBtn.GetSavedValue() == bIsChecked
                         && WAS_DEFAULT(nWhich,rOldSet) );

    if ( bDataChanged )
        rCoreSet.ClearItem( nWhich );

    else
        rCoreSet.Put( ScViewObjectModeItem( nWhich, bIsChecked
                                                    ? VOBJ_MODE_SHOW
                                                    : VOBJ_MODE_HIDE ) );
    return bDataChanged;
}

//------------------------------------------------------------------------

static sal_Bool lcl_PutScaleItem( sal_uInt16               nWhich,
                      SfxItemSet&           rCoreSet,
                      const SfxItemSet&     rOldSet,
                      const ListBox&        rListBox,
                      sal_uInt16                nLBEntry,
                      const SpinField&      rEd,
                      sal_uInt16                nValue )
{
    sal_Bool bIsSel = (rListBox.GetSelectEntryPos() == nLBEntry);
    sal_Bool bDataChanged = (rListBox.GetSavedValue() != nLBEntry) ||
                        (rEd.GetSavedValue() != rEd.GetText()) ||
                        !WAS_DEFAULT( nWhich, rOldSet );

    if( bDataChanged )
        rCoreSet.Put( SfxUInt16Item( nWhich, bIsSel ? nValue : 0 ) );
    else
        rCoreSet.ClearItem( nWhich );

    return bDataChanged;
}


static sal_Bool lcl_PutScaleItem2( sal_uInt16               nWhich,
                      SfxItemSet&           rCoreSet,
                      const SfxItemSet&     rOldSet,
                      const ListBox&        rListBox,
                      sal_uInt16                nLBEntry,
                      const NumericField&   rEd1,
                      const NumericField&   rEd2 )
{
    sal_uInt16 nValue1 = (sal_uInt16)rEd1.GetValue();
    sal_uInt16 nValue2 = (sal_uInt16)rEd2.GetValue();
    sal_Bool bIsSel = (rListBox.GetSelectEntryPos() == nLBEntry);
    sal_Bool bDataChanged = (rListBox.GetSavedValue() != nLBEntry) ||
                        (rEd1.GetSavedValue() != rEd1.GetText()) ||
                        (rEd2.GetSavedValue() != rEd2.GetText()) ||
                        !WAS_DEFAULT( nWhich, rOldSet );

    if( bDataChanged )
    {
        ScPageScaleToItem aItem;
        if( bIsSel )
            aItem.Set( nValue1, nValue2 );
        rCoreSet.Put( aItem );
    }
    else
        rCoreSet.ClearItem( nWhich );

    return bDataChanged;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
