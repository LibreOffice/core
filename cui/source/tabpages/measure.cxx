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

#include <strings.hrc>
#include <dialmgr.hxx>

#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/measctrl.hxx>
#include <svx/ofaitem.hxx>
#include <svx/strarray.hxx>
#include <svx/svdview.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmlhitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmuitm.hxx>
#include <svtools/unitconv.hxx>

#include <measure.hxx>

const WhichRangesContainer SvxMeasurePage::pRanges(
    svl::Items<SDRATTR_MEASURE_FIRST, SDRATTR_MEASURE_LAST>);

/*************************************************************************
|*
|* Dialog to change measure-attributes
|*
\************************************************************************/
SvxMeasureDialog::SvxMeasureDialog(weld::Window* pParent, const SfxItemSet& rInAttrs,
                                const SdrView* pSdrView)
    : SfxSingleTabDialogController(pParent, &rInAttrs)
{
    auto xPage = std::make_unique<SvxMeasurePage>(get_content_area(), this, rInAttrs);

    xPage->SetView(pSdrView);
    xPage->Construct();

    SetTabPage(std::move(xPage));
    m_xDialog->set_title(CuiResId(RID_CUISTR_DIMENSION_LINE));
}

/*************************************************************************
|*
|* Tabpage for changing measure-attributes
|*
\************************************************************************/

SvxMeasurePage::SvxMeasurePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SvxTabPage(pPage, pController, "cui/ui/dimensionlinestabpage.ui", "DimensionLinesTabPage", rInAttrs)
    , rOutAttrs(rInAttrs)
    , aAttrSet(*rInAttrs.GetPool())
    , pView(nullptr)
    , eUnit(MapUnit::Map100thMM)
    , bPositionModified(false)
    , m_aCtlPosition(this)
    , m_xMtrFldLineDist(m_xBuilder->weld_metric_spin_button("MTR_LINE_DIST", FieldUnit::MM))
    , m_xMtrFldHelplineOverhang(m_xBuilder->weld_metric_spin_button("MTR_FLD_HELPLINE_OVERHANG", FieldUnit::MM))
    , m_xMtrFldHelplineDist(m_xBuilder->weld_metric_spin_button("MTR_FLD_HELPLINE_DIST", FieldUnit::MM))
    , m_xMtrFldHelpline1Len(m_xBuilder->weld_metric_spin_button("MTR_FLD_HELPLINE1_LEN", FieldUnit::MM))
    , m_xMtrFldHelpline2Len(m_xBuilder->weld_metric_spin_button("MTR_FLD_HELPLINE2_LEN", FieldUnit::MM))
    , m_xTsbBelowRefEdge(m_xBuilder->weld_check_button("TSB_BELOW_REF_EDGE"))
    , m_xMtrFldDecimalPlaces(m_xBuilder->weld_spin_button("MTR_FLD_DECIMALPLACES"))
    , m_xTsbAutoPosV(m_xBuilder->weld_check_button("TSB_AUTOPOSV"))
    , m_xTsbAutoPosH(m_xBuilder->weld_check_button("TSB_AUTOPOSH"))
    , m_xTsbShowUnit(m_xBuilder->weld_check_button("TSB_SHOW_UNIT"))
    , m_xLbUnit(m_xBuilder->weld_combo_box("LB_UNIT"))
    , m_xTsbParallel(m_xBuilder->weld_check_button("TSB_PARALLEL"))
    , m_xFtAutomatic(m_xBuilder->weld_label("STR_MEASURE_AUTOMATIC"))
    , m_xCtlPosition(new weld::CustomWeld(*m_xBuilder, "CTL_POSITION", m_aCtlPosition))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, "CTL_PREVIEW", m_aCtlPreview))
{
    m_aCtlPreview.SetAttributes(rInAttrs);

    FillUnitLB();

    const FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_xMtrFldLineDist, eFUnit );
    SetFieldUnit( *m_xMtrFldHelplineOverhang, eFUnit );
    SetFieldUnit( *m_xMtrFldHelplineDist, eFUnit );
    SetFieldUnit( *m_xMtrFldHelpline1Len, eFUnit );
    SetFieldUnit( *m_xMtrFldHelpline2Len, eFUnit );
    if( eFUnit == FieldUnit::MM )
    {
        m_xMtrFldLineDist->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldHelplineOverhang->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldHelplineDist->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldHelpline1Len->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldHelpline2Len->set_increments(50, 500, FieldUnit::NONE);
    }

    m_xTsbAutoPosV->connect_toggled(LINK( this, SvxMeasurePage, ClickAutoPosHdl_Impl));
    m_xTsbAutoPosH->connect_toggled(LINK(this, SvxMeasurePage, ClickAutoPosHdl_Impl));

    Link<weld::MetricSpinButton&,void> aLink(LINK(this, SvxMeasurePage, ChangeAttrEditHdl_Impl));
    m_xMtrFldLineDist->set_range(-10000, 10000, FieldUnit::MM);
    m_xMtrFldLineDist->connect_value_changed(aLink);
    m_xMtrFldHelplineOverhang->connect_value_changed(aLink);
    m_xMtrFldHelplineOverhang->set_range(-10000, 10000, FieldUnit::MM);
    m_xMtrFldHelplineDist->connect_value_changed(aLink);
    m_xMtrFldHelplineDist->set_range(-10000, 10000, FieldUnit::MM);
    m_xMtrFldHelpline1Len->connect_value_changed(aLink);
    m_xMtrFldHelpline1Len->set_range(-10000, 10000, FieldUnit::MM);
    m_xMtrFldHelpline2Len->connect_value_changed(aLink);
    m_xMtrFldHelpline2Len->set_range(-10000, 10000, FieldUnit::MM);
    m_xMtrFldDecimalPlaces->connect_value_changed(LINK(this, SvxMeasurePage, ChangeAttrSpinHdl_Impl));
    m_xTsbBelowRefEdge->connect_toggled(LINK(this, SvxMeasurePage, ChangeAttrClickHdl_Impl));
    m_xTsbParallel->connect_toggled( LINK( this, SvxMeasurePage, ChangeAttrClickHdl_Impl));
    m_xTsbShowUnit->connect_toggled(LINK(this, SvxMeasurePage, ChangeAttrClickHdl_Impl));
    m_xLbUnit->connect_changed(LINK(this, SvxMeasurePage, ChangeAttrListBoxHdl_Impl));
}

