/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriescontext.hxx,v $
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

