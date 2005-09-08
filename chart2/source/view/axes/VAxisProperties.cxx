/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VAxisProperties.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:37:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "VAxisProperties.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE2_HPP_
#include <com/sun/star/text/WritingMode2.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

sal_Int32 lcl_calcTickLengthForDepth(sal_Int32 nDepth,sal_Int32 nTickmarkStyle)
{
    sal_Int32 nWidth = AXIS2D_TICKLENGTH; //@maybefuturetodo this length could be offered by the model
    double fPercent = 1.0;
    switch(nDepth)
    {
        case 0:
            fPercent = 1.0;
            break;
        case 1:
            fPercent = 0.75;//percentage like in the old chart
            break;
        case 2:
            fPercent = 0.5;
            break;
        default:
            fPercent = 0.3;
            break;
    }
    if(nTickmarkStyle==3)//inner and outer tickmarks
        fPercent*=2.0;
    return static_cast<sal_Int32>(nWidth*fPercent);
}

double lcl_getTickOffset(sal_Int32 nLength,sal_Int32 nTickmarkStyle)
{
    double fPercent = 0.0; //0<=fPercent<=1
    //0.0: completly inner
    //1.0: completly outer
    //0.5: half and half

    /*
    nTickmarkStyle:
    1: inner tickmarks
    2: outer tickmarks
    3: inner and outer tickmarks
    */
    switch(nTickmarkStyle)
    {
        case 1:
            fPercent = 0.0;
            break;
        case 2:
            fPercent = 1.0;
            break;
        default:
            fPercent = 0.5;
            break;
    }
    return fPercent*nLength;
}

VLineProperties AxisProperties::makeLinePropertiesForDepth( sal_Int32 nDepth ) const
{
    //@todo get this from somewhere; maybe for each subincrement
    //so far the model does not offer different settings for each tick depth
    return m_aLineProperties;
}

TickmarkProperties AxisProperties::makeTickmarkProperties(
                        sal_Int32 nDepth ) const
{
    /*
    nTickmarkStyle:
    1: inner tickmarks
    2: outer tickmarks
    3: inner and outer tickmarks
    */
    sal_Int32 nTickmarkStyle = 1;
    if(nDepth==0)
    {
        nTickmarkStyle = m_nMajorTickmarks;
        if(!nTickmarkStyle)
        {
            //create major tickmarks as if they were minor tickmarks
            nDepth = 1;
            nTickmarkStyle = m_nMinorTickmarks;
        }
    }
    else if( nDepth==1)
    {
        nTickmarkStyle = m_nMinorTickmarks;
    }

    if( m_fInnerDirectionSign == 0.0 )
    {
        if( nTickmarkStyle != 0 )
            nTickmarkStyle = 3; //inner and outer tickmarks
    }

    TickmarkProperties aTickmarkProperties;
    aTickmarkProperties.Length = lcl_calcTickLengthForDepth(nDepth,nTickmarkStyle);
    aTickmarkProperties.RelativePos = static_cast<sal_Int32>(lcl_getTickOffset(aTickmarkProperties.Length,nTickmarkStyle));
    aTickmarkProperties.aLineProperties = this->makeLinePropertiesForDepth( nDepth );
    return aTickmarkProperties;
}

//--------------------------------------------------------------------------

AxisProperties::AxisProperties( const uno::Reference< XAxis >& xAxisModel
                              , const ::com::sun::star::awt::Size& rReferenceSize )
    : m_xAxisModel(xAxisModel)
    , m_aReferenceSize(rReferenceSize)
    , m_bIsMainAxis(true)
    , m_pfMainLinePositionAtOtherAxis(NULL)
    , m_pfExrtaLinePositionAtOtherAxis(NULL)
    , m_fInnerDirectionSign(1.0)
    , m_bLabelsOutside(true)
    , m_aLabelAlignment(LABEL_ALIGN_RIGHT_TOP)
    , m_bDisplayLabels( true )
//    , m_eRelativeLabelPosition(LEFTORBOTTOM_OF_AXIS)
    , m_nMajorTickmarks(1)
    , m_nMinorTickmarks(1)
    , m_aTickmarkPropertiesList()
    , m_aLineProperties()
{
}

