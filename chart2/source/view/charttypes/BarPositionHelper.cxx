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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "BarPositionHelper.hxx"
#include "Linear3DTransformation.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"
#include "DateHelper.hxx"
#include <com/sun/star/chart/TimeUnit.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

BarPositionHelper::BarPositionHelper( bool /* bSwapXAndY */ )
        : CategoryPositionHelper( 1 )
{
    AllowShiftXAxisPos(true);
    AllowShiftZAxisPos(true);
}

BarPositionHelper::BarPositionHelper( const BarPositionHelper& rSource )
        : CategoryPositionHelper( rSource )
        , PlottingPositionHelper( rSource )
{
}

BarPositionHelper::~BarPositionHelper()
{
}

PlottingPositionHelper* BarPositionHelper::clone() const
{
    BarPositionHelper* pRet = new BarPositionHelper(*this);
    return pRet;
}

void BarPositionHelper::updateSeriesCount( double fSeriesCount )
{
    m_fSeriesCount = fSeriesCount;
}

double BarPositionHelper::getScaledSlotPos( double fUnscaledLogicX, double fSeriesNumber ) const
{
    if( m_bDateAxis )
        fUnscaledLogicX = DateHelper::RasterizeDateValue( fUnscaledLogicX, m_aNullDate, m_nTimeResolution );
    double fScaledLogicX(fUnscaledLogicX);
    doLogicScaling(&fScaledLogicX,NULL,NULL);
    fScaledLogicX = CategoryPositionHelper::getScaledSlotPos( fScaledLogicX, fSeriesNumber );
    return fScaledLogicX;

}

void BarPositionHelper::setScaledCategoryWidth( double fScaledCategoryWidth )
{
    m_fScaledCategoryWidth = fScaledCategoryWidth;
    CategoryPositionHelper::setCategoryWidth( m_fScaledCategoryWidth );
}
//.............................................................................
} //namespace chart
//.............................................................................
