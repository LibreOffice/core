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

#ifndef OOX_DRAWINGML_TEXTBODYCONTEXT_HXX
#define OOX_DRAWINGML_TEXTBODYCONTEXT_HXX

#include <com/sun/star/text/XText.hpp>

#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textrun.hxx"
#include "oox/core/contexthandler.hxx"

namespace oox { namespace drawingml {

class TextBodyContext : public ::oox::core::ContextHandler
{
public:
    TextBodyContext( ::oox::core::ContextHandler& rParent, TextBody& rTextBody );

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    TextBody&           mrTextBody;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxText;
};

// CT_RegularTextRun
class RegularTextRunContext : public ::oox::core::ContextHandler
{
public:
    RegularTextRunContext( ::oox::core::ContextHandler& rParent, TextRunPtr pRunPtr );

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    TextRunPtr          mpRunPtr;
    bool                mbIsInText;
};

} }

#endif  //  OOX_DRAWINGML_TEXTBODYCONTEXT_HXX
