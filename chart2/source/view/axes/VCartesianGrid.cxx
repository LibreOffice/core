/*************************************************************************
 *
 *  $RCSfile: VCartesianGrid.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2004-01-17 13:09:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "VCartesianGrid.hxx"
#include "TickmarkHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <drafts/com/sun/star/chart2/XIdentifiable.hpp>
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
using namespace ::drafts::com::sun::star::chart2;

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

void addLine2D( drawing::PointSequenceSequence& rPoints, sal_Int32 nIndex
             , bool bIsYGrid , sal_Int32 nScreenTickValue
             , sal_Int32 nOrthogonalStart, sal_Int32 nOrthogonalEnd
              )
{
    rPoints[nIndex].realloc(2);
    rPoints[nIndex][0].X = bIsYGrid ? nOrthogonalStart : nScreenTickValue;
    rPoints[nIndex][0].Y = bIsYGrid ? nScreenTickValue : nOrthogonalStart;
    rPoints[nIndex][1].X = bIsYGrid ? nOrthogonalEnd   : nScreenTickValue;
    rPoints[nIndex][1].Y = bIsYGrid ? nScreenTickValue : nOrthogonalEnd;
}

struct GridLinePoints
{
    uno::Sequence< double > P0;
    uno::Sequence< double > P1;
    uno::Sequence< double > P2;

    GridLinePoints( const uno::Sequence< double >& rMinEdgeSeq, const uno::Sequence< double >& rMaxEdgeSeq );
    void update( double fScaledTickValue );
};

GridLinePoints::GridLinePoints( const uno::Sequence< double >& rMinEdgeSeq
                               , const uno::Sequence< double >& rMaxEdgeSeq )
{
    P0.realloc(3);
    P0[0]=rMinEdgeSeq[0];
    P0[1]=rMinEdgeSeq[1];
    P0[2]=rMaxEdgeSeq[2];

    P1.realloc(3);
    P1[0]=rMinEdgeSeq[0];
    P1[1]=rMaxEdgeSeq[1];
    P1[2]=rMinEdgeSeq[2];

    P2.realloc(3);
    P2[0]=rMaxEdgeSeq[0];
    P2[1]=rMaxEdgeSeq[1];
    P2[2]=rMinEdgeSeq[2];
}

void GridLinePoints::update( double fScaledTickValue )
{
    P0[1] = P1[1] = P2[1] = fScaledTickValue;
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
        uno::Sequence< double > aMinEdgeSeq, aMaxEdgeSeq;
        m_pPosHelper->getScreenValuesForMinimum( aMinEdgeSeq );
        m_pPosHelper->getScreenValuesForMaximum( aMaxEdgeSeq );
        sal_Int32 nOrthogonalDimensionIndex = nDimensionIndex==1 ? 0 : 1;
        sal_Int32 nOrthogonalScreenPositionStart=static_cast<sal_Int32>(aMinEdgeSeq[nOrthogonalDimensionIndex]);
        sal_Int32 nOrthogonalScreenPositionEnd  =static_cast<sal_Int32>(aMaxEdgeSeq[nOrthogonalDimensionIndex]);

        drawing::PointSequenceSequence aHandlesPoints(1);

        sal_Int32 nLinePropertiesCount = aLinePropertiesList.size();
        for( sal_Int32 nDepth=0
            ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
            ; aDepthIter++, nDepth++ )
        {
            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PointSequenceSequence aPoints(nPointCount);

            ::std::vector< TickInfo >::const_iterator       aTickIter = (*aDepthIter).begin();
            const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
            sal_Int32 nRealPointCount = 0;
            for( ; aTickIter != aTickEnd; aTickIter++ )
            {
                if( !(*aTickIter).bPaintIt )
                    continue;
                addLine2D( aPoints, nRealPointCount, 1==nDimensionIndex, (*aTickIter).nScreenTickValue
                    , nOrthogonalScreenPositionStart, nOrthogonalScreenPositionEnd );
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
            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PolyPolygonShape3D aPoints;
            aPoints.SequenceX.realloc(nPointCount);
            aPoints.SequenceY.realloc(nPointCount);
            aPoints.SequenceZ.realloc(nPointCount);
            uno::Sequence< double > aLogicMinEdge, aLogicMaxEdge;
            m_pPosHelper->getLogicMinimum( aLogicMinEdge );
            m_pPosHelper->getLogicMaximum( aLogicMaxEdge );

            if(!m_pPosHelper->isMathematicalOrientationX())
            {
                double fHelp = aLogicMinEdge[0];
                aLogicMinEdge[0] = aLogicMaxEdge[0];
                aLogicMaxEdge[0] = fHelp;
            }
            if(!m_pPosHelper->isMathematicalOrientationY())
            {
                double fHelp = aLogicMinEdge[1];
                aLogicMinEdge[1] = aLogicMaxEdge[1];
                aLogicMaxEdge[1] = fHelp;
            }
            if(m_pPosHelper->isMathematicalOrientationZ())//z axis in draw is reverse to mathematical
            {
                double fHelp = aLogicMinEdge[2];
                aLogicMinEdge[2] = aLogicMaxEdge[2];
                aLogicMaxEdge[2] = fHelp;
            }

            GridLinePoints aGridLinePoints( aLogicMinEdge, aLogicMaxEdge );

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
