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

#include "importcontext.hxx"
#include <vector>

namespace sax_fastparser { class FastAttributeList; }

class ScXMLDDELinksContext : public ScXMLImportContext
{
public:
    ScXMLDDELinksContext( ScXMLImport& rImport);

    virtual ~ScXMLDDELinksContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

struct ScDDELinkCell
{
    OUString sValue;
    double fValue;
    bool bString;
    bool bEmpty;
};

typedef std::vector<ScDDELinkCell> ScDDELinkCells;

class ScXMLDDELinkContext : public ScXMLImportContext
{
    ScDDELinkCells  aDDELinkTable;
    ScDDELinkCells  aDDELinkRow;
    OUString   sApplication;
    OUString   sTopic;
    OUString   sItem;
    sal_Int32       nPosition;
    sal_Int32       nColumns;
    sal_Int32       nRows;
    sal_uInt8       nMode;

public:
    ScXMLDDELinkContext( ScXMLImport& rImport);

    virtual ~ScXMLDDELinkContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    void SetApplication(const OUString& sValue) { sApplication = sValue; }
    void SetTopic(const OUString& sValue) { sTopic = sValue; }
    void SetItem(const OUString& sValue) { sItem = sValue; }
    void SetMode(const sal_uInt8 nValue) { nMode = nValue; }
    void CreateDDELink();
    void AddColumns(const sal_Int32 nValue) { nColumns += nValue; }
    void AddRows(const sal_Int32 nValue) { nRows += nValue; }
    void AddCellToRow(const ScDDELinkCell& aCell);
    void AddRowsToTable(const sal_Int32 nRows);

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDDESourceContext : public ScXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

public:
    ScXMLDDESourceContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDESourceContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDDETableContext : public ScXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

public:
    ScXMLDDETableContext( ScXMLImport& rImport,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDETableContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLDDEColumnContext : public ScXMLImportContext
{
public:
    ScXMLDDEColumnContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDEColumnContext() override;
};

class ScXMLDDERowContext : public ScXMLImportContext
{
    ScXMLDDELinkContext*    pDDELink;
    sal_Int32               nRows;

public:
    ScXMLDDERowContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDERowContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDDECellContext : public ScXMLImportContext
{
    OUString        sValue;
    double          fValue;
    sal_Int32       nCells;
    bool            bString;
    bool            bString2;
    bool            bEmpty;

    ScXMLDDELinkContext* pDDELink;

public:
    ScXMLDDECellContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDECellContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
