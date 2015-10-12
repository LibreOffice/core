/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_SOURCE_SHAPE_SHAPEDRAWINGFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_SOURCE_SHAPE_SHAPEDRAWINGFRAGMENTHANDLER_HXX

#include "oox/core/fragmenthandler2.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"

namespace oox { namespace shape {

/// Generic (i.e. not specific to PPTX) handler for the prerendered diagram parsing.
class ShapeDrawingFragmentHandler : public oox::core::FragmentHandler2
{
public:
    ShapeDrawingFragmentHandler( oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, oox::drawingml::ShapePtr pGroupShapePtr ) throw();
    virtual ~ShapeDrawingFragmentHandler() throw();
    virtual void SAL_CALL endDocument() throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::oox::core::ContextHandlerRef onCreateContext(sal_Int32 Element, const AttributeList& rAttribs ) override;

private:
        oox::drawingml::ShapePtr        mpGroupShapePtr;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
