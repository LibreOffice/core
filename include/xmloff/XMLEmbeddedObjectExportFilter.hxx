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

#ifndef INCLUDED_XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX
#define INCLUDED_XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>

class XMLEmbeddedObjectExportFilter : public ::cppu::WeakImplHelper3<
             ::com::sun::star::xml::sax::XExtendedDocumentHandler,
             ::com::sun::star::lang::XServiceInfo,
             ::com::sun::star::lang::XInitialization>
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > xHandler;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XExtendedDocumentHandler > xExtHandler;

public:
    XMLEmbeddedObjectExportFilter( const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > & rHandler ) throw();
    virtual ~XMLEmbeddedObjectExportFilter () throw();

    // ::com::sun::star::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endDocument()
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL startElement(const OUString& aName,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endElement(const OUString& aName)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL characters(const OUString& aChars)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                       const OUString& aData)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA() throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endCDATA() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL comment(const OUString& sComment)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL allowLineBreak()
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL unknown(const OUString& sString)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

};

#endif // INCLUDED_XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
