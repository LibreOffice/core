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


#include "dlg_InsertTrendline.hrc"
#include "res_Trendline.hxx"
#include "res_Trendline_IDs.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "chartview/ChartSfxItemIds.hxx"

#include <vector>
#include <algorithm>

namespace
{
template< class T >
    long lcl_getRightEdge( T & rControl )
{
    return rControl.CalcMinimumSize().Width() + rControl.GetPosPixel().X() - rControl.GetParent()->GetPosPixel().X();
}

template< class T >
    void lcl_AdjustControlSize( T & rControl )
{
    Size aSize( rControl.GetSizePixel());
    aSize.setWidth( rControl.CalcMinimumSize().Width());
    rControl.SetSizePixel( aSize );
}

void lcl_AdjustControlSize( Control & rControl, long nRightEdge )
{
    Size aSize( rControl.GetSizePixel());
    Point aPosition( rControl.GetPosPixel());
    aSize.setWidth( nRightEdge - aPosition.getX());
    rControl.SetSizePixel( aSize );
}

} // anonymous namespace

namespace chart
{

TrendlineResources::TrendlineResources( Window * pParent, const SfxItemSet& rInAttrs, bool bNoneAvailable ) :
        m_aFLType( pParent, SchResId( FL_TYPE )),

        m_aRBNone(        pParent, SchResId( RB_NONE        )),
        m_aRBLinear(      pParent, SchResId( RB_LINEAR      )),
        m_aRBLogarithmic( pParent, SchResId( RB_LOGARITHMIC )),
        m_aRBExponential( pParent, SchResId( RB_EXPONENTIAL )),
        m_aRBPower(       pParent, SchResId( RB_POWER       )),

        m_aFINone(        pParent, SchResId( FI_NONE        )),
        m_aFILinear(      pParent, SchResId( FI_LINEAR      )),
        m_aFILogarithmic( pParent, SchResId( FI_LOGARITHMIC )),
        m_aFIExponential( pParent, SchResId( FI_EXPONENTIAL )),
        m_aFIPower(       pParent, SchResId( FI_POWER       )),

        m_aFLEquation(             pParent, SchResId( FL_EQUATION               )),
        m_aCBShowEquation(         pParent, SchResId( CB_SHOW_EQUATION          )),
        m_aCBShowCorrelationCoeff( pParent, SchResId( CB_SHOW_CORRELATION_COEFF )),

        m_eTrendLineType( CHREGRESS_NONE ),
        m_bNoneAvailable( bNoneAvailable ),
        m_bTrendLineUnique( true )
{
    FillValueSets();

    if( m_bNoneAvailable )
        m_aRBNone.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    else
        m_aRBNone.Hide();

    m_aRBLinear.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBLogarithmic.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBExponential.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));
    m_aRBPower.SetClickHdl( LINK(this, TrendlineResources, SelectTrendLine ));

    Reset( rInAttrs );
    UpdateControlStates();
}

TrendlineResources::~TrendlineResources()
{}

long TrendlineResources::adjustControlSizes()
{
    // calculate right edge
    ::std::vector< long > aControlRightEdges;
    aControlRightEdges.push_back( lcl_getRightEdge( m_aRBNone ));
    aControlRightEdges.push_back( lcl_getRightEdge( m_aRBLinear ));
    aControlRightEdges.push_back( lcl_getRightEdge( m_aRBLogarithmic ));
    aControlRightEdges.push_back( lcl_getRightEdge( m_aRBExponential ));
    aControlRightEdges.push_back( lcl_getRightEdge( m_aRBPower ));
    aControlRightEdges.push_back( lcl_getRightEdge( m_aCBShowEquation ));
    aControlRightEdges.push_back( lcl_getRightEdge( m_aCBShowCorrelationCoeff ));

    lcl_AdjustControlSize( m_aRBNone );
    lcl_AdjustControlSize( m_aRBLinear );
    lcl_AdjustControlSize( m_aRBLogarithmic );
    lcl_AdjustControlSize( m_aRBExponential );
    lcl_AdjustControlSize( m_aRBPower );
    lcl_AdjustControlSize( m_aCBShowEquation );
    lcl_AdjustControlSize( m_aCBShowCorrelationCoeff );

    // Note: FixedLine has no CalcMinimumSize, workaround: use a FixedText
    FixedText aDummyTextCtrl( m_aFLType.GetParent());
    aDummyTextCtrl.SetText( m_aFLType.GetText());
    aControlRightEdges.push_back( lcl_getRightEdge( aDummyTextCtrl ));
    aDummyTextCtrl.SetText( m_aFLEquation.GetText());
    aControlRightEdges.push_back( lcl_getRightEdge( aDummyTextCtrl ));

    long nRightEdgeOfControls = *(::std::max_element( aControlRightEdges.begin(), aControlRightEdges.end()));
    // leave some more space after the longest text
    nRightEdgeOfControls += m_aFLType.LogicToPixel( Size( 6, 0 ), MapMode( MAP_APPFONT )).getWidth();

    lcl_AdjustControlSize( m_aFLType, nRightEdgeOfControls );
    lcl_AdjustControlSize( m_aFLEquation, nRightEdgeOfControls );

    return nRightEdgeOfControls;
}

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
    else if( pRadioButton == &m_aRBNone )
    {
        OSL_ASSERT( m_bNoneAvailable );
        m_eTrendLineType = CHREGRESS_NONE;
    }
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
            m_eTrendLineType = pItem->GetValue();
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
            case CHREGRESS_NONE:
                OSL_ASSERT( m_bNoneAvailable );
                m_aRBNone.Check();
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
    return sal_True;
}

void TrendlineResources::FillValueSets()
{
    if( m_bNoneAvailable )
        m_aFINone.SetImage(    Image( SchResId( BMP_REGRESSION_NONE   ) ) );
    m_aFILinear.SetImage(      Image( SchResId( BMP_REGRESSION_LINEAR ) ) );
    m_aFILogarithmic.SetImage( Image( SchResId( BMP_REGRESSION_LOG    ) ) );
    m_aFIExponential.SetImage( Image( SchResId( BMP_REGRESSION_EXP    ) ) );
    m_aFIPower.SetImage(       Image( SchResId( BMP_REGRESSION_POWER  ) ) );
}

void TrendlineResources::UpdateControlStates()
{
    if( m_bNoneAvailable )
    {
        bool bEnableEquationControls = !m_bTrendLineUnique || (m_eTrendLineType != CHREGRESS_NONE);
        m_aCBShowEquation.Enable( bEnableEquationControls );
        m_aCBShowCorrelationCoeff.Enable( bEnableEquationControls );
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
