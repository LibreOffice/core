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
WpgContext::WpgContext(FragmentHandler2 const& rParent, const oox::drawingml::ShapePtr& pMaster)
    : FragmentHandler2(rParent)
    , m_bFullWPGSupport(false)
{
    mpShape = std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.GroupShape"_ustr);
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
            if (m_bFullWPGSupport)
            {
                return new oox::shape::WpsContext(*this, uno::Reference<drawing::XShape>(), mpShape,
                                                  std::make_shared<oox::drawingml::Shape>(
                                                      u"com.sun.star.drawing.CustomShape"_ustr,
                                                      /*bDefaultHeight=*/false));
            }

            // Don't set default character height, Writer has its own way to set
            // the default, and if we don't set it here, editeng properly inherits
            // it.
            return new oox::drawingml::ShapeContext(
                *this, mpShape,
                std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.CustomShape"_ustr,
                                                        /*bDefaultHeight=*/false));
        }
        case XML_pic:
            return new oox::drawingml::GraphicShapeContext(
                *this, mpShape,
                std::make_shared<oox::drawingml::Shape>(
                    u"com.sun.star.drawing.GraphicObjectShape"_ustr));
        case XML_grpSp:
        {
            if (m_bFullWPGSupport)
            {
                rtl::Reference<WpgContext> pWPGShape = new oox::shape::WpgContext(*this, mpShape);
                pWPGShape->setFullWPGSupport(m_bFullWPGSupport);
                return pWPGShape;
            }

            return new oox::drawingml::ShapeGroupContext(
                *this, mpShape,
                std::make_shared<oox::drawingml::Shape>(u"com.sun.star.drawing.GroupShape"_ustr));
        }
        case XML_graphicFrame:
        {
            auto pShape = std::make_shared<oox::drawingml::Shape>(
                u"com.sun.star.drawing.GraphicObjectShape"_ustr);
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