SvxMeasurePage::~SvxMeasurePage()
{
    m_xCtlPreview.reset();
    m_xCtlPosition.reset();
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void SvxMeasurePage::Reset( const SfxItemSet* rAttrs )
{
    SfxItemPool* pPool = rAttrs->GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    eUnit = pPool->GetMetric( SDRATTR_MEASURELINEDIST );

    const SfxPoolItem* pItem = GetItem( *rAttrs, SDRATTR_MEASURELINEDIST );

    // SdrMeasureLineDistItem
    if( pItem == nullptr )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_MEASURELINEDIST );
    SetMetricValue(*m_xMtrFldLineDist, static_cast<const SdrMetricItem*>(pItem)->GetValue(), eUnit);
    m_xMtrFldLineDist->save_value();

    // SdrMeasureHelplineOverhangItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem == nullptr )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_MEASUREHELPLINEOVERHANG );
    SetMetricValue(*m_xMtrFldHelplineOverhang, static_cast<const SdrMetricItem*>(pItem)->GetValue(),
                   eUnit);
    m_xMtrFldHelplineOverhang->save_value();

    // SdrMeasureHelplineDistItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINEDIST );
    if( pItem == nullptr )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_MEASUREHELPLINEDIST );
    SetMetricValue(*m_xMtrFldHelplineDist, static_cast<const SdrMetricItem*>(pItem)->GetValue(),
                   eUnit);
    m_xMtrFldHelplineDist->save_value();

    // SdrMeasureHelpline1LenItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem == nullptr )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_MEASUREHELPLINE1LEN );
    SetMetricValue(*m_xMtrFldHelpline1Len, static_cast<const SdrMetricItem*>(pItem)->GetValue(),
                   eUnit);
    m_xMtrFldHelpline1Len->save_value();

    // SdrMeasureHelpline2LenItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem == nullptr )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_MEASUREHELPLINE2LEN );
    SetMetricValue(*m_xMtrFldHelpline2Len, static_cast<const SdrMetricItem*>(pItem)->GetValue(),
                   eUnit);
    m_xMtrFldHelpline2Len->save_value();

    // SdrMeasureBelowRefEdgeItem
    if( rAttrs->GetItemState( SDRATTR_MEASUREBELOWREFEDGE ) != SfxItemState::INVALID )
    {
        m_xTsbBelowRefEdge->set_state( rAttrs->Get( SDRATTR_MEASUREBELOWREFEDGE ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
    {
        m_xTsbBelowRefEdge->set_state( TRISTATE_INDET );
    }
    m_xTsbBelowRefEdge->save_state();

    // SdrMeasureDecimalPlacesItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREDECIMALPLACES );
    if( pItem == nullptr )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_MEASUREDECIMALPLACES );
    m_xMtrFldDecimalPlaces->set_value(
      static_cast<const SdrMeasureDecimalPlacesItem*>(pItem)->GetValue());
    m_xMtrFldDecimalPlaces->save_value();

    // SdrMeasureTextRota90Item
    // Attention: negate !
    if( rAttrs->GetItemState( SDRATTR_MEASURETEXTROTA90 ) != SfxItemState::INVALID )
    {
        m_xTsbParallel->set_state( rAttrs->Get( SDRATTR_MEASURETEXTROTA90 ).
                        GetValue() ? TRISTATE_FALSE : TRISTATE_TRUE );
    }
    else
    {
        m_xTsbParallel->set_state( TRISTATE_INDET );
    }
    m_xTsbParallel->save_state();

    // SdrMeasureShowUnitItem
    if( rAttrs->GetItemState( SDRATTR_MEASURESHOWUNIT ) != SfxItemState::INVALID )
    {
        m_xTsbShowUnit->set_state( rAttrs->Get( SDRATTR_MEASURESHOWUNIT ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
    {
        m_xTsbShowUnit->set_state( TRISTATE_INDET );
    }
    m_xTsbShowUnit->save_state();

    // SdrMeasureUnitItem
    if( rAttrs->GetItemState( SDRATTR_MEASUREUNIT ) != SfxItemState::INVALID )
    {
        tools::Long nFieldUnit = static_cast<tools::Long>(rAttrs->Get( SDRATTR_MEASUREUNIT ).GetValue());

        for (sal_Int32 i = 0; i < m_xLbUnit->get_count(); ++i)
        {
            if (m_xLbUnit->get_id(i).toInt32() == nFieldUnit)
            {
                m_xLbUnit->set_active(i);
                break;
            }
        }
    }
    else
    {
        m_xLbUnit->set_active(-1);
    }
    m_xLbUnit->save_value();

    // Position
    if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTVPOS ) != SfxItemState::INVALID )
    {
        css::drawing::MeasureTextVertPos eVPos =
                    rAttrs->Get( SDRATTR_MEASURETEXTVPOS ).GetValue();
        {
            if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTHPOS ) != SfxItemState::INVALID )
            {
                css::drawing::MeasureTextHorzPos eHPos =
                            rAttrs->Get( SDRATTR_MEASURETEXTHPOS ).GetValue();
                RectPoint eRP = RectPoint::MM;
                switch( eVPos )
                {
                case css::drawing::MeasureTextVertPos_EAST:
                    switch( eHPos )
                    {
                    case css::drawing::MeasureTextHorzPos_LEFTOUTSIDE:    eRP = RectPoint::LT; break;
                    case css::drawing::MeasureTextHorzPos_INSIDE:         eRP = RectPoint::MT; break;
                    case css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE:   eRP = RectPoint::RT; break;
                    case css::drawing::MeasureTextHorzPos_AUTO:          eRP = RectPoint::MT; break;
                    default: break;
                    }
                    break;
                case css::drawing::MeasureTextVertPos_CENTERED:
                    switch( eHPos )
                    {
                    case css::drawing::MeasureTextHorzPos_LEFTOUTSIDE:    eRP = RectPoint::LM; break;
                    case css::drawing::MeasureTextHorzPos_INSIDE:         eRP = RectPoint::MM; break;
                    case css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE:   eRP = RectPoint::RM; break;
                    case css::drawing::MeasureTextHorzPos_AUTO:          eRP = RectPoint::MM; break;
                    default: break;
                    }
                    break;
                case css::drawing::MeasureTextVertPos_WEST:
                    switch( eHPos )
                    {
                    case css::drawing::MeasureTextHorzPos_LEFTOUTSIDE:    eRP = RectPoint::LB; break;
                    case css::drawing::MeasureTextHorzPos_INSIDE:         eRP = RectPoint::MB; break;
                    case css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE:   eRP = RectPoint::RB; break;
                    case css::drawing::MeasureTextHorzPos_AUTO:          eRP = RectPoint::MB; break;
                    default: break;
                    }
                    break;
                case css::drawing::MeasureTextVertPos_AUTO:
                    switch( eHPos )
                    {
                    case css::drawing::MeasureTextHorzPos_LEFTOUTSIDE:    eRP = RectPoint::LM; break;
                    case css::drawing::MeasureTextHorzPos_INSIDE:         eRP = RectPoint::MM; break;
                    case css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE:   eRP = RectPoint::RM; break;
                    case css::drawing::MeasureTextHorzPos_AUTO:          eRP = RectPoint::MM; break;
                    default: break;
                    }
                    break;
                 default: ;//prevent warning
                }

                CTL_STATE nState = CTL_STATE::NONE;

                if (eHPos == css::drawing::MeasureTextHorzPos_AUTO)
                {
                    m_xTsbAutoPosH->set_state( TRISTATE_TRUE );
                    nState = CTL_STATE::NOHORZ;
                }

                if (eVPos == css::drawing::MeasureTextVertPos_AUTO)
                {
                    m_xTsbAutoPosV->set_state( TRISTATE_TRUE );
                    nState |= CTL_STATE::NOVERT;
                }

                m_aCtlPosition.SetState(nState);
                m_aCtlPosition.SetActualRP(eRP);
            }
        }
    }
    else
    {
        m_aCtlPosition.Reset();
        m_xTsbAutoPosV->set_state( TRISTATE_INDET );
        m_xTsbAutoPosH->set_state( TRISTATE_INDET );
    }

    // put the attributes to the preview-control,
    // otherwise the control don't know about
    // the settings of the dialog (#67930)
    ChangeAttrHdl_Impl(m_xTsbShowUnit.get());
    m_aCtlPreview.SetAttributes(*rAttrs);

    bPositionModified = false;
}

