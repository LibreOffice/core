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
#pragma once

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <cppuhelper/implbase.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>

class XMLVersionListExport final : public SvXMLExport
{
private:
    const css::uno::Sequence < css::util::RevisionTag >& maVersions;
public:
    XMLVersionListExport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Sequence < css::util::RevisionTag >& rVersions,
        const OUString &rFileName,
        css::uno::Reference< css::xml::sax::XDocumentHandler > const &rHandler );

    ErrCode     exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) override;
    void        ExportAutoStyles_() override {}
    void        ExportMasterStyles_ () override {}
    void        ExportContent_() override {}
};

class XMLVersionListImport final : public SvXMLImport
{
private:
    css::uno::Sequence < css::util::RevisionTag >& maVersions;

    virtual SvXMLImportContext *CreateFastContext( sal_Int32 Element,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

public:

    XMLVersionListImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        css::uno::Sequence < css::util::RevisionTag >& rVersions );
    virtual ~XMLVersionListImport() throw() override;

    css::uno::Sequence < css::util::RevisionTag >&
        GetList() { return maVersions; }
};

class XMLVersionListContext final : public SvXMLImportContext
{
private:
    XMLVersionListImport & GetImport() { return static_cast<XMLVersionListImport&>(SvXMLImportContext::GetImport()); }

public:

    XMLVersionListContext( XMLVersionListImport& rImport );

    virtual ~XMLVersionListContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext(sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttribs) override;
};

class XMLVersionContext final : public SvXMLImportContext
{
private:
    static bool         ParseISODateTimeString(
                                const OUString& rString,
                                css::util::DateTime& rDateTime );

public:

    XMLVersionContext( XMLVersionListImport& rImport,
                          const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );

    virtual ~XMLVersionContext() override;
};

class XMLVersionListPersistence final : public ::cppu::WeakImplHelper< css::document::XDocumentRevisionListPersistence, css::lang::XServiceInfo >
{
public:
    virtual css::uno::Sequence< css::util::RevisionTag > SAL_CALL load( const css::uno::Reference< css::embed::XStorage >& Storage ) override;
    virtual void SAL_CALL store( const css::uno::Reference< css::embed::XStorage >& Storage, const css::uno::Sequence< css::util::RevisionTag >& List ) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
