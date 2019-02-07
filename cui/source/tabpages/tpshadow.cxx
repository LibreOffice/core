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
#include <svx/colorbox.hxx>
#include <svx/dialogs.hrc>

#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/svdattr.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <svx/dlgutil.hxx>
#include <cuitabline.hxx>
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

SvxShadowTabPage::SvxShadowTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage(pParent, "cui/ui/shadowtabpage.ui", "ShadowTabPage", rInAttrs)
    , m_rOutAttrs(rInAttrs)
    , m_pnColorListState(nullptr)
    , m_nPageType(PageType::Area)
    , m_nDlgType(0)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , m_aCtlPosition(this)
    , m_xTsbShowShadow(m_xBuilder->weld_check_button("TSB_SHOW_SHADOW"))
    , m_xGridShadow(m_xBuilder->weld_widget("gridSHADOW"))
    , m_xMtrDistance(m_xBuilder->weld_metric_spin_button("MTR_FLD_DISTANCE", FieldUnit::CM))
    , m_xLbShadowColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_SHADOW_COLOR"), pParent.GetFrameWeld()))
    , m_xMtrTransparent(m_xBuilder->weld_metric_spin_button("MTR_SHADOW_TRANSPARENT", FieldUnit::PERCENT))
    , m_xCtlPosition(new weld::CustomWeld(*m_xBuilder, "CTL_POSITION", m_aCtlPosition))
    , m_xCtlXRectPreview(new weld::CustomWeld(*m_xBuilder, "CTL_COLOR_PREVIEW", m_aCtlXRectPreview))
{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FieldUnit::M:
        case FieldUnit::KM:
            eFUnit = FieldUnit::MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_xMtrDistance, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    m_ePoolUnit = pPool->GetMetric( SDRATTR_SHADOWXDIST );

    // setting the output device
    drawing::FillStyle eXFS = drawing::FillStyle_SOLID;
    if( m_rOutAttrs.GetItemState( XATTR_FILLSTYLE ) != SfxItemState::DONTCARE )
    {
        eXFS = static_cast<const XFillStyleItem&>( m_rOutAttrs.
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue();
        switch( eXFS )
        {
            case drawing::FillStyle_SOLID:
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    m_rXFSet.Put( m_rOutAttrs.Get( XATTR_FILLCOLOR ) );
                }
            break;

            case drawing::FillStyle_GRADIENT:
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLGRADIENT ) )
                {
                    m_rXFSet.Put( m_rOutAttrs.Get( XATTR_FILLGRADIENT ) );
                }
            break;

            case drawing::FillStyle_HATCH:
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    m_rXFSet.Put( m_rOutAttrs.Get( XATTR_FILLHATCH ) );
                }
            break;

            case drawing::FillStyle_BITMAP:
            {
                if( SfxItemState::DONTCARE != m_rOutAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    m_rXFSet.Put( m_rOutAttrs.Get( XATTR_FILLBITMAP ) );
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
    m_aCtlXRectPreview.SetRectangleAttributes(m_aXFillAttr.GetItemSet());

    m_xTsbShowShadow->connect_toggled(LINK( this, SvxShadowTabPage, ClickShadowHdl_Impl));
    m_xLbShadowColor->SetSelectHdl( LINK( this, SvxShadowTabPage, SelectShadowHdl_Impl ) );
    Link<weld::MetricSpinButton&,void> aLink = LINK( this, SvxShadowTabPage, ModifyShadowHdl_Impl );
    m_xMtrTransparent->connect_value_changed(aLink);
    m_xMtrDistance->connect_value_changed(aLink);
}

SvxShadowTabPage::~SvxShadowTabPage()
{
    disposeOnce();
}

void SvxShadowTabPage::dispose()
{
    m_xCtlXRectPreview.reset();
    m_xLbShadowColor.reset();
    m_xCtlPosition.reset();
    SvxTabPage::dispose();
}

void SvxShadowTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxUInt16Item* pPageTypeItem = rSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    if (pPageTypeItem)
        SetPageType(static_cast<PageType>(pPageTypeItem->GetValue()));

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

                SfxItemSet rAttribs( rSet );
                // rSet contains shadow attributes too, but we want
                // to use it for updating rectangle attributes only,
                // so set the shadow to none here
                SdrOnOffItem aItem( makeSdrShadowItem( false ));
                rAttribs.Put( aItem );

                m_aCtlXRectPreview.SetRectangleAttributes( rAttribs );
                ModifyShadowHdl_Impl( *m_xMtrTransparent );
            }
            m_nPageType = PageType::Shadow;
        }
    }
}


