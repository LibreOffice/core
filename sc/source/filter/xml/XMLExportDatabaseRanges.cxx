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

#include "XMLExportDatabaseRanges.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <sax/tools/converter.hxx>
#include "xmlexprt.hxx"
#include "XMLExportIterator.hxx"
#include "XMLConverter.hxx"
#include "unonames.hxx"
#include "dbdata.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "XMLExportSharedData.hxx"
#include "rangeutl.hxx"
#include "subtotalparam.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"

#include "svx/dataaccessdescriptor.hxx"

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/table/TableSortField.hpp>
#include <com/sun/star/table/TableSortFieldType.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <comphelper/extract.hxx>

#include <map>

//! not found in unonames.hxx
#define SC_USERLIST "UserList"

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLExportDatabaseRanges::ScXMLExportDatabaseRanges(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pDoc( NULL )
{
}

ScXMLExportDatabaseRanges::~ScXMLExportDatabaseRanges()
{
}

ScMyEmptyDatabaseRangesContainer ScXMLExportDatabaseRanges::GetEmptyDatabaseRanges()
{
    ScMyEmptyDatabaseRangesContainer aSkipRanges;
    if (rExport.GetModel().is())
    {
        uno::Reference <beans::XPropertySet> xPropertySet (rExport.GetModel(), uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Reference <sheet::XDatabaseRanges> xDatabaseRanges(xPropertySet->getPropertyValue(OUString(SC_UNO_DATABASERNG)), uno::UNO_QUERY);
            rExport.CheckAttrList();
            if (xDatabaseRanges.is())
            {
                uno::Sequence <OUString> aRanges(xDatabaseRanges->getElementNames());
                sal_Int32 nDatabaseRangesCount = aRanges.getLength();
                for (sal_Int32 i = 0; i < nDatabaseRangesCount; ++i)
                {
                    OUString sDatabaseRangeName(aRanges[i]);
                    uno::Reference <sheet::XDatabaseRange> xDatabaseRange(xDatabaseRanges->getByName(sDatabaseRangeName), uno::UNO_QUERY);
                    if (xDatabaseRange.is())
                    {
                        uno::Reference <beans::XPropertySet> xDatabaseRangePropertySet (xDatabaseRange, uno::UNO_QUERY);
                        if (xDatabaseRangePropertySet.is() &&
                            ::cppu::any2bool(xDatabaseRangePropertySet->getPropertyValue(OUString(SC_UNONAME_STRIPDAT))))
                        {
                            uno::Sequence <beans::PropertyValue> aImportProperties(xDatabaseRange->getImportDescriptor());
                            sal_Int32 nLength = aImportProperties.getLength();
                            sheet::DataImportMode nSourceType = sheet::DataImportMode_NONE;
                            for (sal_Int32 j = 0; j < nLength; ++j)
                                if ( aImportProperties[j].Name == SC_UNONAME_SRCTYPE )
                                    aImportProperties[j].Value >>= nSourceType;
                            if (nSourceType != sheet::DataImportMode_NONE)
                            {
                                table::CellRangeAddress aArea = xDatabaseRange->getDataArea();
                                aSkipRanges.AddNewEmptyDatabaseRange(aArea);

                                // #105276#; set last row/column so default styles are collected
                                rExport.GetSharedData()->SetLastColumn(aArea.Sheet, aArea.EndColumn);
                                rExport.GetSharedData()->SetLastRow(aArea.Sheet, aArea.EndRow);
                            }
                        }
                    }
                }
            }
        }
    }
    return aSkipRanges;
}

namespace {

class WriteDatabaseRange : public ::std::unary_function<ScDBData, void>
{
    ScXMLExport& mrExport;
    ScDocument* mpDoc;
    sal_Int32 mnCounter;
    ScDBCollection::RangeType meRangeType;
public:

    WriteDatabaseRange(ScXMLExport& rExport, ScDocument* pDoc) :
        mrExport(rExport), mpDoc(pDoc), mnCounter(0), meRangeType(ScDBCollection::GlobalNamed) {}

    void setRangeType(ScDBCollection::RangeType eNew)
    {
        meRangeType = eNew;
    }

    void operator() (const ::std::pair<SCTAB, const ScDBData*>& r)
    {
        if (meRangeType != ScDBCollection::SheetAnonymous)
            return;

        // name
        OUStringBuffer aBuf;
        aBuf.appendAscii(STR_DB_LOCAL_NONAME);
        aBuf.append(static_cast<sal_Int32>(r.first)); // appended number equals sheet index on import.

        write(aBuf.makeStringAndClear(), *r.second);
    }

    void operator() (const ScDBData& rData)
    {
        if (meRangeType == ScDBCollection::GlobalAnonymous)
        {
            // name
            OUStringBuffer aBuf;
            aBuf.appendAscii(STR_DB_GLOBAL_NONAME);
            aBuf.append(++mnCounter); // 1-based, for entirely arbitrary reasons.  The numbers are ignored on import.

            write(aBuf.makeStringAndClear(), rData);
        }
        else if (meRangeType == ScDBCollection::GlobalNamed)
            write(rData.GetName(), rData);
    }

private:
    void write(const OUString& rName, const ScDBData& rData)
    {
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, rName);

        // range
        ScRange aRange;
        rData.GetArea(aRange);
        OUString aRangeStr;
        ScRangeStringConverter::GetStringFromRange(
            aRangeStr, aRange, mpDoc, ::formula::FormulaGrammar::CONV_OOO);
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, aRangeStr);

        // various boolean flags.
        if (rData.HasImportSelection())
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_SELECTION, XML_TRUE);
        if (rData.HasAutoFilter())
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_FILTER_BUTTONS, XML_TRUE);
        if (rData.IsKeepFmt())
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_STYLES, XML_TRUE);
        if (rData.IsDoSize())
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_SIZE, XML_FALSE);
        if (rData.IsStripData())
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_HAS_PERSISTENT_DATA, XML_FALSE);

        ScQueryParam aQueryParam;
        rData.GetQueryParam(aQueryParam);
        if (!aQueryParam.bHasHeader)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONTAINS_HEADER, XML_FALSE);

        ScSortParam aSortParam;
        rData.GetSortParam(aSortParam);
        if (!aSortParam.bByRow)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, XML_COLUMN);

        sal_Int32 nRefresh = rData.GetRefreshDelay();
        if (nRefresh)
        {
            OUStringBuffer aBuf;
            ::sax::Converter::convertDuration(aBuf,
                static_cast<double>(nRefresh) / 86400.0);
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, aBuf.makeStringAndClear());
        }

        SvXMLElementExport aElemDR(mrExport, XML_NAMESPACE_TABLE, XML_DATABASE_RANGE, true, true);

        writeImport(rData);
        writeFilter(rData);
        writeSort(rData);
        writeSubtotals(rData);
    }

    void writeImport(const ScDBData& rData)
    {
        ScImportParam aParam;
        rData.GetImportParam(aParam);

        OUString sDatabaseName;
        OUString sConRes;

        ::svx::ODataAccessDescriptor aDescriptor;
        aDescriptor.setDataSource(aParam.aDBName);
        if (aDescriptor.has(::svx::daDataSource))
        {
            sDatabaseName = aParam.aDBName;
        }
        else if (aDescriptor.has(::svx::daConnectionResource))
        {
            sConRes = aParam.aDBName;
        }

        sheet::DataImportMode nSourceType = sheet::DataImportMode_NONE;
        if (aParam.bImport)
        {
            if (aParam.bSql)
                nSourceType = sheet::DataImportMode_SQL;
            else if (aParam.nType == ScDbQuery)
                nSourceType = sheet::DataImportMode_QUERY;
            else
                nSourceType = sheet::DataImportMode_TABLE;
        }

        switch (nSourceType)
        {
            case sheet::DataImportMode_NONE : break;
            case sheet::DataImportMode_QUERY :
            {
                if (!sDatabaseName.isEmpty())
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, sDatabaseName);
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_QUERY_NAME, aParam.aStatement);
                SvXMLElementExport aElemID(mrExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY, true, true);
                if (!sConRes.isEmpty())
                {
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sConRes );
                    SvXMLElementExport aElemCR(mrExport, XML_NAMESPACE_FORM, XML_CONNECTION_RESOURCE, true, true);
                }
            }
            break;
            case sheet::DataImportMode_TABLE :
            {
                if (!sDatabaseName.isEmpty())
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, sDatabaseName);
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, aParam.aStatement);
                SvXMLElementExport aElemID(mrExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE, true, true);
                if (!sConRes.isEmpty())
                {
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sConRes );
                    SvXMLElementExport aElemCR(mrExport, XML_NAMESPACE_FORM, XML_CONNECTION_RESOURCE, true, true);
                }
            }
            break;
            case sheet::DataImportMode_SQL :
            {
                if (!sDatabaseName.isEmpty())
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, sDatabaseName);
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SQL_STATEMENT, aParam.aStatement);
                if (!aParam.bNative)
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TRUE);
                SvXMLElementExport aElemID(mrExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, true, true);
                if (!sConRes.isEmpty())
                {
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sConRes );
                    SvXMLElementExport aElemCR(mrExport, XML_NAMESPACE_FORM, XML_CONNECTION_RESOURCE, true, true);
                }
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    void writeSort(const ScDBData& rData)
    {
        ScSortParam aParam;
        rData.GetSortParam(aParam);

        // Count sort items first.
        size_t nSortCount = 0;
        for (; nSortCount < aParam.GetSortKeyCount(); ++nSortCount)
        {
            if (!aParam.maKeyState[nSortCount].bDoSort)
                break;
        }

        if (!nSortCount)
            // Nothing to export.
            return;

        ScAddress aOutPos(aParam.nDestCol, aParam.nDestRow, aParam.nDestTab);

        if (!aParam.bIncludePattern)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT, XML_FALSE);

        if (!aParam.bInplace)
        {
            OUString aStr;
            ScRangeStringConverter::GetStringFromAddress(
                aStr, aOutPos, mpDoc, ::formula::FormulaGrammar::CONV_OOO);
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, aStr);
        }

        if (aParam.bCaseSens)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);

        mrExport.AddLanguageTagAttributes( XML_NAMESPACE_TABLE, XML_NAMESPACE_TABLE, aParam.aCollatorLocale, false);
        if (!aParam.aCollatorAlgorithm.isEmpty())
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ALGORITHM, aParam.aCollatorAlgorithm);

        SvXMLElementExport aElemS(mrExport, XML_NAMESPACE_TABLE, XML_SORT, true, true);

        ScRange aRange;
        rData.GetArea(aRange);
        SCCOLROW nFieldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();

        for (size_t i = 0; i < nSortCount; ++i)
        {
            // Convert field value from absolute to relative.
            SCCOLROW nField = aParam.maKeyState[i].nField - nFieldStart;
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, OUString::number(nField));

            if (!aParam.maKeyState[i].bAscending)
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORDER, XML_DESCENDING);

            if (aParam.bUserDef)
            {
                OUStringBuffer aBuf;
                aBuf.appendAscii(SC_USERLIST);
                aBuf.append(aParam.nUserIndex);
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, aBuf.makeStringAndClear());
            }
            else
            {
                // Right now we only support automatic field type.  In the
                // future we may support numeric or alphanumeric field type.
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_AUTOMATIC);
            }

            SvXMLElementExport aElemSb(mrExport, XML_NAMESPACE_TABLE, XML_SORT_BY, true, true);
        }
    }

    OUString getOperatorXML(const ScQueryEntry& rEntry, bool bRegExp) const
    {
        switch (rEntry.eOp)
        {
            case SC_BEGINS_WITH:
                return GetXMLToken(XML_BEGINS_WITH);
            case SC_BOTPERC:
                return GetXMLToken(XML_BOTTOM_PERCENT);
            case SC_BOTVAL:
                return GetXMLToken(XML_BOTTOM_VALUES);
            case SC_CONTAINS:
                return GetXMLToken(XML_CONTAINS);
            case SC_DOES_NOT_BEGIN_WITH:
                return GetXMLToken(XML_DOES_NOT_BEGIN_WITH);
            case SC_DOES_NOT_CONTAIN:
                return GetXMLToken(XML_DOES_NOT_CONTAIN);
            case SC_DOES_NOT_END_WITH:
                return GetXMLToken(XML_DOES_NOT_END_WITH);
            case SC_ENDS_WITH:
                return GetXMLToken(XML_ENDS_WITH);
            case SC_EQUAL:
            {
                if (rEntry.IsQueryByEmpty())
                    return GetXMLToken(XML_EMPTY);
                else if (rEntry.IsQueryByNonEmpty())
                    return GetXMLToken(XML_NOEMPTY);

                if (bRegExp)
                    return GetXMLToken(XML_MATCH);
                else
                    return OUString("=");
            }
            case SC_GREATER:
                return OUString(">");
            case SC_GREATER_EQUAL:
                return OUString(">=");
            case SC_LESS:
                return OUString("<");
            case SC_LESS_EQUAL:
                return OUString("<=");
            case SC_NOT_EQUAL:
                if (bRegExp)
                    return GetXMLToken(XML_NOMATCH);
                else
                    return OUString("!=");
            case SC_TOPPERC:
                return GetXMLToken(XML_TOP_PERCENT);
            case SC_TOPVAL:
                return GetXMLToken(XML_TOP_VALUES);
            default:
                ;
        }
        return OUString("=");
    }

    class WriteSetItem : public std::unary_function<ScQueryEntry::Item, void>
    {
        ScXMLExport& mrExport;
    public:
        WriteSetItem(ScXMLExport& r) : mrExport(r) {}
        void operator() (const ScQueryEntry::Item& rItem) const
        {
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, rItem.maString);
            SvXMLElementExport aElem(mrExport, XML_NAMESPACE_TABLE, XML_FILTER_SET_ITEM, true, true);
        }
    };

    void writeCondition(const ScQueryEntry& rEntry, SCCOLROW nFieldStart, bool bCaseSens, bool bRegExp)
    {
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, OUString::number(rEntry.nField - nFieldStart));
        if (bCaseSens)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);

        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (rItems.empty())
        {
            OSL_FAIL("Query entry has no items at all!  It must have at least one!");
            return;
        }

        else if (rItems.size() == 1)
        {
            // Single item condition.
            const ScQueryEntry::Item& rItem = rItems.front();
            if (rItem.meType == ScQueryEntry::ByString)
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, rItem.maString);
            else
            {
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_NUMBER);
                OUStringBuffer aBuf;
                ::sax::Converter::convertDouble(aBuf, rItem.mfVal);
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, aBuf.makeStringAndClear());
            }

            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, getOperatorXML(rEntry, bRegExp));
            SvXMLElementExport aElemC(mrExport, XML_NAMESPACE_TABLE, XML_FILTER_CONDITION, true, true);
        }
        else
        {
            // Multi-item condition.
            OSL_ASSERT(rItems.size() > 1);

            // Store the 1st value for backward compatibility.
            const ScQueryEntry::Item& rItem = rItems.front();
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, rItem.maString);
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, OUString("="));
            SvXMLElementExport aElemC(mrExport, XML_NAMESPACE_TABLE, XML_FILTER_CONDITION, true, true);

            std::for_each(rItems.begin(), rItems.end(), WriteSetItem(mrExport));
        }
    }

    void writeFilter(const ScDBData& rData)
    {
        ScQueryParam aParam;
        rData.GetQueryParam(aParam);
        size_t nCount = 0;
        for (size_t n = aParam.GetEntryCount(); nCount < n; ++nCount)
        {
            if (!aParam.GetEntry(nCount).bDoQuery)
                break;
        }

        if (!nCount)
            // No filter criteria to save. Bail out.
            return;

        if (!aParam.bInplace)
        {
            OUString aAddrStr;
            ScRangeStringConverter::GetStringFromAddress(
                aAddrStr, ScAddress(aParam.nDestCol, aParam.nDestRow, aParam.nDestTab), mpDoc, ::formula::FormulaGrammar::CONV_OOO);
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, aAddrStr);
        }

        ScRange aAdvSource;
        if (rData.GetAdvancedQuerySource(aAdvSource))
        {
            OUString aAddrStr;
            ScRangeStringConverter::GetStringFromRange(
                aAddrStr, aAdvSource, mpDoc, ::formula::FormulaGrammar::CONV_OOO);
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS, aAddrStr);
        }

        if (!aParam.bDuplicate)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES, XML_FALSE);

        SvXMLElementExport aElemF(mrExport, XML_NAMESPACE_TABLE, XML_FILTER, true, true);

        bool bAnd = false;
        bool bOr = false;

        for (size_t i = 0; i < nCount; ++i)
        {
            const ScQueryEntry& rEntry = aParam.GetEntry(i);
            if (rEntry.eConnect == SC_AND)
                bAnd = true;
            else
                bOr = true;
        }

        // Note that export field index values are relative to the first field.
        ScRange aRange;
        rData.GetArea(aRange);
        SCCOLROW nFieldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();

        if (bOr && !bAnd)
        {
            SvXMLElementExport aElemOr(mrExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, true, true);
            for (size_t i = 0; i < nCount; ++i)
                writeCondition(aParam.GetEntry(i), nFieldStart, aParam.bCaseSens, aParam.bRegExp);
        }
        else if (bAnd && !bOr)
        {
            SvXMLElementExport aElemAnd(mrExport, XML_NAMESPACE_TABLE, XML_FILTER_AND, true, true);
            for (size_t i = 0; i < nCount; ++i)
                writeCondition(aParam.GetEntry(i), nFieldStart, aParam.bCaseSens, aParam.bRegExp);
        }
        else if (nCount == 1)
        {
            writeCondition(aParam.GetEntry(0), nFieldStart, aParam.bCaseSens, aParam.bRegExp);
        }
        else
        {
            SvXMLElementExport aElemC(mrExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, true, true);
            ScQueryEntry aPrevEntry = aParam.GetEntry(0);
            ScQueryConnect eConnect = aParam.GetEntry(1).eConnect;
            bool bOpenAndElement = false;
            OUString aName = mrExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XML_FILTER_AND));

            if (eConnect == SC_AND)
            {
                mrExport.StartElement(aName, true);
                bOpenAndElement = true;
            }
            else
                bOpenAndElement = false;

            for (size_t i = 1; i < nCount; ++i)
            {
                const ScQueryEntry& rEntry = aParam.GetEntry(i);
                if (eConnect != rEntry.eConnect)
                {
                    eConnect = rEntry.eConnect;
                    if (rEntry.eConnect == SC_AND)
                    {
                        mrExport.StartElement(aName, true );
                        bOpenAndElement = true;
                        writeCondition(aPrevEntry, nFieldStart, aParam.bCaseSens, aParam.bRegExp);
                        aPrevEntry = rEntry;
                        if (i == nCount - 1)
                        {
                            writeCondition(aPrevEntry, nFieldStart, aParam.bCaseSens, aParam.bRegExp);
                            mrExport.EndElement(aName, true);
                            bOpenAndElement = false;
                        }
                    }
                    else
                    {
                        writeCondition(aPrevEntry, nFieldStart, aParam.bCaseSens, aParam.bRegExp);
                        aPrevEntry = rEntry;
                        if (bOpenAndElement)
                        {
                            mrExport.EndElement(aName, true);
                            bOpenAndElement = false;
                        }
                        if (i == nCount - 1)
                            writeCondition(aPrevEntry, nFieldStart, aParam.bCaseSens, aParam.bRegExp);
                    }
                }
                else
                {
                    writeCondition(aPrevEntry, nFieldStart, aParam.bCaseSens, aParam.bRegExp);
                    aPrevEntry = rEntry;
                    if (i == nCount - 1)
                        writeCondition(aPrevEntry, nFieldStart, aParam.bCaseSens, aParam.bRegExp);
                }
            }
            if(bOpenAndElement)
                mrExport.EndElement(aName, true);
        }
    }

    void writeSubtotals(const ScDBData& rData)
    {
        ScSubTotalParam aParam;
        rData.GetSubTotalParam(aParam);

        size_t nCount = 0;
        for (; nCount < MAXSUBTOTAL; ++nCount)
        {
            if (!aParam.bGroupActive[nCount])
                break;
        }

        if (!nCount)
            return;

        if (!aParam.bIncludePattern)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT, XML_FALSE);

        if (aParam.bPagebreak)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PAGE_BREAKS_ON_GROUP_CHANGE, XML_TRUE);

        if (aParam.bCaseSens)
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);

        SvXMLElementExport aElemSTRs(mrExport, XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULES, true, true);

        if (aParam.bDoSort)
        {
            if (!aParam.bAscending)
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORDER, XML_DESCENDING);

            if (aParam.bUserDef)
            {
                OUStringBuffer aBuf;
                aBuf.appendAscii(SC_USERLIST);
                aBuf.append(static_cast<sal_Int32>(aParam.nUserIndex));
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, aBuf.makeStringAndClear());
            }
            SvXMLElementExport aElemSGs(mrExport, XML_NAMESPACE_TABLE, XML_SORT_GROUPS, true, true);
        }

        for (size_t i = 0; i < MAXSUBTOTAL; ++i)
        {
            if (!aParam.bGroupActive[i])
                // We're done!
                break;

            sal_Int32 nFieldCol = static_cast<sal_Int32>(aParam.nField[i]);
            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_BY_FIELD_NUMBER, OUString::number(nFieldCol));
            SvXMLElementExport aElemSTR(mrExport, XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULE, true, true);

            for (SCCOL j = 0, n = aParam.nSubTotals[i]; j < n; ++j)
            {
                sal_Int32 nCol = static_cast<sal_Int32>(aParam.pSubTotals[i][j]);
                ScSubTotalFunc eFunc = aParam.pFunctions[i][j];

                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, OUString::number(nCol));
                OUString aFuncStr;
                ScXMLConverter::GetStringFromFunction(aFuncStr, eFunc);
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, aFuncStr);

                SvXMLElementExport aElemSTF(mrExport, XML_NAMESPACE_TABLE, XML_SUBTOTAL_FIELD, true, true);
            }
        }
    }
};

}

