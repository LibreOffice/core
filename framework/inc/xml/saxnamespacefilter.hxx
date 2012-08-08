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

#ifndef __FRAMEWORK_SERVICES_SAXNAMESPACEFILTER_HXX_
#define __FRAMEWORK_SERVICES_SAXNAMESPACEFILTER_HXX_

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <threadhelp/threadhelpbase.hxx>
#include <xml/xmlnamespaces.hxx>
#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <cppuhelper/implbase1.hxx>

#include <stack>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC SaxNamespaceFilter : public ThreadHelpBase, // Struct for right initalization of mutex member! Must be first of baseclasses.
                           public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
{
    public:
        SaxNamespaceFilter( ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rSax1DocumentHandler );
        virtual ~SaxNamespaceFilter();

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
        throw ( ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endDocument(void)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL startElement(
            const rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > &xAttribs)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endElement(const rtl::OUString& aName)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL characters(const rtl::OUString& aChars)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL ignorableWhitespace(const rtl::OUString& aWhitespaces)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL processingInstruction(const rtl::OUString& aTarget,
                                                    const rtl::OUString& aData)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDocumentLocator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > &xLocator)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

    protected:
        typedef ::std::stack< XMLNamespaces > NamespaceStack;

        ::rtl::OUString getErrorLineString();

        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > m_xLocator;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler;
        NamespaceStack  m_aNamespaceStack;
        sal_Int32       m_nDepth;

        rtl::OUString m_aXMLAttributeNamespace;
        rtl::OUString m_aXMLAttributeType;

};

}

#endif // __FRAMEWORK_SERVICES_SAXNAMESPACEFILTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
