/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "WpgContext.hxx"
#include "WpsContext.hxx"
#include <sal/log.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace com::sun::star;

namespace oox
{
namespace shape
{

WpgContext::WpgContext(ContextHandler2Helper const& rParent)
    : ContextHandler2(rParent)
{
    mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.GroupShape"));
    mpShape->setWps(true);
}

WpgContext::~WpgContext() = default;

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
    case XML_wsp:
    {
        // Don't set default character height, Writer has its own way to set
        // the default, and if we don't set it here, editeng properly inherits
        // it.
        oox::drawingml::ShapePtr pShape(new oox::drawingml::Shape("com.sun.star.drawing.CustomShape", /*bDefaultHeight=*/false));
        return new oox::drawingml::ShapeContext(*this, mpShape, pShape);
        // return new oox::shape::WpsContext(*this, uno::Reference<drawing::XShape>(),
        //                                   mpShape, pShape);
    }
    case XML_pic:
        return new oox::drawingml::GraphicShapeContext(*this, mpShape, std::make_shared<oox::drawingml::Shape>("com.sun.star.drawing.GraphicObjectShape"));
    case XML_grpSp:
    {
        return new oox::drawingml::ShapeGroupContext(*this, mpShape, std::make_shared<oox::drawingml::Shape>("com.sun.star.drawing.GroupShape"));
    }
    case XML_graphicFrame:
        break;
    default:
        SAL_WARN("oox", "WpgContext::createFastChildContext: unhandled element: " << getBaseToken(nElementToken));
        break;
    }
    return nullptr;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
