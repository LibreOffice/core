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
#include <sfx2/htmlmode.hxx>
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

SvxBorderTabPage::SvxBorderTabPage(Window* pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "BorderPage", "cui/ui/borderpage.ui", rCoreAttrs)

,




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
    get(m_pWndPresets, "presets");
    get(m_pUserDefFT, "userdefft");
    get(m_pFrameSel, "framesel");
    get(m_pLbLineStyle, "linestylelb");
    get(m_pLbLineColor, "linecolorlb");
    get(m_pLineWidthMF, "linewidthmf");

    get(m_pSpacingFrame, "spacing");
    get(m_pLeftFT, "leftft");
    get(m_pLeftMF, "leftmf");
    get(m_pRightFT, "rightft");
    get(m_pRightMF, "rightmf");
    get(m_pTopFT, "topft");
    get(m_pTopMF, "topmf");
    get(m_pBottomFT, "bottomft");
    get(m_pBottomMF, "bottommf");
    get(m_pSynchronizeCB, "sync");

    get(m_pShadowFrame, "shadow");
    get(m_pWndShadows, "shadows");
    get(m_pFtShadowSize, "distanceft");
    get(m_pEdShadowSize, "distancemf");
    get(m_pFtShadowColor, "shadowcolorft");
    get(m_pLbShadowColor, "shadowcolorlb");

    get(m_pPropertiesFrame, "properties");
    get(m_pMergeWithNextCB, "mergewithnext");
    get(m_pMergeAdjacentBordersCB, "mergeadjacent");

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

    SetFieldUnit(*m_pEdShadowSize, eFUnit);

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
            SetFieldUnit(*m_pLeftMF, eFUnit);
            SetFieldUnit(*m_pRightMF, eFUnit);
            SetFieldUnit(*m_pTopMF, eFUnit);
            SetFieldUnit(*m_pBottomMF, eFUnit);
            m_pSynchronizeCB->SetClickHdl(LINK(this, SvxBorderTabPage, SyncHdl_Impl));
            m_pLeftMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_pRightMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_pTopMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_pBottomMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
        }
        else
        {
            m_pSpacingFrame->Hide();
        }
        bIsDontCare = !pBoxInfo->IsValid( VALID_DISABLE );
    }
    if(!mbUseMarginItem && eFUnit == FUNIT_MM && SFX_MAPUNIT_TWIP == rCoreAttrs.GetPool()->GetMetric( GetWhich( SID_ATTR_BORDER_INNER ) ))
    {
        //#i91548# changing the number of decimal digits changes the minimum values, too
        lcl_SetDecimalDigitsTo1(*m_pLeftMF);
        lcl_SetDecimalDigitsTo1(*m_pRightMF);
        lcl_SetDecimalDigitsTo1(*m_pTopMF);
        lcl_SetDecimalDigitsTo1(*m_pBottomMF);
        lcl_SetDecimalDigitsTo1(*m_pEdShadowSize);
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
    m_pFrameSel->Initialize( nFlags );

    m_pFrameSel->SetSelectHdl(LINK(this, SvxBorderTabPage, LinesChanged_Impl));
    m_pLbLineStyle->SetSelectHdl( LINK( this, SvxBorderTabPage, SelStyleHdl_Impl ) );
    m_pLbLineColor->SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    m_pLineWidthMF->SetModifyHdl( LINK( this, SvxBorderTabPage, ModifyWidthHdl_Impl ) );
    m_pLbShadowColor->SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    m_pWndPresets->SetSelectHdl( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    m_pWndShadows->SetSelectHdl( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );

    FillValueSets();
    FillLineListBox_Impl();

    // fill ColorBox out of the XColorList
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }

    DBG_ASSERT( pColorTable.is(), "ColorTable not found!" );

    if ( pColorTable.is() )
    {
        // filling the line color box
        m_pLbLineColor->SetUpdateMode( sal_False );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            m_pLbLineColor->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        m_pLbLineColor->SetUpdateMode( sal_True );

        m_pLbShadowColor->CopyEntries(*m_pLbLineColor);
    }

    // connections

    bool bSupportsShadow = !SfxItemPool::IsSlot( GetWhich( SID_ATTR_BORDER_SHADOW ) );
    if( bSupportsShadow )
        AddItemConnection( svx::CreateShadowConnection( rCoreAttrs, *m_pWndShadows, *m_pEdShadowSize, *m_pLbShadowColor ) );
    else
        HideShadowControls();

    if( mbUseMarginItem )
        AddItemConnection( svx::CreateMarginConnection( rCoreAttrs, *m_pLeftMF, *m_pRightMF, *m_pTopMF, *m_pBottomMF ) );
    if( m_pFrameSel->IsBorderEnabled( svx::FRAMEBORDER_TLBR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_TLBR, *m_pFrameSel, svx::FRAMEBORDER_TLBR ) );
    if( m_pFrameSel->IsBorderEnabled( svx::FRAMEBORDER_BLTR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_BLTR, *m_pFrameSel, svx::FRAMEBORDER_BLTR ) );
    // #i43593# - item connection doesn't work for Writer,
    // because the Writer item sets contain these items
    // checkbox "Merge with next paragraph" only visible for Writer dialog format.paragraph
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_BORDER_CONNECT, *m_pMergeWithNextCB, sfx::ITEMCONN_DEFAULT ) );
    m_pMergeWithNextCB->Hide();
    // checkbox "Merge adjacent line styles" only visible for Writer dialog format.table
    AddItemConnection( new sfx::CheckBoxConnection( SID_SW_COLLAPSING_BORDERS, *m_pMergeAdjacentBordersCB, sfx::ITEMCONN_DEFAULT ) );
    m_pMergeAdjacentBordersCB->Hide();
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
    if( m_pFrameSel->IsBorderEnabled( eBorder ) )
    {
        if( bValid )
            m_pFrameSel->ShowBorder( eBorder, pCoreLine );
        else
            m_pFrameSel->SetBorderDontCare( eBorder );
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
            if ( m_pLeftMF->IsVisible() )
            {
                SetMetricValue(*m_pLeftMF,    pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_pRightMF,   pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_pTopMF,     pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_pBottomMF,  pBoxInfoItem->GetDefDist(), eCoreUnit);

                nMinValue = static_cast<long>(m_pLeftMF->GetValue());

                if ( pBoxInfoItem->IsMinDist() )
                {
                    m_pLeftMF->SetFirst( nMinValue );
                    m_pRightMF->SetFirst( nMinValue );
                    m_pTopMF->SetFirst( nMinValue );
                    m_pBottomMF->SetFirst( nMinValue );
                }

                if ( pBoxInfoItem->IsDist() )
                {
                    if( rSet.GetItemState( nWhichBox, sal_True ) >= SFX_ITEM_DEFAULT )
                    {
                        sal_Bool bIsAnyBorderVisible = m_pFrameSel->IsAnyBorderVisible();
                        if( !bIsAnyBorderVisible || !pBoxInfoItem->IsMinDist() )
                        {
                            m_pLeftMF->SetMin( 0 );
                            m_pLeftMF->SetFirst( 0 );
                            m_pRightMF->SetMin( 0 );
                            m_pRightMF->SetFirst( 0 );
                            m_pTopMF->SetMin( 0 );
                            m_pTopMF->SetFirst( 0 );
                            m_pBottomMF->SetMin( 0 );
                            m_pBottomMF->SetFirst( 0 );
                        }
                        long nLeftDist = pBoxItem->GetDistance( BOX_LINE_LEFT);
                        SetMetricValue(*m_pLeftMF, nLeftDist, eCoreUnit);
                        long nRightDist = pBoxItem->GetDistance( BOX_LINE_RIGHT);
                        SetMetricValue(*m_pRightMF, nRightDist, eCoreUnit);
                        long nTopDist = pBoxItem->GetDistance( BOX_LINE_TOP);
                        SetMetricValue( *m_pTopMF, nTopDist, eCoreUnit );
                        long nBottomDist = pBoxItem->GetDistance( BOX_LINE_BOTTOM);
                        SetMetricValue( *m_pBottomMF, nBottomDist, eCoreUnit );

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
                            m_pLeftMF->SetModifyFlag();
                            m_pRightMF->SetModifyFlag();
                            m_pTopMF->SetModifyFlag();
                            m_pBottomMF->SetModifyFlag();
                        }
                    }
                    else
                    {
                        // #106224# different margins -> do not fill the edits
                        m_pLeftMF->SetText( String() );
                        m_pRightMF->SetText( String() );
                        m_pTopMF->SetText( String() );
                        m_pBottomMF->SetText( String() );
                    }
                }
                m_pLeftMF->SaveValue();
                m_pRightMF->SaveValue();
                m_pTopMF->SaveValue();
                m_pBottomMF->SaveValue();
            }
        }
    }
    else
    {
        // avoid ResetFrameLine-calls:
        m_pFrameSel->HideAllBorders();
    }

    //-------------------------------------------------------------
    // depict line (color) in controllers if unambiguous:
    //-------------------------------------------------------------
    {
        // Do all visible lines show the same line widths?
        long nWidth;
        SvxBorderStyle nStyle;
        bool bWidthEq = m_pFrameSel->GetVisibleWidth( nWidth, nStyle );
        if( bWidthEq )
        {
            // Determine the width first as some styles can be missing depending on it
            sal_Int64 nWidthPt =  static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                        sal_Int64( nWidth ), m_pLineWidthMF->GetDecimalDigits( ),
                        MAP_TWIP,m_pLineWidthMF->GetUnit() ));
            m_pLineWidthMF->SetValue( nWidthPt );
            m_pLbLineStyle->SetWidth( nWidth );

            // then set the style
            m_pLbLineStyle->SelectEntry( nStyle );
        }
        else
            m_pLbLineStyle->SelectEntryPos( 1 );

        // Do all visible lines show the same line color?
        Color aColor;
        bool bColorEq = m_pFrameSel->GetVisibleColor( aColor );
        if( !bColorEq )
            aColor.SetColor( COL_BLACK );

        sal_uInt16 nSelPos = m_pLbLineColor->GetEntryPos( aColor );
        if( nSelPos == LISTBOX_ENTRY_NOTFOUND )
            nSelPos = m_pLbLineColor->InsertEntry( aColor, SVX_RESSTR( RID_SVXSTR_COLOR_USER ) );

        m_pLbLineColor->SelectEntryPos( nSelPos );
        m_pLbLineStyle->SetColor( aColor );

        // Select all visible lines, if they are all equal.
        if( bWidthEq && bColorEq )
            m_pFrameSel->SelectAllVisibleBorders();

        // set the current style and color (caches style in control even if nothing is selected)
        SelStyleHdl_Impl(m_pLbLineStyle);
        SelColHdl_Impl(m_pLbLineColor);
    }

    sal_Bool bEnable = m_pWndShadows->GetSelectItemId() > 1 ;
    m_pFtShadowSize->Enable(bEnable);
    m_pEdShadowSize->Enable(bEnable);
    m_pFtShadowColor->Enable(bEnable);
    m_pLbShadowColor->Enable(bEnable);

    m_pWndPresets->SetNoSelection();

    // - no line - should not be selected

    if ( m_pLbLineStyle->GetSelectEntryPos() == 0 )
    {
        m_pLbLineStyle->SelectEntryPos( 1 );
        SelStyleHdl_Impl(m_pLbLineStyle);
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
            m_pShadowFrame->Disable();

            if( !(nSWMode & SW_BORDER_MODE_TABLE) )
            {
                m_pUserDefFT->Disable();
                m_pFrameSel->Disable();
                m_pWndPresets->RemoveItem(3);
                m_pWndPresets->RemoveItem(4);
                m_pWndPresets->RemoveItem(5);
            }
        }
    }

    LinesChanged_Impl( 0 );
    if(m_pLeftMF->GetValue() == m_pRightMF->GetValue() && m_pTopMF->GetValue() == m_pBottomMF->GetValue() && m_pTopMF->GetValue() == m_pLeftMF->GetValue())
        mbSync = true;
    else
        mbSync = false;
    m_pSynchronizeCB->Check(mbSync);
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
        aBoxItem.SetLine( m_pFrameSel->GetFrameBorderStyle( eTypes1[i].first ), eTypes1[i].second );

    //--------------------------------
    // border hor/ver and TableFlag
    //--------------------------------
    TBorderPair eTypes2[] = {
                                TBorderPair(svx::FRAMEBORDER_HOR,BOXINFO_LINE_HORI),
                                TBorderPair(svx::FRAMEBORDER_VER,BOXINFO_LINE_VERT)
                            };
    for (sal_uInt32 j=0; j < SAL_N_ELEMENTS(eTypes2); ++j)
        aBoxInfoItem.SetLine( m_pFrameSel->GetFrameBorderStyle( eTypes2[j].first ), eTypes2[j].second );

    aBoxInfoItem.EnableHor( mbHorEnabled );
    aBoxInfoItem.EnableVer( mbVerEnabled );

    //-------------------
    // inner distance
    //-------------------
    if( m_pLeftMF->IsVisible() )
    {
        // #i40405# enable distance controls for next dialog call
        aBoxInfoItem.SetDist( sal_True );

        if( !mbUseMarginItem )
        {
            // #106224# all edits empty: do nothing
            if( !m_pLeftMF->GetText().isEmpty() || !m_pRightMF->GetText().isEmpty() ||
                !m_pTopMF->GetText().isEmpty() || !m_pBottomMF->GetText().isEmpty() )
            {
                if ( ((mbHorEnabled || mbVerEnabled || (nSWMode & SW_BORDER_MODE_TABLE)) &&
                        (m_pLeftMF->IsModified()||m_pRightMF->IsModified()||
                            m_pTopMF->IsModified()||m_pBottomMF->IsModified()) )||
                     m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_TOP ) != svx::FRAMESTATE_HIDE
                     || m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_BOTTOM ) != svx::FRAMESTATE_HIDE
                     || m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_LEFT ) != svx::FRAMESTATE_HIDE
                     || m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_RIGHT ) != svx::FRAMESTATE_HIDE )
                {
                    SvxBoxInfoItem* pOldBoxInfoItem = (SvxBoxInfoItem*)GetOldItem(
                                                        rCoreAttrs, SID_ATTR_BORDER_INNER );
                    if (
                        !pOldBoxItem ||
                        m_pLeftMF->GetText() != m_pLeftMF->GetSavedValue() ||
                        m_pRightMF->GetText() != m_pRightMF->GetSavedValue() ||
                        m_pTopMF->GetText() != m_pTopMF->GetSavedValue() ||
                        m_pBottomMF->GetText() != m_pBottomMF->GetSavedValue() ||
                        nMinValue == m_pLeftMF->GetValue() ||
                        nMinValue == m_pRightMF->GetValue() ||
                        nMinValue == m_pTopMF->GetValue() ||
                        nMinValue == m_pBottomMF->GetValue() ||
                        (pOldBoxInfoItem && !pOldBoxInfoItem->IsValid(VALID_DISTANCE))
                       )
                    {
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pLeftMF, eCoreUnit ), BOX_LINE_LEFT  );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pRightMF, eCoreUnit ), BOX_LINE_RIGHT );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pTopMF, eCoreUnit ), BOX_LINE_TOP   );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pBottomMF, eCoreUnit ), BOX_LINE_BOTTOM);
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
    aBoxInfoItem.SetValid( VALID_TOP,    m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_TOP )    != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_BOTTOM, m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_BOTTOM ) != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_LEFT,   m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_LEFT )   != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_RIGHT,  m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_RIGHT )  != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_HORI,   m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_HOR )    != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_VERT,   m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_VER )    != svx::FRAMESTATE_DONTCARE );

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
        const SfxPoolItem* pOld = GetOldItem( rCoreAttrs, SID_ATTR_BORDER_INNER, sal_False );

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
    m_pShadowFrame->Hide();
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
    m_pFrameSel->HideAllBorders();
    m_pFrameSel->DeselectAllBorders();

    // Using image ID to find correct line in table above.
    sal_uInt16 nLine = GetPresetImageId( m_pWndPresets->GetSelectItemId() ) - 1;

    // Apply all styles from the table
    for( int nBorder = 0; nBorder < svx::FRAMEBORDERTYPE_COUNT; ++nBorder )
    {
        svx::FrameBorderType eBorder = svx::GetFrameBorderTypeFromIndex( nBorder );
        switch( ppeStates[ nLine ][ nBorder ] )
        {
            case SHOW:  m_pFrameSel->SelectBorder( eBorder );      break;
            case HIDE:  /* nothing to do */                     break;
            case DONT:  m_pFrameSel->SetBorderDontCare( eBorder ); break;
        }
    }

    // Show all lines that have been selected above
    if( m_pFrameSel->IsAnyBorderSelected() )
    {
        // any visible style, but "no-line" in line list box? -> use hair-line
        if( (m_pLbLineStyle->GetSelectEntryPos() == 0) || (m_pLbLineStyle->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND) )
            m_pLbLineStyle->SelectEntryPos( 1 );

        // set current style to all previously selected lines
        SelStyleHdl_Impl(m_pLbLineStyle);
        SelColHdl_Impl(m_pLbLineColor);
    }

    // Presets ValueSet does not show a selection (used as push buttons).
    m_pWndPresets->SetNoSelection();

    LinesChanged_Impl( 0 );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBorderTabPage, SelSdwHdl_Impl)
{
    sal_Bool bEnable = m_pWndShadows->GetSelectItemId() > 1;
    m_pFtShadowSize->Enable(bEnable);
    m_pEdShadowSize->Enable(bEnable);
    m_pFtShadowColor->Enable(bEnable);
    m_pLbShadowColor->Enable(bEnable);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelColHdl_Impl, ListBox *, pLb )
{
    ColorListBox* pColLb = (ColorListBox*)(pLb);

    if (pLb == m_pLbLineColor)
    {
        m_pFrameSel->SetColorToSelection( pColLb->GetSelectEntryColor() );
        m_pLbLineStyle->SetColor( pColLb->GetSelectEntryColor() );
    }

    return 0;
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthHdl_Impl)
{
    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                m_pLineWidthMF->GetValue( ),
                m_pLineWidthMF->GetDecimalDigits( ),
                m_pLineWidthMF->GetUnit(), MAP_TWIP ));
    m_pLbLineStyle->SetWidth( nVal );

    m_pFrameSel->SetStyleToSelection( nVal,
        SvxBorderStyle( m_pLbLineStyle->GetSelectEntryStyle() ) );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelStyleHdl_Impl, ListBox *, pLb )
{
    if (pLb == m_pLbLineStyle)
    {
        sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                    m_pLineWidthMF->GetValue( ),
                    m_pLineWidthMF->GetDecimalDigits( ),
                    m_pLineWidthMF->GetUnit(), MAP_TWIP ));
        m_pFrameSel->SetStyleToSelection ( nVal,
            SvxBorderStyle( m_pLbLineStyle->GetSelectEntryStyle() ) );
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

    // basic initialization of the ValueSet
    m_pWndPresets->SetStyle( m_pWndPresets->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    m_pWndPresets->SetColCount( SVX_BORDER_PRESET_COUNT );

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_PRESET_COUNT; ++nVSIdx )
    {
        m_pWndPresets->InsertItem( nVSIdx );
        m_pWndPresets->SetItemImage( nVSIdx, rImgList.GetImage( GetPresetImageId( nVSIdx ) ) );
        m_pWndPresets->SetItemText( nVSIdx, CUI_RESSTR( GetPresetStringId( nVSIdx ) ) );
    }

    // show the control
    m_pWndPresets->SetNoSelection();
    m_pWndPresets->Show();
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillShadowVS()
{
    ImageList& rImgList = aShadowImgLst;

    // basic initialization of the ValueSet
    m_pWndShadows->SetStyle( m_pWndShadows->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    m_pWndShadows->SetColCount( SVX_BORDER_SHADOW_COUNT );

    // image resource IDs
    static const sal_uInt16 pnImgIds[ SVX_BORDER_SHADOW_COUNT ] =
        { IID_SHADOWNONE, IID_SHADOW_BOT_RIGHT, IID_SHADOW_TOP_RIGHT, IID_SHADOW_BOT_LEFT, IID_SHADOW_TOP_LEFT };
    // string resource IDs for each image
    static const sal_uInt16 pnStrIds[ SVX_BORDER_SHADOW_COUNT ] =
        { RID_SVXSTR_SHADOW_STYLE_NONE, RID_SVXSTR_SHADOW_STYLE_BOTTOMRIGHT, RID_SVXSTR_SHADOW_STYLE_TOPRIGHT, RID_SVXSTR_SHADOW_STYLE_BOTTOMLEFT, RID_SVXSTR_SHADOW_STYLE_TOPLEFT };

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_SHADOW_COUNT; ++nVSIdx )
    {
        m_pWndShadows->InsertItem( nVSIdx );
        m_pWndShadows->SetItemImage( nVSIdx, rImgList.GetImage( pnImgIds[ nVSIdx - 1 ] ) );
        m_pWndShadows->SetItemText( nVSIdx, CUI_RESSTR( pnStrIds[ nVSIdx - 1 ] ) );
    }

    // show the control
    m_pWndShadows->SelectItem( 1 );
    m_pWndShadows->Show();
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

    m_pLbLineStyle->SetSourceUnit( FUNIT_TWIP );

    m_pLbLineStyle->SetNone( SVX_RESSTR( RID_SVXSTR_NONE ) );

    // Simple lines
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( SOLID ), SOLID );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( DOTTED ), DOTTED );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( DASHED ), DASHED );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( FINE_DASHED ), FINE_DASHED );

    // Double lines
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( DOUBLE ), DOUBLE );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_SMALLGAP ), THINTHICK_SMALLGAP, 20 );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_MEDIUMGAP ), THINTHICK_MEDIUMGAP );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_LARGEGAP ), THINTHICK_LARGEGAP );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_SMALLGAP ), THICKTHIN_SMALLGAP, 20 );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_MEDIUMGAP ), THICKTHIN_MEDIUMGAP );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_LARGEGAP ), THICKTHIN_LARGEGAP );

    // Engraved / Embossed
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( EMBOSSED ), EMBOSSED, 15,
            &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor,
            &lcl_mediumColor );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( ENGRAVED ), ENGRAVED, 15,
            &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor,
            &lcl_mediumColor );

    // Inset / Outset
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( OUTSET ), OUTSET, 10,
           &SvxBorderLine::lightColor, &SvxBorderLine::darkColor );
    m_pLbLineStyle->InsertEntry( SvxBorderLine::getWidthImpl( INSET ), INSET, 10,
           &SvxBorderLine::darkColor, &SvxBorderLine::lightColor );

    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                m_pLineWidthMF->GetValue( ),
                m_pLineWidthMF->GetDecimalDigits( ),
                m_pLineWidthMF->GetUnit(), MAP_TWIP ));
    m_pLbLineStyle->SetWidth( nVal );
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(SvxBorderTabPage, LinesChanged_Impl)
{
    if(!mbUseMarginItem && m_pLeftMF->IsVisible())
    {
        sal_Bool bLineSet = m_pFrameSel->IsAnyBorderVisible();
        sal_Bool bMinAllowed = 0 != (nSWMode & (SW_BORDER_MODE_FRAME|SW_BORDER_MODE_TABLE));
        sal_Bool bSpaceModified =   m_pLeftMF->IsModified()||
                                m_pRightMF->IsModified()||
                                m_pTopMF->IsModified()||
                                m_pBottomMF->IsModified();

        if(bLineSet)
        {
            if(!bMinAllowed)
            {
                m_pLeftMF->SetFirst(nMinValue);
                m_pRightMF->SetFirst(nMinValue);
                m_pTopMF->SetFirst(nMinValue);
                m_pBottomMF->SetFirst(nMinValue);
            }
            if(!bSpaceModified)
            {
                m_pLeftMF->SetValue(nMinValue);
                m_pRightMF->SetValue(nMinValue);
                m_pTopMF->SetValue(nMinValue);
                m_pBottomMF->SetValue(nMinValue);
            }
        }
        else
        {
            m_pLeftMF->SetMin(0);
            m_pRightMF->SetMin(0);
            m_pTopMF->SetMin(0);
            m_pBottomMF->SetMin(0);
            m_pLeftMF->SetFirst(0);
            m_pRightMF->SetFirst(0);
            m_pTopMF->SetFirst(0);
            m_pBottomMF->SetFirst(0);
            if(!bSpaceModified)
            {
                m_pLeftMF->SetValue(0);
                m_pRightMF->SetValue(0);
                m_pTopMF->SetValue(0);
                m_pBottomMF->SetValue(0);
            }
        }
        // for tables everything is allowed
        sal_uInt16 nValid = VALID_TOP|VALID_BOTTOM|VALID_LEFT|VALID_RIGHT;

        // for other objects (paragraph, page, frame) the edit is disabled, if there's no border set
        if(!(nSWMode & SW_BORDER_MODE_TABLE))
        {
            if(bLineSet)
            {
                nValid  = (m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_TOP)    == svx::FRAMESTATE_SHOW) ? VALID_TOP : 0;
                nValid |= (m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_BOTTOM) == svx::FRAMESTATE_SHOW) ? VALID_BOTTOM : 0;
                nValid |= (m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_LEFT)   == svx::FRAMESTATE_SHOW) ? VALID_LEFT : 0;
                nValid |= (m_pFrameSel->GetFrameBorderState( svx::FRAMEBORDER_RIGHT ) == svx::FRAMESTATE_SHOW) ? VALID_RIGHT : 0;
            }
            else
                nValid = 0;
        }
        m_pLeftFT->Enable(0 != (nValid&VALID_LEFT));
        m_pRightFT->Enable(0 != (nValid&VALID_RIGHT));
        m_pTopFT->Enable(0 != (nValid&VALID_TOP));
        m_pBottomFT->Enable(0 != (nValid&VALID_BOTTOM));
        m_pLeftMF->Enable(0 != (nValid&VALID_LEFT));
        m_pRightMF->Enable(0 != (nValid&VALID_RIGHT));
        m_pTopMF->Enable(0 != (nValid&VALID_TOP));
        m_pBottomMF->Enable(0 != (nValid&VALID_BOTTOM));
        m_pSynchronizeCB->Enable( m_pRightMF->IsEnabled() || m_pTopMF->IsEnabled() ||
                               m_pBottomMF->IsEnabled() || m_pLeftMF->IsEnabled() );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, ModifyDistanceHdl_Impl, MetricField*, pField)
{
    if ( mbSync )
    {
        sal_Int64 nVal = pField->GetValue();
        if(pField != m_pLeftMF)
            m_pLeftMF->SetValue(nVal);
        if(pField != m_pRightMF)
            m_pRightMF->SetValue(nVal);
        if(pField != m_pTopMF)
            m_pTopMF->SetValue(nVal);
        if(pField != m_pBottomMF)
            m_pBottomMF->SetValue(nVal);
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
        // show checkbox <m_pMergeWithNextCB> for format.paragraph
        if ( nSWMode == SW_BORDER_MODE_PARA )
        {
            m_pMergeWithNextCB->Show();
            m_pPropertiesFrame->Show();
        }
        // show checkbox <m_pMergeAdjacentBordersCB> for format.paragraph
        else if ( nSWMode == SW_BORDER_MODE_TABLE )
        {
            m_pMergeAdjacentBordersCB->Show();
            m_pPropertiesFrame->Show();
        }
    }
    if (pFlagItem)
        if ( ( pFlagItem->GetValue() & SVX_HIDESHADOWCTL ) == SVX_HIDESHADOWCTL )
            HideShadowControls();
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
