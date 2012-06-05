/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef OOX_SHAPE_DRAWINGFRAGMENTHANDLER
#define OOX_SHAPE_DRAWINGFRAGMENTHANDLER

#include "oox/core/fragmenthandler.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"

namespace oox { namespace shape {

/// Generic (i.e. not specific to PPTX) handler for the prerendered diagram parsing.
class ShapeDrawingFragmentHandler : public oox::core::FragmentHandler
{
public:
    ShapeDrawingFragmentHandler( oox::core::XmlFilterBase& rFilter, const rtl::OUString& rFragmentPath, oox::drawingml::ShapePtr pGroupShapePtr ) throw();
    virtual ~ShapeDrawingFragmentHandler() throw();
    virtual void SAL_CALL endDocument() throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference<com::sun::star::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(sal_Int32 Element, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XFastAttributeList>& Attribs ) throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

private:
        oox::drawingml::ShapePtr        mpGroupShapePtr;
};


} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
