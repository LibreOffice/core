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

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/FilterOperator.hpp>
#include <com/sun/star/sheet/FilterOperator2.hpp>
#include <com/sun/star/sheet/TableFilterField2.hpp>

#include "xmldrani.hxx"
#include "xmldpimp.hxx"
#include "queryentry.hxx"

#include <stack>
#include <vector>

class ScXMLImport;
struct ScQueryParam;

class ScXMLFilterContext : public SvXMLImportContext
{
    struct ConnStackItem
    {
        bool mbOr;
        int  mnCondCount;
        ConnStackItem(bool bOr);
    };
    ScQueryParam& mrQueryParam;
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    css::table::CellAddress aOutputPosition;
    css::table::CellRangeAddress aConditionSourceRangeAddress;
    bool        bSkipDuplicates;
    bool        bCopyOutputData;
    bool        bConditionSourceRange;
    std::vector<ConnStackItem> maConnStack;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLFilterContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScQueryParam& rParam,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLFilterContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;

    void OpenConnection(bool b);
    void CloseConnection();
    bool GetConnection();
};

class ScXMLAndContext : public SvXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLAndContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                     const OUString& rLName,
                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                     ScQueryParam& rParam,
                     ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLAndContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLOrContext : public SvXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLOrContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                    const OUString& rLName,
                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                    ScQueryParam& rParam,
                    ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLOrContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLConditionContext : public SvXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    ScQueryEntry::QueryItemsType maQueryItems;
    OUString sDataType;
    OUString sConditionValue;
    OUString sOperator;
    sal_Int32   nField;
    bool        bIsCaseSensitive;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLConditionContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                           const OUString& rLName,
                           const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                           ScQueryParam& rParam,
                           ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLConditionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;

    static void GetOperator(const OUString& aOpStr, ScQueryParam& rParam, ScQueryEntry& rEntry);
    void AddSetItem(const ScQueryEntry::Item& rItem);
};

class ScXMLSetItemContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const;
    ScXMLImport& GetScImport();
public:
    ScXMLSetItemContext(ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLConditionContext& rParent);

    virtual ~ScXMLSetItemContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList) override;

    virtual void EndElement() override;
};

// Datapilot (Core)

class ScXMLDPFilterContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    ScQueryParam    aFilterFields;
    ScAddress       aOutputPosition;
    ScRange         aConditionSourceRangeAddress;
    sal_uInt8   nFilterFieldCount;
    bool        bSkipDuplicates:1;
    bool        bCopyOutputData:1;
    bool        bUseRegularExpressions:1;
    bool        bIsCaseSensitive:1;
    bool        bConnectionOr:1;
    bool        bNextConnectionOr:1;
    bool        bConditionSourceRange:1;
    ::std::stack<bool>  aConnectionOrStack;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPFilterContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDataPilotTableContext* pTempDataPilotTableContext);

    virtual ~ScXMLDPFilterContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;

    void SetIsCaseSensitive(const bool bTemp) { bIsCaseSensitive = bTemp; }
    void SetUseRegularExpressions(const bool bTemp) { if (!bUseRegularExpressions) bUseRegularExpressions = bTemp;}

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

class ScXMLDPAndContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPAndContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPAndContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDPOrContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPOrContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPOrContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDPConditionContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    OUString sDataType;
    OUString sConditionValue;
    OUString sOperator;
    sal_Int32   nField;
    bool        bIsCaseSensitive;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDPConditionContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPConditionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    static void getOperatorXML(
        const OUString& sTempOperator, ScQueryOp& aFilterOperator, bool& bUseRegularExpressions);
    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
