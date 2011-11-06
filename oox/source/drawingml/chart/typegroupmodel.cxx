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



#include "oox/drawingml/chart/typegroupmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

UpDownBarsModel::UpDownBarsModel() :
    mnGapWidth( 150 )
{
}

UpDownBarsModel::~UpDownBarsModel()
{
}

// ============================================================================

TypeGroupModel::TypeGroupModel( sal_Int32 nTypeId ) :
    mfSplitPos( 0.0 ),
    mnBarDir( XML_col ),
    mnBubbleScale( 100 ),
    mnFirstAngle( 0 ),
    mnGapDepth( 150 ),
    mnGapWidth( 150 ),
    mnGrouping( XML_standard ),
    mnHoleSize( 10 ),
    mnOfPieType( XML_pie ),
    mnOverlap( 0 ),
    mnRadarStyle( XML_standard ),
    mnScatterStyle( XML_marker ),
    mnSecondPieSize( 75 ),
    mnShape( XML_box ),
    mnSizeRepresents( XML_area ),
    mnSplitType( XML_auto ),
    mnTypeId( nTypeId ),
    mbBubble3d( false ),
    mbShowMarker( false ),
    mbShowNegBubbles( false ),
    mbSmooth( false ),
    mbVaryColors( false ),
    mbWireframe( false )
{
}

TypeGroupModel::~TypeGroupModel()
{
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox
