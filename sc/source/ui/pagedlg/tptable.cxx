/*************************************************************************
 *
 *  $RCSfile: tptable.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#define _TPTABLE_CXX
#include "scitems.hxx"

#include "tptable.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "pagedlg.hrc"

#undef _TPTABLE_CXX

// STATIC DATA -----------------------------------------------------------

static USHORT pPageTableRanges[] =
{
    ATTR_PAGE_NOTES, ATTR_PAGE_FIRSTPAGENO,
    0
};

BOOL lcl_PutVObjModeItem( USHORT            nWhich,
                          SfxItemSet&       rCoreSet,
                          const SfxItemSet& rOldSet,
                          const CheckBox&   rBtn );

BOOL lcl_PutScaleItem( USHORT               nWhich,
                       SfxItemSet&          rCoreSet,
                       const SfxItemSet&    rOldSet,
                       const RadioButton&   rBtn,
                       const NumericField&  rEd );

BOOL lcl_PutBoolItem( USHORT            nWhich,
                      SfxItemSet&       rCoreSet,
                      const SfxItemSet& rOldSet,
                      BOOL              bIsChecked,
                      BOOL              bSavedValue );

//------------------------------------------------------------------------

#define PAGENO_HDL          LINK(this,ScTablePage,PageNoHdl)
#define PAGEDIR_HDL         LINK(this,ScTablePage,PageDirHdl)
#define SCALE_HDL           LINK(this,ScTablePage,ScaleHdl)
#define WAS_DEFAULT(w,s)    (SFX_ITEM_DEFAULT==(s).GetItemState((w),TRUE))
#define GET_BOOL(sid,set)   ((const SfxBoolItem&)((set).Get(GetWhich((sid))))).GetValue()
#define GET_USHORT(sid,set) (USHORT)((const SfxUInt16Item&)((set).Get(GetWhich((sid))))).GetValue()
#define GET_SHOW(sid,set)   ( ScVObjMode( ((const ScViewObjectModeItem&)((set).Get(GetWhich((sid))))).GetValue() ) \
                              == VOBJ_MODE_SHOW )

//========================================================================

ScTablePage::ScTablePage( Window*               pParent,
                          const SfxItemSet&     rCoreAttrs )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_TABLE ),
                          rCoreAttrs ),

        aBtnHeaders         ( this, ScResId( BTN_HEADER ) ),
        aBtnGrid            ( this, ScResId( BTN_GRID ) ),
        aBtnNotes           ( this, ScResId( BTN_NOTES ) ),
        aBtnObjects         ( this, ScResId( BTN_OBJECTS ) ),
        aBtnCharts          ( this, ScResId( BTN_CHARTS ) ),
        aBtnDrawings        ( this, ScResId( BTN_DRAWINGS ) ),
        aBtnFormulas        ( this, ScResId( BTN_FORMULAS ) ),
        aBtnNullVals        ( this, ScResId( BTN_NULLVALS ) ),
        aGbPrint            ( this, ScResId( GB_PRINT ) ),
        aBtnTopDown         ( this, ScResId( BTN_TOPDOWN ) ),
        aBtnLeftRight       ( this, ScResId( BTN_LEFTRIGHT ) ),
        aBmpPageDir         ( this, ScResId( BMP_PAGEDIR ) ),
        aImgTopDown         ( ScResId( IMG_TOPDOWN ) ),
        aImgLeftRight       ( ScResId( IMG_LEFTRIGHT ) ),
        aBtnPageNo          ( this, ScResId( BTN_PAGENO ) ),
        aEdPageNo           ( this, ScResId( ED_PAGENO ) ),
        aGbPageDir          ( this, ScResId( GB_PAGEDIR ) ),
        aBtnScaleAll        ( this, ScResId( BTN_SCALEALL ) ),
        aBtnScalePageNum    ( this, ScResId( BTN_SCALEPAGENUM ) ),
        aEdScaleAll         ( this, ScResId( ED_SCALEALL ) ),
        aEdScalePageNum     ( this, ScResId( ED_SCALEPAGENUM ) ),
        aGbScale            ( this, ScResId( GB_SCALE ) )
{
    SetExchangeSupport();
    aBtnPageNo          .SetClickHdl( PAGENO_HDL );
    aBtnTopDown         .SetClickHdl( PAGEDIR_HDL );
    aBtnLeftRight       .SetClickHdl( PAGEDIR_HDL );
    aBtnScaleAll        .SetClickHdl( SCALE_HDL );
    aBtnScalePageNum    .SetClickHdl( SCALE_HDL );
    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScTablePage::~ScTablePage()
{
}

//------------------------------------------------------------------------

USHORT* __EXPORT ScTablePage::GetRanges()
{
    return pPageTableRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTablePage::Create( Window*            pParent,
                                          const SfxItemSet& rCoreSet )
{
    return ( new ScTablePage( pParent, rCoreSet ) );
}

// -----------------------------------------------------------------------

void __EXPORT ScTablePage::Reset( const SfxItemSet& rCoreSet )
{
    BOOL    bTopDown = GET_BOOL( SID_SCATTR_PAGE_TOPDOWN, rCoreSet );
    USHORT  nWhich   = 0;

    //-----------
    // BOOL-Flags
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
    USHORT nPage = GET_USHORT(SID_SCATTR_PAGE_FIRSTPAGENO,rCoreSet);
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

    if ( rCoreSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        USHORT nScale = ((const SfxUInt16Item&)rCoreSet.Get(nWhich)).GetValue();

        aBtnScaleAll.Check( (nScale>0) );
        aEdScaleAll .SetValue( (nScale>0) ? nScale : 100 );
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETOPAGES);

    if ( rCoreSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        USHORT nPages = ((const SfxUInt16Item&)rCoreSet.Get(nWhich)).GetValue();

        aBtnScalePageNum.Check( (nPages>0) );
        aEdScalePageNum .SetValue( (nPages>0) ? nPages : 1 );
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
    aBtnScaleAll    .SaveValue();
    aBtnScalePageNum.SaveValue();
    aBtnCharts      .SaveValue();
    aBtnObjects     .SaveValue();
    aBtnDrawings    .SaveValue();
    aBtnPageNo      .SaveValue();
    aEdPageNo       .SaveValue();
    aEdScaleAll     .SaveValue();
    aEdScalePageNum .SaveValue();
}

// -----------------------------------------------------------------------

BOOL __EXPORT ScTablePage::FillItemSet( SfxItemSet& rCoreSet )
{
    const SfxItemSet&   rOldSet      = GetItemSet();
    USHORT              nWhichPageNo = GetWhich(SID_SCATTR_PAGE_FIRSTPAGENO);
    BOOL                bDataChanged = FALSE;

    //-----------
    // BOOL-Flags
    //-----------

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NOTES),
                                     rCoreSet, rOldSet,
                                     aBtnNotes.IsChecked(),
                                     aBtnNotes.GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_GRID),
                                     rCoreSet, rOldSet,
                                     aBtnGrid.IsChecked(),
                                     aBtnGrid.GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_HEADERS),
                                     rCoreSet, rOldSet,
                                     aBtnHeaders.IsChecked(),
                                     aBtnHeaders.GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_TOPDOWN),
                                     rCoreSet, rOldSet,
                                     aBtnTopDown.IsChecked(),
                                     aBtnTopDown.GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_FORMULAS),
                                     rCoreSet, rOldSet,
                                     aBtnFormulas.IsChecked(),
                                     aBtnFormulas.GetSavedValue() );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NULLVALS),
                                     rCoreSet, rOldSet,
                                     aBtnNullVals.IsChecked(),
                                     aBtnNullVals.GetSavedValue() );

    //------------------
    // Erste Druckseite:
    //------------------
    BOOL bUseValue = aBtnPageNo.IsChecked();

    if (   WAS_DEFAULT(nWhichPageNo,rOldSet)
        && (    !bUseValue && bUseValue == aBtnPageNo.GetSavedValue()
            || (   bUseValue && bUseValue == aBtnPageNo.GetSavedValue()
                && aEdPageNo.GetText() == aEdPageNo.GetSavedValue() ) ) )
    {
            rCoreSet.ClearItem( nWhichPageNo );
    }
    else
    {
        UINT16 nPage = (UINT16)( aBtnPageNo.IsChecked()
                                    ? aEdPageNo.GetValue()
                                    : 0 );

        rCoreSet.Put( SfxUInt16Item( nWhichPageNo, nPage ) );
        bDataChanged = TRUE;
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

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALE),
                                      rCoreSet, rOldSet,
                                      aBtnScaleAll, aEdScaleAll );

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALETOPAGES),
                                      rCoreSet, rOldSet,
                                      aBtnScalePageNum, aEdScalePageNum );

    return bDataChanged;
}

//------------------------------------------------------------------------

int __EXPORT ScTablePage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------------

IMPL_LINK( ScTablePage, PageDirHdl, RadioButton*, EMPTYARG )
{
    aBmpPageDir.SetBitmap( aBtnLeftRight.IsChecked()
                                ? aImgLeftRight
                                : aImgTopDown );
    return NULL;
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

    return NULL;
}

//------------------------------------------------------------------------

IMPL_LINK( ScTablePage, ScaleHdl, RadioButton*, pBtn )
{
    if ( aBtnScaleAll.IsChecked() )
    {
        aEdScalePageNum .Disable();
        aEdScaleAll     .Enable();

        if ( pBtn )
            aEdScaleAll.GrabFocus();
    }
    else if ( aBtnScalePageNum.IsChecked() )
    {
        aEdScaleAll     .Disable();
        aEdScalePageNum .Enable();

        if ( pBtn )
            aEdScalePageNum.GrabFocus();
    }

    return NULL;
}

//========================================================================
// Hilfsfunktionen fuer FillItemSet:
//========================================================================

BOOL lcl_PutBoolItem( USHORT            nWhich,
                     SfxItemSet&        rCoreSet,
                     const SfxItemSet&  rOldSet,
                     BOOL               bIsChecked,
                     BOOL               bSavedValue )
{
    BOOL bDataChanged = (   bSavedValue == bIsChecked
                         && WAS_DEFAULT(nWhich,rOldSet) );

    if ( bDataChanged )
        rCoreSet.ClearItem(nWhich);
    else
        rCoreSet.Put( SfxBoolItem( nWhich, bIsChecked ) );

    return bDataChanged;
}

//------------------------------------------------------------------------

BOOL lcl_PutVObjModeItem( USHORT            nWhich,
                         SfxItemSet&        rCoreSet,
                         const SfxItemSet&  rOldSet,
                         const CheckBox&    rBtn )
{
    BOOL bIsChecked   = rBtn.IsChecked();
    BOOL bDataChanged = (   rBtn.GetSavedValue() == bIsChecked
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

BOOL lcl_PutScaleItem( USHORT               nWhich,
                      SfxItemSet&           rCoreSet,
                      const SfxItemSet&     rOldSet,
                      const RadioButton&    rBtn,
                      const NumericField&   rEd )
{
    UINT16  nValue          = (UINT16)rEd.GetValue();
    BOOL    bIsChecked      = rBtn.IsChecked();
    BOOL    bSaveValue      = rBtn.GetSavedValue();
    BOOL    bDataChanged    = FALSE;

    if (   (bSaveValue && bIsChecked)
        || (!bSaveValue && !bIsChecked) )
    {
        if (   (rEd.GetSavedValue() == rEd.GetText())
            && WAS_DEFAULT(nWhich,rOldSet) )
        {
            rCoreSet.ClearItem( nWhich );
        }
        else
        {
//          rCoreSet.Put( SfxUInt16Item( nWhich, bIsChecked ? nValue : 0 ) );
            if ( bIsChecked )
                rCoreSet.Put( SfxUInt16Item( nWhich, nValue ) );
            else
                rCoreSet.Put( SfxUInt16Item( nWhich, 0 ) );

            bDataChanged = TRUE;
        }
    }
    else if ( bSaveValue && !bIsChecked )
    {
        rCoreSet.Put( SfxUInt16Item( nWhich, 0 ) );
        bDataChanged = TRUE;
    }
    else if ( !bSaveValue && bIsChecked )
    {
        rCoreSet.Put( SfxUInt16Item( nWhich, nValue ) );
        bDataChanged = TRUE;
    }

    return bDataChanged;
}



