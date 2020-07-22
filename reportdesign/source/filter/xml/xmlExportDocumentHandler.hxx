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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORTDOCUMENTHANDLER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORTDOCUMENTHANDLER_HXX

#include <sal/config.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <comphelper/uno3.hxx>
#include <xmloff/xmltoken.hxx>

namespace rptxml
{

OUString lcl_createAttribute(const xmloff::token::XMLTokenEnum& _eNamespace,const xmloff::token::XMLTokenEnum& _eAttribute);

typedef ::cppu::WeakAggImplHelper3< css::xml::sax::XDocumentHandler
                                ,   css::lang::XInitialization
                                ,   css::lang::XServiceInfo>   ExportDocumentHandler_BASE;

class ExportDocumentHandler : public ExportDocumentHandler_BASE
{
public:
    explicit ExportDocumentHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

private:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    DECLARE_XTYPEPROVIDER( )

    // css::xml::sax::XDocumentHandler:
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL startElement(const OUString & aName, const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs) override;
    virtual void SAL_CALL endElement(const OUString & aName) override;
    virtual void SAL_CALL characters(const OUString & aChars) override;
    virtual void SAL_CALL ignorableWhitespace(const OUString & aWhitespaces) override;
    virtual void SAL_CALL processingInstruction(const OUString & aTarget, const OUString & aData) override;
    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator > & xLocator) override;

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    void exportTableRows();
private:
    ExportDocumentHandler(ExportDocumentHandler const &) = delete;
    void operator =(ExportDocumentHandler const &) = delete;

    virtual ~ExportDocumentHandler() override;

    ::osl::Mutex                                              m_aMutex;
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    css::uno::Reference< css::xml::sax::XDocumentHandler >    m_xDelegatee;
    css::uno::Reference< css::uno::XAggregation >             m_xProxy;
    css::uno::Reference< css::lang::XTypeProvider >           m_xTypeProvider;
    css::uno::Reference< css::lang::XServiceInfo >            m_xServiceInfo;
    css::uno::Reference< css::chart2::XChartDocument >        m_xModel;
    css::uno::Reference< css::chart2::data::XDatabaseDataProvider >   m_xDatabaseDataProvider;
    css::uno::Sequence< OUString >                            m_aColumns;
    sal_Int32                                                 m_nColumnCount;
    bool m_bTableRowsStarted;
    bool m_bFirstRowExported;
    bool m_bCountColumnHeader;
};

} // namespace rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORTDOCUMENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
