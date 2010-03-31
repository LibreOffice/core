/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_CHART_AXISCONVERTER_HXX
#define OOX_DRAWINGML_CHART_AXISCONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XAxis; }
    namespace chart2 { class XCoordinateSystem; }
} } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

const sal_Int32 API_PRIM_AXESSET = 0;
const sal_Int32 API_SECN_AXESSET = 1;

const sal_Int32 API_X_AXIS = 0;
const sal_Int32 API_Y_AXIS = 1;
const sal_Int32 API_Z_AXIS = 2;

// ============================================================================

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
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >& rxCoordSystem,
                            TypeGroupConverter& rTypeGroup,
                            const AxisModel* pCrossingAxis,
                            sal_Int32 nAxesSetIdx,
                            sal_Int32 nAxisIdx );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

