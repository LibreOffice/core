/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillpropertiesgroupcontext.hxx,v $
 * $Revision: 1.6 $
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

#ifndef OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP
#define OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP

#include "oox/core/contexthandler.hxx"
#include <com/sun/star/drawing/BitmapMode.hpp>

namespace oox { namespace drawingml {

struct FillProperties;

// ---------------------------------------------------------------------

class FillPropertiesGroupContext : public ::oox::core::ContextHandler
{
public:
    FillPropertiesGroupContext( ::oox::core::ContextHandler& rParent, FillProperties& rFillProperties, sal_Int32 nContext ) throw();

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > StaticCreateContext( oox::core::ContextHandler& rParent,
        ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs, FillProperties& rFillProperties )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

protected:
    FillProperties& mrFillProperties;
};

// ---------------------------------------------------------------------

class FillPropertiesContext : public ::oox::core::ContextHandler
{
public:
    FillPropertiesContext( oox::core::ContextHandler& rParent, FillProperties& rFillProperties ) throw();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    FillProperties& mrFillProperties;
};

// ---------------------------------------------------------------------

class BlipFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    BlipFillPropertiesContext( oox::core::ContextHandler& rParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes,
            FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    rtl::OUString   msEmbed;
    rtl::OUString   msLink;
};

} }

#endif // OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP
