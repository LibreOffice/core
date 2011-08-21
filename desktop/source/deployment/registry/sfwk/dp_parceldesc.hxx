/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
namespace css = ::com::sun::star;
namespace dp_registry
{
namespace backend
{
namespace sfwk
{

typedef ::cppu::WeakImplHelper1< css::xml::sax::XDocumentHandler > t_DocHandlerImpl;

class ParcelDescDocHandler : public t_DocHandlerImpl
{
private:
    bool m_bIsParsed;
    ::rtl::OUString m_sLang;
    sal_Int32 skipIndex;
public:
    ParcelDescDocHandler():m_bIsParsed( false ), skipIndex( 0 ){}
    ::rtl::OUString getParcelLanguage() { return m_sLang; }
    bool isParsed() { return m_bIsParsed; }
    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL endDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL startElement( const ::rtl::OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs )
        throw ( css::xml::sax::SAXException,
            css::uno::RuntimeException );

    virtual void SAL_CALL endElement( const ::rtl::OUString & aName )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL characters( const ::rtl::OUString & aChars )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString & aWhitespaces )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL processingInstruction(
        const ::rtl::OUString & aTarget, const ::rtl::OUString & aData )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );
};
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
