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



#include "oox/drawingml/chart/seriesmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

DataLabelModelBase::DataLabelModelBase() :
    mbDeleted( false )
{
}

DataLabelModelBase::~DataLabelModelBase()
{
}

// ============================================================================

DataLabelModel::DataLabelModel() :
    mnIndex( -1 )
{
}

DataLabelModel::~DataLabelModel()
{
}

// ============================================================================

DataLabelsModel::DataLabelsModel() :
    mbShowLeaderLines( false )
{
}

DataLabelsModel::~DataLabelsModel()
{
}

// ============================================================================

PictureOptionsModel::PictureOptionsModel() :
    mfStackUnit( 1.0 ),
    mnPictureFormat( XML_stretch ),
    mbApplyToFront( false ),
    mbApplyToSides( false ),
    mbApplyToEnd( false )
{
}

PictureOptionsModel::~PictureOptionsModel()
{
}

// ============================================================================

ErrorBarModel::ErrorBarModel() :
    mfValue( 0.0 ),
    mnDirection( XML_y ),
    mnTypeId( XML_both ),
    mnValueType( XML_fixedVal ),
    mbNoEndCap( false )
{
}

ErrorBarModel::~ErrorBarModel()
{
}

// ============================================================================

TrendlineLabelModel::TrendlineLabelModel()
{
}

TrendlineLabelModel::~TrendlineLabelModel()
{
}

// ============================================================================

TrendlineModel::TrendlineModel() :
    mnOrder( 2 ),
    mnPeriod( 2 ),
    mnTypeId( XML_linear ),
    mbDispEquation( false ),
    mbDispRSquared( false )
{
}

TrendlineModel::~TrendlineModel()
{
}

// ============================================================================

DataPointModel::DataPointModel() :
    mnIndex( -1 ),
    mbInvertNeg( false )
{
}

DataPointModel::~DataPointModel()
{
}

// ============================================================================

SeriesModel::SeriesModel() :
    mnExplosion( 0 ),
    mnIndex( -1 ),
    mnMarkerSize( 5 ),
    mnMarkerSymbol( XML_auto ),
    mnOrder( -1 ),
    mbBubble3d( false ),
    mbInvertNeg( false ),
    mbSmooth( false )
{
}

SeriesModel::~SeriesModel()
{
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox
