/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriesmodel.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_DRAWINGML_CHART_SERIESMODEL_HXX
#define OOX_DRAWINGML_CHART_SERIESMODEL_HXX

#include "oox/drawingml/chart/modelbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct SeriesData
{
    sal_Int32           mnIndex;            /// Unique series index.
    sal_Int32           mnOrder;            /// Series order used for automatic formatting.

    explicit            SeriesData();
};

// ----------------------------------------------------------------------------

class SeriesSourceModel;

class SeriesModel : public ModelData< SeriesData >
{
public:
    explicit            SeriesModel();
    virtual             ~SeriesModel();

    /** Creates and returns a new source data model object for the series title. */
    SeriesSourceModel&  createTitleSource();
    /** Creates and returns a new source data model object for the category values. */
    SeriesSourceModel&  createCategorySource();
    /** Creates and returns a new source data model object for the series values. */
    SeriesSourceModel&  createValueSource();
    /** Creates and returns a new source data model object for the point values (e.g. bubble sizes). */
    SeriesSourceModel&  createPointSource();

private:
    typedef ::boost::shared_ptr< SeriesSourceModel > SeriesSourceRef;

    SeriesSourceRef     mxTitleSource;
    SeriesSourceRef     mxCategSource;
    SeriesSourceRef     mxValueSource;
    SeriesSourceRef     mxPointSource;
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

