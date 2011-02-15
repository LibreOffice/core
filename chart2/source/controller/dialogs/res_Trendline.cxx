/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_InsertTrendline.hrc"
#include "res_Trendline.hxx"
#include "res_Trendline_IDs.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "Bitmaps_HC.hrc"
#include "chartview/ChartSfxItemIds.hxx"

#include <vector>
#include <algorithm>

// macro for selecting a normal or high contrast bitmap the stack variable
// bIsHighContrast must exist and reflect the correct state
#define SELECT_IMAGE(name) Image( SchResId( bIsHighContrast ? name ## _HC : name ))

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

enum StatTrendLine
{
    TRENDLINE_NONE,
    TRENDLINE_LINE,
    TRENDLINE_LOG,
    TRENDLINE_EXP,
    TRENDLINE_POW
};

TrendlineResources::TrendlineResources( Window * pParent, const SfxItemSet& rInAttrs, bool bNoneAvailable ) :
        m_aFLType( pParent, SchResId( FL_TYPE )),

        m_aRBNone( pParent, SchResId( RB_NONE )),
        m_aRBLinear( pParent, SchResId( RB_LINEAR )),
        m_aRBLogarithmic( pParent, SchResId( RB_LOGARITHMIC )),
        m_aRBExponential( pParent, SchResId( RB_EXPONENTIAL )),
        m_aRBPower( pParent, SchResId( RB_POWER )),

        m_aFINone( pParent, SchResId( FI_NONE )),
        m_aFILinear( pParent, SchResId( FI_LINEAR )),
        m_aFILogarithmic( pParent, SchResId( FI_LOGARITHMIC )),
        m_aFIExponential( pParent, SchResId( FI_EXPONENTIAL )),
        m_aFIPower( pParent, SchResId( FI_POWER )),

        m_aFLEquation( pParent, SchResId( FL_EQUATION )),
        m_aCBShowEquation( pParent, SchResId( CB_SHOW_EQUATION )),
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
    SfxItemState aState = SFX_ITEM_UNKNOWN;

    aState = rInAttrs.GetItemState( SCHATTR_REGRESSION_TYPE, sal_True, &pPoolItem );
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
    bool bIsHighContrast = ( true && m_aFLType.GetSettings().GetStyleSettings().GetHighContrastMode() );

    if( m_bNoneAvailable )
        m_aFINone.SetImage( SELECT_IMAGE( BMP_REGRESSION_NONE ));
    m_aFILinear.SetImage( SELECT_IMAGE( BMP_REGRESSION_LINEAR ));
    m_aFILogarithmic.SetImage( SELECT_IMAGE( BMP_REGRESSION_LOG ));
    m_aFIExponential.SetImage( SELECT_IMAGE( BMP_REGRESSION_EXP ));
    m_aFIPower.SetImage( SELECT_IMAGE( BMP_REGRESSION_POWER ));
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
