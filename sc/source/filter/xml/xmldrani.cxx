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

#include "xmldrani.hxx"
#include "xmlimprt.hxx"
#include "xmlfilti.hxx"
#include "xmlsorti.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "docuno.hxx"
#include "dbdata.hxx"
#include "datauno.hxx"
#include "attrib.hxx"
#include "unonames.hxx"
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"
#include "queryentry.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlerror.hxx>

#include <sax/tools/converter.hxx>

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <comphelper/extract.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>

#include <memory>

#define SC_USERLIST             "UserList"

using namespace com::sun::star;
using namespace xmloff::token;
using ::rtl::OUString;

//------------------------------------------------------------------

ScXMLDatabaseRangesContext::ScXMLDatabaseRangesContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // has no attributes
    rImport.LockSolarMutex();
}

ScXMLDatabaseRangesContext::~ScXMLDatabaseRangesContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLDatabaseRangesContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDatabaseRangesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATABASE_RANGE :
        {
            pContext = new ScXMLDatabaseRangeContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDatabaseRangesContext::EndElement()
{
}

ScXMLDatabaseRangeContext::ScXMLDatabaseRangeContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpQueryParam(new ScQueryParam),
    sDatabaseRangeName(RTL_CONSTASCII_USTRINGPARAM(STR_DB_LOCAL_NONAME)),
    aSortSequence(),
    nRefresh(0),
    nSubTotalsUserListIndex(0),
    mbValidRange(true),
    bContainsSort(false),
    bContainsSubTotal(false),
    bNative(true),
    bIsSelection(false),
    bKeepFormats(false),
    bMoveCells(false),
    bStripData(false),
    bAutoFilter(false),
    bSubTotalsBindFormatsToContent(false),
    bSubTotalsIsCaseSensitive(false),
    bSubTotalsInsertPageBreaks(false),
    bSubTotalsSortGroups(false),
    bSubTotalsEnabledUserList(false),
    bSubTotalsAscending(true),
    bFilterConditionSourceRange(false),
    bHasHeader(true),
    bByRow(false),
    meRangeType(ScDBCollection::GlobalNamed)
{
    nSourceType = sheet::DataImportMode_NONE;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATABASE_RANGE_ATTR_NAME :
            {
                sDatabaseRangeName = sValue;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION :
            {
                bIsSelection = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES :
            {
                bKeepFormats = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE :
            {
                bMoveCells = !IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA :
            {
                bStripData = !IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION :
            {
                bByRow = !IsXMLToken(sValue, XML_COLUMN);
                mpQueryParam->bByRow = bByRow;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER :
            {
                bHasHeader = IsXMLToken(sValue, XML_TRUE);
                mpQueryParam->bHasHeader = bHasHeader;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS :
            {
                bAutoFilter = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScDocument* pDoc = GetScImport().GetDocument();
                sal_Int32 nOffset = 0;
                if (!ScRangeStringConverter::GetRangeFromString(
                    maRange, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset))
                    mbValidRange = false;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_REFRESH_DELAY :
            {
                double fTime;
                if (::sax::Converter::convertDuration( fTime, sValue ))
                    nRefresh = Max( (sal_Int32)(fTime * 86400.0), (sal_Int32)0 );
            }
            break;
        }
    }

    mpQueryParam->nTab = maRange.aStart.Tab();
    mpQueryParam->nCol1 = maRange.aStart.Col();
    mpQueryParam->nRow1 = maRange.aStart.Row();
    mpQueryParam->nCol2 = maRange.aEnd.Col();
    mpQueryParam->nRow2 = maRange.aEnd.Row();

    if (sDatabaseRangeName.matchAsciiL(STR_DB_LOCAL_NONAME, strlen(STR_DB_LOCAL_NONAME)))
        meRangeType = ScDBCollection::SheetAnonymous;
    else if (sDatabaseRangeName.matchAsciiL(STR_DB_GLOBAL_NONAME, strlen(STR_DB_GLOBAL_NONAME)))
        meRangeType = ScDBCollection::GlobalAnonymous;
}

ScXMLDatabaseRangeContext::~ScXMLDatabaseRangeContext()
{
}

SvXMLImportContext *ScXMLDatabaseRangeContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDatabaseRangeElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATABASE_RANGE_SOURCE_SQL :
        {
            pContext = new ScXMLSourceSQLContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATABASE_RANGE_SOURCE_TABLE :
        {
            pContext = new ScXMLSourceTableContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATABASE_RANGE_SOURCE_QUERY :
        {
            pContext = new ScXMLSourceQueryContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_FILTER :
        {
            pContext = new ScXMLFilterContext(
                GetScImport(), nPrefix, rLName, xAttrList, *mpQueryParam, this);
        }
        break;
        case XML_TOK_SORT :
        {
            bContainsSort = true;
            pContext = new ScXMLSortContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES :
        {
            bContainsSubTotal = true;
            pContext = new ScXMLSubTotalRulesContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

ScDBData* ScXMLDatabaseRangeContext::ConvertToDBData(const OUString& rName)
{
    if (!mbValidRange)
        return NULL;

    ScDocument* pDoc = GetScImport().GetDocument();

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDBData> pData(
        new ScDBData(rName, maRange.aStart.Tab(), maRange.aStart.Col(), maRange.aStart.Row(), maRange.aEnd.Col(), maRange.aEnd.Row(), bByRow, bHasHeader));
    SAL_WNODEPRECATED_DECLARATIONS_POP

    pData->SetAutoFilter(bAutoFilter);
    pData->SetKeepFmt(bKeepFormats);
    pData->SetDoSize(bMoveCells);
    pData->SetStripData(bStripData);

    pData->SetQueryParam(*mpQueryParam);

    if (bFilterConditionSourceRange)
    {
        ScRange aAdvSource;
        ScUnoConversion::FillScRange(aAdvSource, aFilterConditionSourceRangeAddress);
        pData->SetAdvancedQuerySource(&aAdvSource);
    }

    {
        ScImportParam aParam;
        aParam.bNative = bNative;
        aParam.aDBName = sDatabaseName.isEmpty() ? sConnectionResource : sDatabaseName;
        aParam.aStatement = sSourceObject;
        sheet::DataImportMode eMode = static_cast<sheet::DataImportMode>(nSourceType);
        switch (eMode)
        {
            case sheet::DataImportMode_NONE:
                aParam.bImport = false;
                break;
            case sheet::DataImportMode_SQL:
                aParam.bImport = true;
                aParam.bSql    = true;
                break;
            case sheet::DataImportMode_TABLE:
                aParam.bImport = true;
                aParam.bSql    = false;
                aParam.nType   = ScDbTable;
                break;
            case sheet::DataImportMode_QUERY:
                aParam.bImport = true;
                aParam.bSql    = false;
                aParam.nType   = ScDbQuery;
                break;
            default:
                OSL_FAIL("Unknown data import mode");
                aParam.bImport = false;
        }
        pData->SetImportParam(aParam);
    }

    if (bContainsSort)
    {
        size_t nOldSize = aSortSequence.getLength();
        aSortSequence.realloc(nOldSize + 1);
        beans::PropertyValue aProperty;
        aProperty.Name = OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ORIENT));
        table::TableOrientation eOrient = mpQueryParam->bByRow ?
            table::TableOrientation_ROWS : table::TableOrientation_COLUMNS;
        aProperty.Value <<= eOrient;
        aSortSequence[nOldSize] = aProperty;
        ScSortParam aParam;
        ScSortDescriptor::FillSortParam(aParam, aSortSequence);

        SCCOLROW nStartPos = aParam.bByRow ? maRange.aStart.Col() : maRange.aStart.Row();
        for (size_t i = 0; i < aParam.GetSortKeyCount(); ++i)
        {
            if (!aParam.maKeyState[i].bDoSort)
                break;
            aParam.maKeyState[i].nField += nStartPos;
        }

        pData->SetSortParam(aParam);
    }

    if (bContainsSubTotal)
    {
        ScSubTotalParam aParam;
        aParam.bIncludePattern = bSubTotalsBindFormatsToContent;
        aParam.bUserDef = bSubTotalsEnabledUserList;
        aParam.nUserIndex = nSubTotalsUserListIndex;
        aParam.bPagebreak = bSubTotalsInsertPageBreaks;
        aParam.bCaseSens = bSubTotalsIsCaseSensitive;
        aParam.bDoSort = bSubTotalsSortGroups;
        aParam.bAscending = bSubTotalsAscending;
        aParam.bUserDef = bSubTotalsEnabledUserList;
        aParam.nUserIndex = nSubTotalsUserListIndex;
        std::vector <ScSubTotalRule>::iterator itr = aSubTotalRules.begin(), itrEnd = aSubTotalRules.end();
        for (size_t nPos = 0; itr != itrEnd; ++itr, ++nPos)
        {
            if (nPos >= MAXSUBTOTAL)
                break;

            const uno::Sequence<sheet::SubTotalColumn>& rColumns = itr->aSubTotalColumns;
            sal_Int32 nColCount = rColumns.getLength();
            sal_Int16 nGroupColumn = itr->nSubTotalRuleGroupFieldNumber;
            aParam.bGroupActive[nPos] = true;
            aParam.nField[nPos] = static_cast<SCCOL>(nGroupColumn);

            SCCOL nCount = static_cast<SCCOL>(nColCount);
            aParam.nSubTotals[nPos] = nCount;
            if (nCount != 0)
            {
                aParam.pSubTotals[nPos] = new SCCOL[nCount];
                aParam.pFunctions[nPos] = new ScSubTotalFunc[nCount];

                const sheet::SubTotalColumn* pAry = rColumns.getConstArray();
                for (SCCOL i = 0; i < nCount; ++i)
                {
                    aParam.pSubTotals[nPos][i] = static_cast<SCCOL>(pAry[i].Column);
                    aParam.pFunctions[nPos][i] =
                        ScDataUnoConversion::GeneralToSubTotal( pAry[i].Function );
                }
            }
            else
            {
                aParam.pSubTotals[nPos] = NULL;
                aParam.pFunctions[nPos] = NULL;
            }
        }

        pData->SetSubTotalParam(aParam);
    }

    if (pData->HasImportParam() && !pData->HasImportSelection())
    {
        pData->SetRefreshDelay(nRefresh);
        pData->SetRefreshHandler(pDoc->GetDBCollection()->GetRefreshHandler());
        pData->SetRefreshControl(pDoc->GetRefreshTimerControlAddress());
    }

    return pData.release();
}

namespace {

bool setAutoFilterFlags(ScDocument& rDoc, const ScDBData& rData)
{
    if (!rData.HasAutoFilter())
        return false;

    // Set autofilter flags so that the buttons get displayed.
    ScRange aRange;
    rData.GetArea(aRange);
    rDoc.ApplyFlagsTab(
        aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aStart.Row(),
        aRange.aStart.Tab(), SC_MF_AUTO);
    return false;
}

}

void ScXMLDatabaseRangeContext::EndElement()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    if (!pDoc)
        return;

    if (meRangeType == ScDBCollection::SheetAnonymous)
    {
        OUString aName(RTL_CONSTASCII_USTRINGPARAM(STR_DB_LOCAL_NONAME));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDBData> pData(ConvertToDBData(aName));
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (pData.get())
        {
            ScRange aRange;
            pData->GetArea(aRange);

            setAutoFilterFlags(*pDoc, *pData);
            pDoc->SetAnonymousDBData(aRange.aStart.Tab(), pData.release());
        }
        return;
    }
    else if (meRangeType == ScDBCollection::GlobalAnonymous)
    {
        OUString aName(RTL_CONSTASCII_USTRINGPARAM(STR_DB_GLOBAL_NONAME));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDBData> pData(ConvertToDBData(aName));
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (pData.get())
        {
            ScRange aRange;
            pData->GetArea(aRange);

            if (setAutoFilterFlags(*pDoc, *pData))
                pDoc->SetAnonymousDBData(aRange.aStart.Tab(), pData.release());
            else
                pDoc->GetDBCollection()->getAnonDBs().insert(pData.release());
        }
        return;
    }
    else if (meRangeType == ScDBCollection::GlobalNamed)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDBData> pData(ConvertToDBData(sDatabaseRangeName));
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (pData.get())
        {
            setAutoFilterFlags(*pDoc, *pData);
            pDoc->GetDBCollection()->getNamedDBs().insert(pData.release());
        }
    }
}

ScXMLSourceSQLContext::ScXMLSourceSQLContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceSQLAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME :
            {
                sDBName = sValue;
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT :
            {
                pDatabaseRangeContext->SetSourceObject(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT :
            {
                pDatabaseRangeContext->SetNative(IsXMLToken(sValue, XML_TRUE));
            }
            break;
        }
    }
    pDatabaseRangeContext->SetSourceType(sheet::DataImportMode_SQL);
}

ScXMLSourceSQLContext::~ScXMLSourceSQLContext()
{
}

SvXMLImportContext *ScXMLSourceSQLContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ( nPrefix == XML_NAMESPACE_FORM )
    {
        if (IsXMLToken(rLName, XML_CONNECTION_RESOURCE) && sDBName.isEmpty())
        {
            pContext = new ScXMLConResContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceSQLContext::EndElement()
{
    if (!sDBName.isEmpty())
        pDatabaseRangeContext->SetDatabaseName(sDBName);
}

ScXMLSourceTableContext::ScXMLSourceTableContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceTableAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME :
            {
                sDBName = sValue;
            }
            break;
            case XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME :
            {
                pDatabaseRangeContext->SetSourceObject(sValue);
            }
            break;
        }
    }
    pDatabaseRangeContext->SetSourceType(sheet::DataImportMode_TABLE);
}

ScXMLSourceTableContext::~ScXMLSourceTableContext()
{
}

SvXMLImportContext *ScXMLSourceTableContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ( nPrefix == XML_NAMESPACE_FORM )
    {
        if (IsXMLToken(rLName, XML_CONNECTION_RESOURCE) && sDBName.isEmpty())
        {
            pContext = new ScXMLConResContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceTableContext::EndElement()
{
    if (!sDBName.isEmpty())
        pDatabaseRangeContext->SetDatabaseName(sDBName);
}

ScXMLSourceQueryContext::ScXMLSourceQueryContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceQueryAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME :
            {
                sDBName = sValue;
            }
            break;
            case XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME :
            {
                pDatabaseRangeContext->SetSourceObject(sValue);
            }
            break;
        }
    }
    pDatabaseRangeContext->SetSourceType(sheet::DataImportMode_QUERY);
}

ScXMLSourceQueryContext::~ScXMLSourceQueryContext()
{
}

SvXMLImportContext *ScXMLSourceQueryContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ( nPrefix == XML_NAMESPACE_FORM )
    {
        if (IsXMLToken(rLName, XML_CONNECTION_RESOURCE) && sDBName.isEmpty())
        {
            pContext = new ScXMLConResContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceQueryContext::EndElement()
{
    if (!sDBName.isEmpty())
        pDatabaseRangeContext->SetDatabaseName(sDBName);
}

ScXMLConResContext::ScXMLConResContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext( pTempDatabaseRangeContext )
{
    rtl::OUString sConRes;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_XLINK)
        {
            if (IsXMLToken(aLocalName, XML_HREF))
                sConRes = sValue;
        }
    }
    if (!sConRes.isEmpty())
        pDatabaseRangeContext->SetConnectionResource(sConRes);
}

ScXMLConResContext::~ScXMLConResContext()
{
}

SvXMLImportContext *ScXMLConResContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLConResContext::EndElement()
{
}

ScXMLSubTotalRulesContext::ScXMLSubTotalRulesContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSubTotalRulesAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT :
            {
                pDatabaseRangeContext->SetSubTotalsBindFormatsToContent(IsXMLToken(sValue, XML_TRUE));
            }
            break;
            case XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE :
            {
                pDatabaseRangeContext->SetSubTotalsIsCaseSensitive(IsXMLToken(sValue, XML_TRUE));
            }
            break;
            case XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE :
            {
                pDatabaseRangeContext->SetSubTotalsInsertPageBreaks(IsXMLToken(sValue, XML_TRUE));
            }
            break;
        }
    }
}

ScXMLSubTotalRulesContext::~ScXMLSubTotalRulesContext()
{
}

SvXMLImportContext *ScXMLSubTotalRulesContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDatabaseRangeSubTotalRulesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SUBTOTAL_RULES_SORT_GROUPS :
        {
            pContext = new ScXMLSortGroupsContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
        break;
        case XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE :
        {
            pContext = new ScXMLSubTotalRuleContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSubTotalRulesContext::EndElement()
{
}

ScXMLSortGroupsContext::ScXMLSortGroupsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext)
{
    pDatabaseRangeContext->SetSubTotalsSortGroups(true);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSubTotalRulesSortGroupsAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE :
            {
                if (sValue.getLength() > 8)
                {
                    rtl::OUString sTemp = sValue.copy(0, 8);
                    if (sTemp.compareToAscii(SC_USERLIST) == 0)
                    {
                        pDatabaseRangeContext->SetSubTotalsEnabledUserList(true);
                        sTemp = sValue.copy(8);
                        pDatabaseRangeContext->SetSubTotalsUserListIndex(static_cast<sal_Int16>(sTemp.toInt32()));
                    }
                    else
                    {
                        //if (IsXMLToken(sValue, XML_AUTOMATIC))
                            //aSortField.FieldType = util::SortFieldType_AUTOMATIC;
                            // is not supported by StarOffice
                    }
                }
                else
                {
                    //if (IsXMLToken(sValue, XML_TEXT))
                        //aSortField.FieldType = util::SortFieldType_ALPHANUMERIC;
                        // is not supported by StarOffice
                    //else if (IsXMLToken(sValue, XML_NUMBER))
                        //aSortField.FieldType = util::SortFieldType_NUMERIC;
                        // is not supported by StarOffice
                }
            }
            break;
            case XML_TOK_SORT_GROUPS_ATTR_ORDER :
            {
                if (IsXMLToken(sValue, XML_ASCENDING))
                    pDatabaseRangeContext->SetSubTotalsAscending(true);
                else
                    pDatabaseRangeContext->SetSubTotalsAscending(false);
            }
            break;
        }
    }
}

ScXMLSortGroupsContext::~ScXMLSortGroupsContext()
{
}

SvXMLImportContext *ScXMLSortGroupsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSortGroupsContext::EndElement()
{
}

ScXMLSubTotalRuleContext::ScXMLSubTotalRuleContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSubTotalRulesSubTotalRuleAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER :
            {
                aSubTotalRule.nSubTotalRuleGroupFieldNumber = static_cast<sal_Int16>(sValue.toInt32());
            }
            break;
        }
    }
}

