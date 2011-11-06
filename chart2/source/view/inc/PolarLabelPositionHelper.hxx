/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CHART2_VIEW_POLARLABELPOSITIONHELPER_HXX
#define _CHART2_VIEW_POLARLABELPOSITIONHELPER_HXX

#include "LabelPositionHelper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/Position3D.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PolarPlottingPositionHelper;

class PolarLabelPositionHelper : public LabelPositionHelper
{
public:
    PolarLabelPositionHelper(
        PolarPlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~PolarLabelPositionHelper();

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignmentForLogicValues(
                        LabelAlignment& rAlignment
                        , double fLogicValueOnAngleAxis
                        , double fLogicValueOnRadiusAxis
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection=0 ) const;

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignmentForUnitCircleValues(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement /*see ::com::sun::star::chart::DataLabelPlacement*/
                        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection=0 ) const;

private:
    PolarPlottingPositionHelper*    m_pPosHelper;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
