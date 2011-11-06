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



#include "oox/drawingml/chart/axismodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

AxisDispUnitsModel::AxisDispUnitsModel() :
    mfCustomUnit( 0.0 ),
    mnBuiltInUnit( XML_TOKEN_INVALID )
{
}

AxisDispUnitsModel::~AxisDispUnitsModel()
{
}

// ============================================================================

AxisModel::AxisModel( sal_Int32 nTypeId ) :
    mnAxisId( -1 ),
    mnAxisPos( XML_TOKEN_INVALID ),
    mnCrossAxisId( -1 ),
    mnCrossBetween( XML_between ),
    mnCrossMode( XML_autoZero ),
    mnLabelAlign( XML_ctr ),
    mnLabelOffset( 100 ),
    mnMajorTickMark( XML_out ),
    mnMajorTimeUnit( XML_days ),
    mnMinorTickMark( XML_none ),
    mnMinorTimeUnit( XML_days ),
    mnOrientation( XML_minMax ),
    mnTickLabelPos( XML_nextTo ),
    mnTickLabelSkip( 0 ),
    mnTickMarkSkip( 0 ),
    mnTypeId( nTypeId ),
    mbAuto( false ),
    mbDeleted( false ),
    mbNoMultiLevel( false )
{
}

AxisModel::~AxisModel()
{
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox
