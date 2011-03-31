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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "xmltabi.hxx"
#include "xmlimprt.hxx"
#include "xmlrowi.hxx"
#include "xmlcoli.hxx"
#include "xmlsceni.hxx"
#include "xmlexternaltabi.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "olinetab.hxx"
#include "XMLConverter.hxx"
#include "XMLTableShapesContext.hxx"
#include "XMLTableSourceContext.hxx"
#include "XMLStylesImportHelper.hxx"
#include "rangeutl.hxx"
#include "externalrefmgr.hxx"
#include "sheetdata.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/formsimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventsImportContext.hxx>

#include <tools/urlobj.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/table/CellAddress.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

/**
 * Determine whether this table is an external reference cache from its
 * name.  There is currently no way of determining whether a table is a
 * regular table or an external reference cache other than examining the
 * name itself.  We should probably introduce a new boolean value for
 * table:table element and use it instead of doing this, to make it more
 * reliable and future-proof.
 *
 * @param rName
 *
 * @return
 */
static bool lcl_isExternalRefCache(const rtl::OUString& rName, rtl::OUString& rUrl, rtl::OUString& rExtTabName)
{
    // 'file:///path/to/file.ods'#MySheet
    // 'file:///path/to/file.ods'#MySheet with space
    // 'file:///path/to/file's.ods'#Sheet (Notice the quote in the file name.
    //  That's allowed.)

    if ( rName.toChar() != '\'' )       // initial quote
        return false;

    // #i114504# Other schemes besides "file:" are also allowed.
    // CompareProtocolScheme is quick, only looks at the start of the string.
    INetProtocol eProt = INetURLObject::CompareProtocolScheme( rName.copy(1) );
    if ( eProt == INET_PROT_NOT_VALID )
        return false;

    rtl::OUString aPrefix = INetURLObject::GetScheme( eProt );
    sal_Int32 nPrefLen = aPrefix.getLength();

    rtl::OUStringBuffer aUrlBuf, aTabNameBuf;
    aUrlBuf.append( aPrefix );
    sal_Int32 n = rName.getLength();
    const sal_Unicode* p = rName.getStr();

    bool bInUrl = true;
    sal_Unicode cPrev = 0;
    for (sal_Int32 i = nPrefLen+1; i < n; ++i)      // start the loop after quote and prefix
    {
        const sal_Unicode c = p[i];
        if (bInUrl)
        {
            // parsing file URL
            if (c == '#')
            {
                if (cPrev != '\'')
                    return false;

                rUrl = aUrlBuf.makeStringAndClear();
                rUrl = rUrl.copy(0, rUrl.getLength()-1); // remove the trailing single-quote.
                bInUrl = false;
            }
            else
                aUrlBuf.append(c);
        }
        else
            // parsing sheet name.
            aTabNameBuf.append(c);

        cPrev = c;
    }

    if (bInUrl)
        return false;

    if (aTabNameBuf.getLength() == 0)
        return false;

    rExtTabName = aTabNameBuf.makeStringAndClear();

    return true;
}

ScXMLExternalTabData::ScXMLExternalTabData() :
    mpCacheTable(), mnRow(0), mnCol(0), mnFileId(0)
{
}

//------------------------------------------------------------------