/*************************************************************************
|*
|* Fill the delivered Item-Set with dialogbox-attributes
|*
\************************************************************************/

bool SvxMeasurePage::FillItemSet( SfxItemSet* rAttrs)
{
    bool     bModified = false;
    sal_Int32    nValue;
    TriState eState;

    if( m_xMtrFldLineDist->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldLineDist, eUnit );
        rAttrs->Put( makeSdrMeasureLineDistItem( nValue ) );
        bModified = true;
    }

    if( m_xMtrFldHelplineOverhang->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldHelplineOverhang, eUnit );
        rAttrs->Put( makeSdrMeasureHelplineOverhangItem( nValue ) );
        bModified = true;
    }

    if( m_xMtrFldHelplineDist->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldHelplineDist, eUnit );
        rAttrs->Put( makeSdrMeasureHelplineDistItem( nValue ) );
        bModified = true;
    }

    if( m_xMtrFldHelpline1Len->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldHelpline1Len, eUnit );
        rAttrs->Put( makeSdrMeasureHelpline1LenItem( nValue ) );
        bModified = true;
    }

    if( m_xMtrFldHelpline2Len->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldHelpline2Len, eUnit );
        rAttrs->Put( makeSdrMeasureHelpline2LenItem( nValue ) );
        bModified = true;
    }

    eState = m_xTsbBelowRefEdge->get_state();
    if( m_xTsbBelowRefEdge->get_state_changed_from_saved() )
    {
        rAttrs->Put( SdrMeasureBelowRefEdgeItem( TRISTATE_TRUE == eState ) );
        bModified = true;
    }

    if( m_xMtrFldDecimalPlaces->get_value_changed_from_saved() )
    {
        nValue = m_xMtrFldDecimalPlaces->get_value();
        rAttrs->Put(
            SdrMeasureDecimalPlacesItem(
                sal::static_int_cast< sal_Int16 >( nValue ) ) );
        bModified = true;
    }

    eState = m_xTsbParallel->get_state();
    if( m_xTsbParallel->get_state_changed_from_saved() )
    {
        rAttrs->Put( SdrMeasureTextRota90Item( TRISTATE_FALSE == eState ) );
        bModified = true;
    }

    eState = m_xTsbShowUnit->get_state();
    if( m_xTsbShowUnit->get_state_changed_from_saved() )
    {
        rAttrs->Put( SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, TRISTATE_TRUE == eState ) );
        bModified = true;
    }

    int nPos = m_xLbUnit->get_active();
    if( m_xLbUnit->get_value_changed_from_saved() )
    {
        if (nPos != -1)
        {
            sal_uInt16 nFieldUnit = m_xLbUnit->get_id(nPos).toUInt32();
            FieldUnit _eUnit = static_cast<FieldUnit>(nFieldUnit);
            rAttrs->Put( SdrMeasureUnitItem( _eUnit ) );
            bModified = true;
        }
    }

    if( bPositionModified )
    {
        // Position
        css::drawing::MeasureTextVertPos eVPos;
        css::drawing::MeasureTextHorzPos eHPos;

        RectPoint eRP = m_aCtlPosition.GetActualRP();
        switch( eRP )
        {
            default:
            case RectPoint::LT: eVPos = css::drawing::MeasureTextVertPos_EAST;
                        eHPos = css::drawing::MeasureTextHorzPos_LEFTOUTSIDE; break;
            case RectPoint::LM: eVPos = css::drawing::MeasureTextVertPos_CENTERED;
                        eHPos = css::drawing::MeasureTextHorzPos_LEFTOUTSIDE; break;
            case RectPoint::LB: eVPos = css::drawing::MeasureTextVertPos_WEST;
                        eHPos = css::drawing::MeasureTextHorzPos_LEFTOUTSIDE; break;
            case RectPoint::MT: eVPos = css::drawing::MeasureTextVertPos_EAST;
                        eHPos = css::drawing::MeasureTextHorzPos_INSIDE; break;
            case RectPoint::MM: eVPos = css::drawing::MeasureTextVertPos_CENTERED;
                        eHPos = css::drawing::MeasureTextHorzPos_INSIDE; break;
            case RectPoint::MB: eVPos = css::drawing::MeasureTextVertPos_WEST;
                        eHPos = css::drawing::MeasureTextHorzPos_INSIDE; break;
            case RectPoint::RT: eVPos = css::drawing::MeasureTextVertPos_EAST;
                        eHPos = css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE; break;
            case RectPoint::RM: eVPos = css::drawing::MeasureTextVertPos_CENTERED;
                        eHPos = css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE; break;
            case RectPoint::RB: eVPos = css::drawing::MeasureTextVertPos_WEST;
                        eHPos = css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE; break;
        }
        if (m_xTsbAutoPosH->get_state() == TRISTATE_TRUE)
            eHPos = css::drawing::MeasureTextHorzPos_AUTO;

        if (m_xTsbAutoPosV->get_state() == TRISTATE_TRUE)
            eVPos = css::drawing::MeasureTextVertPos_AUTO;

        if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTVPOS ) != SfxItemState::INVALID )
        {
            css::drawing::MeasureTextVertPos eOldVPos = rOutAttrs.Get(SDRATTR_MEASURETEXTVPOS).GetValue();
            if( eOldVPos != eVPos )
            {
                rAttrs->Put( SdrMeasureTextVPosItem( eVPos ) );
                bModified = true;
            }
        }
        else
        {
            rAttrs->Put( SdrMeasureTextVPosItem( eVPos ) );
            bModified = true;
        }

        if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTHPOS ) != SfxItemState::INVALID )
        {
            css::drawing::MeasureTextHorzPos eOldHPos = rOutAttrs.Get( SDRATTR_MEASURETEXTHPOS ).GetValue();
            if( eOldHPos != eHPos )
            {
                rAttrs->Put( SdrMeasureTextHPosItem( eHPos ) );
                bModified = true;
            }
        }
        else
        {
            rAttrs->Put( SdrMeasureTextHPosItem( eHPos ) );
            bModified = true;
        }
    }

    return bModified;
}