void ScXMLExportDatabaseRanges::WriteDatabaseRanges()
{
    typedef ::std::map<SCTAB, const ScDBData*> SheetLocalDBs;

    pDoc = rExport.GetDocument();
    if (!pDoc)
        return;

    // Get sheet-local anonymous ranges.
    SCTAB nTabCount = pDoc->GetTableCount();
    SheetLocalDBs aSheetDBs;
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        const ScDBData* p = pDoc->GetAnonymousDBData(i);
        if (p)
            aSheetDBs.insert(SheetLocalDBs::value_type(i, p));
    }

    bool bHasRanges = !aSheetDBs.empty();

    // See if we have global ranges.
    ScDBCollection* pDBCollection = pDoc->GetDBCollection();
    if (pDBCollection)
    {
        if (!pDBCollection->getNamedDBs().empty() || !pDBCollection->getAnonDBs().empty())
            bHasRanges = true;
    }

    if (!bHasRanges)
        // No ranges to export. Bail out.
        return;

    SvXMLElementExport aElemDRs(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_RANGES, true, true);

    WriteDatabaseRange func(rExport, pDoc);

    if (pDBCollection)
    {
        // Write global named ranges.
        func.setRangeType(ScDBCollection::GlobalNamed);
        const ScDBCollection::NamedDBs& rNamedDBs = pDBCollection->getNamedDBs();
        ::std::for_each(rNamedDBs.begin(), rNamedDBs.end(), func);

        // Add global anonymous DB ranges.
        func.setRangeType(ScDBCollection::GlobalAnonymous);
        const ScDBCollection::AnonDBs& rAnonDBs = pDBCollection->getAnonDBs();
        ::std::for_each(rAnonDBs.begin(), rAnonDBs.end(), func);
    }

    // Write sheet-local ranges.
    func.setRangeType(ScDBCollection::SheetAnonymous);
    ::std::for_each(aSheetDBs.begin(), aSheetDBs.end(), func);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
