/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "LockedCanvasContext.hxx"
#include <oox/drawingml/shapegroupcontext.hxx>

using namespace com::sun::star;

namespace oox { namespace shape {

LockedCanvasContext::LockedCanvasContext( ContextHandler& rParent )
: ContextHandler( rParent )
{
}

LockedCanvasContext::~LockedCanvasContext()
{
}

oox::drawingml::ShapePtr LockedCanvasContext::getShape()
{
    return mpShape;
}

uno::Reference< xml::sax::XFastContextHandler > LockedCanvasContext::createFastChildContext( sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttribs*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( getBaseToken( aElementToken ) )
    {
    case XML_lockedCanvas:
        break;
    case XML_nvGrpSpPr:
        break;
    case XML_grpSpPr:
        break;
    case XML_sp:
        {
            oox::drawingml::ShapePtr pMasterShape;
            mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.CustomShape"));
            xRet = new oox::drawingml::ShapeContext( *this, pMasterShape, mpShape );
        }
        break;
    case XML_grpSp:
        {
            oox::drawingml::ShapePtr pMasterShape;
            mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.GroupShape"));
            xRet = new oox::drawingml::ShapeGroupContext( *this, pMasterShape, mpShape );
        }
        break;
    default:
        SAL_WARN("oox", "LockedCanvasContext::createFastChildContext: unhandled element:" << getBaseToken(aElementToken));
        break;
    }
    return xRet;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
