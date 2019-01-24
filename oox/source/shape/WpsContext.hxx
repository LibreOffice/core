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

#include <oox/core/contexthandler2.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace drawing
{
class XShape;
}
}
}
}

namespace oox
{
namespace shape
{
/// Wps is the drawingML equivalent of v:shape.
class WpsContext final : public oox::drawingml::ShapeContext
{
public:
    WpsContext(oox::core::ContextHandler2Helper const& rParent,
               css::uno::Reference<css::drawing::XShape> xShape,
               oox::drawingml::ShapePtr const& pMasterShapePtr,
               oox::drawingml::ShapePtr const& pShapePtr);
    ~WpsContext() override;

    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElementToken,
                                                 const oox::AttributeList& rAttribs) override;

private:
    css::uno::Reference<css::drawing::XShape> mxShape;
};
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