ScXMLTableContext::ScXMLTableContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempIsSubTable,
                                      const sal_Int32 nSpannedCols) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pExternalRefInfo(NULL),
    nStartOffset(-1),
    bStartFormPage(sal_False),
    bPrintEntireSheet(sal_True)
{
    // get start offset in file (if available)
    nStartOffset = GetScImport().GetByteOffset();

    if (!bTempIsSubTable)
    {
        sal_Bool bProtection(sal_False);
        rtl::OUString sName;
        rtl::OUString sStyleName;
        rtl::OUString sPassword;
        sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
            rtl::OUString aLocalName;
            sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName ));
            const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

            switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
            {
                case XML_TOK_TABLE_NAME:
                        sName = sValue;
                    break;
                case XML_TOK_TABLE_STYLE_NAME:
                        sStyleName = sValue;
                    break;
                case XML_TOK_TABLE_PROTECTION:
                        bProtection = IsXMLToken(sValue, XML_TRUE);
                    break;
                case XML_TOK_TABLE_PRINT_RANGES:
                        sPrintRanges = sValue;
                    break;
                case XML_TOK_TABLE_PASSWORD:
                        sPassword = sValue;
                    break;
                case XML_TOK_TABLE_PRINT:
                    {
                        if (IsXMLToken(sValue, XML_FALSE))
                            bPrintEntireSheet = sal_False;
                    }
                    break;
            }
        }

        rtl::OUString aExtUrl, aExtTabName;
        if (lcl_isExternalRefCache(sName, aExtUrl, aExtTabName))
        {
            // This is an external ref cache table.
            pExternalRefInfo.reset(new ScXMLExternalTabData);
            pExternalRefInfo->maFileUrl = aExtUrl;
            ScDocument* pDoc = GetScImport().GetDocument();
            if (pDoc)
            {
                ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
                pExternalRefInfo->mnFileId = pRefMgr->getExternalFileId(aExtUrl);
                pExternalRefInfo->mpCacheTable = pRefMgr->getCacheTable(pExternalRefInfo->mnFileId, aExtTabName, true);
                pExternalRefInfo->mpCacheTable->setWholeTableCached();
            }
        }
        else
        {
            // This is a regular table.
            GetScImport().GetTables().NewSheet(sName, sStyleName, bProtection, sPassword);
        }
    }
    else
    {
        GetScImport().GetTables().NewTable(nSpannedCols);
    }
}

ScXMLTableContext::~ScXMLTableContext()
{
}

