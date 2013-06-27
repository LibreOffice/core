/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OOX_SHAPE_LOCKEDCANVASCONTEXT
#define OOX_SHAPE_LOCKEDCANVASCONTEXT

#include "oox/core/contexthandler2.hxx"
#include "oox/drawingml/shape.hxx"

namespace oox { namespace shape {

/// Locked canvas is kind of a container for drawingml shapes: it can even contain group shapes.
class LockedCanvasContext : public oox::core::ContextHandler2
{
public:
    LockedCanvasContext( oox::core::ContextHandler2Helper& rParent );
    virtual ~LockedCanvasContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 Element, const ::oox::AttributeList& rAttribs ) SAL_OVERRIDE;

    oox::drawingml::ShapePtr getShape();

protected:

    oox::drawingml::ShapePtr mpShape;
};


} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
