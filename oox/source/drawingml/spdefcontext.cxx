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

#include "oox/drawingml/spdefcontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textbodypropertiescontext.hxx"
#include "oox/drawingml/textliststylecontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

spDefContext::spDefContext( ContextHandler2Helper& rParent, Shape& rDefaultObject )
: ContextHandler2( rParent )
, mrDefaultObject( rDefaultObject )
{
}

ContextHandlerRef spDefContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrDefaultObject );
        case A_TOKEN( bodyPr ):
        {
            TextBodyPtr xTextBody( new TextBody );
            mrDefaultObject.setTextBody( xTextBody );
            return new TextBodyPropertiesContext( *this, rAttribs, xTextBody->getTextProperties() );
        }
        case A_TOKEN( lstStyle ):
            return new TextListStyleContext( *this, *mrDefaultObject.getMasterTextListStyle() );
        case A_TOKEN( style ):
            break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
