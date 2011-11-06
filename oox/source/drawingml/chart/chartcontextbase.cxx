/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/drawingml/chart/chartcontextbase.hxx"

#include "oox/drawingml/chart/modelbase.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

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
