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

#include <com/sun/star/document/XDocumentProperties.hpp>

#include <xmloff/xmlexp.hxx>

class XMLMetaExportComponent final : public SvXMLExport
{
    css::uno::Reference< css::document::XDocumentProperties > mxDocProps;

public:
    XMLMetaExportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString const & implementationName, SvXMLExportFlags nFlags
        );

    virtual ~XMLMetaExportComponent() override;

private:
    // export the events off all autotexts
    virtual ErrCode exportDoc(
        enum ::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID ) override;

    // accept XDocumentProperties in addition to XModel
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // override
    virtual void ExportMeta_() override;

    // methods without content:
    virtual void ExportAutoStyles_() override;
    virtual void ExportMasterStyles_() override;
    virtual void ExportContent_() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
