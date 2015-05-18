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

#include "drawingml/chart/seriesmodel.hxx"
#include <oox/token/tokens.hxx>

namespace oox {
namespace drawingml {
namespace chart {

DataLabelModelBase::DataLabelModelBase(bool bMSO2007Doc) :
    mbDeleted( !bMSO2007Doc )
{
}

DataLabelModelBase::~DataLabelModelBase()
{
}

DataLabelModel::DataLabelModel(bool bMSO2007Doc) :
    DataLabelModelBase(bMSO2007Doc),
    mnIndex( -1 )
{
}

DataLabelModel::~DataLabelModel()
{
}

DataLabelsModel::DataLabelsModel(bool bMSO2007Doc) :
    DataLabelModelBase(bMSO2007Doc),
    mbShowLeaderLines( !bMSO2007Doc )
{
}

DataLabelsModel::~DataLabelsModel()
{
}

PictureOptionsModel::PictureOptionsModel(bool bMSO2007Doc) :
    mfStackUnit( 1.0 ),
    mnPictureFormat( XML_stretch ),
    mbApplyToFront( !bMSO2007Doc ),
    mbApplyToSides( !bMSO2007Doc ),
    mbApplyToEnd( !bMSO2007Doc )
{
}

PictureOptionsModel::~PictureOptionsModel()
{
}

ErrorBarModel::ErrorBarModel(bool bMSO2007Doc) :
    mfValue( 0.0 ),
    mnDirection( XML_y ),
    mnTypeId( XML_both ),
    mnValueType( XML_fixedVal ),
    mbNoEndCap( !bMSO2007Doc )
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

TrendlineModel::TrendlineModel(bool bMSO2007Doc) :
    mnOrder( 2 ),
    mnPeriod( 2 ),
    mnTypeId( XML_linear ),
    mbDispEquation( !bMSO2007Doc ),
    mbDispRSquared( !bMSO2007Doc )
{
}

TrendlineModel::~TrendlineModel()
{
}

DataPointModel::DataPointModel(bool bMSO2007Doc) :
    mnIndex( -1 ),
    mbInvertNeg( !bMSO2007Doc )
{
}

DataPointModel::~DataPointModel()
{
}

SeriesModel::SeriesModel(bool bMSO2007Doc) :
    mnExplosion( 0 ),
    mnIndex( -1 ),
    mnMarkerSize( 5 ),
    mnMarkerSymbol( XML_auto ),
    mnOrder( -1 ),
    mbBubble3d( !bMSO2007Doc ),
    mbInvertNeg( !bMSO2007Doc ),
    mbSmooth( !bMSO2007Doc )
{
}

SeriesModel::~SeriesModel()
{
}

}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
