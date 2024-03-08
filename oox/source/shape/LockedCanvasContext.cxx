/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "LockedCanvasContext.hxx"
#include <sal/log.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/connectorshapecontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace com::sun::star;

namespace oox::shape
{
LockedCanvasContext::LockedCanvasContext(FragmentHandler2 const& rParent)
    : FragmentHandler2(rParent)
{
    mpShapePtr = std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.GroupShape"_ustr);
    mpShapePtr->setLockedCanvas(true); // will be "LockedCanvas" in InteropGrabBag
}

LockedCanvasContext::~LockedCanvasContext() = default;

::oox::core::ContextHandlerRef
LockedCanvasContext::onCreateContext(sal_Int32 nElementToken, const ::oox::AttributeList& rAttribs)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_nvGrpSpPr: // CT_GvmlGroupShapeNonVisual, child see at end
            return this;
        case XML_grpSpPr: // CT_GroupShapeProperties
            return new oox::drawingml::ShapePropertiesContext(*this, *mpShapePtr);
        case XML_txSp: // CT_GvmlTextShape
            break;
        case XML_sp: // CT_GvmlShape
        {
            return new oox::drawingml::ShapeContext(
                *this, mpShapePtr,
                std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.CustomShape"_ustr,
                                                        true));
        }
        case XML_cxnSp: // CT_GvmlConnector
        {
            oox::drawingml::ShapePtr pShape = std::make_shared<oox::drawingml::Shape>(
                u"com.sun.star.drawing.ConnectorShape"_ustr);
            return new oox::drawingml::ConnectorShapeContext(*this, mpShapePtr, pShape,
                                                             pShape->getConnectorShapeProperties());
        }
        case XML_pic: // CT_GvmlPicture
        {
            return new oox::drawingml::GraphicShapeContext(
                *this, mpShapePtr,
                std::make_shared<oox::drawingml::Shape>(
                    u"com.sun.star.drawing.GraphicObjectShape"_ustr));
        }
        case XML_graphicFrame: // CT_GvmlGraphicObjectFrame
        {
            return new oox::drawingml::GraphicalObjectFrameContext(
                *this, mpShapePtr,
                std::make_shared<oox::drawingml::Shape>(
                    u"com.sun.star.drawing.GraphicObjectShape"_ustr),
                true);
        }
        case XML_grpSp: // CT_GvmlGroupShape
        {
            return new oox::drawingml::ShapeGroupContext(
                *this, mpShapePtr,
                std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.GroupShape"_ustr));
        }
        // mandatory child elements of CT_GvmlGroupShapeNonVisual
        case XML_cNvPr: // CT_NonVisualDrawingProps
        {
            mpShapePtr->setHidden(rAttribs.getBool(XML_hidden, false));
            mpShapePtr->setId(rAttribs.getStringDefaulted(XML_id));
            mpShapePtr->setName(rAttribs.getStringDefaulted(XML_name));
            break;
        }
        case XML_cNvGrpSpPr: // CT_NonVisualGroupDrawingShapeProps
            break;
        default:
            SAL_WARN("oox", "LockedCanvasContext::createFastChildContext: unhandled element:"
                                << getBaseToken(nElementToken));
            break;
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
