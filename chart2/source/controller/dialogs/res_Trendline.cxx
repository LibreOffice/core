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
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "chartview/ChartSfxItemIds.hxx"

#include <svl/intitem.hxx>
#include <sfx2/tabdlg.hxx>

#include <vector>
#include <algorithm>

namespace chart
{

TrendlineResources::TrendlineResources( Window * pParent, const SfxItemSet& rInAttrs ) :
        m_eTrendLineType( CHREGRESS_LINEAR ),
        m_bTrendLineUnique( true )
{
    ((SfxTabPage*)pParent)->get(m_pRB_Linear,"linear");
    ((SfxTabPage*)pParent)->get(m_pRB_Logarithmic,"logarithmic");
    ((SfxTabPage*)pParent)->get(m_pRB_Exponential,"exponential");
    ((SfxTabPage*)pParent)->get(m_pRB_Power,"power");
    ((SfxTabPage*)pParent)->get(m_pRB_Polynomial,"polynomial");
    ((SfxTabPage*)pParent)->get(m_pRB_MovingAverage,"movingAverage");
    ((SfxTabPage*)pParent)->get(m_pNF_Degree,"degree");
    ((SfxTabPage*)pParent)->get(m_pNF_Period,"period");
    ((SfxTabPage*)pParent)->get(m_pNF_ExtrapolateForward,"extrapolateForward");
    ((SfxTabPage*)pParent)->get(m_pNF_ExtrapolateBackward,"extrapolateBackward");
    ((SfxTabPage*)pParent)->get(m_pCB_SetIntercept,"setIntercept");
    ((SfxTabPage*)pParent)->get(m_pNF_InterceptValue,"interceptValue");
    ((SfxTabPage*)pParent)->get(m_pCB_ShowEquation,"showEquation");
    ((SfxTabPage*)pParent)->get(m_pCB_ShowCorrelationCoeff,"showCorrelationCoefficient");
    ((SfxTabPage*)pParent)->get(m_pFI_Linear,"imageLinear");
    ((SfxTabPage*)pParent)->get(m_pFI_Logarithmic,"imageLogarithmic");
    ((SfxTabPage*)pParent)->get(m_pFI_Exponential,"imageExponential");
    ((SfxTabPage*)pParent)->get(m_pFI_Power,"imagePower");
    ((SfxTabPage*)pParent)->get(m_pFI_Polynomial,"imagePolynomial");
    ((SfxTabPage*)pParent)->get(m_pFI_MovingAverage,"imageMovingAverage");
    FillValueSets();

    Link aLink = LINK(this, TrendlineResources, SelectTrendLine );
    m_pRB_Linear->SetClickHdl( aLink );
    m_pRB_Logarithmic->SetClickHdl( aLink );
    m_pRB_Exponential->SetClickHdl( aLink );
    m_pRB_Power->SetClickHdl( aLink );
    m_pRB_Polynomial->SetClickHdl( aLink );
    m_pRB_MovingAverage->SetClickHdl( aLink );

    aLink = LINK(this, TrendlineResources, ChangeNumericField );
    m_pNF_Degree->SetModifyHdl( aLink );
    m_pNF_Period->SetModifyHdl( aLink );
    m_pNF_InterceptValue->SetModifyHdl( aLink );

    m_pNF_ExtrapolateForward->SetMin( SAL_MIN_INT64 );
    m_pNF_ExtrapolateForward->SetMax( SAL_MAX_INT64 );
    m_pNF_ExtrapolateBackward->SetMin( SAL_MIN_INT64 );
    m_pNF_ExtrapolateBackward->SetMax( SAL_MAX_INT64 );
    m_pNF_InterceptValue->SetMin( SAL_MIN_INT64 );
    m_pNF_InterceptValue->SetMax( SAL_MAX_INT64 );

    Reset( rInAttrs );
    UpdateControlStates();
}

TrendlineResources::~TrendlineResources()
{}

IMPL_LINK( TrendlineResources, SelectTrendLine, RadioButton *, pRadioButton )
{
    if( pRadioButton == m_pRB_Linear )
        m_eTrendLineType = CHREGRESS_LINEAR;
    else if( pRadioButton == m_pRB_Logarithmic )
        m_eTrendLineType = CHREGRESS_LOG;
    else if( pRadioButton == m_pRB_Exponential )
        m_eTrendLineType = CHREGRESS_EXP;
    else if( pRadioButton == m_pRB_Power )
        m_eTrendLineType = CHREGRESS_POWER;
    else if( pRadioButton == m_pRB_Polynomial )
        m_eTrendLineType = CHREGRESS_POLYNOMIAL;
    else if( pRadioButton == m_pRB_MovingAverage )
        m_eTrendLineType = CHREGRESS_MOVING_AVERAGE;
    m_bTrendLineUnique = true;

    UpdateControlStates();

    return 0;
}

void TrendlineResources::Reset( const SfxItemSet& rInAttrs )
{
    const SfxPoolItem *pPoolItem = NULL;

    SfxItemState aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_TYPE, sal_True, &pPoolItem );
    m_bTrendLineUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
    {
        const SvxChartRegressItem * pItem = dynamic_cast< const SvxChartRegressItem * >( pPoolItem );
        if( pItem )
        {
            m_eTrendLineType = pItem->GetValue();
        }
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_DEGREE, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_Int32 nDegree = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_pNF_Degree->SetValue( nDegree );
    }
    else
    {
        m_pNF_Degree->SetValue( 2 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_PERIOD, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_Int32 nPeriod = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_pNF_Period->SetValue( nPeriod );
    }
    else
    {
        m_pNF_Period->SetValue( 2 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        double nValue = static_cast< const SvxDoubleItem * >( pPoolItem )->GetValue() * 100;
        m_pNF_ExtrapolateForward->SetValue( (sal_Int64) nValue );
    }
    else
    {
        m_pNF_ExtrapolateForward->SetValue( 0 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        double nValue = static_cast< const SvxDoubleItem * >( pPoolItem )->GetValue() * 100;
        m_pNF_ExtrapolateBackward->SetValue( (sal_Int64) nValue );
    }
    else
    {
        m_pNF_ExtrapolateBackward->SetValue( 0 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_INTERCEPT_VALUE, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        double nValue = static_cast< const SvxDoubleItem * >( pPoolItem )->GetValue() * 10000;
        m_pNF_InterceptValue->SetValue( (sal_Int64) nValue );
    }
    else
    {
        m_pNF_InterceptValue->SetValue( 0 );
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SET_INTERCEPT, sal_True, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        m_pCB_SetIntercept->EnableTriState( sal_True );
        m_pCB_SetIntercept->SetState( STATE_DONTKNOW );
    }
    else
    {
        m_pCB_SetIntercept->EnableTriState( sal_False );
        if( aState == SFX_ITEM_SET )
            m_pCB_SetIntercept->Check( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SHOW_EQUATION, sal_True, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        m_pCB_ShowEquation->EnableTriState( sal_True );
        m_pCB_ShowEquation->SetState( STATE_DONTKNOW );
    }
    else
    {
        m_pCB_ShowEquation->EnableTriState( sal_False );
        if( aState == SFX_ITEM_SET )
            m_pCB_ShowEquation->Check( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SHOW_COEFF, sal_True, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        m_pCB_ShowCorrelationCoeff->EnableTriState( sal_True );
        m_pCB_ShowCorrelationCoeff->SetState( STATE_DONTKNOW );
    }
    else
    {
        m_pCB_ShowCorrelationCoeff->EnableTriState( sal_False );
        if( aState == SFX_ITEM_SET )
            m_pCB_ShowCorrelationCoeff->Check( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    if( m_bTrendLineUnique )
    {
        switch( m_eTrendLineType )
        {
            case CHREGRESS_LINEAR :
                m_pRB_Linear->Check();
                break;
            case CHREGRESS_LOG :
                m_pRB_Logarithmic->Check();
                break;
            case CHREGRESS_EXP :
                m_pRB_Exponential->Check();
                break;
            case CHREGRESS_POWER :
                m_pRB_Power->Check();
                break;
            case CHREGRESS_POLYNOMIAL :
                m_pRB_Polynomial->Check();
                break;
            case CHREGRESS_MOVING_AVERAGE :
                m_pRB_MovingAverage->Check();
                break;
            default:
                break;
        }
    }
}

sal_Bool TrendlineResources::FillItemSet(SfxItemSet& rOutAttrs) const
{
    if( m_bTrendLineUnique )
        rOutAttrs.Put( SvxChartRegressItem( m_eTrendLineType, SCHATTR_REGRESSION_TYPE ));

    if( m_pCB_ShowEquation->GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_REGRESSION_SHOW_EQUATION, m_pCB_ShowEquation->IsChecked() ));

    if( m_pCB_ShowCorrelationCoeff->GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_REGRESSION_SHOW_COEFF, m_pCB_ShowCorrelationCoeff->IsChecked() ));

    sal_Int32 aDegree = m_pNF_Degree->GetValue();
    rOutAttrs.Put(SfxInt32Item( SCHATTR_REGRESSION_DEGREE, aDegree ) );

    sal_Int32 aPeriod = m_pNF_Period->GetValue();
    rOutAttrs.Put(SfxInt32Item( SCHATTR_REGRESSION_PERIOD, aPeriod ) );

    double aExtrapolateForwardValue = m_pNF_ExtrapolateForward->GetValue() / 100.0;
    rOutAttrs.Put(SvxDoubleItem( aExtrapolateForwardValue, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD ) );

    double aExtrapolateBackwardValue = m_pNF_ExtrapolateBackward->GetValue() / 100.0;
    rOutAttrs.Put(SvxDoubleItem( aExtrapolateBackwardValue, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD ) );

    if( m_pCB_SetIntercept->GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_REGRESSION_SET_INTERCEPT, m_pCB_SetIntercept->IsChecked() ));

    double aInterceptValue = m_pNF_InterceptValue->GetValue() / 10000.0;
    rOutAttrs.Put(SvxDoubleItem( aInterceptValue, SCHATTR_REGRESSION_INTERCEPT_VALUE ) );

    return sal_True;
}

void TrendlineResources::FillValueSets()
{
    m_pFI_Linear->SetImage(       Image( SchResId( BMP_REGRESSION_LINEAR          ) ) );
    m_pFI_Logarithmic->SetImage(  Image( SchResId( BMP_REGRESSION_LOG             ) ) );
    m_pFI_Exponential->SetImage(  Image( SchResId( BMP_REGRESSION_EXP             ) ) );
    m_pFI_Power->SetImage(        Image( SchResId( BMP_REGRESSION_POWER           ) ) );
    m_pFI_Polynomial->SetImage(   Image( SchResId( BMP_REGRESSION_POLYNOMIAL      ) ) );
    m_pFI_MovingAverage->SetImage(Image( SchResId( BMP_REGRESSION_MOVING_AVERAGE  ) ) );
}

void TrendlineResources::UpdateControlStates()
{
}

IMPL_LINK( TrendlineResources, ChangeNumericField, NumericField *, pNumericField)
{
    if( pNumericField == m_pNF_Degree )
    {
        if( !m_pRB_Polynomial->IsChecked() )
        {
                m_pRB_Polynomial->Check();
                SelectTrendLine(m_pRB_Polynomial);
        }
    }
    else if( pNumericField == m_pNF_Period )
    {
        if( !m_pRB_MovingAverage->IsChecked() )
        {
                m_pRB_MovingAverage->Check();
                SelectTrendLine(m_pRB_MovingAverage);
        }
    }
    else if( pNumericField == m_pNF_InterceptValue )
    {
        if( !m_pCB_SetIntercept->IsChecked() )
                m_pCB_SetIntercept->Check();
    }
    UpdateControlStates();

    return 0;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
