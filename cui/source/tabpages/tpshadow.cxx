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

#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <cuires.hrc>
#include "tabarea.hrc"
#include <svx/svdattr.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <cuitabline.hxx>
#include "paragrph.hrc"
#include <svx/xlineit0.hxx>
#include <sfx2/request.hxx>

static sal_uInt16 pShadowRanges[] =
{
    SDRATTR_SHADOWCOLOR,
    SDRATTR_SHADOWTRANSPARENCE,
    SID_ATTR_FILL_SHADOW,
    SID_ATTR_FILL_SHADOW,
    0
};

SvxShadowTabPage::SvxShadowTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage          ( pParent,
                          "ShadowTabPage",
                          "cui/ui/shadowtabpage.ui",
                          rInAttrs ),
    rOutAttrs           ( rInAttrs ),
    eRP                 ( RP_LT ),
    nPageType           ( 0 ),
    nDlgType            ( 0 ),
    pbAreaTP            ( 0 ),
    bDisable            ( sal_False ),
    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    get(m_pTsbShowShadow,"TSB_SHOW_SHADOW");
    get(m_pGridShadow,"gridSHADOW");
    get(m_pCtlPosition,"CTL_POSITION");
    get(m_pMtrDistance,"MTR_FLD_DISTANCE");
    get(m_pLbShadowColor,"LB_SHADOW_COLOR");
    get(m_pMtrTransparent,"MTR_SHADOW_TRANSPARENT");
    get(m_pCtlXRectPreview,"CTL_COLOR_PREVIEW");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_pMtrDistance, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SDRATTR_SHADOWXDIST );

    // setting the output device
    XFillStyle eXFS = XFILL_SOLID;
    if( rOutAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXFS = (XFillStyle) ( ( ( const XFillStyleItem& ) rOutAttrs.
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
        switch( eXFS )
        {
            //case XFILL_NONE: --> NOTHING

            case XFILL_SOLID:
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rOutAttrs.Get( XATTR_FILLCOLOR ) );
                    rXFSet.Put( aColorItem );
                }
            break;

            case XFILL_GRADIENT:
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLGRADIENT ) )
                {
                    XFillGradientItem aGradientItem( ( ( const XFillGradientItem& )
                                            rOutAttrs.Get( XATTR_FILLGRADIENT ) ) );
                    rXFSet.Put( aGradientItem );
                }
            break;

            case XFILL_HATCH:
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    XFillHatchItem aHatchItem( ( ( const XFillHatchItem& )
                                    rOutAttrs.Get( XATTR_FILLHATCH ) ) );
                    rXFSet.Put( aHatchItem );
                }
            break;

            case XFILL_BITMAP:
            {
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    XFillBitmapItem aBitmapItem( ( const XFillBitmapItem& )
                                        rOutAttrs.Get( XATTR_FILLBITMAP ) );
                    rXFSet.Put( aBitmapItem );
                }
            }
            break;
            case XFILL_NONE : break;
        }
    }
    else
    {
        rXFSet.Put( XFillColorItem( String(), COL_LIGHTRED ) );
    }

    if(XFILL_NONE == eXFS)
    {
        // #i96350#
        // fallback to solid fillmode when no fill mode is provided to have
        // a reasonable shadow preview. The used color will be a set one or
        // the default (currently blue8)
        eXFS = XFILL_SOLID;
    }

    rXFSet.Put( XFillStyleItem( eXFS ) );
    m_pCtlXRectPreview->SetRectangleAttributes(aXFillAttr.GetItemSet());
    //aCtlXRectPreview.SetFillAttr( aXFillAttr );

    m_pTsbShowShadow->SetClickHdl( LINK( this, SvxShadowTabPage, ClickShadowHdl_Impl ) );
    Link aLink = LINK( this, SvxShadowTabPage, ModifyShadowHdl_Impl );
    m_pLbShadowColor->SetSelectHdl( aLink );
    m_pMtrTransparent->SetModifyHdl( aLink );
    m_pMtrDistance->SetModifyHdl( aLink );

}

// -----------------------------------------------------------------------

void SvxShadowTabPage::Construct()
{
    m_pLbShadowColor->Fill( pColorList );

    if( bDisable )
    {
        m_pTsbShowShadow->Disable();
        m_pGridShadow->Disable();
    }
}

// -----------------------------------------------------------------------

void SvxShadowTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_uInt16 nPos;
    sal_uInt16 nCount;

    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());

    if( nDlgType == 0 )
    {
        if( pColorList.is() )
        {
            // ColorList
            if( *pnColorListState & CT_CHANGED ||
                *pnColorListState & CT_MODIFIED )
            {
                if( *pnColorListState & CT_CHANGED )
                {
                    SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( GetParentDialog() );
                    if( pArea )
                    {
                        pColorList = pArea->GetNewColorList();
                    }
                    else
                    {
                        SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( GetParentDialog() );
                        if( pLine )
                            pColorList = pLine->GetNewColorList();
                    }
                }

                m_pCtlXRectPreview->SetRectangleAttributes(rSet);
                // aLbShadowColor
                nPos = m_pLbShadowColor->GetSelectEntryPos();
                m_pLbShadowColor->Clear();
                m_pLbShadowColor->Fill( pColorList );
                nCount = m_pLbShadowColor->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbShadowColor->SelectEntryPos( 0 );
                else
                    m_pLbShadowColor->SelectEntryPos( nPos );

                ModifyShadowHdl_Impl( this );
            }
            nPageType = PT_SHADOW;
        }
    }
}

// -----------------------------------------------------------------------

int SvxShadowTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

sal_Bool SvxShadowTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    const SfxPoolItem*  pOld = NULL;
    sal_Bool                bModified = sal_False;

    if( !bDisable )
    {
        TriState eState = m_pTsbShowShadow->GetState();
        if( eState != m_pTsbShowShadow->GetSavedValue() )
        {
            SdrShadowItem aItem( sal::static_int_cast< sal_Bool >( eState ) );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOW );
            if ( !pOld || !( *(const SdrShadowItem*)pOld == aItem ) )
            {
                rAttrs.Put( aItem );
                bModified = sal_True;
            }
        }

        // shadow removal
        // a bit intricate inquiry whether there was something changed,
        // as the items can't be displayed directly on controls
        sal_Int32 nX = 0L, nY = 0L;
        sal_Int32 nXY = GetCoreValue( *m_pMtrDistance, ePoolUnit );

        switch( m_pCtlPosition->GetActualRP() )
        {
            case RP_LT: nX = nY = -nXY;      break;
            case RP_MT: nY = -nXY;           break;
            case RP_RT: nX = nXY; nY = -nXY; break;
            case RP_LM: nX = -nXY;           break;
            case RP_RM: nX = nXY;            break;
            case RP_LB: nX = -nXY; nY = nXY; break;
            case RP_MB: nY = nXY;            break;
            case RP_RB: nX = nY = nXY;       break;
            case RP_MM: break;
        }

        // If the values of the shadow distances==SFX_ITEM_DONTCARE and the displayed
        // string in the respective MetricField=="", then the comparison of the old
        // and the new distance values would return a wrong result because in such a
        // case the new distance values would matche the default values of the MetricField !!!!
        if ( !m_pMtrDistance->IsEmptyFieldValue()                                  ||
             rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SFX_ITEM_DONTCARE ||
             rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SFX_ITEM_DONTCARE    )
        {
            sal_Int32 nOldX = 9876543; // impossible value, so DontCare
            sal_Int32 nOldY = 9876543;
            if( rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SFX_ITEM_DONTCARE &&
                rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SFX_ITEM_DONTCARE )
            {
                nOldX = ( ( const SdrShadowXDistItem& ) rOutAttrs.
                                    Get( SDRATTR_SHADOWXDIST ) ).GetValue();
                nOldY = ( ( const SdrShadowYDistItem& ) rOutAttrs.
                                    Get( SDRATTR_SHADOWYDIST ) ).GetValue();
            }
            SdrShadowXDistItem aXItem( nX );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWXDIST );
            if ( nX != nOldX &&
                ( !pOld || !( *(const SdrShadowXDistItem*)pOld == aXItem ) ) )
            {
                rAttrs.Put( aXItem );
                bModified = sal_True;
            }
            SdrShadowYDistItem aYItem( nY );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWYDIST );
            if ( nY != nOldY &&
                ( !pOld || !( *(const SdrShadowYDistItem*)pOld == aYItem ) ) )
            {
                rAttrs.Put( aYItem );
                bModified = sal_True;
            }
        }

        // ShadowColor
        sal_uInt16 nPos = m_pLbShadowColor->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != m_pLbShadowColor->GetSavedValue() )
        {
            SdrShadowColorItem aItem(m_pLbShadowColor->GetSelectEntryColor());
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWCOLOR );
            if ( !pOld || !( *(const SdrShadowColorItem*)pOld == aItem ) )
            {
                rAttrs.Put( aItem );
                bModified = sal_True;
            }
        }

        // transparency
        sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
        if( nVal != (sal_uInt16)m_pMtrTransparent->GetSavedValue().toInt32() )
        {
            SdrShadowTransparenceItem aItem( nVal );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWTRANSPARENCE );
            if ( !pOld || !( *(const SdrShadowTransparenceItem*)pOld == aItem ) )
            {
                rAttrs.Put( aItem );
                bModified = sal_True;
            }
        }
    }

    rAttrs.Put (CntUInt16Item(SID_PAGE_TYPE,nPageType));

    return( bModified );
}

