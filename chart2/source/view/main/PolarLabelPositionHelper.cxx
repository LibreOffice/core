/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolarLabelPositionHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:12:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "PolarLabelPositionHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif
#ifndef _BGFX_VECTOR_B2IVECTOR_HXX
#include <basegfx/vector/b2ivector.hxx>
#endif

#include <com/sun/star/chart/DataLabelPlacement.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

PolarLabelPositionHelper::PolarLabelPositionHelper(
                    PolarPlottingPositionHelper* pPosHelper
                    , sal_Int32 nDimensionCount
                    , const uno::Reference< drawing::XShapes >& xLogicTarget
                    , ShapeFactory* pShapeFactory )
                    : LabelPositionHelper( pPosHelper, nDimensionCount, xLogicTarget, pShapeFactory )
                    , m_pPosHelper(pPosHelper)
{
}

PolarLabelPositionHelper::~PolarLabelPositionHelper()
{
}

awt::Point PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForLogicValues(
        LabelAlignment& rAlignment
        , double fLogicValueOnAngleAxis
        , double fLogicValueOnRadiusAxis
        , double fLogicZ
        , sal_Int32 nScreenValueOffsetInRadiusDirection ) const
{
    double fUnitCircleAngleDegree = m_pPosHelper->transformToAngleDegree( fLogicValueOnAngleAxis );
    double fUnitCircleRadius = m_pPosHelper->transformToRadius( fLogicValueOnRadiusAxis );

    return getLabelScreenPositionAndAlignmentForUnitCircleValues(
           rAlignment, ::com::sun::star::chart::DataLabelPlacement::OUTSIDE
           , fUnitCircleAngleDegree, 0.0
           , fUnitCircleRadius, fUnitCircleRadius, fLogicZ, nScreenValueOffsetInRadiusDirection );
}

awt::Point PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues(
        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement
        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
        , double fLogicZ
        , sal_Int32 nScreenValueOffsetInRadiusDirection ) const
{
    bool bCenter = (nLabelPlacement != ::com::sun::star::chart::DataLabelPlacement::OUTSIDE)
                && (nLabelPlacement != ::com::sun::star::chart::DataLabelPlacement::INSIDE);

    double fAngleDegree = fUnitCircleStartAngleDegree + fUnitCircleWidthAngleDegree/2.0;
    double fRadius = 0.0;
    if( !bCenter ) //e.g. for pure pie chart(one ring only) or for angle axis of polyar coordinate system
        fRadius = fUnitCircleOuterRadius;
    else
        fRadius = fUnitCircleInnerRadius + (fUnitCircleOuterRadius-fUnitCircleInnerRadius)/2.0 ;

    awt::Point aRet( this->transformSceneToScreenPosition(
        m_pPosHelper->transformUnitCircleToScene( fAngleDegree, fRadius, fLogicZ+0.5 ) ) );

    if(3==m_nDimensionCount && nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::OUTSIDE)
    {
        //check wether the upper or the downer edge is more distant from the center
        //take the farest point to put the label to

        awt::Point aP0( this->transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( 0, 0, fLogicZ ) ) );
        awt::Point aP1(aRet);
        awt::Point aP2( this->transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( fAngleDegree, fRadius, fLogicZ-0.5 ) ) );

        ::basegfx::B2DVector aV0( aP0.X, aP0.Y );
        ::basegfx::B2DVector aV1( aP1.X, aP1.Y );
        ::basegfx::B2DVector aV2( aP2.X, aP2.Y );

        double fL1 = ::basegfx::B2DVector(aV1-aV0).getLength();
        double fL2 = ::basegfx::B2DVector(aV2-aV0).getLength();

        if(fL2>fL1)
            aRet = aP2;

        //calculate new angle for alignment
        double fDX = aRet.X-aP0.X;
        double fDY = aRet.Y-aP0.Y;
        fDY*=-1.0;//drawing layer has inverse y values
        if( fDX != 0.0 )
        {
            fAngleDegree = atan(fDY/fDX)*180.0/F_PI;
            if(fDX<0.0)
                fAngleDegree+=180.0;
        }
        else
        {
            if(fDY>0.0)
                fAngleDegree = 90.0;
            else
                fAngleDegree = 270.0;
        }
    }
    //------------------------------
    //set LabelAlignment
    if( !bCenter )
    {
        while(fAngleDegree>360.0)
            fAngleDegree-=360.0;
        while(fAngleDegree<0.0)
            fAngleDegree+=360.0;

        bool bOutside = nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;

        if(fAngleDegree==0.0)
            rAlignment = LABEL_ALIGN_CENTER;
        else if(fAngleDegree<=22.5)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
        else if(fAngleDegree<67.5)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT_TOP : LABEL_ALIGN_LEFT_BOTTOM;
        else if(fAngleDegree<112.5)
            rAlignment = bOutside ? LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;
        else if(fAngleDegree<=157.5)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT_TOP : LABEL_ALIGN_RIGHT_BOTTOM;
        else if(fAngleDegree<=202.5)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT : LABEL_ALIGN_RIGHT;
        else if(fAngleDegree<247.5)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT_BOTTOM : LABEL_ALIGN_RIGHT_TOP;
        else if(fAngleDegree<292.5)
            rAlignment = bOutside ? LABEL_ALIGN_BOTTOM : LABEL_ALIGN_TOP;
        else if(fAngleDegree<337.5)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT_BOTTOM : LABEL_ALIGN_LEFT_TOP;
        else
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
    }
    else
    {
        rAlignment = LABEL_ALIGN_CENTER;
    }

    //add a scaling independent Offset if requested
    if( nScreenValueOffsetInRadiusDirection != 0)
    {
        awt::Point aOrigin( this->transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( 0.0, 0.0, fLogicZ+0.5 ) ) );
        basegfx::B2IVector aDirection( aRet.X- aOrigin.X, aRet.Y- aOrigin.Y );
        aDirection.setLength(nScreenValueOffsetInRadiusDirection);
        aRet.X += aDirection.getX();
        aRet.Y += aDirection.getY();
    }

    return aRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
