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



#ifndef _CHART2_VIEW_LABELPOSITIONHELPER_HXX
#define _CHART2_VIEW_LABELPOSITIONHELPER_HXX

#include "LabelAlignment.hxx"
#include "PropertyMapper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PlottingPositionHelper;
class ShapeFactory;

class LabelPositionHelper
{
public:
    LabelPositionHelper(
        PlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~LabelPositionHelper();

    ::com::sun::star::awt::Point transformSceneToScreenPosition(
            const ::com::sun::star::drawing::Position3D& rScenePosition3D ) const;

    static void changeTextAdjustment( tAnySequence& rPropValues, const tNameSequence& rPropNames, LabelAlignment eAlignment);
    static void doDynamicFontResize(  tAnySequence& rPropValues, const tNameSequence& rPropNames
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisModelProps
                    , const ::com::sun::star::awt::Size& rNewReferenceSize );

    static void correctPositionForRotation( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape2DText
                    , LabelAlignment eLabelAlignment, const double fRotationAngle, bool bRotateAroundCenter );

private:
    LabelPositionHelper();

protected:
    PlottingPositionHelper*  m_pPosHelper;
    sal_Int32                m_nDimensionCount;

private:
    //these members are only necessary for transformation from 3D to 2D
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >    m_xLogicTarget;
    ShapeFactory*                                           m_pShapeFactory;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
