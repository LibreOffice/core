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

#include <drawingml/chart/typegroupmodel.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml::chart {

UpDownBarsModel::UpDownBarsModel() :
    mnGapWidth( 150 )
{
}

UpDownBarsModel::~UpDownBarsModel()
{
}

TypeGroupModel::TypeGroupModel( sal_Int32 nTypeId, bool bMSO2007Doc ) :
    mfSplitPos( 0.0 ),
    mnBarDir( XML_col ),
    mnBubbleScale( 100 ),
    mnFirstAngle( 0 ),
    mnGapDepth( 150 ),
    mnGapWidth( 150 ),
    mnGrouping( bMSO2007Doc ? XML_standard : XML_clustered ),
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
    mbBubble3d( !bMSO2007Doc ),
    mbShowMarker( !bMSO2007Doc ),
    mbShowNegBubbles( !bMSO2007Doc ),
    mbSmooth( !bMSO2007Doc ),
    mbVaryColors( !bMSO2007Doc ),
    mbWireframe( !bMSO2007Doc )
{
}

TypeGroupModel::~TypeGroupModel()
{
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
