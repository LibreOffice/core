/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_XMLMETAE_HXX
#define INCLUDED_XMLOFF_XMLMETAE_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>

#include <cppuhelper/implbase1.hxx>
#include <xmloff/xmltoken.hxx>

#include <vector>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


class SvXMLExport;

/** export meta data from an XDocumentProperties instance.

    <p>
    This class will start the export at the office:meta element,
    not at the root element. This means that when <method>Export</method>
    is called here, the document root element must already be written, but
    office:meta must <em>not</em> be written.
    </p>
 */
class XMLOFF_DLLPUBLIC SvXMLMetaExport : public ::cppu::WeakImplHelper1<
                css::xml::sax::XDocumentHandler >
{
private:
    SvXMLExport& mrExport;
    css::uno::Reference< css::document::XDocumentProperties> mxDocProps;
    /// counts levels of the xml document. necessary for special handling.
    int m_level;
    /// preserved namespaces. necessary because we do not write the root node.
    std::vector< css::beans::StringPair > m_preservedNSs;

    SAL_DLLPRIVATE void SimpleStringElement(
        const OUString& rText, sal_uInt16 nNamespace,
        enum ::xmloff::token::XMLTokenEnum eElementName );
    SAL_DLLPRIVATE void SimpleDateTimeElement(
        const css::util::DateTime & rDate, sal_uInt16 nNamespace,
        enum ::xmloff::token::XMLTokenEnum eElementName );

    /// currently unused; for exporting via the XDocumentProperties interface
    SAL_DLLPRIVATE void _MExport();

public:
    SvXMLMetaExport( SvXMLExport& i_rExport,
        const css::uno::Reference< css::document::XDocumentProperties>& i_rDocProps);

    virtual ~SvXMLMetaExport();

    /// export via XSAXWriter interface, with fallback to _MExport
    void Export();

    static OUString GetISODateTimeString(
                        const css::util::DateTime& rDateTime );

    // css::xml::sax::XDocumentHandler:
    virtual void SAL_CALL startDocument()
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endDocument()
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL startElement(const OUString & i_rName,
        const css::uno::Reference<
                css::xml::sax::XAttributeList > & i_xAttribs)
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endElement(const OUString & i_rName)
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL characters(const OUString & i_rChars)
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace(
        const OUString & i_rWhitespaces)
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL processingInstruction(
        const OUString & i_rTarget, const OUString & i_rData)
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference<
                css::xml::sax::XLocator > & i_xLocator)
        throw (css::uno::RuntimeException,
               css::xml::sax::SAXException, std::exception) override;

};

#endif // INCLUDED_XMLOFF_XMLMETAE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
