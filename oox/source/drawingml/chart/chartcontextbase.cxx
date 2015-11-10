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

#include "drawingml/chart/chartcontextbase.hxx"

#include "oox/drawingml/chart/modelbase.hxx"
#include "drawingml/shapepropertiescontext.hxx"

namespace oox {
namespace drawingml {
namespace chart {

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

ShapePrWrapperContext::ShapePrWrapperContext( ContextHandler2Helper& rParent, Shape& rModel ) :
    ContextBase< Shape >( rParent, rModel )
{
}

ShapePrWrapperContext::~ShapePrWrapperContext()
{
}

ContextHandlerRef ShapePrWrapperContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    return (isRootElement() && (nElement == C_TOKEN( spPr ))) ? new ShapePropertiesContext( *this, mrModel ) : nullptr;
}

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
                    return nullptr;
                case C_TOKEN( y ):
                    mrModel.mfY = rAttribs.getDouble( XML_val, 0.0 );
                    return nullptr;
                case C_TOKEN( w ):
                    mrModel.mfW = rAttribs.getDouble( XML_val, 0.0 );
                    return nullptr;
                case C_TOKEN( h ):
                    mrModel.mfH = rAttribs.getDouble( XML_val, 0.0 );
                    return nullptr;
                case C_TOKEN( xMode ):
                    mrModel.mnXMode = rAttribs.getToken( XML_val, XML_factor );
                    return nullptr;
                case C_TOKEN( yMode ):
                    mrModel.mnYMode = rAttribs.getToken( XML_val, XML_factor );
                    return nullptr;
                case C_TOKEN( wMode ):
                    mrModel.mnWMode = rAttribs.getToken( XML_val, XML_factor );
                    return nullptr;
                case C_TOKEN( hMode ):
                    mrModel.mnHMode = rAttribs.getToken( XML_val, XML_factor );
                    return nullptr;
                case C_TOKEN( layoutTarget ):
                    mrModel.mnTarget = rAttribs.getToken( XML_val, XML_outer );
                    return nullptr;
            }
        break;
    }
    return nullptr;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
