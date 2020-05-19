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
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace com::sun::star;

namespace oox::shape
{
LockedCanvasContext::LockedCanvasContext(FragmentHandler2 const& rParent)
    : FragmentHandler2(rParent)
{
}

LockedCanvasContext::~LockedCanvasContext() = default;

::oox::core::ContextHandlerRef
LockedCanvasContext::onCreateContext(sal_Int32 nElementToken,
                                     const ::oox::AttributeList& /*rAttribs*/)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_lockedCanvas:
        case XML_nvGrpSpPr:
        case XML_grpSpPr:
            break;
        case XML_sp:
        {
            oox::drawingml::ShapePtr pMasterShape;
            mpShape = std::make_shared<oox::drawingml::Shape>("com.sun.star.drawing.CustomShape");
            mpShape->setLockedCanvas(true);
            return new oox::drawingml::ShapeContext(*this, pMasterShape, mpShape);
        }
        case XML_grpSp:
        {
            oox::drawingml::ShapePtr pMasterShape;
            mpShape = std::make_shared<oox::drawingml::Shape>("com.sun.star.drawing.GroupShape");
            mpShape->setLockedCanvas(true);
            return new oox::drawingml::ShapeGroupContext(*this, pMasterShape, mpShape);
        }
        default:
            SAL_WARN("oox", "LockedCanvasContext::createFastChildContext: unhandled element:"
                                << getBaseToken(nElementToken));
            break;
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