/*************************************************************************
|*
|* The View have to set at the measure-object to be able to notify
|* unit and floatingpoint-values
|*
\************************************************************************/

void SvxMeasurePage::Construct()
{
    DBG_ASSERT( pView, "No valid View transferred!" );

    // TTTT
    // pMeasureObj is member of SvxXMeasurePreview and can only be accessed due to
    // SvxMeasurePage being a friend. It has its own SdrModel (also in SvxXMeasurePreview)
    // and 'setting' the SdrModel is a hack. The comment above about 'notify unit and
    // floatingpoint-values' is not clear, but has to be done another way - if needed.
    // Checked on original aw080, is just commented out there, too.

    m_aCtlPreview.Invalidate();
}

std::unique_ptr<SfxTabPage> SvxMeasurePage::Create(weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet* rAttrs)
{
    return std::make_unique<SvxMeasurePage>(pPage, pController, *rAttrs);
}

void SvxMeasurePage::PointChanged(weld::DrawingArea* pDrawingArea, RectPoint /*eRP*/)
{
    ChangeAttrHdl_Impl(pDrawingArea);
}

IMPL_LINK( SvxMeasurePage, ClickAutoPosHdl_Impl, weld::Toggleable&, rBox, void )
{
    if (m_xTsbAutoPosH->get_state() == TRISTATE_TRUE)
    {
        switch( m_aCtlPosition.GetActualRP() )
        {
            case RectPoint::LT:
            case RectPoint::RT:
                m_aCtlPosition.SetActualRP( RectPoint::MT );
            break;

            case RectPoint::LM:
            case RectPoint::RM:
                m_aCtlPosition.SetActualRP( RectPoint::MM );
            break;

            case RectPoint::LB:
            case RectPoint::RB:
                m_aCtlPosition.SetActualRP( RectPoint::MB );
            break;
            default: ;//prevent warning
        }
    }
    if (m_xTsbAutoPosV->get_state() == TRISTATE_TRUE)
    {
        switch( m_aCtlPosition.GetActualRP() )
        {
            case RectPoint::LT:
            case RectPoint::LB:
                m_aCtlPosition.SetActualRP( RectPoint::LM );
            break;

            case RectPoint::MT:
            case RectPoint::MB:
                m_aCtlPosition.SetActualRP( RectPoint::MM );
            break;

            case RectPoint::RT:
            case RectPoint::RB:
                m_aCtlPosition.SetActualRP( RectPoint::RM );
            break;
            default: ;//prevent warning
        }
    }
    ChangeAttrHdl_Impl(&rBox);
}

