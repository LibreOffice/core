/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seriesmodel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:51:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

