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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLFILTI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLFILTI_HXX

#include "importcontext.hxx"
#include <queryentry.hxx>
#include <queryparam.hxx>

#include <stack>
#include <vector>

class ScXMLImport;
class ScXMLDatabaseRangeContext;
class ScXMLDataPilotTableContext;
namespace sax_fastparser { class FastAttributeList; }

class ScXMLFilterContext : public ScXMLImportContext
{
    struct ConnStackItem
    {
        bool const mbOr;
        int  mnCondCount;
        explicit ConnStackItem(bool bOr);
    };
    ScQueryParam& mrQueryParam;
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    ScAddress   aOutputPosition;
    ScRange     aConditionSourceRangeAddress;
    bool        bSkipDuplicates;
    bool        bCopyOutputData;
    bool        bConditionSourceRange;
    std::vector<ConnStackItem> maConnStack;

public:

    ScXMLFilterContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScQueryParam& rParam,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLFilterContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void OpenConnection(bool b);
    void CloseConnection();
    bool GetConnection();
};

class ScXMLAndContext : public ScXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

public:

    ScXMLAndContext( ScXMLImport& rImport,
                     ScQueryParam& rParam,
                     ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLAndContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLOrContext : public ScXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

public:

    ScXMLOrContext( ScXMLImport& rImport,
                    ScQueryParam& rParam,
                    ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLOrContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLConditionContext : public ScXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    ScQueryEntry::QueryItemsType maQueryItems;
    OUString sDataType;
    OUString sConditionValue;
    OUString sOperator;
    sal_Int32   nField;
    bool        bIsCaseSensitive;

public:

    ScXMLConditionContext( ScXMLImport& rImport, sal_Int32 nElement,
                           const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                           ScQueryParam& rParam,
                           ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLConditionContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    static void GetOperator(const OUString& aOpStr, ScQueryParam& rParam, ScQueryEntry& rEntry);
    void AddSetItem(const ScQueryEntry::Item& rItem);
};

class ScXMLSetItemContext : public ScXMLImportContext
{
public:
    ScXMLSetItemContext(ScXMLImport& rImport, sal_Int32 nElement,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLConditionContext& rParent);

    virtual ~ScXMLSetItemContext() override;
};

// Datapilot (Core)

class ScXMLDPFilterContext : public ScXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    ScQueryParam    aFilterFields;
    utl::SearchParam::SearchType eSearchType;
    sal_uInt8   nFilterFieldCount;
    bool        bSkipDuplicates:1;
    bool        bIsCaseSensitive:1;
    bool        bConnectionOr:1;
    bool        bNextConnectionOr:1;
    ::std::stack<bool>  aConnectionOrStack;

public:

    ScXMLDPFilterContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDataPilotTableContext* pTempDataPilotTableContext);

    virtual ~ScXMLDPFilterContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void SetIsCaseSensitive(const bool bTemp) { bIsCaseSensitive = bTemp; }
    void SetSearchType(const utl::SearchParam::SearchType eTemp)
    {
        if (eSearchType == utl::SearchParam::SearchType::Normal)
            eSearchType = eTemp;
    }

    void OpenConnection(const bool bVal)
    {
        bool bTemp = bConnectionOr;
        bConnectionOr = bNextConnectionOr;
        bNextConnectionOr = bVal;
        aConnectionOrStack.push(bTemp);
    }

    void CloseConnection()
    {
        bool bTemp;
        if (aConnectionOrStack.empty())
            bTemp = false;
        else
        {
            bTemp = aConnectionOrStack.top();
            aConnectionOrStack.pop();
        }
        bConnectionOr = bTemp;
        bNextConnectionOr = bTemp;
    }

    bool GetConnection() { bool bTemp = bConnectionOr; bConnectionOr = bNextConnectionOr; return bTemp; }
    void AddFilterField (const ScQueryEntry& aFilterField);
};

class ScXMLDPAndContext : public ScXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;
public:

    ScXMLDPAndContext( ScXMLImport& rImport,
                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPAndContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDPOrContext : public ScXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;
public:

    ScXMLDPOrContext( ScXMLImport& rImport,
                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPOrContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDPConditionContext : public ScXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    OUString sDataType;
    OUString sConditionValue;
    OUString sOperator;
    sal_Int32   nField;
    bool        bIsCaseSensitive;

public:

    ScXMLDPConditionContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPConditionContext() override;

    static void getOperatorXML(
        const OUString& sTempOperator, ScQueryOp& aFilterOperator, utl::SearchParam::SearchType& rSearchType);
    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
