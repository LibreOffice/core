/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawingfragmenthandler.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_VML_DRAWINGFRAGMENTHANDLER
#define OOX_VML_DRAWINGFRAGMENTHANDLER

#include "oox/core/fragmenthandler.hxx"
#include "oox/vml/drawing.hxx"
#include "oox/vml/shape.hxx"

namespace oox { namespace vml {

class DrawingFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DrawingFragmentHandler( ::oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath,
        std::vector< ShapePtr >& rShapes, std::vector< ShapePtr >& rShapeTypes ) throw();
    virtual ~DrawingFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > StaticCreateContext( oox::core::ContextHandler& rParent,
        sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs,
            std::vector< ShapePtr >& rShapes, std::vector< ShapePtr >& rShapeTypes)
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );


private:

    std::vector< ShapePtr >&        mrShapes;
    std::vector< ShapePtr >&        mrShapeTypes;
    rtl::OUString                   maFragmentPath;
};

} }

#endif // OOX_VML_DRAWINGFRAGMENTHANDLER
