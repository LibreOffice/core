/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillpropertiesgroupcontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:40:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP
#define OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP

#include "oox/core/contexthandler.hxx"
#include "oox/drawingml/fillproperties.hxx"

namespace oox { namespace core {
    class PropertyMap;
} }

namespace oox { namespace drawingml {


// ---------------------------------------------------------------------

class FillPropertiesGroupContext : public ::oox::core::ContextHandler
{
public:
    FillPropertiesGroupContext( oox::core::ContextHandler& rParent, ::com::sun::star::drawing::FillStyle eFillStyle, FillProperties& rFillProperties ) throw();

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > StaticCreateContext( oox::core::ContextHandler& rParent,
        ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs, FillProperties& rFillProperties )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

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
    ::com::sun::star::drawing::BitmapMode   meBitmapMode;
    rtl::OUString   msEmbed;
    rtl::OUString   msLink;
};

} }

#endif // OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP
