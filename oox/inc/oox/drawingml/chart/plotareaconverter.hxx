/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: plotareaconverter.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef OOX_DRAWINGML_CHART_PLOTAREACONVERTER_HXX
#define OOX_DRAWINGML_CHART_PLOTAREACONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XDiagram; }
} } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct View3DModel;
class TypeGroupConverter;

class View3DConverter : public ConverterBase< View3DModel >
{
public:
    explicit            View3DConverter( const ConverterRoot& rParent, View3DModel& rModel );
    virtual             ~View3DConverter();

    /** Converts the OOXML plot area model to a chart2 diagram. */
    void                convertModelToDocument(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& rxDiagram,
                            TypeGroupConverter& rTypeGroup );
};

// ============================================================================

struct PlotAreaModel;

class PlotAreaConverter : public ConverterBase< PlotAreaModel >
{
public:
    explicit            PlotAreaConverter( const ConverterRoot& rParent, PlotAreaModel& rModel );
    virtual             ~PlotAreaConverter();

    /** Converts the OOXML plot area model to a chart2 diagram. Returns an
        automatic chart title from a single series title, if possible. */
    ::rtl::OUString     convertModelToDocument( View3DModel& rView3DModel );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

