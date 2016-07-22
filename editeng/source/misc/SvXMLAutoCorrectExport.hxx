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
#ifndef INCLUDED_EDITENG_SOURCE_MISC_SVXMLAUTOCORRECTEXPORT_HXX
#define INCLUDED_EDITENG_SOURCE_MISC_SVXMLAUTOCORRECTEXPORT_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/svxacorr.hxx>

class SvXMLAutoCorrectExport : public SvXMLExport
{
private:
    const SvxAutocorrWordList   *pAutocorr_List;
public:
    SvXMLAutoCorrectExport(
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        const SvxAutocorrWordList * pNewAutocorr_List,
        const OUString &rFileName,
        css::uno::Reference< css::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SvXMLAutoCorrectExport() {}
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID) override;
    void ExportAutoStyles_() override {}
    void ExportMasterStyles_ () override {}
    void ExportContent_() override {}
};

class SvStringsISortDtor;

class SvXMLExceptionListExport : public SvXMLExport
{
private:
    const SvStringsISortDtor & rList;
public:
    SvXMLExceptionListExport(
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        const SvStringsISortDtor &rNewList,
        const OUString &rFileName,
        css::uno::Reference< css::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SvXMLExceptionListExport() {}
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID) override;
    void ExportAutoStyles_() override {}
    void ExportMasterStyles_ () override {}
    void ExportContent_() override {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
