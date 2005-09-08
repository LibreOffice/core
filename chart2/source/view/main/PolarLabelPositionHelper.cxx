/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolarLabelPositionHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:49:51 $
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

#include "PolarLabelPositionHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"

// header for class Vector2D
#ifndef _VECTOR2D_HXX
#include <tools/vector2d.hxx>
#endif

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

awt::Point PolarLabelPositionHelper::getLabelScreenPositionAndAlignment(
        LabelAlignment& rAlignment, bool bOutsidePosition
        , double fStartLogicValueOnAngleAxis, double fEndLogicValueOnAngleAxis
        , double fLogicInnerRadius, double fLogicOuterRadius
        , double fLogicZ) const
{
    double fWidthAngleDegree = m_pPosHelper->getWidthAngleDegree( fStartLogicValueOnAngleAxis, fEndLogicValueOnAngleAxis );
    double fStartAngleDegree = m_pPosHelper->transformToAngleDegree( fStartLogicValueOnAngleAxis );
    double fAngleDegree = fStartAngleDegree + fWidthAngleDegree/2.0;
    double fAnglePi     = fAngleDegree*F_PI/180.0;

    double fInnerRadius = m_pPosHelper->transformToRadius(fLogicInnerRadius);
    double fOuterRadius = m_pPosHelper->transformToRadius(fLogicOuterRadius);

    double fRadius = 0.0;
    if( bOutsidePosition ) //e.g. for pure pie chart(one ring only) or for angle axis of polyar coordinate system
    {
        fRadius = fOuterRadius;
        if(3!=m_nDimensionCount)
            fRadius += 0.1*fOuterRadius;
    }
    else
        fRadius = fInnerRadius + (fOuterRadius-fInnerRadius)/2.0 ;

    if(3==m_nDimensionCount)
        fAnglePi *= -1.0;
    drawing::Position3D aLogicPos(fRadius*cos(fAnglePi),fRadius*sin(fAnglePi),fLogicZ+0.5);
    awt::Point aRet( this->transformLogicToScreenPosition( aLogicPos ) );

    if(3==m_nDimensionCount)
    {
        //check wether the upper or the downer edge is more distant from the center
        //take the farest point to put the label to
        drawing::Position3D aLogicPos2(fRadius*cos(fAnglePi),fRadius*sin(fAnglePi),fLogicZ-0.5);
        drawing::Position3D aLogicCenter(0,0,fLogicZ);

        awt::Point aP0( this->transformLogicToScreenPosition(
                        drawing::Position3D(0,0,fLogicZ) ) );
        awt::Point aP1(aRet);
        awt::Point aP2( this->transformLogicToScreenPosition(
                        drawing::Position3D(fRadius*cos(fAnglePi),fRadius*sin(fAnglePi),fLogicZ-0.5) ) );

        Vector2D aV0( aP0.X, aP0.Y );
        Vector2D aV1( aP1.X, aP1.Y );
        Vector2D aV2( aP2.X, aP2.Y );

        double fL1 = (aV1-aV0).GetLength();
        double fL2 = (aV2-aV0).GetLength();

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
    if( bOutsidePosition )
    {
        while(fAngleDegree>360.0)
            fAngleDegree-=360.0;
        while(fAngleDegree<0.0)
            fAngleDegree+=360.0;

        if(fAngleDegree==0.0)
            rAlignment = LABEL_ALIGN_CENTER;
        else if(fAngleDegree<=22.5)
            rAlignment = LABEL_ALIGN_RIGHT;
        else if(fAngleDegree<67.5)
            rAlignment = LABEL_ALIGN_RIGHT_TOP;
        else if(fAngleDegree<112.5)
            rAlignment = LABEL_ALIGN_TOP;
        else if(fAngleDegree<=157.5)
            rAlignment = LABEL_ALIGN_LEFT_TOP;
        else if(fAngleDegree<=202.5)
            rAlignment = LABEL_ALIGN_LEFT;
        else if(fAngleDegree<247.5)
            rAlignment = LABEL_ALIGN_LEFT_BOTTOM;
        else if(fAngleDegree<292.5)
            rAlignment = LABEL_ALIGN_BOTTOM;
        else if(fAngleDegree<337.5)
            rAlignment = LABEL_ALIGN_RIGHT_BOTTOM;
        else
            rAlignment = LABEL_ALIGN_RIGHT;
    }
    else
    {
        rAlignment = LABEL_ALIGN_CENTER;
    }
    return aRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
