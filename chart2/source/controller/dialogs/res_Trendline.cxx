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
#include "tp_Trendline.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "chartview/ChartSfxItemIds.hxx"

#include <svl/intitem.hxx>

#include <vector>
#include <algorithm>

namespace chart
{

TrendlineResources::TrendlineResources( Window * pParent, const SfxItemSet& rInAttrs ) :
        m_aFLType( pParent, SchResId( FL_TYPE )),

        m_aRBLinear(        pParent, SchResId( RB_LINEAR            )),
        m_aRBLogarithmic(   pParent, SchResId( RB_LOGARITHMIC       )),
        m_aRBExponential(   pParent, SchResId( RB_EXPONENTIAL       )),
        m_aRBPower(         pParent, SchResId( RB_POWER             )),
        m_aRBPolynomial(    pParent, SchResId( RB_POLYNOMIAL        )),
        m_aRBMovingAverage( pParent, SchResId( RB_MOVING_AVERAGE    )),

        m_aFILinear(        pParent, SchResId( FI_LINEAR            )),
        m_aFILogarithmic(   pParent, SchResId( FI_LOGARITHMIC       )),
        m_aFIExponential(   pParent, SchResId( FI_EXPONENTIAL       )),
        m_aFIPower(         pParent, SchResId( FI_POWER             )),
        m_aFIPolynomial(    pParent, SchResId( FI_POLYNOMIAL       )),
        m_aFIMovingAverage( pParent, SchResId( FI_MOVING_AVERAGE    )),

        m_aFT_Degree(       pParent, SchResId( FT_DEGREE            )),
        m_aNF_Degree(       pParent, SchResId( NF_DEGREE            )),
        m_aFT_Period(       pParent, SchResId( FT_PERIOD            )),
        m_aNF_Period(       pParent, SchResId( NF_PERIOD            )),

        m_aFT_ExtrapolateForward(   pParent, SchResId( FT_EXTRAPOLATE_FORWARD            )),
        m_aNF_ExtrapolateForward(   pParent, SchResId( NF_EXTRAPOLATE_FORWARD            )),
        m_aFT_ExtrapolateBackward(  pParent, SchResId( FT_EXTRAPOLATE_BACKWARD           )),
        m_aNF_ExtrapolateBackward(  pParent, SchResId( NF_EXTRAPOLATE_BACKWARD           )),
        m_aCB_SetIntercept(         pParent, SchResId( CB_SET_INTERCEPT          )),
        m_aNF_InterceptValue(       pParent, SchResId( NF_INTERCEPT_VALUE            )),

        m_aFLEquation(             pParent, SchResId( FL_EQUATION               )),
        m_aCBShowEquation(         pParent, SchResId( CB_SHOW_EQUATION          )),
        m_aCBShowCorrelationCoeff( pParent, SchResId( CB_SHOW_CORRELATION_COEFF )),

        m_eTrendLineType( CHREGRESS_LINEAR ),
        m_bTrendLineUnique( true )
{
    FillValueSets();

    m_aRBLinear.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBLogarithmic.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBExponential.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBPower.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBPolynomial.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBMovingAverage.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));

    m_aNF_InterceptValue.SetMin( SAL_MIN_INT64 );
    m_aNF_InterceptValue.SetMax( SAL_MAX_INT64 );

    Reset( rInAttrs );
    UpdateControlStates();
}

TrendlineResources::~TrendlineResources()
{}


