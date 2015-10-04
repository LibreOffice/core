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
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <cuires.hrc>
#include <svx/svdattr.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <cuitabline.hxx>
#include "paragrph.hrc"
#include <svx/xlineit0.hxx>
#include <sfx2/request.hxx>

using namespace com::sun::star;

const sal_uInt16 SvxShadowTabPage::pShadowRanges[] =
{
    SDRATTR_SHADOWCOLOR,
    SDRATTR_SHADOWTRANSPARENCE,
    SID_ATTR_FILL_SHADOW,
    SID_ATTR_FILL_SHADOW,
    SID_ATTR_SHADOW_TRANSPARENCE,
    SID_ATTR_SHADOW_YDISTANCE,
    0
};

SvxShadowTabPage::SvxShadowTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage          ( pParent,
                          "ShadowTabPage",
                          "cui/ui/shadowtabpage.ui",
                          rInAttrs ),
    m_rOutAttrs           ( rInAttrs ),
    m_eRP                 ( RP_LT ),
    m_pnColorListState    ( 0 ),
    m_nPageType           ( 0 ),
    m_nDlgType            ( 0 ),
    m_pbAreaTP            ( 0 ),
    m_bDisable            ( false ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
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
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    m_ePoolUnit = pPool->GetMetric( SDRATTR_SHADOWXDIST );

    // setting the output device
    drawing::FillStyle eXFS = drawing::FillStyle_SOLID;
    if( m_rOutAttrs.GetItemState( XATTR_FILLSTYLE ) != SfxItemState::DONTCARE )
    {
        eXFS = (drawing::FillStyle) ( static_cast<const XFillStyleItem&>( m_rOutAttrs.
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
        switch( eXFS )
        {
            case drawing::FillStyle_SOLID:
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( static_cast<const XFillColorItem&>(
                                        m_rOutAttrs.Get( XATTR_FILLCOLOR ) ) );
                    m_rXFSet.Put( aColorItem );
                }
            break;

            case drawing::FillStyle_GRADIENT:
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLGRADIENT ) )
                {
                    XFillGradientItem aGradientItem( static_cast<const XFillGradientItem&>(
                                            m_rOutAttrs.Get( XATTR_FILLGRADIENT ) ) );
                    m_rXFSet.Put( aGradientItem );
                }
            break;

            case drawing::FillStyle_HATCH:
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    XFillHatchItem aHatchItem( static_cast<const XFillHatchItem& >(
                                    m_rOutAttrs.Get( XATTR_FILLHATCH ) ) );
                    m_rXFSet.Put( aHatchItem );
                }
            break;

            case drawing::FillStyle_BITMAP:
            {
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    XFillBitmapItem aBitmapItem( static_cast<const XFillBitmapItem& >(
                                        m_rOutAttrs.Get( XATTR_FILLBITMAP ) ) );
                    m_rXFSet.Put( aBitmapItem );
                }
            }
            break;
            case drawing::FillStyle_NONE : break;
            default: break;
        }
    }
    else
    {
        m_rXFSet.Put( XFillColorItem( OUString(), COL_LIGHTRED ) );
    }

    if(drawing::FillStyle_NONE == eXFS)
    {
        // #i96350#
        // fallback to solid fillmode when no fill mode is provided to have
        // a reasonable shadow preview. The used color will be a set one or
        // the default (currently blue8)
        eXFS = drawing::FillStyle_SOLID;
    }

    m_rXFSet.Put( XFillStyleItem( eXFS ) );
    m_pCtlXRectPreview->SetRectangleAttributes(m_aXFillAttr.GetItemSet());
    //aCtlXRectPreview.SetFillAttr( aXFillAttr );

    m_pTsbShowShadow->SetClickHdl( LINK( this, SvxShadowTabPage, ClickShadowHdl_Impl ) );
    m_pLbShadowColor->SetSelectHdl( LINK( this, SvxShadowTabPage, SelectShadowHdl_Impl ) );
    Link<> aLink = LINK( this, SvxShadowTabPage, ModifyShadowHdl_Impl );
    m_pMtrTransparent->SetModifyHdl( aLink );
    m_pMtrDistance->SetModifyHdl( aLink );
}

SvxShadowTabPage::~SvxShadowTabPage()
{
    disposeOnce();
}

void SvxShadowTabPage::dispose()
{
    m_pTsbShowShadow.clear();
    m_pGridShadow.clear();
    m_pCtlPosition.clear();
    m_pMtrDistance.clear();
    m_pLbShadowColor.clear();
    m_pMtrTransparent.clear();
    m_pCtlXRectPreview.clear();
    SvxTabPage::dispose();
}

void SvxShadowTabPage::Construct()
{
    m_pLbShadowColor->Fill( m_pColorList );

    if( m_bDisable )
    {
        m_pTsbShowShadow->Disable();
        m_pGridShadow->Disable();
    }
}



void SvxShadowTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_Int32 nPos;
    sal_Int32 nCount;

    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,false);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());

    if( m_nDlgType == 0 )
    {
        if( m_pColorList.is() )
        {
            // ColorList
            if( *m_pnColorListState & ChangeType::CHANGED ||
                *m_pnColorListState & ChangeType::MODIFIED )
            {
                if( *m_pnColorListState & ChangeType::CHANGED )
                {
                    SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( GetParentDialog() );
                    if( pArea )
                    {
                        m_pColorList = pArea->GetNewColorList();
                    }
                    else
                    {
                        SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( GetParentDialog() );
                        if( pLine )
                            m_pColorList = pLine->GetNewColorList();
                    }
                }

                // aLbShadowColor
                nPos = m_pLbShadowColor->GetSelectEntryPos();
                m_pLbShadowColor->Clear();
                m_pLbShadowColor->Fill( m_pColorList );
                nCount = m_pLbShadowColor->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbShadowColor->SelectEntryPos( 0 );
                else
                    m_pLbShadowColor->SelectEntryPos( nPos );

                SfxItemSet rAttribs( rSet );
                // rSet contains shadow attributes too, but we want
                // to use it for updating rectangle attributes only,
                // so set the shadow to none here
                SdrOnOffItem aItem( makeSdrShadowItem( false ));
                rAttribs.Put( aItem );

                m_pCtlXRectPreview->SetRectangleAttributes( rAttribs );
                ModifyShadowHdl_Impl( this );
            }
            m_nPageType = PT_SHADOW;
        }
    }
}



SfxTabPage::sfxpg SvxShadowTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return LEAVE_PAGE;
}



