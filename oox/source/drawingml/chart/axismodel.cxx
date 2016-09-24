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

#include "drawingml/chart/axismodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

AxisDispUnitsModel::AxisDispUnitsModel() :
    mfCustomUnit( 0.0 )
{
}

AxisDispUnitsModel::~AxisDispUnitsModel()
{
}

AxisModel::AxisModel( sal_Int32 nTypeId, bool bMSO2007Doc ) :
    mnAxisId( -1 ),
    mnAxisPos( XML_TOKEN_INVALID ),
    mnCrossAxisId( -1 ),
    mnCrossBetween( XML_between ),
    mnCrossMode( XML_autoZero ),
    mnLabelAlign( XML_ctr ),
    mnLabelOffset( 100 ),
    mnMajorTickMark( bMSO2007Doc ? XML_out : XML_cross ),
    mnMajorTimeUnit( XML_days ),
    mnMinorTickMark( bMSO2007Doc ? XML_none : XML_cross ),
    mnMinorTimeUnit( XML_days ),
    mnOrientation( XML_minMax ),
    mnTickLabelPos( XML_nextTo ),
    mnTickLabelSkip( 0 ),
    mnTickMarkSkip( 0 ),
    mnTypeId( nTypeId ),
    mbAuto( false ),
    mbDeleted( !bMSO2007Doc ),
    mbNoMultiLevel( false )
{
}

AxisModel::~AxisModel()
{
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