IMPL_LINK( TrendlineResources, SelectTrendLine, RadioButton *, pRadioButton )
{
    if( pRadioButton == &m_aRBLinear )
        m_eTrendLineType = CHREGRESS_LINEAR;
    else if( pRadioButton == &m_aRBLogarithmic )
        m_eTrendLineType = CHREGRESS_LOG;
    else if( pRadioButton == &m_aRBExponential )
        m_eTrendLineType = CHREGRESS_EXP;
    else if( pRadioButton == &m_aRBPower )
        m_eTrendLineType = CHREGRESS_POWER;
    else if( pRadioButton == &m_aRBPolynomial )
        m_eTrendLineType = CHREGRESS_POLYNOMIAL;
    else if( pRadioButton == &m_aRBMovingAverage )
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
        m_aNF_Degree.SetValue( nDegree );
    }
    else
    {
        m_aNF_Period.SetValue( 2 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_PERIOD, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_Int32 nPeriod = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_aNF_Period.SetValue( nPeriod );
    }
    else
    {
        m_aNF_Period.SetValue( 2 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        double nValue = static_cast< const SvxDoubleItem * >( pPoolItem )->GetValue() * 100;
        m_aNF_ExtrapolateForward.SetValue( (sal_Int64) nValue );
    }
    else
    {
        m_aNF_ExtrapolateForward.SetValue( 0 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        double nValue = static_cast< const SvxDoubleItem * >( pPoolItem )->GetValue() * 100;
        m_aNF_ExtrapolateBackward.SetValue( (sal_Int64) nValue );
    }
    else
    {
        m_aNF_ExtrapolateBackward.SetValue( 0 );
    }

    if( rInAttrs.GetItemState( SCHATTR_REGRESSION_INTERCEPT_VALUE, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        double nValue = static_cast< const SvxDoubleItem * >( pPoolItem )->GetValue() * 10000;
        m_aNF_InterceptValue.SetValue( (sal_Int64) nValue );
    }
    else
    {
        m_aNF_InterceptValue.SetValue( 0 );
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SET_INTERCEPT, sal_True, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        m_aCB_SetIntercept.EnableTriState( sal_True );
        m_aCB_SetIntercept.SetState( STATE_DONTKNOW );
    }
    else
    {
        m_aCB_SetIntercept.EnableTriState( sal_False );
        if( aState == SFX_ITEM_SET )
            m_aCB_SetIntercept.Check( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SHOW_EQUATION, sal_True, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        m_aCBShowEquation.EnableTriState( sal_True );
        m_aCBShowEquation.SetState( STATE_DONTKNOW );
    }
    else
    {
        m_aCBShowEquation.EnableTriState( sal_False );
        if( aState == SFX_ITEM_SET )
            m_aCBShowEquation.Check( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_SHOW_COEFF, sal_True, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        m_aCBShowCorrelationCoeff.EnableTriState( sal_True );
        m_aCBShowCorrelationCoeff.SetState( STATE_DONTKNOW );
    }
    else
    {
        m_aCBShowCorrelationCoeff.EnableTriState( sal_False );
        if( aState == SFX_ITEM_SET )
            m_aCBShowCorrelationCoeff.Check( static_cast< const SfxBoolItem * >( pPoolItem )->GetValue());
    }

    if( m_bTrendLineUnique )
    {
        switch( m_eTrendLineType )
        {
            case CHREGRESS_LINEAR :
                m_aRBLinear.Check();
                break;
            case CHREGRESS_LOG :
                m_aRBLogarithmic.Check();
                break;
            case CHREGRESS_EXP :
                m_aRBExponential.Check();
                break;
            case CHREGRESS_POWER :
                m_aRBPower.Check();
                break;
            case CHREGRESS_POLYNOMIAL :
                m_aRBPolynomial.Check();
                break;
            case CHREGRESS_MOVING_AVERAGE :
                m_aRBMovingAverage.Check();
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

    if( m_aCBShowEquation.GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_REGRESSION_SHOW_EQUATION, m_aCBShowEquation.IsChecked() ));

    if( m_aCBShowCorrelationCoeff.GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_REGRESSION_SHOW_COEFF, m_aCBShowCorrelationCoeff.IsChecked() ));

    sal_Int32 aDegree = m_aNF_Degree.GetValue();
    rOutAttrs.Put(SfxInt32Item( SCHATTR_REGRESSION_DEGREE, aDegree ) );

    sal_Int32 aPeriod = m_aNF_Period.GetValue();
    rOutAttrs.Put(SfxInt32Item( SCHATTR_REGRESSION_PERIOD, aPeriod ) );

    double aExtrapolateForwardValue = m_aNF_ExtrapolateForward.GetValue() / 100.0;
    rOutAttrs.Put(SvxDoubleItem( aExtrapolateForwardValue, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD ) );

    double aExtrapolateBackwardValue = m_aNF_ExtrapolateBackward.GetValue() / 100.0;
    rOutAttrs.Put(SvxDoubleItem( aExtrapolateBackwardValue, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD ) );

    if( m_aCB_SetIntercept.GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_REGRESSION_SET_INTERCEPT, m_aCB_SetIntercept.IsChecked() ));

    double aInterceptValue = m_aNF_InterceptValue.GetValue() / 10000.0;
    rOutAttrs.Put(SvxDoubleItem( aInterceptValue, SCHATTR_REGRESSION_INTERCEPT_VALUE ) );

    return sal_True;
}

void TrendlineResources::FillValueSets()
{
    m_aFILinear.SetImage(       Image( SchResId( BMP_REGRESSION_LINEAR          ) ) );
    m_aFILogarithmic.SetImage(  Image( SchResId( BMP_REGRESSION_LOG             ) ) );
    m_aFIExponential.SetImage(  Image( SchResId( BMP_REGRESSION_EXP             ) ) );
    m_aFIPower.SetImage(        Image( SchResId( BMP_REGRESSION_POWER           ) ) );
    m_aFIPolynomial.SetImage(   Image( SchResId( BMP_REGRESSION_POLYNOMIAL      ) ) );
    m_aFIMovingAverage.SetImage(Image( SchResId( BMP_REGRESSION_MOVING_AVERAGE  ) ) );
}

void TrendlineResources::UpdateControlStates()
{
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
