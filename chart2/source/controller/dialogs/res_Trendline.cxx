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

#include "res_Trendline.hxx"
#include <bitmaps.hlst>
#include <chartview/ChartSfxItemIds.hxx>

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/weld.hxx>

namespace chart
{

static void lcl_setValue(weld::FormattedSpinButton& rFmtField, double fValue )
{
    rFmtField.set_value(fValue);
//TODO    rFmtField.SetDefaultValue( fValue );
}

TrendlineResources::TrendlineResources(weld::Builder& rBuilder, const SfxItemSet& rInAttrs)
    : m_eTrendLineType(SvxChartRegress::Linear)
    , m_bTrendLineUnique(true)
    , m_pNumFormatter(nullptr)
    , m_nNbPoints(0)
    , m_xRB_Linear(rBuilder.weld_radio_button("linear"))
    , m_xRB_Logarithmic(rBuilder.weld_radio_button("logarithmic"))
    , m_xRB_Exponential(rBuilder.weld_radio_button("exponential"))
    , m_xRB_Power(rBuilder.weld_radio_button("power"))
    , m_xRB_Polynomial(rBuilder.weld_radio_button("polynomial"))
    , m_xRB_MovingAverage(rBuilder.weld_radio_button("movingAverage"))
    , m_xFI_Linear(rBuilder.weld_image("imageLinear"))
    , m_xFI_Logarithmic(rBuilder.weld_image("imageLogarithmic"))
    , m_xFI_Exponential(rBuilder.weld_image("imageExponential"))
    , m_xFI_Power(rBuilder.weld_image("imagePower"))
    , m_xFI_Polynomial(rBuilder.weld_image("imagePolynomial"))
    , m_xFI_MovingAverage(rBuilder.weld_image("imageMovingAverage"))
    , m_xNF_Degree(rBuilder.weld_spin_button("degree"))
    , m_xNF_Period(rBuilder.weld_spin_button("period"))
    , m_xEE_Name(rBuilder.weld_entry("entry_name"))
    , m_xFmtFld_ExtrapolateForward(rBuilder.weld_formatted_spin_button("extrapolateForward"))
    , m_xFmtFld_ExtrapolateBackward(rBuilder.weld_formatted_spin_button("extrapolateBackward"))
    , m_xCB_SetIntercept(rBuilder.weld_check_button("setIntercept"))
    , m_xFmtFld_InterceptValue(rBuilder.weld_formatted_spin_button("interceptValue"))
    , m_xCB_ShowEquation(rBuilder.weld_check_button("showEquation"))
    , m_xEE_XName(rBuilder.weld_entry("entry_Xname"))
    , m_xEE_YName(rBuilder.weld_entry("entry_Yname"))
    , m_xCB_ShowCorrelationCoeff(rBuilder.weld_check_button("showCorrelationCoefficient"))
{
    FillValueSets();

    Link<weld::ToggleButton&,void> aLink = LINK(this, TrendlineResources, SelectTrendLine);
    m_xRB_Linear->connect_toggled( aLink );
    m_xRB_Logarithmic->connect_toggled( aLink );
    m_xRB_Exponential->connect_toggled( aLink );
    m_xRB_Power->connect_toggled( aLink );
    m_xRB_Polynomial->connect_toggled( aLink );
    m_xRB_MovingAverage->connect_toggled( aLink );

    Link<weld::SpinButton&,void> aLink2 = LINK(this, TrendlineResources, ChangeSpinValue);
    m_xNF_Degree->connect_value_changed(aLink2);
    m_xNF_Period->connect_value_changed(aLink2);
    m_xFmtFld_InterceptValue->connect_value_changed(LINK(this, TrendlineResources, ChangeFormattedValue));

    m_xCB_ShowEquation->connect_toggled(LINK(this, TrendlineResources, ShowEquation));

    Reset( rInAttrs );
    UpdateControlStates();
}

TrendlineResources::~TrendlineResources()
{}

IMPL_LINK_NOARG(TrendlineResources, SelectTrendLine, weld::ToggleButton&, void)
{
    if (m_xRB_Linear->get_active())
        m_eTrendLineType = SvxChartRegress::Linear;
    else if (m_xRB_Logarithmic->get_active())
        m_eTrendLineType = SvxChartRegress::Log;
    else if (m_xRB_Exponential->get_active())
        m_eTrendLineType = SvxChartRegress::Exp;
    else if (m_xRB_Power->get_active())
        m_eTrendLineType = SvxChartRegress::Power;
    else if (m_xRB_Polynomial->get_active())
        m_eTrendLineType = SvxChartRegress::Polynomial;
    else if (m_xRB_MovingAverage->get_active())
        m_eTrendLineType = SvxChartRegress::MovingAverage;
    m_bTrendLineUnique = true;

    UpdateControlStates();
}

void TrendlineResources::Reset( const SfxItemSet& rInAttrs )
{
    const SfxPoolItem *pPoolItem = nullptr;

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_CURVE_NAME, true, &pPoolItem ) == SfxItemState::SET )
    {
        OUString aName = static_cast< const SfxStringItem* >(pPoolItem)->GetValue();
        m_xEE_Name->set_text(aName);
    }
    else
    {
        m_xEE_Name->set_text("");
    }
    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_XNAME, true, &pPoolItem ) == SfxItemState::SET )
    {
        OUString aName = static_cast< const SfxStringItem* >(pPoolItem)->GetValue();
        m_xEE_XName->set_text(aName);
    }
    else
    {
        m_xEE_XName->set_text("x");
    }
    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_YNAME, true, &pPoolItem ) == SfxItemState::SET )
    {
        OUString aName = static_cast< const SfxStringItem* >(pPoolItem)->GetValue();
        m_xEE_YName->set_text(aName);
    }
    else
    {
        m_xEE_YName->set_text("f(x)");
    }

    SfxItemState aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_TYPE, true, &pPoolItem );
    m_bTrendLineUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        const SvxChartRegressItem * pItem = dynamic_cast< const SvxChartRegressItem * >( pPoolItem );
        if( pItem )
        {
            m_eTrendLineType = pItem->GetValue();
        }
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_DEGREE, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nDegree = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_xNF_Degree->set_value( nDegree );
    }
    else
    {
        m_xNF_Degree->set_value( 2 );
    }

    m_xNF_Degree->save_value();

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_PERIOD, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nPeriod = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_xNF_Period->set_value( nPeriod );
    }
    else
    {
        m_xNF_Period->set_value( 2 );
    }

    m_xNF_Period->save_value();

    double nValue = 0.0;
    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD, true, &pPoolItem ) == SfxItemState::SET )
    {
        nValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue() ;
    }
    lcl_setValue(*m_xFmtFld_ExtrapolateForward, nValue);

    nValue = 0.0;
    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD, true, &pPoolItem ) == SfxItemState::SET )
    {
        nValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue() ;
    }
    lcl_setValue(*m_xFmtFld_ExtrapolateBackward, nValue);

    nValue = 0.0;
    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_INTERCEPT_VALUE, true, &pPoolItem ) == SfxItemState::SET )
    {
        nValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue() ;
    }
    lcl_setValue(*m_xFmtFld_InterceptValue, nValue);

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SET_INTERCEPT, true, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_xCB_SetIntercept->set_state(TRISTATE_INDET);
    }
    else
    {
        if( aState == SfxItemState::SET )
            m_xCB_SetIntercept->set_active( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SHOW_EQUATION, true, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_xCB_ShowEquation->set_state(TRISTATE_INDET);
    }
    else
    {
        if( aState == SfxItemState::SET )
            m_xCB_ShowEquation->set_active( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SHOW_COEFF, true, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_xCB_ShowCorrelationCoeff->set_state(TRISTATE_INDET);
    }
    else
    {
        if( aState == SfxItemState::SET )
            m_xCB_ShowCorrelationCoeff->set_active( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    if( m_bTrendLineUnique )
    {
        switch( m_eTrendLineType )
        {
            case SvxChartRegress::Linear :
                m_xRB_Linear->set_active(true);
                break;
            case SvxChartRegress::Log :
                m_xRB_Logarithmic->set_active(true);
                break;
            case SvxChartRegress::Exp :
                m_xRB_Exponential->set_active(true);
                break;
            case SvxChartRegress::Power :
                m_xRB_Power->set_active(true);
                break;
            case SvxChartRegress::Polynomial :
                m_xRB_Polynomial->set_active(true);
                break;
            case SvxChartRegress::MovingAverage :
                m_xRB_MovingAverage->set_active(true);
                break;
            default:
                break;
        }
    }
}

void TrendlineResources::FillItemSet(SfxItemSet* rOutAttrs) const
{
    if( m_bTrendLineUnique )
        rOutAttrs->Put( SvxChartRegressItem( m_eTrendLineType, SCHATTR_REGRESSION_TYPE ));

    if( m_xCB_ShowEquation->get_state() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_REGRESSION_SHOW_EQUATION, m_xCB_ShowEquation->get_active() ));

    if( m_xCB_ShowCorrelationCoeff->get_state() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_REGRESSION_SHOW_COEFF, m_xCB_ShowCorrelationCoeff->get_active() ));

    OUString aName = m_xEE_Name->get_text();
    rOutAttrs->Put(SfxStringItem(SCHATTR_REGRESSION_CURVE_NAME, aName));
    aName = m_xEE_XName->get_text();
    if ( aName.isEmpty() )
        aName = "x";
    rOutAttrs->Put(SfxStringItem(SCHATTR_REGRESSION_XNAME, aName));
    aName = m_xEE_YName->get_text();
    if ( aName.isEmpty() )
        aName = "f(x)";
    rOutAttrs->Put(SfxStringItem(SCHATTR_REGRESSION_YNAME, aName));

    sal_Int32 aDegree = m_xNF_Degree->get_value();
    rOutAttrs->Put(SfxInt32Item( SCHATTR_REGRESSION_DEGREE, aDegree ) );

    sal_Int32 aPeriod = m_xNF_Period->get_value();
    rOutAttrs->Put(SfxInt32Item( SCHATTR_REGRESSION_PERIOD, aPeriod ) );

    sal_uInt32 nIndex = 0;
    double aValue = 0.0;
    (void)m_pNumFormatter->IsNumberFormat(m_xFmtFld_ExtrapolateForward->get_text(),nIndex,aValue);
    rOutAttrs->Put(SvxDoubleItem( aValue, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD ) );

    aValue = 0.0;
    (void)m_pNumFormatter->IsNumberFormat(m_xFmtFld_ExtrapolateBackward->get_text(),nIndex,aValue);
    rOutAttrs->Put(SvxDoubleItem( aValue, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD ) );

    if( m_xCB_SetIntercept->get_state() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_REGRESSION_SET_INTERCEPT, m_xCB_SetIntercept->get_active() ));

    aValue = 0.0;
    (void)m_pNumFormatter->IsNumberFormat(m_xFmtFld_InterceptValue->get_text(),nIndex,aValue);
    rOutAttrs->Put(SvxDoubleItem( aValue, SCHATTR_REGRESSION_INTERCEPT_VALUE ) );
}

