/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seriescontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:50:49 $
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

#ifndef OOX_DRAWINGML_CHART_SERIESCONTEXT_HXX
#define OOX_DRAWINGML_CHART_SERIESCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

class SeriesModel;

/** Handler for a data series context (c:ser element).
 */
class SeriesContext : public ChartContextBase< SeriesModel >
{
public:
    explicit            SeriesContext(
                            ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel,
                            sal_Int32 nTitleSrcId, sal_Int32 nCategSrcId,
                            sal_Int32 nValueSrcId, sal_Int32 nPointSrcId = XML_TOKEN_INVALID );
    virtual             ~SeriesContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    sal_Int32           mnTitleSrcId;       /// XML element identifier for title source values.
    sal_Int32           mnCategSrcId;       /// XML element identifier for category source values.
    sal_Int32           mnValueSrcId;       /// XML element identifier for series source values.
    sal_Int32           mnPointSrcId;       /// XML element identifier for point source values (e.g. bubble sizes).
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

