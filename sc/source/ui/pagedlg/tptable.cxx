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

#undef SC_DLLIMPLEMENTATION

#include "scitems.hxx"

#include <vcl/settings.hxx>

#include "tptable.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "pagedlg.hrc"


// Static Data

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


#define PAGENO_HDL          LINK(this,ScTablePage,PageNoHdl)
#define PAGEDIR_HDL         LINK(this,ScTablePage,PageDirHdl)
#define SCALE_HDL           LINK(this,ScTablePage,ScaleHdl)

#define WAS_DEFAULT(w,s)    (SFX_ITEM_DEFAULT==(s).GetItemState((w),true))
#define GET_BOOL(sid,set)   ((const SfxBoolItem&)((set).Get(GetWhich((sid))))).GetValue()
#define GET_USHORT(sid,set) (sal_uInt16)((const SfxUInt16Item&)((set).Get(GetWhich((sid))))).GetValue()
#define GET_SHOW(sid,set)   ( ScVObjMode( ((const ScViewObjectModeItem&)((set).Get(GetWhich((sid))))).GetValue() ) \
                              == VOBJ_MODE_SHOW )
// List box entries "Scaling mode"
#define SC_TPTABLE_SCALE_PERCENT    0
#define SC_TPTABLE_SCALE_TO         1
#define SC_TPTABLE_SCALE_TO_PAGES   2


ScTablePage::ScTablePage( Window* pParent, const SfxItemSet& rCoreAttrs ) :

        SfxTabPage( pParent, "SheetPrintPage","modules/scalc/ui/sheetprintpage.ui", rCoreAttrs )
{
    get(m_pBtnTopDown,"radioBTN_TOPDOWN");
    get(m_pBtnLeftRight,"radioBTN_LEFTRIGHT");
    get(m_pBmpPageDir,"imageBMP_PAGEDIR");
    get(m_pBtnPageNo,"checkBTN_PAGENO");
    get(m_pEdPageNo,"spinED_PAGENO");

    get(m_pBtnHeaders,"checkBTN_HEADER");
    get(m_pBtnGrid,"checkBTN_GRID");
    get(m_pBtnNotes,"checkBTN_NOTES");
    get(m_pBtnObjects,"checkBTN_OBJECTS");
    get(m_pBtnCharts,"checkBTN_CHARTS");
    get(m_pBtnDrawings,"checkBTN_DRAWINGS");
    get(m_pBtnFormulas,"checkBTN_FORMULAS");
    get(m_pBtnNullVals,"checkBTN_NULLVALS");

    get(m_pLbScaleMode,"comboLB_SCALEMODE");
    get(m_pBxScaleAll,"boxSCALEALL");
    get(m_pEdScaleAll,"spinED_SCALEALL");
    get(m_pGrHeightWidth,"gridWH");
    get(m_pEdScalePageWidth,"spinED_SCALEPAGEWIDTH");
    get(m_pEdScalePageHeight,"spinED_SCALEPAGEHEIGHT");
    get(m_pBxScalePageNum,"boxNP");
    get(m_pEdScalePageNum,"spinED_SCALEPAGENUM");

    SetExchangeSupport();

    m_pBtnPageNo->SetClickHdl( PAGENO_HDL );
    m_pBtnTopDown->SetClickHdl( PAGEDIR_HDL );
    m_pBtnLeftRight->SetClickHdl( PAGEDIR_HDL );
    m_pLbScaleMode->SetSelectHdl( SCALE_HDL );

}

void ScTablePage::ShowImage()
{
    Image aImg = Image( ScResId( (m_pBtnLeftRight->IsChecked()) ? IMG_LEFTRIGHT : IMG_TOPDOWN ) );
    m_pBmpPageDir->SetImage( aImg );
    m_pBmpPageDir->SetOutputSizePixel( aImg.GetSizePixel() );
}

ScTablePage::~ScTablePage()
{
}

sal_uInt16* ScTablePage::GetRanges()
{
    return pPageTableRanges;
}

SfxTabPage* ScTablePage::Create( Window* pParent, const SfxItemSet& rCoreSet )
{
    return ( new ScTablePage( pParent, rCoreSet ) );
}

