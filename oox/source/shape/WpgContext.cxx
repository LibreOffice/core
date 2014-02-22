/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "WpgContext.hxx"
#include <oox/drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>

using namespace com::sun::star;

namespace oox
{
namespace shape
{

WpgContext::WpgContext(ContextHandler2Helper& rParent)
    : ContextHandler2(rParent)
{
    mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.GroupShape"));
    mpShape->setWps(true);
}

WpgContext::~WpgContext()
{
}

oox::drawingml::ShapePtr WpgContext::getShape()
{
    return mpShape;
}

oox::core::ContextHandlerRef WpgContext::onCreateContext(sal_Int32 nElementToken, const oox::AttributeList& /*rAttribs*/)
{
    switch (getBaseToken(nElementToken))
    {
    case XML_wgp:
        break;
    case XML_cNvGrpSpPr:
        break;
    case XML_grpSpPr:
        return new oox::drawingml::ShapePropertiesContext(*this, *mpShape);
        break;
    case XML_wsp:
    {
        
        
        
        oox::drawingml::ShapePtr pShape(new oox::drawingml::Shape("com.sun.star.drawing.CustomShape", /*bDefaultHeight=*/false));
        return new oox::drawingml::ShapeContext(*this, mpShape, pShape);
    }
    break;
    case XML_pic:
        return new oox::drawingml::GraphicShapeContext(*this, mpShape, oox::drawingml::ShapePtr(new oox::drawingml::Shape("com.sun.star.drawing.GraphicObjectShape")));
        break;
    case XML_grpSp:
    {
        return new oox::drawingml::ShapeGroupContext(*this, mpShape, oox::drawingml::ShapePtr(new oox::drawingml::Shape("com.sun.star.drawing.GroupShape")));
    }
    break;
    default:
        SAL_WARN("oox", "WpgContext::createFastChildContext: unhandled element: " << getBaseToken(nElementToken));
        break;
    }
    return 0;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
