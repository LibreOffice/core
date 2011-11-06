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



#ifndef _CHART2_BARPOSITIONHELPER_HXX
#define _CHART2_BARPOSITIONHELPER_HXX

#include "PlottingPositionHelper.hxx"
#include "CategoryPositionHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class BarPositionHelper : public CategoryPositionHelper, public PlottingPositionHelper
{
public:
    BarPositionHelper( bool bSwapXAndY=true );
    BarPositionHelper( const BarPositionHelper& rSource );
    virtual ~BarPositionHelper();

    virtual PlottingPositionHelper* clone() const;

    void                updateSeriesCount( double fSeriesCount ); /*only enter the size of x stacked series*/

    virtual double getScaledSlotPos( double fCategoryX, double fSeriesNumber ) const;
    virtual void setScaledCategoryWidth( double fScaledCategoryWidth );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
