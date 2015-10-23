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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_AXISCONVERTER_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_AXISCONVERTER_HXX

#include <drawingml/chart/converterbase.hxx>

namespace com { namespace sun { namespace star {
    namespace chart2 { class XAxis; }
    namespace chart2 { class XCoordinateSystem; }
} } }

namespace oox {
namespace drawingml {
namespace chart {



struct AxisDispUnitsModel;

class AxisDispUnitsConverter : public ConverterBase< AxisDispUnitsModel >
{
public:
    explicit            AxisDispUnitsConverter(
                            const ConverterRoot& rParent,
                            AxisDispUnitsModel& rModel );
    virtual             ~AxisDispUnitsConverter();

    /** Creates a chart2 axis and inserts it into the passed coordinate system. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XAxis >& rxAxis);
};

struct AxisModel;
class TypeGroupConverter;

class AxisConverter : public ConverterBase< AxisModel >
{
public:
    explicit            AxisConverter(
                            const ConverterRoot& rParent,
                            AxisModel& rModel );
    virtual             ~AxisConverter();

    /** Creates a chart2 axis and inserts it into the passed coordinate system. */
    void convertFromModel(
        const css::uno::Reference<css::chart2::XCoordinateSystem>& rxCoordSystem,
        RefVector<TypeGroupConverter>& rTypeGroups, const AxisModel* pCrossingAxis,
        sal_Int32 nAxesSetIdx, sal_Int32 nAxisIdx );
};



} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
