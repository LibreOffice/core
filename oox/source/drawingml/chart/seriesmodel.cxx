/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "oox/drawingml/chart/seriesmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {



DataLabelModelBase::DataLabelModelBase() :
    mbDeleted( false )
{
}

DataLabelModelBase::~DataLabelModelBase()
{
}



DataLabelModel::DataLabelModel() :
    mnIndex( -1 )
{
}

DataLabelModel::~DataLabelModel()
{
}



DataLabelsModel::DataLabelsModel() :
    mbShowLeaderLines( false )
{
}

DataLabelsModel::~DataLabelsModel()
{
}



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



TrendlineLabelModel::TrendlineLabelModel()
{
}

TrendlineLabelModel::~TrendlineLabelModel()
{
}



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



DataPointModel::DataPointModel() :
    mnIndex( -1 ),
    mbInvertNeg( false )
{
}

DataPointModel::~DataPointModel()
{
}



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



} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