bool SvxShadowTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    bool                bModified = false;

    if( !m_bDisable )
    {
        const SfxPoolItem*  pOld = NULL;

        if( m_pTsbShowShadow->IsValueChangedFromSaved() )
        {
            TriState eState = m_pTsbShowShadow->GetState();
            assert(eState != TRISTATE_INDET);
                // given how m_pTsbShowShadow is set up and saved in Reset(),
                // eState == TRISTATE_INDET would imply
                // !IsValueChangedFromSaved()
            SdrOnOffItem aItem( makeSdrShadowItem(eState == TRISTATE_TRUE) );
            pOld = GetOldItem( *rAttrs, SDRATTR_SHADOW );
            if ( !pOld || !( *static_cast<const SdrOnOffItem*>(pOld) == aItem ) )
            {
                rAttrs->Put( aItem );
                bModified = true;
            }
        }

        // shadow removal
        // a bit intricate inquiry whether there was something changed,
        // as the items can't be displayed directly on controls
        sal_Int32 nX = 0L, nY = 0L;
        sal_Int32 nXY = GetCoreValue( *m_pMtrDistance, m_ePoolUnit );

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

        // If the values of the shadow distances==SfxItemState::DONTCARE and the displayed
        // string in the respective MetricField=="", then the comparison of the old
        // and the new distance values would return a wrong result because in such a
        // case the new distance values would match the default values of the MetricField !!!!
        if ( !m_pMtrDistance->IsEmptyFieldValue()                                  ||
             m_rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SfxItemState::DONTCARE ||
             m_rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SfxItemState::DONTCARE    )
        {
            sal_Int32 nOldX = 9876543; // impossible value, so DontCare
            sal_Int32 nOldY = 9876543;
            if( m_rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SfxItemState::DONTCARE &&
                m_rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SfxItemState::DONTCARE )
            {
                nOldX = static_cast<const SdrMetricItem&>( m_rOutAttrs.
                                    Get( SDRATTR_SHADOWXDIST ) ).GetValue();
                nOldY = static_cast<const SdrMetricItem&>( m_rOutAttrs.
                                    Get( SDRATTR_SHADOWYDIST ) ).GetValue();
            }
            SdrMetricItem aXItem( makeSdrShadowXDistItem(nX) );
            pOld = GetOldItem( *rAttrs, SDRATTR_SHADOWXDIST );
            if ( nX != nOldX &&
                ( !pOld || !( *static_cast<const SdrMetricItem*>(pOld) == aXItem ) ) )
            {
                rAttrs->Put( aXItem );
                bModified = true;
            }
            SdrMetricItem aYItem( makeSdrShadowYDistItem(nY) );
            pOld = GetOldItem( *rAttrs, SDRATTR_SHADOWYDIST );
            if ( nY != nOldY &&
                ( !pOld || !( *static_cast<const SdrMetricItem*>(pOld) == aYItem ) ) )
            {
                rAttrs->Put( aYItem );
                bModified = true;
            }
        }

        // ShadowColor
        sal_Int32 nPos = m_pLbShadowColor->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            m_pLbShadowColor->IsValueChangedFromSaved() )
        {
            XColorItem aItem(makeSdrShadowColorItem(m_pLbShadowColor->GetSelectEntryColor()));
            pOld = GetOldItem( *rAttrs, SDRATTR_SHADOWCOLOR );
            if ( !pOld || !( *static_cast<const XColorItem*>(pOld) == aItem ) )
            {
                rAttrs->Put( aItem );
                bModified = true;
            }
        }

        // transparency
        sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
        if( m_pMtrTransparent->IsValueChangedFromSaved() )
        {
            SdrPercentItem aItem( makeSdrShadowTransparenceItem(nVal) );
            pOld = GetOldItem( *rAttrs, SDRATTR_SHADOWTRANSPARENCE );
            if ( !pOld || !( *static_cast<const SdrPercentItem*>(pOld) == aItem ) )
            {
                rAttrs->Put( aItem );
                bModified = true;
            }
        }
    }

    rAttrs->Put (CntUInt16Item(SID_PAGE_TYPE,m_nPageType));

    return bModified;
}