void TrendlineResources::FillValueSets()
{
    m_xFI_Linear->set_from_icon_name(BMP_REGRESSION_LINEAR);
    m_xFI_Logarithmic->set_from_icon_name(BMP_REGRESSION_LOG);
    m_xFI_Exponential->set_from_icon_name(BMP_REGRESSION_EXP);
    m_xFI_Power->set_from_icon_name(BMP_REGRESSION_POWER);
    m_xFI_Polynomial->set_from_icon_name(BMP_REGRESSION_POLYNOMIAL);
    m_xFI_MovingAverage->set_from_icon_name(BMP_REGRESSION_MOVING_AVERAGE);
}

void TrendlineResources::UpdateControlStates()
{
    if( m_nNbPoints > 0 )
    {
        sal_Int32 nMaxValue = m_nNbPoints - 1 + (m_xCB_SetIntercept->get_active() ? 1 : 0);
        m_xNF_Degree->set_max(nMaxValue);
        m_xNF_Period->set_max(m_nNbPoints - 1);
    }
    bool bMovingAverage = ( m_eTrendLineType == SvxChartRegress::MovingAverage );
    bool bInterceptAvailable = ( m_eTrendLineType == SvxChartRegress::Linear )
                            || ( m_eTrendLineType == SvxChartRegress::Polynomial )
                            || ( m_eTrendLineType == SvxChartRegress::Exp );
    m_xFmtFld_ExtrapolateForward->set_sensitive( !bMovingAverage );
    m_xFmtFld_ExtrapolateBackward->set_sensitive( !bMovingAverage );
    m_xCB_SetIntercept->set_sensitive( bInterceptAvailable );
    m_xFmtFld_InterceptValue->set_sensitive( bInterceptAvailable );
    if( bMovingAverage )
    {
        m_xCB_ShowEquation->set_state(TRISTATE_FALSE);
        m_xCB_ShowCorrelationCoeff->set_state(TRISTATE_FALSE);
    }
    m_xCB_ShowEquation->set_sensitive( !bMovingAverage );
    m_xCB_ShowCorrelationCoeff->set_sensitive( !bMovingAverage );
    m_xEE_XName->set_sensitive( !bMovingAverage && m_xCB_ShowEquation->get_active() );
    m_xEE_YName->set_sensitive( !bMovingAverage && m_xCB_ShowEquation->get_active() );
}

