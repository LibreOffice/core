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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#define _SVX_BORDER_CXX

#include <cuires.hrc>
#include "border.hrc"
#include "helpid.hrc"

#include <svx/xtable.hxx>               // XColorList
#include <svx/drawitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include "border.hxx"
#include <svx/dlgutil.hxx>
#include <dialmgr.hxx>
#include "svx/htmlmode.hxx"
#include <vcl/msgbox.hxx>
#include "svx/flagsdef.hxx"
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <sfx2/itemconnect.hxx>
#include <sal/macros.h>
#include "borderconn.hxx"

using namespace ::editeng;

// -----------------------------------------------------------------------

/*
 * [Description:]
 * TabPage for setting the border attributes.
 * Needs
 *      a SvxShadowItem: shadow
 *      a SvxBoxItem:    lines left, right, top, bottom,
 *      a SvxBoxInfo:    lines vertical, horizontal, distance, flags
 *
 * Lines can have three conditions:
 *      1. Show     ( -> valid values )
 *      2. Hide     ( -> NULL-Pointer )
 *      3. DontCare ( -> special Valid-Flags in the InfoItem )
 */

// static ----------------------------------------------------------------

static sal_uInt16 pRanges[] =
{
    SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_SHADOW,
    SID_ATTR_ALIGN_MARGIN,      SID_ATTR_ALIGN_MARGIN,
    SID_ATTR_BORDER_CONNECT,    SID_ATTR_BORDER_CONNECT,
    SID_SW_COLLAPSING_BORDERS,  SID_SW_COLLAPSING_BORDERS,
    SID_ATTR_BORDER_DIAG_TLBR,  SID_ATTR_BORDER_DIAG_BLTR,
    0
};

// -----------------------------------------------------------------------
static void lcl_SetDecimalDigitsTo1(MetricField& rField)
{
    sal_Int64 nMin = rField.Denormalize( rField.GetMin( FUNIT_TWIP ) );
    rField.SetDecimalDigits(1);
    rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
}
// -----------------------------------------------------------------------

SvxBorderTabPage::SvxBorderTabPage( Window* pParent,
                                    const SfxItemSet& rCoreAttrs )

    :   SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_BORDER ), rCoreAttrs ),

        aFlBorder       ( this, CUI_RES( FL_BORDER ) ),
        aDefaultFT      ( this, CUI_RES( FT_DEFAULT ) ),
        aWndPresets     ( this, CUI_RES( WIN_PRESETS ) ),
        aUserDefFT      ( this, CUI_RES( FT_USERDEF ) ),
        aFrameSel       ( this, CUI_RES( WIN_FRAMESEL ) ),

        aFlSep1         ( this, CUI_RES( FL_SEPARATOR1 ) ),
        aFlLine         ( this, CUI_RES( FL_LINE ) ),
        aStyleFT        ( this, CUI_RES( FT_STYLE ) ),
        aLbLineStyle    ( this, CUI_RES( LB_LINESTYLE ) ),
        aColorFT        ( this, CUI_RES( FT_COLOR ) ),
        aLbLineColor    ( this, CUI_RES( LB_LINECOLOR ) ),
        aWidthFT        ( this, CUI_RES( FT_WIDTH ) ),
        aLineWidthMF    ( this, CUI_RES( MF_LINEWIDTH ) ),

        aFlSep2         ( this, CUI_RES( FL_SEPARATOR2 ) ),
        aDistanceFL     ( this, CUI_RES( FL_DISTANCE ) ),
        aLeftFT         ( this, CUI_RES( FT_LEFT ) ),
        aLeftMF         ( this, CUI_RES( MF_LEFT ) ),
        aRightFT        ( this, CUI_RES( FT_RIGHT ) ),
        aRightMF        ( this, CUI_RES( MF_RIGHT ) ),
        aTopFT          ( this, CUI_RES( FT_TOP ) ),
        aTopMF          ( this, CUI_RES( MF_TOP ) ),
        aBottomFT       ( this, CUI_RES( FT_BOTTOM ) ),
        aBottomMF       ( this, CUI_RES( MF_BOTTOM ) ),
        aSynchronizeCB  ( this, CUI_RES( CB_SYNC ) ),

        aFlShadow       ( this, CUI_RES( FL_SHADOW ) ),
        aFtShadowPos    ( this, CUI_RES( FT_SHADOWPOS ) ),
        aWndShadows     ( this, CUI_RES( WIN_SHADOWS ) ),
        aFtShadowSize   ( this, CUI_RES( FT_SHADOWSIZE ) ),
        aEdShadowSize   ( this, CUI_RES( ED_SHADOWSIZE ) ),
        aFtShadowColor  ( this, CUI_RES( FT_SHADOWCOLOR ) ),
        aLbShadowColor  ( this, CUI_RES( LB_SHADOWCOLOR ) ),
        aPropertiesFL   ( this, CUI_RES( FL_PROPERTIES ) ),
        aMergeWithNextCB( this, CUI_RES( CB_MERGEWITHNEXT ) ),
        aMergeAdjacentBordersCB( this, CUI_RES( CB_MERGEADJACENTBORDERS ) ),
        aShadowImgLst( CUI_RES(IL_SDW_BITMAPS)),
        aBorderImgLst( CUI_RES(IL_PRE_BITMAPS)),
        nMinValue(0),
        nSWMode(0),
        mbHorEnabled( false ),
        mbVerEnabled( false ),
        mbTLBREnabled( false ),
        mbBLTREnabled( false ),
        mbUseMarginItem( false ),
        mbSync(true)

