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



#ifndef _CHART2_BARCHART_HXX
#define _CHART2_BARCHART_HXX

#include "VSeriesPlotter.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
class BarPositionHelper;

class BarChart : public VSeriesPlotter
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    BarChart( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount );
    virtual ~BarChart();

    virtual void createShapes();
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    //-------------------
    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;
    virtual bool keepAspectRatio() const;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

private: //methods
    //no default constructor
    BarChart();

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createDataPoint3D_Bar(
                          const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
                        , const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet >& xObjectProperties
                        , sal_Int32 nGeometry3D );

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignment(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement
                        , double fScaledX, double fScaledLowerYValue, double fScaledUpperYValue, double fScaledZ
                        , double fScaledLowerBarDepth, double fScaledUpperBarDepth, double fBaseValue
                        , BarPositionHelper* pPosHelper ) const;

    virtual PlottingPositionHelper& getPlottingPositionHelper( sal_Int32 nAxisIndex ) const;//nAxisIndex indicates whether the position belongs to the main axis ( nAxisIndex==0 ) or secondary axis ( nAxisIndex==1 )

    void adaptOverlapAndGapwidthForGroupBarsPerAxis();

private: //member
    BarPositionHelper*                   m_pMainPosHelper;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_aOverlapSequence;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_aGapwidthSequence;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
