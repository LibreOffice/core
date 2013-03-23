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
#if 1

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/TableFilterField2.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/TableOrientation.hpp>

#include "dbdata.hxx"

#include <boost/scoped_ptr.hpp>

class ScDBData;
struct ScQueryParam;
class ScXMLImport;

class ScXMLDatabaseRangesContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDatabaseRangesContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDatabaseRangesContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

struct ScSubTotalRule
{
    sal_Int16 nSubTotalRuleGroupFieldNumber;
    com::sun::star::uno::Sequence <com::sun::star::sheet::SubTotalColumn> aSubTotalColumns;
};

class ScXMLDatabaseRangeContext : public SvXMLImportContext
{
    boost::scoped_ptr<ScQueryParam> mpQueryParam;
    ScRange maRange;
    rtl::OUString   sDatabaseRangeName;
    rtl::OUString   sConnectionResource;
    rtl::OUString   sDatabaseName;
    rtl::OUString   sSourceObject;
    com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue> aSortSequence;
    std::vector < ScSubTotalRule > aSubTotalRules;
    com::sun::star::table::CellRangeAddress aFilterConditionSourceRangeAddress;
    com::sun::star::sheet::DataImportMode nSourceType;
    sal_Int32       nRefresh;
    sal_Int16       nSubTotalsUserListIndex;
    bool            mbValidRange;
    bool            bContainsSort;
    bool            bContainsSubTotal;
    bool            bNative;
    bool            bIsSelection;
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

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    ScDBData* ConvertToDBData(const ::rtl::OUString& rName);

public:

    ScXMLDatabaseRangeContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDatabaseRangeContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetDatabaseName(const rtl::OUString sTempDatabaseName) { sDatabaseName = sTempDatabaseName; }
    void SetConnectionResource(const rtl::OUString sTempConRes) { sConnectionResource = sTempConRes; }
    void SetSourceObject(const rtl::OUString sTempSourceObject) { sSourceObject = sTempSourceObject; }
    void SetSourceType(const com::sun::star::sheet::DataImportMode nTempSourceType) { nSourceType = nTempSourceType; }
    void SetNative(const bool bTempNative) { bNative = bTempNative; }
    void SetSubTotalsBindFormatsToContent(const bool bTemp ) { bSubTotalsBindFormatsToContent = bTemp; }
    void SetSubTotalsIsCaseSensitive(const bool bTemp) { bSubTotalsIsCaseSensitive = bTemp; }
    void SetSubTotalsInsertPageBreaks(const bool bTemp) { bSubTotalsInsertPageBreaks = bTemp; }
    void SetSubTotalsEnabledUserList(const bool bTemp) { bSubTotalsEnabledUserList = bTemp; }
    void SetSubTotalsUserListIndex(const sal_Int16 nTemp) { nSubTotalsUserListIndex = nTemp; }
    void SetSubTotalsAscending(const bool bTemp) { bSubTotalsAscending = bTemp; }
    void SetSubTotalsSortGroups(const bool bTemp) { bSubTotalsSortGroups = bTemp; }
    void AddSubTotalRule(const ScSubTotalRule& rRule) { aSubTotalRules.push_back(rRule); }
    void SetSortSequence(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue>& aTempSortSequence) { aSortSequence = aTempSortSequence; }
    void SetFilterConditionSourceRangeAddress(const com::sun::star::table::CellRangeAddress& aTemp) { aFilterConditionSourceRangeAddress = aTemp;
                                                                                                    bFilterConditionSourceRange = true; }
};

class ScXMLSourceSQLContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    rtl::OUString               sDBName;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSourceSQLContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceSQLContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSourceTableContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    rtl::OUString               sDBName;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSourceTableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSourceQueryContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    rtl::OUString               sDBName;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSourceQueryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSourceQueryContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLConResContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLConResContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLConResContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSubTotalRulesContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSubTotalRulesContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRulesContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSortGroupsContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSortGroupsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSortGroupsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLSubTotalRuleContext : public SvXMLImportContext
{
    ScXMLDatabaseRangeContext*  pDatabaseRangeContext;
    ScSubTotalRule              aSubTotalRule;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSubTotalRuleContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSubTotalRuleContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void AddSubTotalColumn(const com::sun::star::sheet::SubTotalColumn aSubTotalColumn)
    { aSubTotalRule.aSubTotalColumns.realloc(aSubTotalRule.aSubTotalColumns.getLength() + 1);
    aSubTotalRule.aSubTotalColumns[aSubTotalRule.aSubTotalColumns.getLength() - 1] = aSubTotalColumn; }
};

class ScXMLSubTotalFieldContext : public SvXMLImportContext
{
    ScXMLSubTotalRuleContext* pSubTotalRuleContext;
    rtl::OUString sFieldNumber;
    rtl::OUString sFunction;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLSubTotalFieldContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLSubTotalRuleContext* pSubTotalRuleContext);

    virtual ~ScXMLSubTotalFieldContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
