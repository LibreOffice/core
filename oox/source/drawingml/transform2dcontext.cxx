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

#include "oox/drawingml/transform2dcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/shape.hxx"

using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::oox::core::ContextHandler;

namespace oox {
namespace drawingml {

// ============================================================================

/** context to import a CT_Transform2D */
Transform2DContext::Transform2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, Shape& rShape ) throw()
: ContextHandler( rParent )
, mrShape( rShape )
{
    AttributeList aAttributeList( xAttribs );
    mrShape.setRotation( aAttributeList.getInteger( XML_rot, 0 ) );	// 60000ths of a degree Positive angles are clockwise; negative angles are counter-clockwise
    mrShape.setFlip( aAttributeList.getBool( XML_flipH, sal_False ), aAttributeList.getBool( XML_flipV, sal_False ) );
}

Reference< XFastContextHandler > Transform2DContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_off:		// horz/vert translation
        mrShape.setPosition( Point( xAttribs->getOptionalValue( XML_x ).toInt32(), xAttribs->getOptionalValue( XML_y ).toInt32() ) );
        break;
    case NMSP_DRAWINGML|XML_ext:		// horz/vert size
        mrShape.setSize( Size( xAttribs->getOptionalValue( XML_cx ).toInt32(), xAttribs->getOptionalValue( XML_cy ).toInt32() ) );
        break;
    case NMSP_DRAWINGML|XML_chOff:	// horz/vert translation of children
        mrShape.setChildPosition( Point( xAttribs->getOptionalValue( XML_x ).toInt32(), xAttribs->getOptionalValue( XML_y ).toInt32() ) );
        break;
    case NMSP_DRAWINGML|XML_chExt:	// horz/vert size of children
        mrShape.setChildSize( Size( xAttribs->getOptionalValue( XML_cx ).toInt32(), xAttribs->getOptionalValue( XML_cy ).toInt32() ) );
        break;
    }

    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
