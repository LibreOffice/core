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

#ifndef _XMLOFF_XMLMETAE_HXX
#define _XMLOFF_XMLMETAE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"

#include <cppuhelper/implbase1.hxx>
#include <xmloff/xmltoken.hxx>

#include <vector>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


class SvXMLExport;

/** export meta data from an <type>XDocumentProperties</type> instance.

    <p>
    This class will start the export at the office:meta element,
    not at the root element. This means that when <method>Export</method>
    is called here, the document root element must already be written, but
    office:meta must <em>not</em> be written.
    </p>
 */
class XMLOFF_DLLPUBLIC SvXMLMetaExport : public ::cppu::WeakImplHelper1<
                ::com::sun::star::xml::sax::XDocumentHandler >
{
private:
    SvXMLExport& mrExport;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;
    /// counts levels of the xml document. necessary for special handling.
    int m_level;
    /// preserved namespaces. necessary because we do not write the root node.
    std::vector< ::com::sun::star::beans::StringPair > m_preservedNSs;

    SAL_DLLPRIVATE void SimpleStringElement(
        const ::rtl::OUString& rText, sal_uInt16 nNamespace,
        enum ::xmloff::token::XMLTokenEnum eElementName );
    SAL_DLLPRIVATE void SimpleDateTimeElement(
        const ::com::sun::star::util::DateTime & rDate, sal_uInt16 nNamespace,
        enum ::xmloff::token::XMLTokenEnum eElementName );

    /// currently unused; for exporting via the XDocumentProperties interface
    SAL_DLLPRIVATE void _MExport();

public:
    SvXMLMetaExport( SvXMLExport& i_rExport,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>& i_rDocProps);

    virtual ~SvXMLMetaExport();

    /// export via XSAXWriter interface, with fallback to _MExport
    void Export();

    static ::rtl::OUString GetISODateTimeString(
                        const ::com::sun::star::util::DateTime& rDateTime );

    // ::com::sun::star::xml::sax::XDocumentHandler:
    virtual void SAL_CALL startDocument()
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL endDocument()
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL startElement(const ::rtl::OUString & i_rName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & i_xAttribs)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL endElement(const ::rtl::OUString & i_rName)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL characters(const ::rtl::OUString & i_rChars)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL ignorableWhitespace(
        const ::rtl::OUString & i_rWhitespaces)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL processingInstruction(
        const ::rtl::OUString & i_rTarget, const ::rtl::OUString & i_rData)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL setDocumentLocator(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XLocator > & i_xLocator)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::xml::sax::SAXException);

};

#endif // _XMLOFF_XMLMETAE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