DeactivateRC SvxShadowTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxShadowTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    bool                bModified = false;

    const SfxPoolItem*  pOld = nullptr;

    if (m_xTsbShowShadow->get_state_changed_from_saved())
    {
        TriState eState = m_xTsbShowShadow->get_state();
        assert(eState != TRISTATE_INDET);
            // given how m_xTsbShowShadow is set up and saved in Reset(),
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
    sal_Int32 nX = 0, nY = 0;
    sal_Int32 nXY = GetCoreValue( *m_xMtrDistance, m_ePoolUnit );

    switch (m_aCtlPosition.GetActualRP())
    {
        case RectPoint::LT: nX = nY = -nXY;      break;
        case RectPoint::MT: nY = -nXY;           break;
        case RectPoint::RT: nX = nXY; nY = -nXY; break;
        case RectPoint::LM: nX = -nXY;           break;
        case RectPoint::RM: nX = nXY;            break;
        case RectPoint::LB: nX = -nXY; nY = nXY; break;
        case RectPoint::MB: nY = nXY;            break;
        case RectPoint::RB: nX = nY = nXY;       break;
        case RectPoint::MM: break;
    }

    // If the values of the shadow distances==SfxItemState::DONTCARE and the displayed
    // string in the respective MetricField=="", then the comparison of the old
    // and the new distance values would return a wrong result because in such a
    // case the new distance values would match the default values of the MetricField !!!!
    if ( !m_xMtrDistance->get_text().isEmpty() ||
         m_rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SfxItemState::DONTCARE ||
         m_rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SfxItemState::DONTCARE    )
    {
        sal_Int32 nOldX = 9876543; // impossible value, so DontCare
        sal_Int32 nOldY = 9876543;
        if( m_rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SfxItemState::DONTCARE &&
            m_rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SfxItemState::DONTCARE )
        {
            nOldX = m_rOutAttrs.Get( SDRATTR_SHADOWXDIST ).GetValue();
            nOldY = m_rOutAttrs.Get( SDRATTR_SHADOWYDIST ).GetValue();
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
    {
        XColorItem aItem(makeSdrShadowColorItem(m_xLbShadowColor->GetSelectEntryColor()));
        pOld = GetOldItem( *rAttrs, SDRATTR_SHADOWCOLOR );
        if ( !pOld || !( *static_cast<const XColorItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    // transparency
    sal_uInt16 nVal = static_cast<sal_uInt16>(m_xMtrTransparent->get_value(FieldUnit::PERCENT));
    if (m_xMtrTransparent->get_value_changed_from_saved())
    {
        SdrPercentItem aItem( makeSdrShadowTransparenceItem(nVal) );
        pOld = GetOldItem( *rAttrs, SDRATTR_SHADOWTRANSPARENCE );
        if ( !pOld || !( *static_cast<const SdrPercentItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    rAttrs->Put (CntUInt16Item(SID_PAGE_TYPE, static_cast<sal_uInt16>(m_nPageType)));

    return bModified;
}


void SvxShadowTabPage::Reset( const SfxItemSet* rAttrs )
{
    // all objects can have a shadow
    // at the moment there are only 8 possible positions where a shadow can be set

    // has a shadow been set?
    if( rAttrs->GetItemState( SDRATTR_SHADOW ) != SfxItemState::DONTCARE )
    {
        if( rAttrs->Get( SDRATTR_SHADOW ).GetValue() )
            m_xTsbShowShadow->set_state(TRISTATE_TRUE);
        else
        {
            m_xTsbShowShadow->set_state(TRISTATE_FALSE);
        }
    }
    else
        m_xTsbShowShadow->set_state(TRISTATE_INDET);

    // distance (only 8 possible positions),
    // so there is only one item evaluated

    if( rAttrs->GetItemState( SDRATTR_SHADOWXDIST ) != SfxItemState::DONTCARE &&
        rAttrs->GetItemState( SDRATTR_SHADOWYDIST ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nX = rAttrs->Get( SDRATTR_SHADOWXDIST ).GetValue();
        sal_Int32 nY = rAttrs->Get( SDRATTR_SHADOWYDIST ).GetValue();

        if( nX != 0 )
            SetMetricValue( *m_xMtrDistance, nX < 0 ? -nX : nX, m_ePoolUnit );
        else
            SetMetricValue( *m_xMtrDistance, nY < 0 ? -nY : nY, m_ePoolUnit );

        // setting the shadow control
        if     ( nX <  0 && nY <  0 ) m_aCtlPosition.SetActualRP( RectPoint::LT );
        else if( nX == 0 && nY <  0 ) m_aCtlPosition.SetActualRP( RectPoint::MT );
        else if( nX >  0 && nY <  0 ) m_aCtlPosition.SetActualRP( RectPoint::RT );
        else if( nX <  0 && nY == 0 ) m_aCtlPosition.SetActualRP( RectPoint::LM );
        // there's no center point anymore
        else if( nX == 0 && nY == 0 ) m_aCtlPosition.SetActualRP( RectPoint::RB );
        else if( nX >  0 && nY == 0 ) m_aCtlPosition.SetActualRP( RectPoint::RM );
        else if( nX <  0 && nY >  0 ) m_aCtlPosition.SetActualRP( RectPoint::LB );
        else if( nX == 0 && nY >  0 ) m_aCtlPosition.SetActualRP( RectPoint::MB );
        else if( nX >  0 && nY >  0 ) m_aCtlPosition.SetActualRP( RectPoint::RB );
    }
    else
    {
        // determine default-distance
        SfxItemPool* pPool = m_rOutAttrs.GetPool();
        {
            sal_Int32 n = pPool->GetDefaultItem(SDRATTR_SHADOWXDIST).GetValue();
            if (n == 0)
                n = pPool->GetDefaultItem(SDRATTR_SHADOWYDIST).GetValue();
            SetMetricValue(*m_xMtrDistance, std::abs(n), m_ePoolUnit);
        }

        // Tristate, e. g. multiple objects have been marked of which some have a shadow and some don't.
        // The text (which shall be displayed) of the MetricFields is set to "" and serves as an
        // identification in the method FillItemSet for the fact that the distance value was NOT changed !!!!
        m_xMtrDistance->set_text( "" );
        m_aCtlPosition.SetActualRP( RectPoint::MM );
    }

    if( rAttrs->GetItemState( SDRATTR_SHADOWCOLOR ) != SfxItemState::DONTCARE )
    {
        m_xLbShadowColor->SelectEntry( rAttrs->Get( SDRATTR_SHADOWCOLOR ).GetColorValue() );
    }
    else
        m_xLbShadowColor->SetNoSelection();

    if( rAttrs->GetItemState( SDRATTR_SHADOWTRANSPARENCE ) != SfxItemState::DONTCARE )
    {
        sal_uInt16 nTransp = rAttrs->Get( SDRATTR_SHADOWTRANSPARENCE ).GetValue();
        m_xMtrTransparent->set_value(nTransp, FieldUnit::PERCENT);
    }
    else
        m_xMtrTransparent->set_text("");

    //aCtlPosition
    m_xMtrDistance->save_value();
    m_xLbShadowColor->SaveValue();
    m_xTsbShowShadow->save_state();

    // #66832# This field was not saved, but used to determine changes.
    // Why? Seems to be the error.
    // It IS the error.
    m_xMtrTransparent->save_value();

    ClickShadowHdl_Impl(*m_xTsbShowShadow);
    ModifyShadowHdl_Impl(*m_xMtrTransparent);
}

VclPtr<SfxTabPage> SvxShadowTabPage::Create( TabPageParent pParent,
                                             const SfxItemSet* rAttrs )
{
    return VclPtr<SvxShadowTabPage>::Create( pParent, *rAttrs );
}

IMPL_LINK_NOARG(SvxShadowTabPage, ClickShadowHdl_Impl, weld::ToggleButton&, void)
{
    if (m_xTsbShowShadow->get_state() == TRISTATE_FALSE)
    {
        m_xGridShadow->set_sensitive(false);
        m_xCtlPosition->set_sensitive(false);
    }
    else
    {
        m_xGridShadow->set_sensitive(true);
        m_xCtlPosition->set_sensitive(true);
    }
    m_aCtlPosition.Invalidate();
    ModifyShadowHdl_Impl(*m_xMtrTransparent);
}

IMPL_LINK_NOARG(SvxShadowTabPage, SelectShadowHdl_Impl, ColorListBox&, void)
{
    ModifyShadowHdl_Impl(*m_xMtrTransparent);
}

IMPL_LINK_NOARG(SvxShadowTabPage, ModifyShadowHdl_Impl, weld::MetricSpinButton&, void)
{
    if (m_xTsbShowShadow->get_state() == TRISTATE_TRUE)
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    else
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    m_rXFSet.Put( XFillColorItem( OUString(), m_xLbShadowColor->GetSelectEntryColor() ) );
    sal_uInt16 nVal = static_cast<sal_uInt16>(m_xMtrTransparent->get_value(FieldUnit::PERCENT));
    m_rXFSet.Put( XFillTransparenceItem( nVal ) );

    // shadow removal
    sal_Int32 nX = 0, nY = 0;
    sal_Int32 nXY = GetCoreValue( *m_xMtrDistance, m_ePoolUnit );
    switch( m_aCtlPosition.GetActualRP() )
    {
        case RectPoint::LT: nX = nY = -nXY;      break;
        case RectPoint::MT: nY = -nXY;           break;
        case RectPoint::RT: nX = nXY; nY = -nXY; break;
        case RectPoint::LM: nX = -nXY;           break;
        case RectPoint::RM: nX = nXY;            break;
        case RectPoint::LB: nX = -nXY; nY = nXY; break;
        case RectPoint::MB: nY = nXY;            break;
        case RectPoint::RB: nX = nY = nXY;       break;
        case RectPoint::MM: break;
    }

    m_aCtlXRectPreview.SetShadowPosition(Point(nX, nY));

    m_aCtlXRectPreview.SetShadowAttributes(m_aXFillAttr.GetItemSet());
    m_aCtlXRectPreview.Invalidate();
}

void SvxShadowTabPage::PointChanged( weld::DrawingArea*, RectPoint )
{
    // repaint shadow
    ModifyShadowHdl_Impl( *m_xMtrTransparent );
}

void SvxShadowTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxColorListItem* pColorListItem = aSet.GetItem<SvxColorListItem>(SID_COLOR_TABLE, false);
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pPageTypeItem)
        SetPageType(static_cast<PageType>(pPageTypeItem->GetValue()));
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
