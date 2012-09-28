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

#include "oox/drawingml/transform2dcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/textbody.hxx"

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
Transform2DContext::Transform2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, Shape& rShape, bool btxXfrm ) throw()
: ContextHandler( rParent )
, mrShape( rShape )
, mbtxXfrm ( btxXfrm )
{
    AttributeList aAttributeList( xAttribs );
    if( !btxXfrm )
    {
        mrShape.setRotation( aAttributeList.getInteger( XML_rot, 0 ) ); // 60000ths of a degree Positive angles are clockwise; negative angles are counter-clockwise
        mrShape.setFlip( aAttributeList.getBool( XML_flipH, sal_False ), aAttributeList.getBool( XML_flipV, sal_False ) );
    }
    else
    {
        mrShape.getTextBody()->getTextProperties().moRotation = aAttributeList.getInteger( XML_rot );
    }
}

Reference< XFastContextHandler > Transform2DContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    if( mbtxXfrm )
    {
        switch( aElementToken )
        {
            case A_TOKEN( off ):
                {
                    OUString sXValue = xAttribs->getOptionalValue( XML_x );
                    OUString sYValue = xAttribs->getOptionalValue( XML_y );
                    if( !sXValue.isEmpty() )
                        mrShape.getTextBody()->getTextProperties().moTextOffX = GetCoordinate( sXValue.toInt32() - mrShape.getPosition().X );
                    if( !sYValue.isEmpty() )
                        mrShape.getTextBody()->getTextProperties().moTextOffY = GetCoordinate( sYValue.toInt32() - mrShape.getPosition().Y );
                }
                break;
            case A_TOKEN( ext ):
                break;
        }
        return 0;
    }

    switch( aElementToken )
    {
    case A_TOKEN( off ):        // horz/vert translation
        mrShape.setPosition( Point( xAttribs->getOptionalValue( XML_x ).toInt32(), xAttribs->getOptionalValue( XML_y ).toInt32() ) );
        break;
    case A_TOKEN( ext ):        // horz/vert size
        mrShape.setSize( Size( xAttribs->getOptionalValue( XML_cx ).toInt32(), xAttribs->getOptionalValue( XML_cy ).toInt32() ) );
        break;
    case A_TOKEN( chOff ):  // horz/vert translation of children
        mrShape.setChildPosition( Point( xAttribs->getOptionalValue( XML_x ).toInt32(), xAttribs->getOptionalValue( XML_y ).toInt32() ) );
        break;
    case A_TOKEN( chExt ):  // horz/vert size of children
        mrShape.setChildSize( Size( xAttribs->getOptionalValue( XML_cx ).toInt32(), xAttribs->getOptionalValue( XML_cy ).toInt32() ) );
        break;
    }

    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
