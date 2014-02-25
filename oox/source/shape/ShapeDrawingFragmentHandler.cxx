/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ShapeDrawingFragmentHandler.hxx"

using namespace com::sun::star;

namespace oox { namespace shape {

ShapeDrawingFragmentHandler::ShapeDrawingFragmentHandler(oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, oox::drawingml::ShapePtr pGroupShapePtr ) throw()
        : FragmentHandler2(rFilter, rFragmentPath),
        mpGroupShapePtr(pGroupShapePtr)
{
}

ShapeDrawingFragmentHandler::~ShapeDrawingFragmentHandler() throw()
{
}

void SAL_CALL ShapeDrawingFragmentHandler::endDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

::oox::core::ContextHandlerRef ShapeDrawingFragmentHandler::onCreateContext(sal_Int32 Element, const AttributeList& /*Attribs*/ )
{
    switch( Element )
    {
        case DSP_TOKEN( spTree ):
            return new oox::drawingml::ShapeGroupContext(*this, oox::drawingml::ShapePtr((oox::drawingml::Shape*)0), mpGroupShapePtr);
        default:
            break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