void SvxShadowTabPage::Reset( const SfxItemSet* rAttrs )
{
    if( !m_bDisable )
    {
        // all objects can have a shadow
        // at the moment there are only 8 possible positions where a shadow can be set

        // has a shadow been set?
        if( rAttrs->GetItemState( SDRATTR_SHADOW ) != SfxItemState::DONTCARE )
        {
            m_pTsbShowShadow->EnableTriState( false );

            if( static_cast<const SdrOnOffItem&>( rAttrs->Get( SDRATTR_SHADOW ) ).GetValue() )
                m_pTsbShowShadow->SetState( TRISTATE_TRUE );
            else
            {
                m_pTsbShowShadow->SetState( TRISTATE_FALSE );
            }
        }
        else
            m_pTsbShowShadow->SetState( TRISTATE_INDET );

        // distance (only 8 possible positions),
        // so there is only one item evaluated

        if( rAttrs->GetItemState( SDRATTR_SHADOWXDIST ) != SfxItemState::DONTCARE &&
            rAttrs->GetItemState( SDRATTR_SHADOWYDIST ) != SfxItemState::DONTCARE )
        {
            sal_Int32 nX = static_cast<const SdrMetricItem&>( rAttrs->Get( SDRATTR_SHADOWXDIST ) ).GetValue();
            sal_Int32 nY = static_cast<const SdrMetricItem&>( rAttrs->Get( SDRATTR_SHADOWYDIST ) ).GetValue();

            if( nX != 0 )
                SetMetricValue( *m_pMtrDistance, nX < 0L ? -nX : nX, m_ePoolUnit );
            else
                SetMetricValue( *m_pMtrDistance, nY < 0L ? -nY : nY, m_ePoolUnit );

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
            SfxItemPool* pPool = m_rOutAttrs.GetPool();
            const SdrMetricItem* pXDistItem = static_cast<const SdrMetricItem*>(&pPool->GetDefaultItem(SDRATTR_SHADOWXDIST));
            const SdrMetricItem* pYDistItem = static_cast<const SdrMetricItem*>(&pPool->GetDefaultItem(SDRATTR_SHADOWYDIST));
            if (pXDistItem && pYDistItem)
            {
                sal_Int32 nX = pXDistItem->GetValue();
                sal_Int32 nY = pYDistItem->GetValue();
                if( nX != 0 )
                    SetMetricValue( *m_pMtrDistance, nX < 0L ? -nX : nX, m_ePoolUnit );
                else
                    SetMetricValue( *m_pMtrDistance, nY < 0L ? -nY : nY, m_ePoolUnit );
            }

            // Tristate, e. g. multiple objects have been marked of which some have a shadow and some don't.
            // The text (which shall be displayed) of the MetricFields is set to "" and serves as an
            // identification in the method FillItemSet for the fact that the distance value was NOT changed !!!!
            m_pMtrDistance->SetText( "" );
            m_pCtlPosition->SetActualRP( RP_MM );
        }

        if( rAttrs->GetItemState( SDRATTR_SHADOWCOLOR ) != SfxItemState::DONTCARE )
        {
            m_pLbShadowColor->SelectEntry( static_cast<const XColorItem&>( rAttrs->Get( SDRATTR_SHADOWCOLOR ) ).GetColorValue() );
        }
        else
            m_pLbShadowColor->SetNoSelection();

        if( rAttrs->GetItemState( SDRATTR_SHADOWTRANSPARENCE ) != SfxItemState::DONTCARE )
        {
            sal_uInt16 nTransp = static_cast<const SdrPercentItem&>( rAttrs->Get( SDRATTR_SHADOWTRANSPARENCE ) ).GetValue();
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



VclPtr<SfxTabPage> SvxShadowTabPage::Create( vcl::Window* pWindow,
                                             const SfxItemSet* rAttrs )
{
    return VclPtr<SvxShadowTabPage>::Create( pWindow, *rAttrs );
}



IMPL_LINK_NOARG_TYPED(SvxShadowTabPage, ClickShadowHdl_Impl, Button*, void)
{
    if( m_pTsbShowShadow->GetState() == TRISTATE_FALSE )
    {
        m_pGridShadow->Disable();
    }
    else
    {
        m_pGridShadow->Enable();
    }
    m_pCtlPosition->Invalidate();

    ModifyShadowHdl_Impl( NULL );
}



IMPL_LINK_TYPED(SvxShadowTabPage, SelectShadowHdl_Impl, ListBox&, rListBox, void)
{
    ModifyShadowHdl_Impl(&rListBox);
}
IMPL_LINK_NOARG(SvxShadowTabPage, ModifyShadowHdl_Impl)
{
    if( m_pTsbShowShadow->GetState() == TRISTATE_TRUE )
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    else
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    sal_Int32 nPos = m_pLbShadowColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_rXFSet.Put( XFillColorItem( OUString(), m_pLbShadowColor->GetSelectEntryColor() ) );
    }
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    XFillTransparenceItem aItem( nVal );
    m_rXFSet.Put( XFillTransparenceItem( aItem ) );

    // shadow removal
    sal_Int32 nX = 0L, nY = 0L;
    sal_Int32 nXY = GetCoreValue( *m_pMtrDistance, m_ePoolUnit );
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

    m_pCtlXRectPreview->SetShadowAttributes(m_aXFillAttr.GetItemSet());
    //aCtlXRectPreview.SetFillAttr( aXFillAttr );
    m_pCtlXRectPreview->Invalidate();

    return 0L;
}



void SvxShadowTabPage::PointChanged( vcl::Window* pWindow, RECT_POINT eRcPt )
{
    m_eRP = eRcPt;

    // repaint shadow
    ModifyShadowHdl_Impl( pWindow );
}

void SvxShadowTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    SFX_ITEMSET_ARG (&aSet,pColorListItem,SvxColorListItem,SID_COLOR_TABLE,false);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,false);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,false);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
