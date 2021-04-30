/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ShapeDrawingFragmentHandler.hxx"

#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/token/namespaces.hxx>

using namespace com::sun::star;

namespace oox::shape {

ShapeDrawingFragmentHandler::ShapeDrawingFragmentHandler(oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, oox::drawingml::ShapePtr const & pGroupShapePtr)
    : FragmentHandler2(rFilter, rFragmentPath)
    , mpGroupShapePtr(pGroupShapePtr)
{
}

ShapeDrawingFragmentHandler::~ShapeDrawingFragmentHandler() noexcept
{
}

void SAL_CALL ShapeDrawingFragmentHandler::endDocument()
{
}

::oox::core::ContextHandlerRef ShapeDrawingFragmentHandler::onCreateContext(sal_Int32 Element, const AttributeList& /*Attribs*/ )
{
    switch( Element )
    {
        case DSP_TOKEN( spTree ):
            return new oox::drawingml::ShapeGroupContext(*this, oox::drawingml::ShapePtr(nullptr), mpGroupShapePtr);
        default:
            break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
