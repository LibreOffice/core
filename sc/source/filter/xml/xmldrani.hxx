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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLDRANI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLDRANI_HXX

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <dbdata.hxx>
#include "importcontext.hxx"

#include <memory>

namespace sax_fastparser { class FastAttributeList; }

struct ScQueryParam;

class ScXMLDatabaseRangesContext : public ScXMLImportContext
{
public:

    ScXMLDatabaseRangesContext( ScXMLImport& rImport );

    virtual ~ScXMLDatabaseRangesContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
                        sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

struct ScSubTotalRule
{
    sal_Int16 nSubTotalRuleGroupFieldNumber;
    css::uno::Sequence <css::sheet::SubTotalColumn> aSubTotalColumns;
};

class ScXMLDatabaseRangeContext : public ScXMLImportContext
{
    std::unique_ptr<ScQueryParam> mpQueryParam;
    ScRange         maRange;
    OUString        sDatabaseRangeName;
    OUString        sConnectionResource;
    OUString        sDatabaseName;
    OUString        sSourceObject;
    css::uno::Sequence <css::beans::PropertyValue> aSortSequence;
    std::vector < ScSubTotalRule > aSubTotalRules;
    ScRange         aFilterConditionSourceRangeAddress;
    css::sheet::DataImportMode nSourceType;
    sal_Int32       nRefresh;
    sal_Int16       nSubTotalsUserListIndex;
    bool            mbValidRange;
    bool            bContainsSort;
    bool            bContainsSubTotal;
    bool            bNative;
    bool            bIsSelection; // TODO: import to document core
    bool            bKeepFormats;
    bool            bMoveCells;
    bool            bStripData;
    bool            bAutoFilter;
    bool            bSubTotalsBindFormatsToContent;
    bool            bSubTotalsIsCaseSensitive;
    bool            bSubTotalsInsertPageBreaks;
    bool            bSubTotalsSortGroups;
    bool            bSubTotalsEnabledUserList;
    bool            bSubTotalsAscending;
    bool            bFilterConditionSourceRange;
    bool            bHasHeader;
    bool            bByRow;
    ScDBCollection::RangeType meRangeType;

    std::unique_ptr<ScDBData> ConvertToDBData(const OUString& rName);

public:

    ScXMLDatabaseRangeContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual ~ScXMLDatabaseRangeContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void SetDatabaseName(const OUString& sTempDatabaseName) { sDatabaseName = sTempDatabaseName; }
    void SetConnectionResource(const OUString& sTempConRes) { sConnectionResource = sTempConRes; }
    void SetSourceObject(const OUString& sTempSourceObject) { sSourceObject = sTempSourceObject; }
    void SetSourceType(const css::sheet::DataImportMode nTempSourceType) { nSourceType = nTempSourceType; }
    void SetNative(const bool bTempNative) { bNative = bTempNative; }
    void SetSubTotalsBindFormatsToContent(const bool bTemp ) { bSubTotalsBindFormatsToContent = bTemp; }
    void SetSubTotalsIsCaseSensitive(const bool bTemp) { bSubTotalsIsCaseSensitive = bTemp; }
    void SetSubTotalsInsertPageBreaks(const bool bTemp) { bSubTotalsInsertPageBreaks = bTemp; }
    void SetSubTotalsEnabledUserList(const bool bTemp) { bSubTotalsEnabledUserList = bTemp; }
    void SetSubTotalsUserListIndex(const sal_Int16 nTemp) { nSubTotalsUserListIndex = nTemp; }
    void SetSubTotalsAscending(const bool bTemp) { bSubTotalsAscending = bTemp; }
    void SetSubTotalsSortGroups(const bool bTemp) { bSubTotalsSortGroups = bTemp; }
    void AddSubTotalRule(const ScSubTotalRule& rRule) { aSubTotalRules.push_back(rRule); }
    void SetSortSequence(const css::uno::Sequence <css::beans::PropertyValue>& aTempSortSequence) { aSortSequence = aTempSortSequence; }
    void SetFilterConditionSourceRangeAddress(const ScRange& aRange) { aFilterConditionSourceRangeAddress = aRange;
                                                                       bFilterConditionSourceRange = true; }
};

class ScXMLSourceSQLContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    OUString               sDBName;

public:

    ScXMLSourceSQLContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceSQLContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLSourceTableContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    OUString               sDBName;

public:

    ScXMLSourceTableContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceTableContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLSourceQueryContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    OUString               sDBName;

public:

    ScXMLSourceQueryContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceQueryContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLConResContext : public ScXMLImportContext
{
public:

    ScXMLConResContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLConResContext() override;
};

class ScXMLSubTotalRulesContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

public:

    ScXMLSubTotalRulesContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRulesContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLSortGroupsContext : public ScXMLImportContext
{
public:

    ScXMLSortGroupsContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSortGroupsContext() override;
};

class ScXMLSubTotalRuleContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    ScSubTotalRule              aSubTotalRule;

public:

    ScXMLSubTotalRuleContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRuleContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void AddSubTotalColumn(const css::sheet::SubTotalColumn& rSubTotalColumn)
    {
        aSubTotalRule.aSubTotalColumns.realloc(aSubTotalRule.aSubTotalColumns.getLength() + 1);
        aSubTotalRule.aSubTotalColumns[aSubTotalRule.aSubTotalColumns.getLength() - 1] = rSubTotalColumn;
    }
};

class ScXMLSubTotalFieldContext : public ScXMLImportContext
{
    ScXMLSubTotalRuleContext* pSubTotalRuleContext;
    OUString sFieldNumber;
    OUString sFunction;

public:

    ScXMLSubTotalFieldContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLSubTotalRuleContext* pSubTotalRuleContext);

    virtual ~ScXMLSubTotalFieldContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
