/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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



#ifndef OOX_PPT_ANIMVARIANTCONTEXT
#define OOX_PPT_ANIMVERIANTCONTEXT


#include <com/sun/star/uno/Any.hxx>

#include "oox/core/contexthandler.hxx"
#include "oox/drawingml/color.hxx"

namespace oox { namespace ppt {

    /** context CT_TLAnimVariant */
    class AnimVariantContext
        : public ::oox::core::ContextHandler
    {
    public:
        AnimVariantContext( ::oox::core::ContextHandler& rParent, ::sal_Int32 aElement, ::com::sun::star::uno::Any & aValue );
        ~AnimVariantContext( ) throw( );
        virtual void SAL_CALL endFastElement( sal_Int32 /*aElement*/ ) throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    private:
        ::sal_Int32 mnElement;
        ::com::sun::star::uno::Any& maValue;
        ::oox::drawingml::Color		maColor;
    };

} }


#endif