// -----------------------------------------------------------------------

void SvxShadowTabPage::Reset( const SfxItemSet& rAttrs )
{
    if( !bDisable )
    {
        // all objects can have a shadow
        // at the moment there are only 8 possible positions where a shadow can be set

        // has a shadow been set?
        if( rAttrs.GetItemState( SDRATTR_SHADOW ) != SFX_ITEM_DONTCARE )
        {
            m_pTsbShowShadow->EnableTriState( sal_False );

            if( ( ( const SdrShadowItem& ) rAttrs.Get( SDRATTR_SHADOW ) ).GetValue() )
                m_pTsbShowShadow->SetState( STATE_CHECK );
            else
            {
                m_pTsbShowShadow->SetState( STATE_NOCHECK );
            }
        }
        else
            m_pTsbShowShadow->SetState( STATE_DONTKNOW );

        // distance (only 8 possible positions),
        // so there is only one item evaluated

        if( rAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SFX_ITEM_DONTCARE &&
            rAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SFX_ITEM_DONTCARE )
        {
            sal_Int32 nX = ( ( const SdrShadowXDistItem& ) rAttrs.Get( SDRATTR_SHADOWXDIST ) ).GetValue();
            sal_Int32 nY = ( ( const SdrShadowYDistItem& ) rAttrs.Get( SDRATTR_SHADOWYDIST ) ).GetValue();

            if( nX != 0 )
                SetMetricValue( *m_pMtrDistance, nX < 0L ? -nX : nX, ePoolUnit );
            else
                SetMetricValue( *m_pMtrDistance, nY < 0L ? -nY : nY, ePoolUnit );

            // setting the shadow control
            if     ( nX <  0L && nY <  0L ) m_pCtlPosition->SetActualRP( RP_LT );
            else if( nX == 0L && nY <  0L ) m_pCtlPosition->SetActualRP( RP_MT );
            else if( nX >  0L && nY <  0L ) m_pCtlPosition->SetActualRP( RP_RT );
            else if( nX <  0L && nY == 0L ) m_pCtlPosition->SetActualRP( RP_LM );
            // there's no center point anymore
            else if( nX == 0L && nY == 0L ) m_pCtlPosition->SetActualRP( RP_RB );
            else if( nX >  0L && nY == 0L ) m_pCtlPosition->SetActualRP( RP_RM );
            else if( nX <  0L && nY >  0L ) m_pCtlPosition->SetActualRP( RP_LB );
            else if( nX == 0L && nY >  0L ) m_pCtlPosition->SetActualRP( RP_MB );
            else if( nX >  0L && nY >  0L ) m_pCtlPosition->SetActualRP( RP_RB );
        }
        else
        {
            // determine default-distance
            SfxItemPool* pPool = rOutAttrs.GetPool();
            SdrShadowXDistItem* pXDistItem = (SdrShadowXDistItem*)&pPool->GetDefaultItem (SDRATTR_SHADOWXDIST);
            SdrShadowYDistItem* pYDistItem = (SdrShadowYDistItem*)&pPool->GetDefaultItem (SDRATTR_SHADOWYDIST);
            if (pXDistItem && pYDistItem)
            {
                sal_Int32 nX = pXDistItem->GetValue();
                sal_Int32 nY = pYDistItem->GetValue();
                if( nX != 0 )
                    SetMetricValue( *m_pMtrDistance, nX < 0L ? -nX : nX, ePoolUnit );
                else
                    SetMetricValue( *m_pMtrDistance, nY < 0L ? -nY : nY, ePoolUnit );
            }

            // Tristate, e. g. multiple objects have been marked of which some have a shadow and some don't.
            // The text (which shall be displayed) of the MetricFields is set to "" and serves as an
            // identification in the method FillItemSet for the fact that the distance value was NOT changed !!!!
            m_pMtrDistance->SetText( "" );
            m_pCtlPosition->SetActualRP( RP_MM );
        }

        if( rAttrs.GetItemState( SDRATTR_SHADOWCOLOR ) != SFX_ITEM_DONTCARE )
        {
            m_pLbShadowColor->SelectEntry( ( ( const SdrShadowColorItem& ) rAttrs.Get( SDRATTR_SHADOWCOLOR ) ).GetColorValue() );
        }
        else
            m_pLbShadowColor->SetNoSelection();

        if( rAttrs.GetItemState( SDRATTR_SHADOWTRANSPARENCE ) != SFX_ITEM_DONTCARE )
        {
            sal_uInt16 nTransp = ( ( const SdrShadowTransparenceItem& ) rAttrs.Get( SDRATTR_SHADOWTRANSPARENCE ) ).GetValue();
            m_pMtrTransparent->SetValue( nTransp );
        }
        else
            m_pMtrTransparent->SetText( "" );

        //aCtlPosition
        m_pMtrDistance->SaveValue();
        m_pLbShadowColor->SaveValue();
        m_pTsbShowShadow->SaveValue();

        // #66832# This field was not saved, but used to determine changes.
        // Why? Seems to be the error.
        // It IS the error.
        m_pMtrTransparent->SaveValue();

        ClickShadowHdl_Impl( NULL );
        ModifyShadowHdl_Impl( NULL );
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxShadowTabPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxShadowTabPage( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

sal_uInt16* SvxShadowTabPage::GetRanges()
{
    return( pShadowRanges );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxShadowTabPage, ClickShadowHdl_Impl)
{
    if( m_pTsbShowShadow->GetState() == STATE_NOCHECK )
    {
        m_pGridShadow->Disable();
    }
    else
    {
        m_pGridShadow->Enable();
    }
    m_pCtlPosition->Invalidate();

    ModifyShadowHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxShadowTabPage, ModifyShadowHdl_Impl)
{
    if( m_pTsbShowShadow->GetState() == STATE_CHECK )
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    sal_uInt16 nPos = m_pLbShadowColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillColorItem( String(), m_pLbShadowColor->GetSelectEntryColor() ) );
    }
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    XFillTransparenceItem aItem( nVal );
    rXFSet.Put( XFillTransparenceItem( aItem ) );

    // shadow removal
    sal_Int32 nX = 0L, nY = 0L;
    sal_Int32 nXY = GetCoreValue( *m_pMtrDistance, ePoolUnit );
    switch( m_pCtlPosition->GetActualRP() )
    {
        case RP_LT: nX = nY = -nXY;      break;
        case RP_MT: nY = -nXY;           break;
        case RP_RT: nX = nXY; nY = -nXY; break;
        case RP_LM: nX = -nXY;           break;
        case RP_RM: nX = nXY;            break;
        case RP_LB: nX = -nXY; nY = nXY; break;
        case RP_MB: nY = nXY;            break;
        case RP_RB: nX = nY = nXY;       break;
        case RP_MM: break;
    }

    m_pCtlXRectPreview->SetShadowPosition(Point(nX, nY));

    m_pCtlXRectPreview->SetShadowAttributes(aXFillAttr.GetItemSet());
    //aCtlXRectPreview.SetFillAttr( aXFillAttr );
    m_pCtlXRectPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxShadowTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    eRP = eRcPt;

    // repaint shadow
    ModifyShadowHdl_Impl( pWindow );
}

void SvxShadowTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pColorListItem,SvxColorListItem,SID_COLOR_TABLE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