SvXMLImportContext *ScXMLTableContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap(GetScImport().GetTableElemTokenMap());
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLName);
    if (pExternalRefInfo.get())
    {
        // We only care about the table-row and table-source elements for
        // external cache data.
        switch (nToken)
        {
            case XML_TOK_TABLE_ROW_GROUP:
            case XML_TOK_TABLE_HEADER_ROWS:
            case XML_TOK_TABLE_ROWS:
                // #i101319# don't discard rows in groups or header (repeat range)
                return new ScXMLExternalRefRowsContext(
                    GetScImport(), nPrefix, rLName, xAttrList, *pExternalRefInfo);
            case XML_TOK_TABLE_ROW:
                return new ScXMLExternalRefRowContext(
                    GetScImport(), nPrefix, rLName, xAttrList, *pExternalRefInfo);
            case XML_TOK_TABLE_SOURCE:
                return new ScXMLExternalRefTabSourceContext(
                    GetScImport(), nPrefix, rLName, xAttrList, *pExternalRefInfo);
            default:
                ;
        }

        return new SvXMLImportContext(GetImport(), nPrefix, rLName);
    }

    SvXMLImportContext *pContext(0);

    switch (nToken)
    {
    case XML_TOK_TABLE_COL_GROUP:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_True );
        break;
    case XML_TOK_TABLE_HEADER_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_True, sal_False );
        break;
    case XML_TOK_TABLE_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_False );
        break;
    case XML_TOK_TABLE_COL:
            pContext = new ScXMLTableColContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList );
        break;
    case XML_TOK_TABLE_ROW_GROUP:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_True );
        break;
    case XML_TOK_TABLE_HEADER_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_True, sal_False );
        break;
    case XML_TOK_TABLE_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_False );
        break;
    case XML_TOK_TABLE_ROW:
            pContext = new ScXMLTableRowContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    case XML_TOK_TABLE_SOURCE:
        pContext = new ScXMLTableSourceContext( GetScImport(), nPrefix, rLName, xAttrList);
        break;
    case XML_TOK_TABLE_SCENARIO:
        pContext = new ScXMLTableScenarioContext( GetScImport(), nPrefix, rLName, xAttrList);
        break;
    case XML_TOK_TABLE_SHAPES:
        pContext = new ScXMLTableShapesContext( GetScImport(), nPrefix, rLName, xAttrList);
        break;
    case XML_TOK_TABLE_FORMS:
        {
            GetScImport().GetFormImport()->startPage(GetScImport().GetTables().GetCurrentXDrawPage());
            bStartFormPage = sal_True;
            pContext = GetScImport().GetFormImport()->createOfficeFormsContext( GetScImport(), nPrefix, rLName );
        }
        break;
    case XML_TOK_TABLE_EVENT_LISTENERS:
    case XML_TOK_TABLE_EVENT_LISTENERS_EXT:
        {
            // use XEventsSupplier interface of the sheet
            uno::Reference<document::XEventsSupplier> xSupplier( GetScImport().GetTables().GetCurrentXSheet(), uno::UNO_QUERY );
            pContext = new XMLEventsImportContext( GetImport(), nPrefix, rLName, xSupplier );
        }
        break;
    default:
        ;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableContext::EndElement()
{
    // get end offset in file (if available)
//    sal_Int32 nEndOffset = GetScImport().GetByteOffset();

    GetScImport().LockSolarMutex();
    GetScImport().GetStylesImportHelper()->EndTable();
    ScDocument* pDoc(GetScImport().GetDocument());
    if (pDoc)
    {
        if (sPrintRanges.getLength())
        {
            uno::Reference< sheet::XPrintAreas > xPrintAreas( GetScImport().GetTables().GetCurrentXSheet(), uno::UNO_QUERY );
            if( xPrintAreas.is() )
            {
                uno::Sequence< table::CellRangeAddress > aRangeList;
                ScRangeStringConverter::GetRangeListFromString( aRangeList, sPrintRanges, pDoc, ::formula::FormulaGrammar::CONV_OOO );
                xPrintAreas->setPrintAreas( aRangeList );
            }
        }
        else if (bPrintEntireSheet) pDoc->SetPrintEntireSheet(static_cast<SCTAB>(GetScImport().GetTables().GetCurrentSheet()));

        ScOutlineTable* pOutlineTable(pDoc->GetOutlineTable(static_cast<SCTAB>(GetScImport().GetTables().GetCurrentSheet()), sal_False));
        if (pOutlineTable)
        {
            ScOutlineArray* pColArray(pOutlineTable->GetColArray());
            sal_Int32 nDepth(pColArray->GetDepth());
            sal_Int32 i;
            for (i = 0; i < nDepth; ++i)
            {
                sal_Int32 nCount(pColArray->GetCount(static_cast<sal_uInt16>(i)));
                for (sal_Int32 j = 0; j < nCount; ++j)
                {
                    ScOutlineEntry* pEntry(pColArray->GetEntry(static_cast<sal_uInt16>(i), static_cast<sal_uInt16>(j)));
                    if (pEntry->IsHidden())
                        pColArray->SetVisibleBelow(static_cast<sal_uInt16>(i), static_cast<sal_uInt16>(j), sal_False);
                }
            }
            ScOutlineArray* pRowArray(pOutlineTable->GetRowArray());
            nDepth = pRowArray->GetDepth();
            for (i = 0; i < nDepth; ++i)
            {
                sal_Int32 nCount(pRowArray->GetCount(static_cast<sal_uInt16>(i)));
                for (sal_Int32 j = 0; j < nCount; ++j)
                {
                    ScOutlineEntry* pEntry(pRowArray->GetEntry(static_cast<sal_uInt16>(i), static_cast<sal_uInt16>(j)));
                    if (pEntry->IsHidden())
                        pRowArray->SetVisibleBelow(static_cast<sal_uInt16>(i), static_cast<sal_uInt16>(j), sal_False);
                }
            }
        }
        if (GetScImport().GetTables().HasDrawPage())
        {
            if (GetScImport().GetTables().HasXShapes())
            {
                GetScImport().GetShapeImport()->popGroupAndSort();
                uno::Reference < drawing::XShapes > xTempShapes(GetScImport().GetTables().GetCurrentXShapes());
                GetScImport().GetShapeImport()->endPage(xTempShapes);
            }
            if (bStartFormPage)
                GetScImport().GetFormImport()->endPage();
        }

        GetScImport().GetTables().DeleteTable();
        GetScImport().ProgressBarIncrement(sal_False);

        // store stream positions
        if (!pExternalRefInfo.get() && nStartOffset >= 0 /* && nEndOffset >= 0 */)
        {
            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
            sal_Int32 nTab = GetScImport().GetTables().GetCurrentSheet();
            // pSheetData->AddStreamPos( nTab, nStartOffset, nEndOffset );
            pSheetData->StartStreamPos( nTab, nStartOffset );
        }
    }
    GetScImport().UnlockSolarMutex();
}