ScXMLSubTotalRuleContext::~ScXMLSubTotalRuleContext()
{
}

SvXMLImportContext *ScXMLSubTotalRuleContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetSubTotalRulesSubTotalRuleElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD :
        {
            pContext = new ScXMLSubTotalFieldContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSubTotalRuleContext::EndElement()
{
    if (pDatabaseRangeContext)
        pDatabaseRangeContext->AddSubTotalRule(aSubTotalRule);
}

ScXMLSubTotalFieldContext::ScXMLSubTotalFieldContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLSubTotalRuleContext* pTempSubTotalRuleContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pSubTotalRuleContext(pTempSubTotalRuleContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSubTotalRuleSubTotalFieldAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER :
            {
                sFieldNumber = sValue;
            }
            break;
            case XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION :
            {
                sFunction = sValue;
            }
            break;
        }
    }
}

ScXMLSubTotalFieldContext::~ScXMLSubTotalFieldContext()
{
}

SvXMLImportContext *ScXMLSubTotalFieldContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSubTotalFieldContext::EndElement()
{
    sheet::SubTotalColumn aSubTotalColumn;
    aSubTotalColumn.Column = sFieldNumber.toInt32();
    aSubTotalColumn.Function = ScXMLConverter::GetFunctionFromString( sFunction );
    pSubTotalRuleContext->AddSubTotalColumn(aSubTotalColumn);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
