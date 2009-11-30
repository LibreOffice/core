/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartcontextbase.cxx,v $
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

#include "oox/drawingml/chart/chartcontextbase.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/chart/modelbase.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

ShapePrWrapperContext::ShapePrWrapperContext( ContextHandler2Helper& rParent, Shape& rModel ) :
    ContextBase< Shape >( rParent, rModel )
{
}

ShapePrWrapperContext::~ShapePrWrapperContext()
{
}

ContextHandlerRef ShapePrWrapperContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    return (isRootElement() && (nElement == C_TOKEN( spPr ))) ? new ShapePropertiesContext( *this, mrModel ) : 0;
}

// ============================================================================

LayoutContext::LayoutContext( ContextHandler2Helper& rParent, LayoutModel& rModel ) :
    ContextBase< LayoutModel >( rParent, rModel )
{
}

LayoutContext::~LayoutContext()
{
}

ContextHandlerRef LayoutContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( layout ):
            switch( nElement )
            {
                case C_TOKEN( manualLayout ):
                    mrModel.mbAutoLayout = false;
                    return this;
            }
        break;

        case C_TOKEN( manualLayout ):
            switch( nElement )
            {
                case C_TOKEN( x ):
                    mrModel.mfX = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( y ):
                    mrModel.mfY = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( w ):
                    mrModel.mfW = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( h ):
                    mrModel.mfH = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( xMode ):
                    mrModel.mnXMode = rAttribs.getToken( XML_val, XML_factor );
                    return 0;
                case C_TOKEN( yMode ):
                    mrModel.mnYMode = rAttribs.getToken( XML_val, XML_factor );
                    return 0;
                case C_TOKEN( wMode ):
                    mrModel.mnWMode = rAttribs.getToken( XML_val, XML_factor );
                    return 0;
                case C_TOKEN( hMode ):
                    mrModel.mnHMode = rAttribs.getToken( XML_val, XML_factor );
                    return 0;
                case C_TOKEN( layoutTarget ):
                    mrModel.mnTarget = rAttribs.getToken( XML_val, XML_outer );
                    return 0;
            }
        break;
    }
    return 0;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