IMPL_LINK(TrendlineResources, ChangeSpinValue, weld::SpinButton&, rNumericField, void)
{
    if (&rNumericField == m_xNF_Degree.get())
    {
        if (!m_xRB_Polynomial->get_active() && m_xNF_Degree->get_value_changed_from_saved())
        {
            m_xRB_Polynomial->set_active(true);
            SelectTrendLine(*m_xRB_Polynomial);
        }
    }
    else if (&rNumericField == m_xNF_Period.get())
    {
        if (!m_xRB_MovingAverage->get_active() && m_xNF_Period->get_value_changed_from_saved())
        {
            m_xRB_MovingAverage->set_active(true);
            SelectTrendLine(*m_xRB_MovingAverage);
        }
    }
    UpdateControlStates();
}

IMPL_LINK_NOARG(TrendlineResources, ChangeFormattedValue, weld::FormattedSpinButton&, void)
{
    if (!m_xCB_SetIntercept->get_active())
        m_xCB_SetIntercept->set_active(true);
    UpdateControlStates();
}

void TrendlineResources::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumFormatter = pFormatter;
    m_xFmtFld_ExtrapolateForward->set_formatter( m_pNumFormatter );
    m_xFmtFld_ExtrapolateBackward->set_formatter( m_pNumFormatter );
    m_xFmtFld_InterceptValue->set_formatter( m_pNumFormatter );
}

void TrendlineResources::SetNbPoints( sal_Int32 nNbPoints )
{
    m_nNbPoints = nNbPoints;
    UpdateControlStates();
}

IMPL_LINK_NOARG(TrendlineResources, ShowEquation, weld::ToggleButton&, void)
{
    m_xEE_XName->set_sensitive(m_xCB_ShowEquation->get_active());
    m_xEE_YName->set_sensitive(m_xCB_ShowEquation->get_active());
    UpdateControlStates();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
