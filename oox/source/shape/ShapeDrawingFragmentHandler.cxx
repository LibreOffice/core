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

#include "ShapeDrawingFragmentHandler.hxx"

using namespace com::sun::star;
using rtl::OUString;

namespace oox { namespace shape {

ShapeDrawingFragmentHandler::ShapeDrawingFragmentHandler(oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, oox::drawingml::ShapePtr pGroupShapePtr ) throw()
        : FragmentHandler(rFilter, rFragmentPath),
        mpGroupShapePtr(pGroupShapePtr)
{
}

ShapeDrawingFragmentHandler::~ShapeDrawingFragmentHandler() throw()
{
}

void SAL_CALL ShapeDrawingFragmentHandler::endDocument() throw (xml::sax::SAXException, uno::RuntimeException)
{
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL ShapeDrawingFragmentHandler::createFastChildContext(sal_Int32 Element, const uno::Reference<xml::sax::XFastAttributeList>& /*Attribs*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< XFastContextHandler > xRet;

    switch( Element )
    {
        case DSP_TOKEN( spTree ):
            xRet.set( new oox::drawingml::ShapeGroupContext(*this, oox::drawingml::ShapePtr((oox::drawingml::Shape*)0), mpGroupShapePtr));
            break;
        default:
            break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
