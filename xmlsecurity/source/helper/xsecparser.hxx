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

#ifndef _XSEC_CTL_PARSER_HXX
#define _XSEC_CTL_PARSER_HXX

#include <xsecctl.hxx>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <cppuhelper/implbase2.hxx>

class XSecParser: public cppu::WeakImplHelper2
<
    com::sun::star::xml::sax::XDocumentHandler,
    com::sun::star::lang::XInitialization
>
/****** XSecController.hxx/CLASS XSecParser ***********************************
 *
 *   NAME
 *  XSecParser -- a SAX parser that can detect security elements
 *
 *   FUNCTION
 *  The XSecParser object is connected on the SAX chain and detects
 *  security elements in the SAX event stream, then notifies
 *  the XSecController.
 *
 *   NOTES
 *  This class is used when importing a document.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    friend class XSecController;
private:
    /*
     * the following members are used to reserve the signature information,
     * including X509IssuerName, X509SerialNumber, and X509Certificate,etc.
     */
    rtl::OUString m_ouX509IssuerName;
    rtl::OUString m_ouX509SerialNumber;
    rtl::OUString m_ouX509Certificate;
    rtl::OUString m_ouDigestValue;
    rtl::OUString m_ouSignatureValue;
    rtl::OUString m_ouDate;

    /*
     * whether inside a particular element
     */
    bool m_bInX509IssuerName;
    bool m_bInX509SerialNumber;
    bool m_bInX509Certificate;
    bool m_bInDigestValue;
    bool m_bInSignatureValue;
    bool m_bInDate;

    /*
     * the XSecController collaborating with XSecParser
     */
    XSecController* m_pXSecController;

    /*
     * the next XDocumentHandler on the SAX chain
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > m_xNextHandler;

    /*
     * this string is used to remember the current handled reference's URI,
     *
     * because it can be decided whether a stream reference is xml based or binary based
     * only after the Transforms element is read in, so we have to reserve the reference's
     * URI when the startElement event is met.
     */
    rtl::OUString m_currentReferenceURI;
    bool m_bReferenceUnresolved;

private:
    rtl::OUString getIdAttr(const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttribs );

public:
    XSecParser( XSecController* pXSecController,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xNextHandler );
    ~XSecParser(){};

    /*
     * XDocumentHandler
     */
    virtual void SAL_CALL startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL startElement(
        const rtl::OUString& aName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endElement( const rtl::OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL characters( const rtl::OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL ignorableWhitespace( const rtl::OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL processingInstruction(
        const rtl::OUString& aTarget,
        const rtl::OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setDocumentLocator(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XLocator >& xLocator )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    /*
     * XInitialization
     */
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw(com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
