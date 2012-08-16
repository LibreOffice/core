/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XMLFILTI_HXX
#define SC_XMLFILTI_HXX

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

    com::sun::star::table::CellAddress aOutputPosition;
    com::sun::star::table::CellRangeAddress aConditionSourceRangeAddress;
    sal_Int16   nUserListIndex;
    bool        bSkipDuplicates;
    bool        bCopyOutputData;
    bool        bEnabledUserList;
    bool        bConditionSourceRange;
    std::vector<ConnStackItem> maConnStack;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLFilterContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScQueryParam& rParam,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLFilterContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void OpenConnection(bool b);
    void CloseConnection();
    bool GetConnection();
};

class ScXMLAndContext : public SvXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLAndContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                     const ::rtl::OUString& rLName,
                     const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                     ScQueryParam& rParam,
                     ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLAndContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLOrContext : public SvXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLOrContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName,
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                    ScQueryParam& rParam,
                    ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLOrContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLConditionContext : public SvXMLImportContext
{
    ScQueryParam& mrQueryParam;
    ScXMLFilterContext* pFilterContext;

    ScQueryEntry::QueryItemsType maQueryItems;
    rtl::OUString sDataType;
    rtl::OUString sConditionValue;
    rtl::OUString sOperator;
    sal_Int32   nField;
    bool        bIsCaseSensitive;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLConditionContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                           const ::rtl::OUString& rLName,
                           const ::com::sun::star::uno::Reference<
                               ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                           ScQueryParam& rParam,
                           ScXMLFilterContext* pTempFilterContext);

    virtual ~ScXMLConditionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void GetOperator(const rtl::OUString& aOpStr, ScQueryParam& rParam, ScQueryEntry& rEntry) const;
    void AddSetItem(const ScQueryEntry::Item& rItem);
};

class ScXMLSetItemContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const;
    ScXMLImport& GetScImport();
public:
    ScXMLSetItemContext(ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLConditionContext& rParent);

    virtual ~ScXMLSetItemContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual void EndElement();
};

// Datapilot (Core)

class ScXMLDPFilterContext : public SvXMLImportContext
{
    ScXMLDataPilotTableContext* pDataPilotTable;

    ScQueryParam    aFilterFields;
    ScAddress       aOutputPosition;
    ScRange         aConditionSourceRangeAddress;
    sal_uInt8   nFilterFieldCount;
    sal_Int16   nUserListIndex;
    bool        bSkipDuplicates:1;
    bool        bCopyOutputData:1;
    bool        bUseRegularExpressions:1;
    bool        bIsCaseSensitive:1;
    bool        bEnabledUserList:1;
    bool        bConnectionOr:1;
    bool        bNextConnectionOr:1;
    bool        bConditionSourceRange:1;
    ::std::stack<bool>  aConnectionOrStack;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPFilterContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTableContext);

    virtual ~ScXMLDPFilterContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

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

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPAndContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPAndContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDPOrContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPOrContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPOrContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDPConditionContext : public SvXMLImportContext
{
    ScXMLDPFilterContext* pFilterContext;

    rtl::OUString sDataType;
    rtl::OUString sConditionValue;
    rtl::OUString sOperator;
    sal_Int32   nField;
    bool        bIsCaseSensitive;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLDPConditionContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext);

    virtual ~ScXMLDPConditionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void getOperatorXML(
        const rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, bool& bUseRegularExpressions) const;
    virtual void EndElement();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
