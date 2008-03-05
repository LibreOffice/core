/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seriesmodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:36:04 $
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

#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/seriessourcemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

SeriesData::SeriesData() :
    mnIndex( -1 ),
    mnOrder( -1 )
{
}

// ----------------------------------------------------------------------------

SeriesModel::SeriesModel()
{
}

SeriesModel::~SeriesModel()
{
}

SeriesSourceModel& SeriesModel::createTitleSource()
{
    mxTitleSource.reset( new SeriesSourceModel );
    return *mxTitleSource;
}

SeriesSourceModel& SeriesModel::createCategorySource()
{
    mxCategSource.reset( new SeriesSourceModel );
    return *mxCategSource;
}

SeriesSourceModel& SeriesModel::createValueSource()
{
    mxValueSource.reset( new SeriesSourceModel );
    return *mxValueSource;
}

SeriesSourceModel& SeriesModel::createPointSource()
{
    mxPointSource.reset( new SeriesSourceModel );
    return *mxPointSource;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

