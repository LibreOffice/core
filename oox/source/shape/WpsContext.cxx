/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpsContext.hxx"
#include <oox/drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/shapestylecontext.hxx>

using namespace com::sun::star;

namespace oox { namespace shape {

WpsContext::WpsContext(ContextHandler2Helper& rParent)
: ContextHandler2(rParent)
{
    mpShape.reset(new oox::drawingml::Shape("com.sun.star.drawing.CustomShape"));
}

WpsContext::~WpsContext()
{
}

oox::drawingml::ShapePtr WpsContext::getShape()
{
    return mpShape;
}

oox::core::ContextHandlerRef WpsContext::onCreateContext(sal_Int32 nElementToken, const oox::AttributeList& /*rAttribs*/)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_wsp:
            break;
        case XML_cNvSpPr:
            break;
        case XML_spPr:
            return new oox::drawingml::ShapePropertiesContext(*this, *mpShape);
            break;
        case XML_style:
            return new oox::drawingml::ShapeStyleContext(*this, *mpShape);
            break;
        case XML_bodyPr:
            break;
        default:
            SAL_WARN("oox", "WpsContext::createFastChildContext: unhandled element:" << getBaseToken(nElementToken));
            break;
    }
    return 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