IMPL_LINK(SvxMeasurePage, ChangeAttrClickHdl_Impl, weld::Toggleable&, r, void)
{
    ChangeAttrHdl_Impl(&r);
}

IMPL_LINK(SvxMeasurePage, ChangeAttrListBoxHdl_Impl, weld::ComboBox&, rBox, void)
{
    ChangeAttrHdl_Impl(&rBox);
}

IMPL_LINK(SvxMeasurePage, ChangeAttrEditHdl_Impl, weld::MetricSpinButton&, rBox, void)
{
    ChangeAttrHdl_Impl(&rBox);
}

IMPL_LINK( SvxMeasurePage, ChangeAttrSpinHdl_Impl, weld::SpinButton&, rBox, void )
{
    ChangeAttrHdl_Impl(&rBox);
}

void SvxMeasurePage::ChangeAttrHdl_Impl( void const * p )
{
    if (p == m_xMtrFldLineDist.get())
    {
        sal_Int32 nValue = GetCoreValue( *m_xMtrFldLineDist, eUnit );
        aAttrSet.Put( makeSdrMeasureLineDistItem( nValue ) );
    }

    if (p == m_xMtrFldHelplineOverhang.get())
    {
        sal_Int32 nValue = GetCoreValue( *m_xMtrFldHelplineOverhang, eUnit );
        aAttrSet.Put( makeSdrMeasureHelplineOverhangItem( nValue) );
    }

    if (p == m_xMtrFldHelplineDist.get())
    {
        sal_Int32 nValue = GetCoreValue( *m_xMtrFldHelplineDist, eUnit );
        aAttrSet.Put( makeSdrMeasureHelplineDistItem( nValue) );
    }

    if (p == m_xMtrFldHelpline1Len.get())
    {
        sal_Int32 nValue = GetCoreValue( *m_xMtrFldHelpline1Len, eUnit );
        aAttrSet.Put( makeSdrMeasureHelpline1LenItem( nValue ) );
    }

    if (p == m_xMtrFldHelpline2Len.get())
    {
        sal_Int32 nValue = GetCoreValue( *m_xMtrFldHelpline2Len, eUnit );
        aAttrSet.Put( makeSdrMeasureHelpline2LenItem( nValue ) );
    }

    if (p == m_xTsbBelowRefEdge.get())
    {
        TriState eState = m_xTsbBelowRefEdge->get_state();
        if( eState != TRISTATE_INDET )
            aAttrSet.Put( SdrMeasureBelowRefEdgeItem( TRISTATE_TRUE == eState ) );
    }

    if (p == m_xMtrFldDecimalPlaces.get())
    {
        sal_Int16 nValue = sal::static_int_cast< sal_Int16 >(
            m_xMtrFldDecimalPlaces->get_value() );
        aAttrSet.Put( SdrMeasureDecimalPlacesItem( nValue ) );
    }

    if (p == m_xTsbParallel.get())
    {
        TriState eState = m_xTsbParallel->get_state();
        if( eState != TRISTATE_INDET )
            aAttrSet.Put( SdrMeasureTextRota90Item( TRISTATE_FALSE == eState ) );
    }

    if (p == m_xTsbShowUnit.get())
    {
        TriState eState = m_xTsbShowUnit->get_state();
        if( eState != TRISTATE_INDET )
            aAttrSet.Put( SdrYesNoItem( SDRATTR_MEASURESHOWUNIT, TRISTATE_TRUE == eState ) );
    }

    if (p == m_xLbUnit.get())
    {
        int nPos = m_xLbUnit->get_active();
        if (nPos != -1)
        {
            sal_uInt16 nFieldUnit = m_xLbUnit->get_id(nPos).toUInt32();
            FieldUnit _eUnit = static_cast<FieldUnit>(nFieldUnit);
            aAttrSet.Put( SdrMeasureUnitItem( _eUnit ) );
        }
    }

    if (p == m_xTsbAutoPosV.get() || p == m_xTsbAutoPosH.get() || p == m_aCtlPosition.GetDrawingArea())
    {
        bPositionModified = true;

        // Position
        RectPoint eRP = m_aCtlPosition.GetActualRP();
        css::drawing::MeasureTextVertPos eVPos;
        css::drawing::MeasureTextHorzPos eHPos;

        switch( eRP )
        {
            default:
            case RectPoint::LT: eVPos = css::drawing::MeasureTextVertPos_EAST;
                        eHPos = css::drawing::MeasureTextHorzPos_LEFTOUTSIDE; break;
            case RectPoint::LM: eVPos = css::drawing::MeasureTextVertPos_CENTERED;
                        eHPos = css::drawing::MeasureTextHorzPos_LEFTOUTSIDE; break;
            case RectPoint::LB: eVPos = css::drawing::MeasureTextVertPos_WEST;
                        eHPos = css::drawing::MeasureTextHorzPos_LEFTOUTSIDE; break;
            case RectPoint::MT: eVPos = css::drawing::MeasureTextVertPos_EAST;
                        eHPos = css::drawing::MeasureTextHorzPos_INSIDE; break;
            case RectPoint::MM: eVPos = css::drawing::MeasureTextVertPos_CENTERED;
                        eHPos = css::drawing::MeasureTextHorzPos_INSIDE; break;
            case RectPoint::MB: eVPos = css::drawing::MeasureTextVertPos_WEST;
                        eHPos = css::drawing::MeasureTextHorzPos_INSIDE; break;
            case RectPoint::RT: eVPos = css::drawing::MeasureTextVertPos_EAST;
                        eHPos = css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE; break;
            case RectPoint::RM: eVPos = css::drawing::MeasureTextVertPos_CENTERED;
                        eHPos = css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE; break;
            case RectPoint::RB: eVPos = css::drawing::MeasureTextVertPos_WEST;
                        eHPos = css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE; break;
        }

        CTL_STATE nState = CTL_STATE::NONE;

        if (m_xTsbAutoPosH->get_state() == TRISTATE_TRUE)
        {
            eHPos = css::drawing::MeasureTextHorzPos_AUTO;
            nState = CTL_STATE::NOHORZ;
        }

        if (m_xTsbAutoPosV->get_state() == TRISTATE_TRUE)
        {
            eVPos = css::drawing::MeasureTextVertPos_AUTO;
            nState |= CTL_STATE::NOVERT;
        }

        if (p == m_xTsbAutoPosV.get() || p == m_xTsbAutoPosH.get())
            m_aCtlPosition.SetState( nState );

        aAttrSet.Put( SdrMeasureTextVPosItem( eVPos ) );
        aAttrSet.Put( SdrMeasureTextHPosItem( eHPos ) );
    }

    m_aCtlPreview.SetAttributes(aAttrSet);
    m_aCtlPreview.Invalidate();
}

void SvxMeasurePage::FillUnitLB()
{
    // fill ListBox with metrics

    FieldUnit nUnit = FieldUnit::NONE;
    OUString aStrMetric(m_xFtAutomatic->get_label());
    m_xLbUnit->append(OUString::number(sal_uInt32(nUnit)), aStrMetric);

    for( sal_uInt32 i = 0; i < SvxFieldUnitTable::Count(); ++i )
    {
        aStrMetric = SvxFieldUnitTable::GetString(i);
        nUnit = SvxFieldUnitTable::GetValue(i);
        m_xLbUnit->append(OUString::number(sal_uInt32(nUnit)), aStrMetric);
    }
}

void SvxMeasurePage::PageCreated(const SfxAllItemSet& aSet)
{
    const OfaPtrItem* pOfaPtrItem = aSet.GetItem<OfaPtrItem>(SID_OBJECT_LIST, false);

    if (pOfaPtrItem)
        SetView( static_cast<SdrView *>(pOfaPtrItem->GetValue()));

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