AxisProperties::AxisProperties( const AxisProperties& rAxisProperties )
    : m_xAxisModel( rAxisProperties.m_xAxisModel )
    , m_aReferenceSize( rAxisProperties.m_aReferenceSize )
    , m_bIsMainAxis( rAxisProperties.m_bIsMainAxis )
    , m_pfMainLinePositionAtOtherAxis( NULL )
    , m_pfExrtaLinePositionAtOtherAxis( NULL )
    , m_fInnerDirectionSign( rAxisProperties.m_fInnerDirectionSign )
    , m_bLabelsOutside( rAxisProperties.m_bLabelsOutside )
    , m_aLabelAlignment( rAxisProperties.m_aLabelAlignment )
    , m_bDisplayLabels( rAxisProperties.m_bDisplayLabels )
//    , m_eRelativeLabelPosition( rAxisProperties.m_eRelativeLabelPosition )
    , m_nMajorTickmarks( rAxisProperties.m_nMajorTickmarks )
    , m_nMinorTickmarks( rAxisProperties.m_nMinorTickmarks )
    , m_aTickmarkPropertiesList( rAxisProperties.m_aTickmarkPropertiesList )
    , m_aLineProperties( rAxisProperties.m_aLineProperties )
{
    if( rAxisProperties.m_pfMainLinePositionAtOtherAxis )
        m_pfMainLinePositionAtOtherAxis = new double(*rAxisProperties.m_pfMainLinePositionAtOtherAxis);
    if( rAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        m_pfExrtaLinePositionAtOtherAxis = new double (*rAxisProperties.m_pfExrtaLinePositionAtOtherAxis);
}

AxisProperties::~AxisProperties()
{
    delete m_pfMainLinePositionAtOtherAxis;
    delete m_pfExrtaLinePositionAtOtherAxis;
}

LabelAlignment lcl_getLabelAlignment( const AxisProperties& rAxisProperties, bool bIsYAxis )
{
    LabelAlignment aRet( LABEL_ALIGN_LEFT );
    if(bIsYAxis)
    {
        if(rAxisProperties.m_bIsMainAxis)
        {
            if( rAxisProperties.m_bLabelsOutside )
                aRet = LABEL_ALIGN_LEFT;
            else
                aRet = LABEL_ALIGN_RIGHT;
        }
        else
        {
            if( !rAxisProperties.m_bLabelsOutside )
                aRet = LABEL_ALIGN_LEFT;
            else
                aRet = LABEL_ALIGN_RIGHT;
        }
    }
    else
    {
        if(rAxisProperties.m_bIsMainAxis )
        {
            if(rAxisProperties.m_bLabelsOutside)
                aRet = LABEL_ALIGN_BOTTOM;
            else
                aRet = LABEL_ALIGN_TOP;
        }
        else
        {
            if(!rAxisProperties.m_bLabelsOutside)
                aRet = LABEL_ALIGN_BOTTOM;
            else
                aRet = LABEL_ALIGN_TOP;
        }
    }
    return aRet;
}

void AxisProperties::init( bool bCartesian )
{
    if( bCartesian )
    {
        sal_Int32 nDimensionIndex = m_xAxisModel->getRepresentedDimension();
        m_fInnerDirectionSign = m_bIsMainAxis ? 1 : -1;
        if(nDimensionIndex==1)
            m_fInnerDirectionSign*=-1;
        m_aLabelAlignment = lcl_getLabelAlignment(*this,nDimensionIndex==1);
    }

    uno::Reference< beans::XPropertySet > xProp =
        uno::Reference<beans::XPropertySet>::query( this->m_xAxisModel );
    if( !xProp.is() )
        return;

    try
    {
        //init LineProperties
        m_aLineProperties.initFromPropertySet( xProp );

        //init display labels
        xProp->getPropertyValue( C2U( "DisplayLabels" ) ) >>= m_bDisplayLabels;

        //init TickmarkProperties
        xProp->getPropertyValue( C2U( "MajorTickmarks" ) ) >>= m_nMajorTickmarks;
        xProp->getPropertyValue( C2U( "MinorTickmarks" ) ) >>= m_nMinorTickmarks;

        sal_Int32 nMaxDepth = 0;
        if(m_nMinorTickmarks!=0)
            nMaxDepth=2;
        else if(m_nMajorTickmarks!=0)
            nMaxDepth=1;

        this->m_aTickmarkPropertiesList.clear();
        for( sal_Int32 nDepth=0; nDepth<nMaxDepth; nDepth++ )
        {
            TickmarkProperties aTickmarkProperties = this->makeTickmarkProperties( nDepth );
            this->m_aTickmarkPropertiesList.push_back( aTickmarkProperties );
        }
    }
    catch( uno::Exception& e )
    {
            e;
    }
}

//-----------------------------------------------------------------------------

AxisLabelProperties::AxisLabelProperties()
                        : aNumberFormat()
                        , eStaggering( SIDE_BY_SIDE )
                        , bLineBreakAllowed( true )
                        , bOverlapAllowed( false )
                        , bStackCharacters( false )
                        , fRotationAngleDegree( 0.0 )
                        , nRhythm( 1 )
                        , bRhythmIsFix(false)
{
    /*
    aLocale.Language = C2U( "en" );
    aLocale.Country  = C2U( "US" );

    //aLocale.Language = C2U( "ar" );
    //aLocale.Country  = C2U( "IR" );

    //aLocale.Language = C2U( "ja" );
    //aLocale.Country  = C2U( "JP" );
    */
}

void AxisLabelProperties::init( const uno::Reference< XAxis >& xAxisModel )
{
    uno::Reference< beans::XPropertySet > xProp =
        uno::Reference<beans::XPropertySet>::query( xAxisModel );
    if(xProp.is())
    {
        try
        {
            if( !( xProp->getPropertyValue( C2U( "NumberFormat" ) ) >>= this->aNumberFormat ) )
            {
                //@todo get number format from calc
            }

            xProp->getPropertyValue( C2U( "TextBreak" ) ) >>= this->bLineBreakAllowed;
            xProp->getPropertyValue( C2U( "TextOverlap" ) ) >>= this->bOverlapAllowed;
            xProp->getPropertyValue( C2U( "StackCharacters" ) ) >>= this->bStackCharacters;
            xProp->getPropertyValue( C2U( "TextRotation" ) ) >>= this->fRotationAngleDegree;

            ::com::sun::star::chart::ChartAxisArrangeOrderType eArrangeOrder;
            xProp->getPropertyValue( C2U( "ArrangeOrder" ) ) >>= eArrangeOrder;
            switch(eArrangeOrder)
            {
                case ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
                    this->eStaggering = SIDE_BY_SIDE;
                    break;
                case ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
                    this->eStaggering = STAGGER_EVEN;
                    break;
                case ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD:
                    this->eStaggering = STAGGER_ODD;
                    break;
                default:
                    this->eStaggering = STAGGER_AUTO;
                    break;
            }
        }
        catch( uno::Exception& e )
        {
             e;
        }
    }
}

/*
sal_Int16 getSwappedWritingMode( sal_Int16 nWritingMode )
{
    //LR_TB == LT
    //RL_TB == RT (Arabic, Hebrew)
    //TB_RL == TR (Japanese, Chinese, Korean)
    // ?? TL (Mongolian) see also text::WritingMode2

    switch(nWritingMode)
    {
        case text::WritingMode2::RL_TB:
            return  text::WritingMode2::TB_RL;
        case text::WritingMode2::TB_RL:
            return  text::WritingMode2::RL_TB;
        case text::WritingMode2::LR_TB:
            return  text::WritingMode2::TB_LR;
        default:
            return  text::WritingMode2::LR_TB;
    }
}
*/

sal_Bool AxisLabelProperties::getIsStaggered() const
{
    if( STAGGER_ODD == eStaggering || STAGGER_EVEN == eStaggering )
        return sal_True;
    return sal_False;
}

//.............................................................................
} //namespace chart
//.............................................................................
