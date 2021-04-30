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

#ifndef XMLOFF_INC_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX
#define XMLOFF_INC_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

class XMLEmbeddedObjectExportFilter final : public cppu::WeakImplHelper<
             css::xml::sax::XExtendedDocumentHandler,
             css::lang::XServiceInfo,
             css::lang::XInitialization>
{
    css::uno::Reference< css::xml::sax::XDocumentHandler >         xHandler;
    css::uno::Reference< css::xml::sax::XExtendedDocumentHandler > xExtHandler;

public:
    XMLEmbeddedObjectExportFilter( const css::uno::Reference< css::xml::sax::XDocumentHandler > & rHandler ) noexcept;
    virtual ~XMLEmbeddedObjectExportFilter () noexcept override;

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL startElement(const OUString& aName,
                              const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs) override;
    virtual void SAL_CALL endElement(const OUString& aName) override;
    virtual void SAL_CALL characters(const OUString& aChars) override;
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                       const OUString& aData) override;
    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator > & xLocator) override;

    // css::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA() override;
    virtual void SAL_CALL endCDATA() override;
    virtual void SAL_CALL comment(const OUString& sComment) override;
    virtual void SAL_CALL allowLineBreak() override;
    virtual void SAL_CALL unknown(const OUString& sString) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

};

#endif // XMLOFF_INC_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
