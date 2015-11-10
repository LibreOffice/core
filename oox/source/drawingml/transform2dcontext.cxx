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

#include "drawingml/transform2dcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/shape.hxx"
#include "drawingml/customshapeproperties.hxx"
#include "drawingml/textbody.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace drawingml {

/** context to import a CT_Transform2D */
Transform2DContext::Transform2DContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, Shape& rShape, bool btxXfrm ) throw()
: ContextHandler2( rParent )
, mrShape( rShape )
, mbtxXfrm ( btxXfrm )
{
    if( !btxXfrm )
    {
        mrShape.setRotation( rAttribs.getInteger( XML_rot, 0 ) ); // 60000ths of a degree Positive angles are clockwise; negative angles are counter-clockwise
        mrShape.setFlip( rAttribs.getBool( XML_flipH, false ), rAttribs.getBool( XML_flipV, false ) );
    }
    else
    {
        if( rAttribs.hasAttribute( XML_rot ) )
            mrShape.getTextBody()->getTextProperties().moRotation = rAttribs.getInteger( XML_rot ).get();
    }
}

ContextHandlerRef Transform2DContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( mbtxXfrm )
    {
        // Workaround: only for rectangles
        const sal_Int32 nType = mrShape.getCustomShapeProperties()->getShapePresetType();
        if( nType == XML_rect || nType == XML_roundRect )
        {
            switch( aElementToken )
            {
                case A_TOKEN( off ):
                    {
                        const OUString sXValue = rAttribs.getString( XML_x ).get();
                        const OUString sYValue = rAttribs.getString( XML_y ).get();

                        if( !sXValue.isEmpty() )
                            mrShape.getTextBody()->getTextProperties().moTextOffLeft = GetCoordinate( sXValue.toInt32() - mrShape.getPosition().X );
                        if( !sYValue.isEmpty() )
                            mrShape.getTextBody()->getTextProperties().moTextOffUpper = GetCoordinate( sYValue.toInt32() - mrShape.getPosition().Y );
                    }
                    break;
                case A_TOKEN( ext ):
                    {
                        const OUString sXValue = rAttribs.getString( XML_cx ).get();
                        const OUString sYValue = rAttribs.getString( XML_cy ).get();

                        if( !sXValue.isEmpty() && nType == XML_rect )
                        {
                            mrShape.getTextBody()->getTextProperties().moTextOffRight = GetCoordinate(mrShape.getSize().Width - sXValue.toInt32());
                            if( mrShape.getTextBody()->getTextProperties().moTextOffLeft )
                               *mrShape.getTextBody()->getTextProperties().moTextOffRight -=  *mrShape.getTextBody()->getTextProperties().moTextOffLeft;
                        }
                        if( !sYValue.isEmpty() )
                        {
                            mrShape.getTextBody()->getTextProperties().moTextOffLower = GetCoordinate(mrShape.getSize().Height - sYValue.toInt32());
                            if( mrShape.getTextBody()->getTextProperties().moTextOffUpper )
                               *mrShape.getTextBody()->getTextProperties().moTextOffLower -=  *mrShape.getTextBody()->getTextProperties().moTextOffUpper;

                        }
                    }
                    break;
            }
        }
        return nullptr;
    }

    switch( aElementToken )
    {
    case A_TOKEN( off ):        // horz/vert translation
        mrShape.setPosition( awt::Point( rAttribs.getString( XML_x ).get().toInt32(), rAttribs.getString( XML_y ).get().toInt32() ) );
        break;
    case A_TOKEN( ext ):        // horz/vert size
        mrShape.setSize( awt::Size( rAttribs.getString( XML_cx ).get().toInt32(), rAttribs.getString( XML_cy ).get().toInt32() ) );
        break;
    case A_TOKEN( chOff ):  // horz/vert translation of children
        mrShape.setChildPosition( awt::Point( rAttribs.getString( XML_x ).get().toInt32(), rAttribs.getString( XML_y ).get().toInt32() ) );
        break;
    case A_TOKEN( chExt ):  // horz/vert size of children
        mrShape.setChildSize( awt::Size( rAttribs.getString( XML_cx ).get().toInt32(), rAttribs.getString( XML_cy ).get().toInt32() ) );
        break;
    }

    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
