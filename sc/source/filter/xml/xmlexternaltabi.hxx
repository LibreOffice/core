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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLEXTERNALTABI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLEXTERNALTABI_HXX

#include <xmloff/xmlictxt.hxx>
#include <rtl/ustrbuf.hxx>
#include "importcontext.hxx"

class ScXMLImport;
struct ScXMLExternalTabData;

class ScXMLExternalRefTabSourceContext : public ScXMLImportContext
{
public:
    ScXMLExternalRefTabSourceContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefTabSourceContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual css::uno::Reference< XFastContextHandler > SAL_CALL createFastChildContext(
                        sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;

    OUString         maRelativeUrl;
    OUString         maTableName;
    OUString         maFilterName;
    OUString         maFilterOptions;
};

class ScXMLExternalRefRowsContext : public ScXMLImportContext
{
public:
    ScXMLExternalRefRowsContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefRowsContext() override;

    virtual css::uno::Reference< XFastContextHandler > SAL_CALL createFastChildContext(
                        sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;
};

class ScXMLExternalRefRowContext : public ScXMLImportContext
{
public:
    ScXMLExternalRefRowContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefRowContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual css::uno::Reference< XFastContextHandler > SAL_CALL createFastChildContext(
                                        sal_Int32 nElement,
                                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;
    sal_Int32               mnRepeatRowCount;
};

class ScXMLExternalRefCellContext : public ScXMLImportContext
{
public:
    ScXMLExternalRefCellContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLExternalTabData& rRefInfo );

    virtual ~ScXMLExternalRefCellContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual css::uno::Reference< XFastContextHandler > SAL_CALL createFastChildContext(
                        sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    void SetCellString(const OUString& rStr);

private:
    ScXMLImport&            mrScImport;
    ScXMLExternalTabData&   mrExternalRefInfo;
    OUString         maCellString;
    double                  mfCellValue;
    sal_Int32               mnRepeatCount;
    sal_Int32               mnNumberFormat;
    sal_Int16               mnCellType;
    bool                    mbIsNumeric;
    bool                    mbIsEmpty;
};

class ScXMLExternalRefCellTextContext : public ScXMLImportContext
{
public:
    ScXMLExternalRefCellTextContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLExternalRefCellContext& rParent );

    virtual ~ScXMLExternalRefCellTextContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    virtual void SAL_CALL characters( const OUString& rChars ) override;

    virtual css::uno::Reference< XFastContextHandler > SAL_CALL createFastChildContext(
                        sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    ScXMLExternalRefCellContext& mrParent;

    OUStringBuffer   maCellStrBuf;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