{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    /*  Use SvxMarginItem instead of margins from SvxBoxItem, if present.
        ->  Remember this state in mbUseMarginItem, because other special handling
            is needed across various functions... */
    mbUseMarginItem = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_MARGIN),sal_True) != SFX_ITEM_UNKNOWN;

    // set metric
    FieldUnit eFUnit = GetModuleFieldUnit( rCoreAttrs );

    if( mbUseMarginItem )
    {
        // copied from SvxAlignmentTabPage
        switch ( eFUnit )
        {
            //  #103396# the default value (1pt) can't be accurately represented in
            //  inches or pica with two decimals, so point is used instead.
            case FUNIT_PICA:
            case FUNIT_INCH:
            case FUNIT_FOOT:
            case FUNIT_MILE:
                eFUnit = FUNIT_POINT;
                break;

            case FUNIT_CM:
            case FUNIT_M:
            case FUNIT_KM:
                eFUnit = FUNIT_MM;
                break;
            default: ;//prevent warning
        }
    }
    else
    {
        switch ( eFUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eFUnit = FUNIT_MM;
                break;
            default: ; //prevent warning
        }
    }

    SetFieldUnit( aEdShadowSize, eFUnit );

    sal_uInt16 nWhich = GetWhich( SID_ATTR_BORDER_INNER, sal_False );
    sal_Bool bIsDontCare = sal_True;

    if ( rCoreAttrs.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        // paragraph or table
        const SvxBoxInfoItem* pBoxInfo =
            (const SvxBoxInfoItem*)&( rCoreAttrs.Get( nWhich ) );

        mbHorEnabled = pBoxInfo->IsHorEnabled();
        mbVerEnabled = pBoxInfo->IsVerEnabled();
        mbTLBREnabled = sfx::ItemWrapperHelper::IsKnownItem( rCoreAttrs, SID_ATTR_BORDER_DIAG_TLBR );
        mbBLTREnabled = sfx::ItemWrapperHelper::IsKnownItem( rCoreAttrs, SID_ATTR_BORDER_DIAG_BLTR );

        if(pBoxInfo->IsDist())
        {
            SetFieldUnit(aLeftMF, eFUnit);
            SetFieldUnit(aRightMF, eFUnit);
            SetFieldUnit(aTopMF, eFUnit);
            SetFieldUnit(aBottomMF, eFUnit);
            aSynchronizeCB.SetClickHdl(LINK(this, SvxBorderTabPage, SyncHdl_Impl));
            aLeftMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            aRightMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            aTopMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            aBottomMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
        }
        else
        {
            aFlSep2.Hide();
            aDistanceFL.Hide();
            aLeftFT.Hide();
            aLeftMF.Hide();
            aRightFT.Hide();
            aRightMF.Hide();
            aTopFT.Hide();
            aTopMF.Hide();
            aBottomFT.Hide();
            aBottomMF.Hide();
            aSynchronizeCB.Hide();
        }
        bIsDontCare = !pBoxInfo->IsValid( VALID_DISABLE );
    }
    if(!mbUseMarginItem && eFUnit == FUNIT_MM && SFX_MAPUNIT_TWIP == rCoreAttrs.GetPool()->GetMetric( GetWhich( SID_ATTR_BORDER_INNER ) ))
    {
        //#i91548# changing the number of decimal digits changes the minimum values, too
        lcl_SetDecimalDigitsTo1(aLeftMF);
        lcl_SetDecimalDigitsTo1(aRightMF);
        lcl_SetDecimalDigitsTo1(aTopMF);
        lcl_SetDecimalDigitsTo1(aBottomMF);
        lcl_SetDecimalDigitsTo1(aEdShadowSize);
    }

    svx::FrameSelFlags nFlags = svx::FRAMESEL_OUTER;
    if( mbHorEnabled )
        nFlags |= svx::FRAMESEL_INNER_HOR;
    if( mbVerEnabled )
        nFlags |= svx::FRAMESEL_INNER_VER;
    if( mbTLBREnabled )
        nFlags |= svx::FRAMESEL_DIAG_TLBR;
    if( mbBLTREnabled )
        nFlags |= svx::FRAMESEL_DIAG_BLTR;
    if( bIsDontCare )
        nFlags |= svx::FRAMESEL_DONTCARE;
    aFrameSel.Initialize( nFlags );

    aFrameSel.SetSelectHdl(LINK(this, SvxBorderTabPage, LinesChanged_Impl));
    aLbLineStyle.SetSelectHdl( LINK( this, SvxBorderTabPage, SelStyleHdl_Impl ) );
    aLbLineColor.SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    aLineWidthMF.SetModifyHdl( LINK( this, SvxBorderTabPage, ModifyWidthHdl_Impl ) );
    aLbShadowColor.SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    aWndPresets.SetSelectHdl( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    aWndShadows.SetSelectHdl( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );

    FillValueSets();
    FillLineListBox_Impl();

    // fill ColorBox out of the XColorList
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pItem       = NULL;
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }

    DBG_ASSERT( pColorTable.is(), "ColorTable not found!" );

    if ( pColorTable.is() )
    {
        // filling the line color box
        aLbLineColor.SetUpdateMode( sal_False );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            aLbLineColor.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        aLbLineColor.SetUpdateMode( sal_True );

        aLbShadowColor.CopyEntries( aLbLineColor );
    }
    FreeResource();

    // connections

    bool bSupportsShadow = !SfxItemPool::IsSlot( GetWhich( SID_ATTR_BORDER_SHADOW ) );
    if( bSupportsShadow )
        AddItemConnection( svx::CreateShadowConnection( rCoreAttrs, aWndShadows, aEdShadowSize, aLbShadowColor ) );
    else
        HideShadowControls();

    if( mbUseMarginItem )
        AddItemConnection( svx::CreateMarginConnection( rCoreAttrs, aLeftMF, aRightMF, aTopMF, aBottomMF ) );
    if( aFrameSel.IsBorderEnabled( svx::FRAMEBORDER_TLBR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_TLBR, aFrameSel, svx::FRAMEBORDER_TLBR ) );
    if( aFrameSel.IsBorderEnabled( svx::FRAMEBORDER_BLTR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_BLTR, aFrameSel, svx::FRAMEBORDER_BLTR ) );
    // #i43593# - item connection doesn't work for Writer,
    // because the Writer item sets contain these items
    // checkbox "Merge with next paragraph" only visible for Writer dialog format.paragraph
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_BORDER_CONNECT, aMergeWithNextCB, sfx::ITEMCONN_DEFAULT ) );
    aMergeWithNextCB.Hide();
    // checkbox "Merge adjacent line styles" only visible for Writer dialog format.table
    AddItemConnection( new sfx::CheckBoxConnection( SID_SW_COLLAPSING_BORDERS, aMergeAdjacentBordersCB, sfx::ITEMCONN_DEFAULT ) );
    aMergeAdjacentBordersCB.Hide();
}

