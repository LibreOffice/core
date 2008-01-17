/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillpropertiesgroupcontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:45 $
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

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#ifndef OOX_CORE_CONTEXT_HXX
#include "oox/core/context.hxx"
#endif
#include "oox/drawingml/fillproperties.hxx"
#include <com/sun/star/drawing/BitmapMode.hpp>

namespace oox { namespace core {
    class PropertyMap;
} }

namespace oox { namespace drawingml {


// ---------------------------------------------------------------------

class FillPropertiesGroupContext : public ::oox::core::Context
{
public:
    FillPropertiesGroupContext( const oox::core::FragmentHandlerRef& xHandler, ::com::sun::star::drawing::FillStyle eFillStyle, ::oox::drawingml::FillProperties& rFillProperties ) throw();

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > StaticCreateContext( const oox::core::FragmentHandlerRef& xHandler,
        ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs, ::oox::drawingml::FillProperties& rFillProperties )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

protected:
    FillProperties& mrFillProperties;
};

// ---------------------------------------------------------------------

class BlipFillPropertiesContext : public FillPropertiesGroupContext
{
public:
    BlipFillPropertiesContext( const oox::core::FragmentHandlerRef& xHandler,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes,
            ::oox::drawingml::FillProperties& rFillProperties ) throw();

    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::drawing::BitmapMode   meBitmapMode;
    sal_Int32       mnWidth, mnHeight;
    rtl::OUString   msEmbed;
    rtl::OUString   msLink;
};

} }

#endif // OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP
