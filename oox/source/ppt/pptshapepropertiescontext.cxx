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

#include <oox/ppt/pptshapepropertiescontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/ppt/pptshape.hxx>

using namespace oox::core;
using namespace ::com::sun::star;

namespace oox::ppt {

// CT_Shape
PPTShapePropertiesContext::PPTShapePropertiesContext( ContextHandler2Helper const & rParent, ::oox::drawingml::Shape& rShape )
: ShapePropertiesContext( rParent, rShape )
{
}

ContextHandlerRef PPTShapePropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    PPTShape* pPPTShape = dynamic_cast<PPTShape*>(&mrShape);
    if (pPPTShape)
        pPPTShape->setHasNoninheritedShapeProperties();

    switch( aElementToken )
    {
        case A_TOKEN( xfrm ):
            mrShape.getShapeProperties().setProperty( PROP_IsPlaceholderDependent, false);
            return ShapePropertiesContext::onCreateContext( aElementToken, rAttribs );

        default:
            return ShapePropertiesContext::onCreateContext( aElementToken, rAttribs );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
