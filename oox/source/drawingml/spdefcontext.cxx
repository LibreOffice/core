/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "oox/drawingml/spdefcontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textbodypropertiescontext.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

spDefContext::spDefContext( ContextHandler& rParent, Shape& rDefaultObject )
: ContextHandler( rParent )
, mrDefaultObject( rDefaultObject )
{
}

Reference< XFastContextHandler > spDefContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_spPr:
        {
            xRet = new ShapePropertiesContext( *this, mrDefaultObject );
            break;
        }
        case NMSP_DRAWINGML|XML_bodyPr:
        {
            TextBodyPtr xTextBody( new TextBody );
            mrDefaultObject.setTextBody( xTextBody );
            xRet = new TextBodyPropertiesContext( *this, xAttribs, xTextBody->getTextProperties() );
            break;
        }
        case NMSP_DRAWINGML|XML_lstStyle:
            xRet.set( new TextListStyleContext( *this, *mrDefaultObject.getMasterTextListStyle() ) );
            break;
        case NMSP_DRAWINGML|XML_style:
            break;
    }
    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