void ScTablePage::Reset( const SfxItemSet& rCoreSet )
{
    sal_Bool    bTopDown = GET_BOOL( SID_SCATTR_PAGE_TOPDOWN, rCoreSet );
    sal_uInt16  nWhich   = 0;

    // sal_Bool flags
    m_pBtnNotes->Check( GET_BOOL(SID_SCATTR_PAGE_NOTES,rCoreSet) );
    m_pBtnGrid->Check( GET_BOOL(SID_SCATTR_PAGE_GRID,rCoreSet) );
    m_pBtnHeaders->Check( GET_BOOL(SID_SCATTR_PAGE_HEADERS,rCoreSet) );
    m_pBtnFormulas->Check( GET_BOOL(SID_SCATTR_PAGE_FORMULAS,rCoreSet) );
    m_pBtnNullVals->Check( GET_BOOL(SID_SCATTR_PAGE_NULLVALS,rCoreSet) );
    m_pBtnTopDown->Check( bTopDown );
    m_pBtnLeftRight->Check( !bTopDown );

    // first printed page:
    sal_uInt16 nPage = GET_USHORT(SID_SCATTR_PAGE_FIRSTPAGENO,rCoreSet);
    m_pBtnPageNo->Check( nPage != 0 );
    m_pEdPageNo->SetValue( (nPage != 0) ? nPage : 1 );
    PageNoHdl( NULL );

    // object representation:
    m_pBtnCharts->Check( GET_SHOW( SID_SCATTR_PAGE_CHARTS, rCoreSet ) );
    m_pBtnObjects->Check( GET_SHOW( SID_SCATTR_PAGE_OBJECTS, rCoreSet ) );
    m_pBtnDrawings->Check( GET_SHOW( SID_SCATTR_PAGE_DRAWINGS, rCoreSet ) );

    // scaling:
    nWhich = GetWhich(SID_SCATTR_PAGE_SCALE);
    if ( rCoreSet.GetItemState( nWhich, true ) >= SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nScale = ((const SfxUInt16Item&)rCoreSet.Get(nWhich)).GetValue();
        if( nScale > 0 )
            m_pLbScaleMode->SelectEntryPos( SC_TPTABLE_SCALE_PERCENT );
        m_pEdScaleAll->SetValue( (nScale > 0) ? nScale : 100 );
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETO);
    if ( rCoreSet.GetItemState( nWhich, true ) >= SFX_ITEM_AVAILABLE )
    {
        const ScPageScaleToItem& rItem = static_cast< const ScPageScaleToItem& >( rCoreSet.Get( nWhich ) );
        sal_uInt16 nWidth = rItem.GetWidth();
        sal_uInt16 nHeight = rItem.GetHeight();

        /*  width==0 and height==0 is invalid state, used as "not selected".
            Dialog shows width=height=1 then. */
        bool bValid = nWidth || nHeight;
        if( bValid )
            m_pLbScaleMode->SelectEntryPos( SC_TPTABLE_SCALE_TO );
        m_pEdScalePageWidth->SetValue( bValid ? nWidth : 1 );
        m_pEdScalePageHeight->SetValue( bValid ? nHeight : 1 );
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETOPAGES);
    if ( rCoreSet.GetItemState( nWhich, true ) >= SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nPages = ((const SfxUInt16Item&)rCoreSet.Get(nWhich)).GetValue();
        if( nPages > 0 )
            m_pLbScaleMode->SelectEntryPos( SC_TPTABLE_SCALE_TO_PAGES );
        m_pEdScalePageNum->SetValue( (nPages > 0) ? nPages : 1 );
    }

    if( m_pLbScaleMode->GetSelectEntryCount() == 0 )
    {
        // fall back to 100%
        OSL_FAIL( "ScTablePage::Reset - missing scaling item" );
        m_pLbScaleMode->SelectEntryPos( SC_TPTABLE_SCALE_PERCENT );
        m_pEdScaleAll->SetValue( 100 );
    }

    PageDirHdl( NULL );
    ScaleHdl( NULL );

    // remember for FillItemSet
    m_pBtnFormulas->SaveValue();
    m_pBtnNullVals->SaveValue();
    m_pBtnNotes->SaveValue();
    m_pBtnGrid->SaveValue();
    m_pBtnHeaders->SaveValue();
    m_pBtnTopDown->SaveValue();
    m_pBtnLeftRight->SaveValue();
    m_pLbScaleMode->SaveValue();
    m_pBtnCharts->SaveValue();
    m_pBtnObjects->SaveValue();
    m_pBtnDrawings->SaveValue();
    m_pBtnPageNo->SaveValue();
    m_pEdPageNo->SaveValue();
    m_pEdScaleAll->SaveValue();
    m_pEdScalePageWidth->SaveValue();
    m_pEdScalePageHeight->SaveValue();
    m_pEdScalePageNum->SaveValue();
}

