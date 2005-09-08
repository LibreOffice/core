/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCartesianGrid.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:38:56 $
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
#include "VCartesianGrid.hxx"
#include "TickmarkHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"

#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#include <vector>
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

struct GridLinePoints
{
    uno::Sequence< double > P0;
    uno::Sequence< double > P1;
    uno::Sequence< double > P2;

    GridLinePoints( const PlottingPositionHelper* pPosHelper, sal_Int32 nDimensionIndex );
    void update( double fScaledTickValue );

    sal_Int32 m_nDimensionIndex;
};

GridLinePoints::GridLinePoints( const PlottingPositionHelper* pPosHelper, sal_Int32 nDimensionIndex )
                : m_nDimensionIndex(nDimensionIndex)
{
    double MinX = pPosHelper->getLogicMinX();
    double MinY = pPosHelper->getLogicMinY();
    double MinZ = pPosHelper->getLogicMinZ();
    double MaxX = pPosHelper->getLogicMaxX();
    double MaxY = pPosHelper->getLogicMaxY();
    double MaxZ = pPosHelper->getLogicMaxZ();

    pPosHelper->doLogicScaling( &MinX,&MinY,&MinZ );
    pPosHelper->doLogicScaling( &MaxX,&MaxY,&MaxZ );

    if(!pPosHelper->isMathematicalOrientationX())
    {
        double fHelp = MinX;
        MinX = MaxX;
        MaxX = fHelp;
    }
    if(!pPosHelper->isMathematicalOrientationY())
    {
        double fHelp = MinY;
        MinY = MaxY;
        MaxY = fHelp;
    }
    if(pPosHelper->isMathematicalOrientationZ())//z axis in draw is reverse to mathematical
    {
        double fHelp = MinZ;
        MinZ = MaxZ;
        MaxZ = fHelp;
    }

    P0.realloc(3);
    P1.realloc(3);
    P2.realloc(3);

    P0[0]=P1[0]=P2[0]=MinX;
    P0[1]=P1[1]=P2[1]=MinY;
    P0[2]=P1[2]=P2[2]=MinZ;

    if(m_nDimensionIndex==0)
    {
        P0[1]=MaxY;
        P2[2]=MaxZ;
    }
    else if(m_nDimensionIndex==1)
    {
        P0[0]=MaxX;
        P2[2]=MaxZ;
    }
    else if(m_nDimensionIndex==2)
    {
        P0[0]=MaxX;
        P2[1]=MaxY;
    }
}

void GridLinePoints::update( double fScaledTickValue )
{
    P0[m_nDimensionIndex] = P1[m_nDimensionIndex] = P2[m_nDimensionIndex] = fScaledTickValue;
}

void addLine2D( drawing::PointSequenceSequence& rPoints, sal_Int32 nIndex
             , const GridLinePoints& rScaledLogicPoints
             , const uno::Reference< XTransformation > & xTransformation
              )
{
    drawing::Position3D aPA = SequenceToPosition3D( xTransformation->transform( rScaledLogicPoints.P0 ) );
    drawing::Position3D aPB = SequenceToPosition3D( xTransformation->transform( rScaledLogicPoints.P1 ) );

    rPoints[nIndex].realloc(2);
    rPoints[nIndex][0].X = static_cast<sal_Int32>(aPA.PositionX);
    rPoints[nIndex][0].Y = static_cast<sal_Int32>(aPA.PositionY);
    rPoints[nIndex][1].X = static_cast<sal_Int32>(aPB.PositionX);
    rPoints[nIndex][1].Y = static_cast<sal_Int32>(aPB.PositionY);
}

