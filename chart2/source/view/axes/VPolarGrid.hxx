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


#ifndef _CHART2_VPOLARGRID_HXX
#define _CHART2_VPOLARGRID_HXX

#include "VAxisOrGridBase.hxx"
#include "Tickmarks.hxx"
#include "VLineProperties.hxx"
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PolarPlottingPositionHelper;

class VPolarGrid : public VAxisOrGridBase
{
//-------------------------------------------------------------------------
// public methods
//-------------------------------------------------------------------------
public:
    VPolarGrid( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > > & rGridPropertiesList //main grid, subgrid, subsubgrid etc
        );
    virtual ~VPolarGrid();

    virtual void createShapes();

    void setIncrements( const std::vector< ExplicitIncrementData >& rIncrements );

    static void createLinePointSequence_ForAngleAxis(
                    ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                    , const ExplicitIncrementData& rIncrement
                    , const ExplicitScaleData& rScale
                    , PolarPlottingPositionHelper* pPosHelper
                    , double fLogicRadius, double fLogicZ );

private: //member
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > > m_aGridPropertiesList;//main grid, subgrid, subsubgrid etc
    PolarPlottingPositionHelper* m_pPosHelper;
    ::std::vector< ExplicitIncrementData >   m_aIncrements;

    void    getAllTickInfos( sal_Int32 nDimensionIndex, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    void    create2DRadiusGrid( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
                    , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
                    , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
                    , const ::std::vector<VLineProperties>& rLinePropertiesList );
#if NOTYET
    void    create2DAngleGrid( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
                    , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
                    , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
                    , const ::std::vector<VLineProperties>& rLinePropertiesList );
#endif
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