sal_Bool ScTablePage::FillItemSet( SfxItemSet& rCoreSet )
{
    const SfxItemSet&   rOldSet      = GetItemSet();
    sal_uInt16              nWhichPageNo = GetWhich(SID_SCATTR_PAGE_FIRSTPAGENO);
    sal_Bool                bDataChanged = false;

    // sal_Bool flags
    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NOTES),
                                     rCoreSet, rOldSet,
                                     m_pBtnNotes->IsChecked(),
                                     m_pBtnNotes->GetSavedValue() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_GRID),
                                     rCoreSet, rOldSet,
                                     m_pBtnGrid->IsChecked(),
                                     m_pBtnGrid->GetSavedValue() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_HEADERS),
                                     rCoreSet, rOldSet,
                                     m_pBtnHeaders->IsChecked(),
                                     m_pBtnHeaders->GetSavedValue() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_TOPDOWN),
                                     rCoreSet, rOldSet,
                                     m_pBtnTopDown->IsChecked(),
                                     m_pBtnTopDown->GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_FORMULAS),
                                     rCoreSet, rOldSet,
                                     m_pBtnFormulas->IsChecked(),
                                     m_pBtnFormulas->GetSavedValue() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NULLVALS),
                                     rCoreSet, rOldSet,
                                     m_pBtnNullVals->IsChecked(),
                                     m_pBtnNullVals->GetSavedValue() != TRISTATE_FALSE );

    // first printed page:
    sal_Bool bUseValue = m_pBtnPageNo->IsChecked();

    if (   WAS_DEFAULT(nWhichPageNo,rOldSet)
        && (    (!bUseValue && bUseValue == m_pBtnPageNo->GetSavedValue())
            || (   bUseValue && bUseValue == m_pBtnPageNo->GetSavedValue()
                   && m_pEdPageNo->GetText() == m_pEdPageNo->GetSavedValue() ) ) )
    {
            rCoreSet.ClearItem( nWhichPageNo );
    }
    else
    {
        sal_uInt16 nPage = (sal_uInt16)( m_pBtnPageNo->IsChecked()
                                    ? m_pEdPageNo->GetValue()
                                    : 0 );

        rCoreSet.Put( SfxUInt16Item( nWhichPageNo, nPage ) );
        bDataChanged = sal_True;
    }

    // object representation:
    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_CHARTS),
                                         rCoreSet, rOldSet, *m_pBtnCharts );

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_OBJECTS),
                                         rCoreSet, rOldSet, *m_pBtnObjects );

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_DRAWINGS),
                                         rCoreSet, rOldSet, *m_pBtnDrawings );

    // scaling:
    if( !m_pEdScalePageWidth->GetValue() && !m_pEdScalePageHeight->GetValue() )
    {
        m_pLbScaleMode->SelectEntryPos( SC_TPTABLE_SCALE_PERCENT );
        m_pEdScaleAll->SetValue( 100 );
    }

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALE),
                                      rCoreSet, rOldSet,
                                      *m_pLbScaleMode, SC_TPTABLE_SCALE_PERCENT,
                                      *m_pEdScaleAll, (sal_uInt16)m_pEdScaleAll->GetValue() );

    bDataChanged |= lcl_PutScaleItem2( GetWhich(SID_SCATTR_PAGE_SCALETO),
                                      rCoreSet, rOldSet,
                                      *m_pLbScaleMode, SC_TPTABLE_SCALE_TO,
                                      *m_pEdScalePageWidth, *m_pEdScalePageHeight );

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALETOPAGES),
                                      rCoreSet, rOldSet,
                                      *m_pLbScaleMode, SC_TPTABLE_SCALE_TO_PAGES,
                                      *m_pEdScalePageNum, (sal_uInt16)m_pEdScalePageNum->GetValue() );

    return bDataChanged;
}

int ScTablePage::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( *pSetP );

    return LEAVE_PAGE;
}

void ScTablePage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        ShowImage();
    SfxTabPage::DataChanged( rDCEvt );
}

// Handler:

IMPL_LINK_NOARG(ScTablePage, PageDirHdl)
{
    ShowImage();
    return 0;
}

IMPL_LINK( ScTablePage, PageNoHdl, CheckBox*, pBtn )
{
    if ( m_pBtnPageNo->IsChecked() )
    {
        m_pEdPageNo->Enable();
        if ( pBtn )
            m_pEdPageNo->GrabFocus();
    }
    else
        m_pEdPageNo->Disable();

    return 0;
}

IMPL_LINK_NOARG(ScTablePage, ScaleHdl)
{
    // controls for Box "Reduce/enlarge"
    m_pBxScaleAll->Show(m_pLbScaleMode->GetSelectEntryPos() == SC_TPTABLE_SCALE_PERCENT);

    // controls for Grid "Scale to width/height"
    m_pGrHeightWidth->Show(m_pLbScaleMode->GetSelectEntryPos() == SC_TPTABLE_SCALE_TO);

    // controls for Box "Scale to pages"
    m_pBxScalePageNum->Show(m_pLbScaleMode->GetSelectEntryPos() == SC_TPTABLE_SCALE_TO_PAGES);

    return 0;
}

// Helper functions for FillItemSet:

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
