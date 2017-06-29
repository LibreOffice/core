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

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/table/TableOrientation.hpp>

#include "dbdata.hxx"
#include "xmlimprt.hxx"
#include "importcontext.hxx"

#include <memory>

class ScDBData;
struct ScQueryParam;

class ScXMLDatabaseRangesContext : public ScXMLImportContext
{
public:

    ScXMLDatabaseRangesContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList);

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

    ScXMLDatabaseRangeContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList);

    virtual ~ScXMLDatabaseRangeContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

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

    ScXMLSourceSQLContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceSQLContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLSourceTableContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    OUString               sDBName;

public:

    ScXMLSourceTableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceTableContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLSourceQueryContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    OUString               sDBName;

public:

    ScXMLSourceQueryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceQueryContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLConResContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;

public:

    ScXMLConResContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLConResContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLSubTotalRulesContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

public:

    ScXMLSubTotalRulesContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<
                                        css::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRulesContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLSortGroupsContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

public:

    ScXMLSortGroupsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSortGroupsContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLSubTotalRuleContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    ScSubTotalRule              aSubTotalRule;

public:

    ScXMLSubTotalRuleContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRuleContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;

    void AddSubTotalColumn(const css::sheet::SubTotalColumn& rSubTotalColumn)
    { aSubTotalRule.aSubTotalColumns.realloc(aSubTotalRule.aSubTotalColumns.getLength() + 1);
    aSubTotalRule.aSubTotalColumns[aSubTotalRule.aSubTotalColumns.getLength() - 1] = rSubTotalColumn; }
};

class ScXMLSubTotalFieldContext : public ScXMLImportContext
{
    ScXMLSubTotalRuleContext* pSubTotalRuleContext;
    OUString sFieldNumber;
    OUString sFunction;

public:

    ScXMLSubTotalFieldContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLSubTotalRuleContext* pSubTotalRuleContext);

    virtual ~ScXMLSubTotalFieldContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
