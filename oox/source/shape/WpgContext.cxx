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
#include <drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace com::sun::star;

namespace oox::shape
{
WpgContext::WpgContext(FragmentHandler2 const& rParent, oox::drawingml::ShapePtr pMaster)
    : FragmentHandler2(rParent)
{
    mpShape = std::make_shared<oox::drawingml::Shape>("com.sun.star.drawing.GroupShape");
    mpShape->setWps(true);
    if (pMaster)
        pMaster->addChild(mpShape);
}

WpgContext::~WpgContext() = default;

oox::core::ContextHandlerRef WpgContext::onCreateContext(sal_Int32 nElementToken,
                                                         const oox::AttributeList& /*rAttribs*/)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_wgp:
        case XML_cNvGrpSpPr:
        case XML_grpSpPr:
            return new oox::drawingml::ShapePropertiesContext(*this, *mpShape);
        case XML_wsp:
        {
            oox::drawingml::ShapePtr pShape = std::make_shared<oox::drawingml::Shape>(
                "com.sun.star.drawing.CustomShape", /*bDefaultHeight=*/false);
            return new oox::shape::WpsContext(*this, uno::Reference<drawing::XShape>(), mpShape,
                                              pShape);
        }
        case XML_pic:
            return new oox::drawingml::GraphicShapeContext(
                *this, mpShape,
                std::make_shared<oox::drawingml::Shape>("com.sun.star.drawing.GraphicObjectShape"));
        case XML_grpSp:
        {
            return new oox::shape::WpgContext(*this, mpShape);
        }
        case XML_graphicFrame:
        {
            auto pShape = std::make_shared<oox::drawingml::Shape>(
                "com.sun.star.drawing.GraphicObjectShape");
            pShape->setWps(true);
            return new oox::drawingml::GraphicalObjectFrameContext(*this, mpShape, pShape,
                                                                   /*bEmbedShapesInChart=*/true);
        }
        default:
            SAL_WARN("oox", "WpgContext::createFastChildContext: unhandled element: "
                                << getBaseToken(nElementToken));
            break;
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