// -----------------------------------------------------------------------

SvxBorderTabPage::~SvxBorderTabPage()
{
}

// -----------------------------------------------------------------------

sal_uInt16* SvxBorderTabPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxBorderTabPage::Create( Window* pParent,
                                      const SfxItemSet& rAttrSet )
{
    return ( new SvxBorderTabPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::ResetFrameLine_Impl( svx::FrameBorderType eBorder, const SvxBorderLine* pCoreLine, bool bValid )
{
    if( aFrameSel.IsBorderEnabled( eBorder ) )
    {
        if( bValid )
            aFrameSel.ShowBorder( eBorder, pCoreLine );
        else
            aFrameSel.SetBorderDontCare( eBorder );
    }
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::Reset( const SfxItemSet& rSet )
{
    SfxTabPage::Reset( rSet );

    const SvxBoxItem*       pBoxItem;
    const SvxBoxInfoItem*   pBoxInfoItem;
    sal_uInt16                  nWhichBox       = GetWhich(SID_ATTR_BORDER_OUTER);
    SfxMapUnit              eCoreUnit;

    pBoxItem  = (const SvxBoxItem*)GetItem( rSet, SID_ATTR_BORDER_OUTER );

    pBoxInfoItem = (const SvxBoxInfoItem*)GetItem( rSet, SID_ATTR_BORDER_INNER, sal_False );

    eCoreUnit = rSet.GetPool()->GetMetric( nWhichBox );

    if ( pBoxItem && pBoxInfoItem ) // -> Don't Care
    {
        ResetFrameLine_Impl( svx::FRAMEBORDER_LEFT,   pBoxItem->GetLeft(),     pBoxInfoItem->IsValid( VALID_LEFT ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_RIGHT,  pBoxItem->GetRight(),    pBoxInfoItem->IsValid( VALID_RIGHT ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_TOP,    pBoxItem->GetTop(),      pBoxInfoItem->IsValid( VALID_TOP ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_BOTTOM, pBoxItem->GetBottom(),   pBoxInfoItem->IsValid( VALID_BOTTOM ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_VER,    pBoxInfoItem->GetVert(), pBoxInfoItem->IsValid( VALID_VERT ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_HOR,    pBoxInfoItem->GetHori(), pBoxInfoItem->IsValid( VALID_HORI ) );

        //-------------------
        // distance inside
        //-------------------
        if( !mbUseMarginItem )
        {
            if ( aLeftMF.IsVisible() )
            {
                SetMetricValue( aLeftMF,    pBoxInfoItem->GetDefDist(), eCoreUnit );
                SetMetricValue( aRightMF,   pBoxInfoItem->GetDefDist(), eCoreUnit );
                SetMetricValue( aTopMF,     pBoxInfoItem->GetDefDist(), eCoreUnit );
                SetMetricValue( aBottomMF,  pBoxInfoItem->GetDefDist(), eCoreUnit );

                nMinValue = static_cast<long>(aLeftMF.GetValue());

                if ( pBoxInfoItem->IsMinDist() )
                {
                    aLeftMF.SetFirst( nMinValue );
                    aRightMF.SetFirst( nMinValue );
                    aTopMF.SetFirst( nMinValue );
                    aBottomMF.SetFirst( nMinValue );
                }

                if ( pBoxInfoItem->IsDist() )
                {
                    if( rSet.GetItemState( nWhichBox, sal_True ) >= SFX_ITEM_DEFAULT )
                    {
                        sal_Bool bIsAnyBorderVisible = aFrameSel.IsAnyBorderVisible();
                        if( !bIsAnyBorderVisible || !pBoxInfoItem->IsMinDist() )
                        {
                            aLeftMF.SetMin( 0 );
                            aLeftMF.SetFirst( 0 );
                            aRightMF.SetMin( 0 );
                            aRightMF.SetFirst( 0 );
                            aTopMF.SetMin( 0 );
                            aTopMF.SetFirst( 0 );
                            aBottomMF.SetMin( 0 );
                            aBottomMF.SetFirst( 0 );
                        }
                        long nLeftDist = pBoxItem->GetDistance( BOX_LINE_LEFT);
                        SetMetricValue( aLeftMF, nLeftDist, eCoreUnit );
                        long nRightDist = pBoxItem->GetDistance( BOX_LINE_RIGHT);
                        SetMetricValue( aRightMF, nRightDist, eCoreUnit );
                        long nTopDist = pBoxItem->GetDistance( BOX_LINE_TOP);
                        SetMetricValue( aTopMF, nTopDist, eCoreUnit );
                        long nBottomDist = pBoxItem->GetDistance( BOX_LINE_BOTTOM);
                        SetMetricValue( aBottomMF, nBottomDist, eCoreUnit );

                        // if the distance is set with no active border line
                        // or it is null with an active border line
                        // no automatic changes should be made
                        const long nDefDist = bIsAnyBorderVisible ? pBoxInfoItem->GetDefDist() : 0;
                        sal_Bool bDiffDist = (nDefDist != nLeftDist ||
                                    nDefDist != nRightDist ||
                                    nDefDist != nTopDist   ||
                                    nDefDist != nBottomDist);
                        if((pBoxItem->GetDistance() ||
                                bIsAnyBorderVisible) && bDiffDist )
                        {
                            aLeftMF.SetModifyFlag();
                            aRightMF.SetModifyFlag();
                            aTopMF.SetModifyFlag();
                            aBottomMF.SetModifyFlag();
                        }
                    }
                    else
                    {
                        // #106224# different margins -> do not fill the edits
                        aLeftMF.SetText( String() );
                        aRightMF.SetText( String() );
                        aTopMF.SetText( String() );
                        aBottomMF.SetText( String() );
                    }
                }
                aLeftMF.SaveValue();
                aRightMF.SaveValue();
                aTopMF.SaveValue();
                aBottomMF.SaveValue();
            }
        }
    }
    else
    {
        // avoid ResetFrameLine-calls:
        aFrameSel.HideAllBorders();
    }

    //-------------------------------------------------------------
    // depict line (color) in controllers if unambiguous:
    //-------------------------------------------------------------
    {
        // Do all visible lines show the same line widths?
        long nWidth;
        SvxBorderStyle nStyle;
        bool bWidthEq = aFrameSel.GetVisibleWidth( nWidth, nStyle );
        if( bWidthEq )
        {
            // Determine the width first as some styles can be missing depending on it
            sal_Int64 nWidthPt =  static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                        sal_Int64( nWidth ), aLineWidthMF.GetDecimalDigits( ),
                        MAP_TWIP,aLineWidthMF.GetUnit() ));
            aLineWidthMF.SetValue( nWidthPt );
            aLbLineStyle.SetWidth( nWidth );

            // then set the style
            aLbLineStyle.SelectEntry( nStyle );
        }
        else
            aLbLineStyle.SelectEntryPos( 1 );

        // Do all visible lines show the same line color?
        Color aColor;
        bool bColorEq = aFrameSel.GetVisibleColor( aColor );
        if( !bColorEq )
            aColor.SetColor( COL_BLACK );

        sal_uInt16 nSelPos = aLbLineColor.GetEntryPos( aColor );
        if( nSelPos == LISTBOX_ENTRY_NOTFOUND )
            nSelPos = aLbLineColor.InsertEntry( aColor, SVX_RESSTR( RID_SVXSTR_COLOR_USER ) );

        aLbLineColor.SelectEntryPos( nSelPos );
        aLbLineStyle.SetColor( aColor );

        // Select all visible lines, if they are all equal.
        if( bWidthEq && bColorEq )
            aFrameSel.SelectAllVisibleBorders();

        // set the current style and color (caches style in control even if nothing is selected)
        SelStyleHdl_Impl( &aLbLineStyle );
        SelColHdl_Impl( &aLbLineColor );
    }

    sal_Bool bEnable = aWndShadows.GetSelectItemId() > 1 ;
    aFtShadowSize.Enable(bEnable);
    aEdShadowSize.Enable(bEnable);
    aFtShadowColor.Enable(bEnable);
    aLbShadowColor.Enable(bEnable);

    aWndPresets.SetNoSelection();

    // - no line - should not be selected

    if ( aLbLineStyle.GetSelectEntryPos() == 0 )
    {
        aLbLineStyle.SelectEntryPos( 1 );
        SelStyleHdl_Impl( &aLbLineStyle );
    }

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            // there are no shadows in Html-mode and only complete borders
            aFtShadowPos  .Disable();
            aWndShadows   .Disable();
            aFtShadowSize .Disable();
            aEdShadowSize .Disable();
            aFtShadowColor.Disable();
            aLbShadowColor.Disable();
            aFlShadow     .Disable();

            if( !(nSWMode & SW_BORDER_MODE_TABLE) )
            {
                aUserDefFT.Disable();
                aFrameSel.Disable();
                aWndPresets.RemoveItem(3);
                aWndPresets.RemoveItem(4);
                aWndPresets.RemoveItem(5);
            }
        }
    }

    LinesChanged_Impl( 0 );
    if(aLeftMF.GetValue() == aRightMF.GetValue() && aTopMF.GetValue() == aBottomMF.GetValue() && aTopMF.GetValue() == aLeftMF.GetValue())
        mbSync = true;
    else
        mbSync = false;
    aSynchronizeCB.Check(mbSync);
}

// -----------------------------------------------------------------------

int SvxBorderTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

sal_Bool SvxBorderTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    bool bAttrsChanged = SfxTabPage::FillItemSet( rCoreAttrs );

    sal_Bool                  bPut          = sal_True;
    sal_uInt16                nBoxWhich     = GetWhich( SID_ATTR_BORDER_OUTER );
    sal_uInt16                nBoxInfoWhich = rCoreAttrs.GetPool()->GetWhich( SID_ATTR_BORDER_INNER, sal_False );
    const SfxItemSet&     rOldSet       = GetItemSet();
    SvxBoxItem            aBoxItem      ( nBoxWhich );
    SvxBoxInfoItem        aBoxInfoItem  ( nBoxInfoWhich );
    SvxBoxItem* pOldBoxItem = (SvxBoxItem*)GetOldItem( rCoreAttrs, SID_ATTR_BORDER_OUTER );

    SfxMapUnit eCoreUnit = rOldSet.GetPool()->GetMetric( nBoxWhich );
    const SfxPoolItem* pOld = 0;

    //------------------
    // outer border:
    //------------------
    typedef ::std::pair<svx::FrameBorderType,sal_uInt16> TBorderPair;
    TBorderPair eTypes1[] = {
                                TBorderPair(svx::FRAMEBORDER_TOP,BOX_LINE_TOP),
                                TBorderPair(svx::FRAMEBORDER_BOTTOM,BOX_LINE_BOTTOM),
                                TBorderPair(svx::FRAMEBORDER_LEFT,BOX_LINE_LEFT),
                                TBorderPair(svx::FRAMEBORDER_RIGHT,BOX_LINE_RIGHT),
                            };

    for (sal_uInt32 i=0; i < SAL_N_ELEMENTS(eTypes1); ++i)
        aBoxItem.SetLine( aFrameSel.GetFrameBorderStyle( eTypes1[i].first ), eTypes1[i].second );

    //--------------------------------
    // border hor/ver and TableFlag
    //--------------------------------
    TBorderPair eTypes2[] = {
                                TBorderPair(svx::FRAMEBORDER_HOR,BOXINFO_LINE_HORI),
                                TBorderPair(svx::FRAMEBORDER_VER,BOXINFO_LINE_VERT)
                            };
    for (sal_uInt32 j=0; j < SAL_N_ELEMENTS(eTypes2); ++j)
        aBoxInfoItem.SetLine( aFrameSel.GetFrameBorderStyle( eTypes2[j].first ), eTypes2[j].second );

    aBoxInfoItem.EnableHor( mbHorEnabled );
    aBoxInfoItem.EnableVer( mbVerEnabled );

    //-------------------
    // inner distance
    //-------------------
    if( aLeftMF.IsVisible() )
    {
        // #i40405# enable distance controls for next dialog call
        aBoxInfoItem.SetDist( sal_True );

        if( !mbUseMarginItem )
        {
            // #106224# all edits empty: do nothing
            if( aLeftMF.GetText().Len() || aRightMF.GetText().Len() ||
                aTopMF.GetText().Len() || aBottomMF.GetText().Len() )
            {
                if ( ((mbHorEnabled || mbVerEnabled || (nSWMode & SW_BORDER_MODE_TABLE)) &&
                        (aLeftMF.IsModified()||aRightMF.IsModified()||
                            aTopMF.IsModified()||aBottomMF.IsModified()) )||
                     aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_TOP ) != svx::FRAMESTATE_HIDE
                     || aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_BOTTOM ) != svx::FRAMESTATE_HIDE
                     || aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_LEFT ) != svx::FRAMESTATE_HIDE
                     || aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_RIGHT ) != svx::FRAMESTATE_HIDE )
                {
                    SvxBoxInfoItem* pOldBoxInfoItem = (SvxBoxInfoItem*)GetOldItem(
                                                        rCoreAttrs, SID_ATTR_BORDER_INNER );
                    if (
                        !pOldBoxItem ||
                        aLeftMF  .GetText() != aLeftMF  .GetSavedValue() ||
                        aRightMF .GetText() != aRightMF .GetSavedValue() ||
                        aTopMF   .GetText() != aTopMF   .GetSavedValue() ||
                        aBottomMF.GetText() != aBottomMF.GetSavedValue() ||
                        nMinValue == aLeftMF  .GetValue() ||
                        nMinValue == aRightMF .GetValue() ||
                        nMinValue == aTopMF   .GetValue() ||
                        nMinValue == aBottomMF.GetValue() ||
                        (pOldBoxInfoItem && !pOldBoxInfoItem->IsValid(VALID_DISTANCE))
                       )
                    {
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue( aLeftMF, eCoreUnit ), BOX_LINE_LEFT  );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue( aRightMF, eCoreUnit ), BOX_LINE_RIGHT );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue( aTopMF, eCoreUnit ), BOX_LINE_TOP   );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue( aBottomMF, eCoreUnit ), BOX_LINE_BOTTOM);
                    }
                    else
                    {
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_LEFT ), BOX_LINE_LEFT);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_RIGHT),  BOX_LINE_RIGHT);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_TOP  ), BOX_LINE_TOP);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_BOTTOM), BOX_LINE_BOTTOM);
                    }
                    aBoxInfoItem.SetValid( VALID_DISTANCE, sal_True );
                }
                else
                    aBoxInfoItem.SetValid( VALID_DISTANCE, sal_False );
            }
        }
    }

    //------------------------------------------
    // note Don't Care Status in the Info-Item:
    //------------------------------------------
    aBoxInfoItem.SetValid( VALID_TOP,    aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_TOP )    != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_BOTTOM, aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_BOTTOM ) != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_LEFT,   aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_LEFT )   != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_RIGHT,  aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_RIGHT )  != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_HORI,   aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_HOR )    != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_VERT,   aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_VER )    != svx::FRAMESTATE_DONTCARE );

    //
    // Put or Clear of the border?
    //
    bPut = sal_True;

    if (   SFX_ITEM_DEFAULT == rOldSet.GetItemState( nBoxWhich,     sal_False ))
    {
        bPut = aBoxItem != (const SvxBoxItem&)(rOldSet.Get(nBoxWhich)) ? sal_True : sal_False;
    }
    if(  SFX_ITEM_DEFAULT == rOldSet.GetItemState( nBoxInfoWhich, sal_False ) )
    {
        const SvxBoxInfoItem& rOldBoxInfo = (const SvxBoxInfoItem&)
                                rOldSet.Get(nBoxInfoWhich);

        aBoxInfoItem.SetMinDist( rOldBoxInfo.IsMinDist() );
        aBoxInfoItem.SetDefDist( rOldBoxInfo.GetDefDist() );
        bPut |= (aBoxInfoItem != rOldBoxInfo );
    }

    if ( bPut )
    {
        if ( !pOldBoxItem || !( *pOldBoxItem == aBoxItem ) )
        {
            rCoreAttrs.Put( aBoxItem );
            bAttrsChanged |= sal_True;
        }
        pOld = GetOldItem( rCoreAttrs, SID_ATTR_BORDER_INNER, sal_False );

        if ( !pOld || !( *(const SvxBoxInfoItem*)pOld == aBoxInfoItem ) )
        {
            rCoreAttrs.Put( aBoxInfoItem );
            bAttrsChanged |= sal_True;
        }
    }
    else
    {
        rCoreAttrs.ClearItem( nBoxWhich );
        rCoreAttrs.ClearItem( nBoxInfoWhich );
    }

    return bAttrsChanged;
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::HideShadowControls()
{
    aFtShadowPos.Hide();
    aWndShadows.Hide();
    aFtShadowSize.Hide();
    aEdShadowSize.Hide();
    aFtShadowColor.Hide();
    aLbShadowColor.Hide();
    aFlShadow.Hide();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBorderTabPage, SelPreHdl_Impl)
{
    const svx::FrameBorderState SHOW = svx::FRAMESTATE_SHOW;
    const svx::FrameBorderState HIDE = svx::FRAMESTATE_HIDE;
    const svx::FrameBorderState DONT = svx::FRAMESTATE_DONTCARE;

    static const svx::FrameBorderState ppeStates[][ svx::FRAMEBORDERTYPE_COUNT ] =
    {                   /*    Left  Right Top   Bot   Hor   Ver   TLBR  BLTR */
/* ---------------------+--------------------------------------------------- */
/* IID_PRE_CELL_NONE    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_ALL     */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_LR      */  { SHOW, SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_TB      */  { HIDE, HIDE, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_L       */  { SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_DIAG    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, SHOW, SHOW },
/* IID_PRE_HOR_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_HOR      */  { HIDE, HIDE, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_ALL      */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, DONT, HIDE, HIDE, HIDE },
/* IID_PRE_VER_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_VER      */  { SHOW, SHOW, HIDE, HIDE, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_ALL      */  { SHOW, SHOW, SHOW, SHOW, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, HIDE, DONT, HIDE, HIDE },
/* IID_PRE_TABLE_NONE   */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER  */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTERH */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_ALL    */  { SHOW, SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER2 */  { SHOW, SHOW, SHOW, SHOW, DONT, DONT, HIDE, HIDE }
    };

    // first hide and deselect all frame borders
    aFrameSel.HideAllBorders();
    aFrameSel.DeselectAllBorders();

    // Using image ID to find correct line in table above.
    sal_uInt16 nLine = GetPresetImageId( aWndPresets.GetSelectItemId() ) - 1;

    // Apply all styles from the table
    for( int nBorder = 0; nBorder < svx::FRAMEBORDERTYPE_COUNT; ++nBorder )
    {
        svx::FrameBorderType eBorder = svx::GetFrameBorderTypeFromIndex( nBorder );
        switch( ppeStates[ nLine ][ nBorder ] )
        {
            case SHOW:  aFrameSel.SelectBorder( eBorder );      break;
            case HIDE:  /* nothing to do */                     break;
            case DONT:  aFrameSel.SetBorderDontCare( eBorder ); break;
        }
    }

    // Show all lines that have been selected above
    if( aFrameSel.IsAnyBorderSelected() )
    {
        // any visible style, but "no-line" in line list box? -> use hair-line
        if( (aLbLineStyle.GetSelectEntryPos() == 0) || (aLbLineStyle.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND) )
            aLbLineStyle.SelectEntryPos( 1 );

        // set current style to all previously selected lines
        SelStyleHdl_Impl( &aLbLineStyle );
        SelColHdl_Impl( &aLbLineColor );
    }

    // Presets ValueSet does not show a selection (used as push buttons).
    aWndPresets.SetNoSelection();

    LinesChanged_Impl( 0 );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBorderTabPage, SelSdwHdl_Impl)
{
    sal_Bool bEnable = aWndShadows.GetSelectItemId() > 1;
    aFtShadowSize.Enable(bEnable);
    aEdShadowSize.Enable(bEnable);
    aFtShadowColor.Enable(bEnable);
    aLbShadowColor.Enable(bEnable);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelColHdl_Impl, ListBox *, pLb )
{
    ColorListBox* pColLb = (ColorListBox*)pLb;

    if ( pLb == &aLbLineColor )
    {
        aFrameSel.SetColorToSelection( pColLb->GetSelectEntryColor() );
        aLbLineStyle.SetColor( pColLb->GetSelectEntryColor() );
    }

    return 0;
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthHdl_Impl)
{
    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                aLineWidthMF.GetValue( ),
                aLineWidthMF.GetDecimalDigits( ),
                aLineWidthMF.GetUnit(), MAP_TWIP ));
    aLbLineStyle.SetWidth( nVal );

    aFrameSel.SetStyleToSelection( nVal,
        SvxBorderStyle( aLbLineStyle.GetSelectEntryStyle() ) );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelStyleHdl_Impl, ListBox *, pLb )
{
    if ( pLb == &aLbLineStyle )
    {
        sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                    aLineWidthMF.GetValue( ),
                    aLineWidthMF.GetDecimalDigits( ),
                    aLineWidthMF.GetUnit(), MAP_TWIP ));
        aFrameSel.SetStyleToSelection ( nVal,
            SvxBorderStyle( aLbLineStyle.GetSelectEntryStyle() ) );
    }

    return 0;
}