void addLine3D( drawing::PolyPolygonShape3D& rPoints, sal_Int32 nIndex
            , const GridLinePoints& rBasePoints
            , const uno::Reference< XTransformation > & xTransformation )
{
    drawing::Position3D aPoint = SequenceToPosition3D( xTransformation->transform( rBasePoints.P0 ) );
    AddPointToPoly( rPoints, aPoint, nIndex );
    aPoint = SequenceToPosition3D( xTransformation->transform( rBasePoints.P1 ) );
    AddPointToPoly( rPoints, aPoint, nIndex );
    aPoint = SequenceToPosition3D( xTransformation->transform( rBasePoints.P2 ) );
    AddPointToPoly( rPoints, aPoint, nIndex );
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

VCartesianGrid::VCartesianGrid( const uno::Reference< XGrid >& xGrid, sal_Int32 nDimensionCount )
            : VMeterBase( uno::Reference<XMeter>::query(xGrid), nDimensionCount )
{
    m_pPosHelper = new PlottingPositionHelper();
}

VCartesianGrid::~VCartesianGrid()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

//static
void VCartesianGrid::fillLinePropertiesFromGridModel( ::std::vector<VLineProperties>& rLinePropertiesList
                                     , uno::Reference< beans::XPropertySet > xProps )
{
    rLinePropertiesList.clear();
    VLineProperties aLineProperties;
    aLineProperties.initFromPropertySet( xProps );
    rLinePropertiesList.assign(2,aLineProperties);
};

void SAL_CALL VCartesianGrid::createShapes()
{
    if( !m_xMeter.is())
        return;
    uno::Reference< beans::XPropertySet > xGridProps( m_xMeter, uno::UNO_QUERY );
    if(!xGridProps.is())
        return;
    //somehow equal to axis tickmarks

    //-----------------------------------------
    //create named group shape
    uno::Reference< XIdentifiable > xIdent( m_xMeter, uno::UNO_QUERY );
    if( ! xIdent.is())
        return;

    uno::Reference< drawing::XShapes > xGroupShape_Shapes(
        this->createGroupShape( m_xLogicTarget
        , ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_GRID, xIdent->getIdentifier() )
        ) );

    //-----------------------------------------

    sal_Int32 nDimensionIndex = m_xMeter->getRepresentedDimension();
    ::std::vector<VLineProperties> aLinePropertiesList;
    fillLinePropertiesFromGridModel( aLinePropertiesList, xGridProps );

    //-----------------------------------------
    //create all scaled tickmark values
    std::auto_ptr< TickmarkHelper > apTickmarkHelper( this->createTickmarkHelper() );
    TickmarkHelper& aTickmarkHelper = *apTickmarkHelper.get();
    ::std::vector< ::std::vector< TickInfo > > aAllTickInfos;
    aTickmarkHelper.getAllTicks( aAllTickInfos );

    //-----------------------------------------
    //create tick mark line shapes
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = aAllTickInfos.begin();
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = aAllTickInfos.end();

    if(aDepthIter == aDepthEnd)//no tickmarks at all
        return;

    if(2==m_nDimension)
    {
        drawing::PointSequenceSequence aHandlesPoints(1);

        sal_Int32 nLinePropertiesCount = aLinePropertiesList.size();
        for( sal_Int32 nDepth=0
            ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
            ; aDepthIter++, nDepth++ )
        {
            GridLinePoints aGridLinePoints( m_pPosHelper, nDimensionIndex );

            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PointSequenceSequence aPoints(nPointCount);

            ::std::vector< TickInfo >::const_iterator       aTickIter = (*aDepthIter).begin();
            const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
            sal_Int32 nRealPointCount = 0;
            for( ; aTickIter != aTickEnd; aTickIter++ )
            {
                if( !(*aTickIter).bPaintIt )
                    continue;
                aGridLinePoints.update( (*aTickIter).fScaledTickValue );
                addLine2D( aPoints, nRealPointCount, aGridLinePoints, m_pPosHelper->getTransformationLogicToScene() );
                nRealPointCount++;
            }
            aPoints.realloc(nRealPointCount);
            m_pShapeFactory->createLine2D( xGroupShape_Shapes, aPoints, aLinePropertiesList[nDepth] );

            //prepare polygon for handle shape:
            sal_Int32 nOldHandleCount = aHandlesPoints[0].getLength();
            aHandlesPoints[0].realloc(nOldHandleCount+nRealPointCount);
            for( sal_Int32 nN = 0; nN<nRealPointCount; nN++)
                aHandlesPoints[0][nOldHandleCount+nN] = aPoints[nN][1];
        }

        //create handle shape:
        VLineProperties aHandleLineProperties;
        aHandleLineProperties.LineStyle    = uno::makeAny( drawing::LineStyle_NONE );
        uno::Reference< drawing::XShape > xHandleShape =
            m_pShapeFactory->createLine2D( xGroupShape_Shapes, aHandlesPoints, aHandleLineProperties );
        m_pShapeFactory->setShapeName( xHandleShape, C2U("HandlesOnly") );
    }
    //-----------------------------------------
    else //if(2!=m_nDimension)
    {
        sal_Int32 nLinePropertiesCount = aLinePropertiesList.size();
        for( sal_Int32 nDepth=0
            ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
            ; aDepthIter++, nDepth++ )
        {
            GridLinePoints aGridLinePoints( m_pPosHelper, nDimensionIndex );

            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PolyPolygonShape3D aPoints;
            aPoints.SequenceX.realloc(nPointCount);
            aPoints.SequenceY.realloc(nPointCount);
            aPoints.SequenceZ.realloc(nPointCount);

            ::std::vector< TickInfo >::const_iterator       aTickIter = (*aDepthIter).begin();
            const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
            sal_Int32 nRealPointCount = 0;
            for( ; aTickIter != aTickEnd; aTickIter++ )
            {
                if( !(*aTickIter).bPaintIt )
                    continue;

                aGridLinePoints.update( (*aTickIter).fScaledTickValue );
                addLine3D( aPoints, nRealPointCount, aGridLinePoints, m_pPosHelper->getTransformationLogicToScene() );
                nRealPointCount+=3;
            }
            aPoints.SequenceX.realloc(nRealPointCount);
            aPoints.SequenceY.realloc(nRealPointCount);
            aPoints.SequenceZ.realloc(nRealPointCount);
            m_pShapeFactory->createLine3D( xGroupShape_Shapes, aPoints, aLinePropertiesList[nDepth] );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
