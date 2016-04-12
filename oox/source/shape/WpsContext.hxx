/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_SOURCE_SHAPE_WPSCONTEXT_HXX
#define INCLUDED_OOX_SOURCE_SHAPE_WPSCONTEXT_HXX

#include "oox/core/contexthandler2.hxx"
#include "oox/drawingml/shape.hxx"

namespace oox
{
namespace shape
{

/// Wps is the drawingML equivalent of v:shape.
class WpsContext : public oox::core::ContextHandler2
{
public:
    WpsContext(oox::core::ContextHandler2Helper& rParent, css::uno::Reference<css::drawing::XShape> xShape);
    virtual ~WpsContext();

    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElementToken, const oox::AttributeList& rAttribs) override;

    const oox::drawingml::ShapePtr& getShape()
    {
        return mpShape;
    }

protected:
    oox::drawingml::ShapePtr mpShape;
    css::uno::Reference<css::drawing::XShape> mxShape;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
