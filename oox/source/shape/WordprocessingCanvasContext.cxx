/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WordprocessingCanvasContext.hxx"
#include "WpsContext.hxx"
#include "WpgContext.hxx"
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/effectpropertiescontext.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/connectorshapecontext.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sal/log.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdobj.hxx>

using namespace com::sun::star;

namespace oox::shape
{
WordprocessingCanvasContext::WordprocessingCanvasContext(FragmentHandler2 const& rParent,
                                                         css::awt::Size& rSize)
    : FragmentHandler2(rParent)
    , m_bFullWPGSupport(true)
{
    mpShapePtr = std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.GroupShape"_ustr);
    mpShapePtr->setSize(rSize);
    mpShapePtr->setWordprocessingCanvas(true); // will be "WordprocessingCanvas" in InteropGrabBag
    mpShapePtr->setWps(true);
    oox::drawingml::ShapePtr pBackground
        = std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.CustomShape"_ustr);
    pBackground->getCustomShapeProperties()->setShapePresetType(XML_rect);
    pBackground->setSize(rSize);
    pBackground->setWordprocessingCanvas(true);
    pBackground->setWPGChild(true);
    pBackground->setWps(true);
    // Fill and Line properties will follow in wpc:bg and wpc:whole child elements of wpc element
    mpShapePtr->addChild(pBackground);
    mpShapePtr->setChildSize(rSize);
}

WordprocessingCanvasContext::~WordprocessingCanvasContext() = default;

::oox::core::ContextHandlerRef
WordprocessingCanvasContext::onCreateContext(sal_Int32 nElementToken,
                                             const ::oox::AttributeList& /*rAttribs*/)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_wpc:
            SAL_INFO("oox", "WordprocessingCanvasContext::createFastChildContext: wpc: "
                                << getBaseToken(nElementToken));
            break;
        case XML_bg: //CT_BackgroundFormatting
            return new oox::drawingml::ShapePropertiesContext(*this,
                                                              *(getShape()->getChildren().front()));
        case XML_whole: // CT_WholeE2oFormatting
            return new oox::drawingml::ShapePropertiesContext(*this,
                                                              *(getShape()->getChildren().front()));
        case XML_wsp: // CT_WordprocessingShape
        {
            oox::drawingml::ShapePtr pShape = std::make_shared<oox::drawingml::Shape>(
                u"com.sun.star.drawing.CustomShape"_ustr, /*bDefaultHeight=*/false);
            return new oox::shape::WpsContext(*this, uno::Reference<drawing::XShape>(), mpShapePtr,
                                              pShape);
        }
        case XML_pic: // CT_Picture
            return new oox::drawingml::GraphicShapeContext(
                *this, mpShapePtr,
                std::make_shared<oox::drawingml::Shape>(
                    u"com.sun.star.drawing.GraphicObjectShape"_ustr));
            break;
        case XML_graphicFrame: // CT_GraphicFrame
            SAL_INFO("oox",
                     "WordprocessingCanvasContext::createFastChildContext: ToDo: graphicFrame: "
                         << getBaseToken(nElementToken));
            break;
        case XML_wgp: // CT_WordprocessingGroup
        {
            rtl::Reference<WpgContext> pWPGContext = new oox::shape::WpgContext(*this, mpShapePtr);
            pWPGContext->setFullWPGSupport(m_bFullWPGSupport);
            return pWPGContext;
        }
        default:
            // includes case XML_contentPart
            // Word uses this for Ink, as <w14:contentPart r:id="rId4"> for example. Thereby rId4 is
            // a reference into the 'ink' folder in the docx package. Import of Ink is not
            // implemented yet. In general it refers to arbitrary XML source.
            SAL_WARN("oox",
                     "WordprocessingCanvasContext::createFastChildContext: unhandled element:"
                         << getBaseToken(nElementToken));
            break;
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