// ============================================================================
// ValueSet handling
// ============================================================================

// number of preset images to show
const sal_uInt16 SVX_BORDER_PRESET_COUNT = 5;

// number of shadow images to show
const sal_uInt16 SVX_BORDER_SHADOW_COUNT = 5;

// ----------------------------------------------------------------------------

sal_uInt16 SvxBorderTabPage::GetPresetImageId( sal_uInt16 nValueSetIdx ) const
{
    // table with all sets of predefined border styles
    static const sal_uInt16 ppnImgIds[][ SVX_BORDER_PRESET_COUNT ] =
    {
        // simple cell without diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_L          },
        // simple cell with diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_DIAG       },
        // with horizontal inner frame border
        {   IID_PRE_HOR_NONE,   IID_PRE_HOR_OUTER,      IID_PRE_HOR_HOR,        IID_PRE_HOR_ALL,    IID_PRE_HOR_OUTER2      },
        // with vertical inner frame border
        {   IID_PRE_VER_NONE,   IID_PRE_VER_OUTER,      IID_PRE_VER_VER,        IID_PRE_VER_ALL,    IID_PRE_VER_OUTER2      },
        // with horizontal and vertical inner frame borders
        {   IID_PRE_TABLE_NONE, IID_PRE_TABLE_OUTER,    IID_PRE_TABLE_OUTERH,   IID_PRE_TABLE_ALL,  IID_PRE_TABLE_OUTER2    }
    };

    // find correct set of presets
    int nLine = 0;
    if( !mbHorEnabled && !mbVerEnabled )
        nLine = (mbTLBREnabled || mbBLTREnabled) ? 1 : 0;
    else if( mbHorEnabled && !mbVerEnabled )
        nLine = 2;
    else if( !mbHorEnabled && mbVerEnabled )
        nLine = 3;
    else
        nLine = 4;

    DBG_ASSERT( (1 <= nValueSetIdx) && (nValueSetIdx <= SVX_BORDER_PRESET_COUNT),
        "SvxBorderTabPage::GetPresetImageId - wrong index" );
    return ppnImgIds[ nLine ][ nValueSetIdx - 1 ];
}

