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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLTABI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLTABI_HXX

#include <externalrefmgr.hxx>
#include "importcontext.hxx"

#include <xmloff/xmlictxt.hxx>
#include <memory>

namespace sax_fastparser { class FastAttributeList; }


struct ScXMLExternalTabData
{
    ScExternalRefCache::TableTypeRef mpCacheTable;
    sal_Int32 mnRow;
    sal_Int32 mnCol;
    sal_uInt16 mnFileId;

    ScXMLExternalTabData();
};

class ScXMLTableContext : public ScXMLImportContext
{
    OUString   sPrintRanges;
    ::std::unique_ptr<ScXMLExternalTabData> pExternalRefInfo;
    sal_Int32       nStartOffset;
    bool            bStartFormPage;
    bool            bPrintEntireSheet;

public:

    ScXMLTableContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual ~ScXMLTableContext() override;

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class ScXMLTableProtectionContext : public ScXMLImportContext
{
public:
    ScXMLTableProtectionContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual ~ScXMLTableProtectionContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