sal_uInt16 SvxBorderTabPage::GetPresetStringId( sal_uInt16 nValueSetIdx ) const
{
    // string resource IDs for each image (in order of the IID_PRE_* image IDs)
    static const sal_uInt16 pnStrIds[] =
    {
        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_PARA_PRESET_ALL,
        RID_SVXSTR_PARA_PRESET_LEFTRIGHT,
        RID_SVXSTR_PARA_PRESET_TOPBOTTOM,
        RID_SVXSTR_PARA_PRESET_ONLYLEFT,
        RID_SVXSTR_PARA_PRESET_DIAGONAL,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_HOR_PRESET_ONLYHOR,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_VER_PRESET_ONLYVER,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_TABLE_PRESET_OUTERHORI,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER
    };
    return pnStrIds[ GetPresetImageId( nValueSetIdx ) - 1 ];
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillPresetVS()
{
    ImageList& rImgList = aBorderImgLst;
    Size aImgSize( rImgList.GetImage( IID_PRE_CELL_NONE ).GetSizePixel() );

    // basic initialization of the ValueSet
    aWndPresets.SetColCount( SVX_BORDER_PRESET_COUNT );
    aWndPresets.SetStyle( aWndPresets.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aWndPresets.SetSizePixel( aWndPresets.CalcWindowSizePixel( aImgSize ) );

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_PRESET_COUNT; ++nVSIdx )
    {
        aWndPresets.InsertItem( nVSIdx );
        aWndPresets.SetItemImage( nVSIdx, rImgList.GetImage( GetPresetImageId( nVSIdx ) ) );
        aWndPresets.SetItemText( nVSIdx, CUI_RESSTR( GetPresetStringId( nVSIdx ) ) );
    }

    // show the control
    aWndPresets.SetNoSelection();
    aWndPresets.Show();
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillShadowVS()
{
    ImageList& rImgList = aShadowImgLst;
    Size aImgSize( rImgList.GetImage( IID_SHADOWNONE ).GetSizePixel() );

    // basic initialization of the ValueSet
    aWndShadows.SetColCount( SVX_BORDER_SHADOW_COUNT );
    aWndShadows.SetStyle( aWndShadows.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aWndShadows.SetSizePixel( aWndShadows.CalcWindowSizePixel( aImgSize ) );

    // image resource IDs
    static const sal_uInt16 pnImgIds[ SVX_BORDER_SHADOW_COUNT ] =
        { IID_SHADOWNONE, IID_SHADOW_BOT_RIGHT, IID_SHADOW_TOP_RIGHT, IID_SHADOW_BOT_LEFT, IID_SHADOW_TOP_LEFT };
    // string resource IDs for each image
    static const sal_uInt16 pnStrIds[ SVX_BORDER_SHADOW_COUNT ] =
        { RID_SVXSTR_SHADOW_STYLE_NONE, RID_SVXSTR_SHADOW_STYLE_BOTTOMRIGHT, RID_SVXSTR_SHADOW_STYLE_TOPRIGHT, RID_SVXSTR_SHADOW_STYLE_BOTTOMLEFT, RID_SVXSTR_SHADOW_STYLE_TOPLEFT };

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_SHADOW_COUNT; ++nVSIdx )
    {
        aWndShadows.InsertItem( nVSIdx );
        aWndShadows.SetItemImage( nVSIdx, rImgList.GetImage( pnImgIds[ nVSIdx - 1 ] ) );
        aWndShadows.SetItemText( nVSIdx, CUI_RESSTR( pnStrIds[ nVSIdx - 1 ] ) );
    }

    // show the control
    aWndShadows.SelectItem( 1 );
    aWndShadows.Show();
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillValueSets()
{
    FillPresetVS();
    FillShadowVS();
}

// ============================================================================
static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

void SvxBorderTabPage::FillLineListBox_Impl()
{
    using namespace ::com::sun::star::table::BorderLineStyle;

    aLbLineStyle.SetSourceUnit( FUNIT_TWIP );

    aLbLineStyle.SetNone( SVX_RESSTR( RID_SVXSTR_NONE ) );

    // Simple lines
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( SOLID ), SOLID );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( DOTTED ), DOTTED );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( DASHED ), DASHED );

    // Double lines
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( DOUBLE ), DOUBLE );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_SMALLGAP ), THINTHICK_SMALLGAP, 20 );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_MEDIUMGAP ), THINTHICK_MEDIUMGAP );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_LARGEGAP ), THINTHICK_LARGEGAP );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_SMALLGAP ), THICKTHIN_SMALLGAP, 20 );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_MEDIUMGAP ), THICKTHIN_MEDIUMGAP );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_LARGEGAP ), THICKTHIN_LARGEGAP );

    // Engraved / Embossed
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( EMBOSSED ), EMBOSSED, 15,
            &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor,
            &lcl_mediumColor );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( ENGRAVED ), ENGRAVED, 15,
            &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor,
            &lcl_mediumColor );

    // Inset / Outset
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( OUTSET ), OUTSET, 10,
           &SvxBorderLine::lightColor, &SvxBorderLine::darkColor );
    aLbLineStyle.InsertEntry( SvxBorderLine::getWidthImpl( INSET ), INSET, 10,
           &SvxBorderLine::darkColor, &SvxBorderLine::lightColor );

    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                aLineWidthMF.GetValue( ),
                aLineWidthMF.GetDecimalDigits( ),
                aLineWidthMF.GetUnit(), MAP_TWIP ));
    aLbLineStyle.SetWidth( nVal );
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(SvxBorderTabPage, LinesChanged_Impl)
{
    if(!mbUseMarginItem && aLeftMF.IsVisible())
    {
        sal_Bool bLineSet = aFrameSel.IsAnyBorderVisible();
        sal_Bool bMinAllowed = 0 != (nSWMode & (SW_BORDER_MODE_FRAME|SW_BORDER_MODE_TABLE));
        sal_Bool bSpaceModified =   aLeftMF  .IsModified()||
                                aRightMF .IsModified()||
                                aTopMF   .IsModified()||
                                aBottomMF.IsModified();

        if(bLineSet)
        {
            if(!bMinAllowed)
            {
                aLeftMF  .SetFirst(nMinValue);
                aRightMF .SetFirst(nMinValue);
                aTopMF   .SetFirst(nMinValue);
                aBottomMF.SetFirst(nMinValue);
            }
            if(!bSpaceModified)
            {
                aLeftMF  .SetValue(nMinValue);
                aRightMF .SetValue(nMinValue);
                aTopMF   .SetValue(nMinValue);
                aBottomMF.SetValue(nMinValue);
            }
        }
        else
        {
            aLeftMF  .SetMin(0);
            aRightMF .SetMin(0);
            aTopMF   .SetMin(0);
            aBottomMF.SetMin(0);
            aLeftMF  .SetFirst(0);
            aRightMF .SetFirst(0);
            aTopMF   .SetFirst(0);
            aBottomMF.SetFirst(0);
            if(!bSpaceModified)
            {
                aLeftMF  .SetValue(0);
                aRightMF .SetValue(0);
                aTopMF   .SetValue(0);
                aBottomMF.SetValue(0);
            }
        }
        // for tables everything is allowed
        sal_uInt16 nValid = VALID_TOP|VALID_BOTTOM|VALID_LEFT|VALID_RIGHT;

        // for border and paragraph the edit is disabled, if there's no border set
        if(nSWMode & (SW_BORDER_MODE_FRAME|SW_BORDER_MODE_PARA))
        {
            if(bLineSet)
            {
                nValid  = (aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_TOP)    == svx::FRAMESTATE_SHOW) ? VALID_TOP : 0;
                nValid |= (aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_BOTTOM) == svx::FRAMESTATE_SHOW) ? VALID_BOTTOM : 0;
                nValid |= (aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_LEFT)   == svx::FRAMESTATE_SHOW) ? VALID_LEFT : 0;
                nValid |= (aFrameSel.GetFrameBorderState( svx::FRAMEBORDER_RIGHT ) == svx::FRAMESTATE_SHOW) ? VALID_RIGHT : 0;
            }
            else
                nValid = 0;
        }
        aLeftFT.Enable(0 != (nValid&VALID_LEFT));
        aRightFT.Enable(0 != (nValid&VALID_RIGHT));
        aTopFT.Enable(0 != (nValid&VALID_TOP));
        aBottomFT.Enable(0 != (nValid&VALID_BOTTOM));
        aLeftMF.Enable(0 != (nValid&VALID_LEFT));
        aRightMF.Enable(0 != (nValid&VALID_RIGHT));
        aTopMF.Enable(0 != (nValid&VALID_TOP));
        aBottomMF.Enable(0 != (nValid&VALID_BOTTOM));
        aSynchronizeCB.Enable( aRightMF.IsEnabled() || aTopMF.IsEnabled() ||
                               aBottomMF.IsEnabled() || aLeftMF.IsEnabled() );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, ModifyDistanceHdl_Impl, MetricField*, pField)
{
    if ( mbSync )
    {
        sal_Int64 nVal = pField->GetValue();
        if(pField != &aLeftMF)
            aLeftMF.SetValue(nVal);
        if(pField != &aRightMF)
            aRightMF.SetValue(nVal);
        if(pField != &aTopMF)
            aTopMF.SetValue(nVal);
        if(pField != &aBottomMF)
            aBottomMF.SetValue(nVal);
    }
    return 0;
}

IMPL_LINK( SvxBorderTabPage, SyncHdl_Impl, CheckBox*, pBox)
{
    mbSync = pBox->IsChecked();
    return 0;
}

void SvxBorderTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        FillValueSets();

    SfxTabPage::DataChanged( rDCEvt );
}

void SvxBorderTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pSWModeItem,SfxUInt16Item,SID_SWMODE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
    if (pSWModeItem)
    {
        nSWMode = pSWModeItem->GetValue();
        // #i43593#
        // show checkbox <aMergeWithNextCB> for format.paragraph
        if ( nSWMode == SW_BORDER_MODE_PARA )
        {
            aMergeWithNextCB.Show();
            aPropertiesFL.Show();
        }
        // show checkbox <aMergeAdjacentBordersCB> for format.paragraph
        else if ( nSWMode == SW_BORDER_MODE_TABLE )
        {
            aMergeAdjacentBordersCB.Show();
            aPropertiesFL.Show();
        }
    }
    if (pFlagItem)
        if ( ( pFlagItem->GetValue() & SVX_HIDESHADOWCTL ) == SVX_HIDESHADOWCTL )
            HideShadowControls();
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
