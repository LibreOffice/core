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

#include "xmltabi.hxx"
#include "xmlimprt.hxx"
#include "xmlrowi.hxx"
#include "xmlcoli.hxx"
#include "xmlsceni.hxx"
#include "xmlexternaltabi.hxx"
#include "xmlnexpi.hxx"
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
#include "xmlcondformat.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventsImportContext.hxx>

#include <tools/urlobj.hxx>
#include <sax/fastattribs.hxx>

using namespace com::sun::star;
using namespace xmloff::token;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::xml::sax::XAttributeList;

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
static bool lcl_isExternalRefCache(const OUString& rName, OUString& rUrl, OUString& rExtTabName)
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
    if ( eProt == INetProtocol::NotValid )
        return false;

    OUString aPrefix = INetURLObject::GetScheme( eProt );
    sal_Int32 nPrefLen = aPrefix.getLength();

    OUStringBuffer aUrlBuf, aTabNameBuf;
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

    if (aTabNameBuf.isEmpty())
        return false;

    rExtTabName = aTabNameBuf.makeStringAndClear();

    return true;
}

ScXMLExternalTabData::ScXMLExternalTabData() :
    mpCacheTable(), mnRow(0), mnCol(0), mnFileId(0)
{
}

ScXMLTableContext::ScXMLTableContext( ScXMLImport& rImport,
                                      sal_Int32 /*nElement*/,
                                      const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) :
    ScXMLImportContext( rImport ),
    nStartOffset(-1),
    bStartFormPage(false),
    bPrintEntireSheet(true)
{
    // get start offset in file (if available)
    nStartOffset = GetScImport().GetByteOffset();

    ScXMLTabProtectionData aProtectData;
    OUString sName;
    OUString sStyleName;

    if ( xAttrList.is() )
    {
        sax_fastparser::FastAttributeList *pAttribList;
        assert( dynamic_cast< sax_fastparser::FastAttributeList *>( xAttrList.get() ) != nullptr );
        pAttribList = static_cast< sax_fastparser::FastAttributeList *>( xAttrList.get() );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();
        for ( auto it = pAttribList->begin(); it != pAttribList->end(); ++it)
        {
            switch( rAttrTokenMap.Get( it.getToken() ) )
            {
                case XML_TOK_TABLE_NAME:
                        sName = it.toString();
                    break;
                case XML_TOK_TABLE_STYLE_NAME:
                        sStyleName = it.toString();
                    break;
                case XML_TOK_TABLE_PROTECTED:
                    aProtectData.mbProtected = IsXMLToken( it.toCString(), XML_TRUE );
                break;
                case XML_TOK_TABLE_PRINT_RANGES:
                        sPrintRanges = it.toString();
                    break;
                case XML_TOK_TABLE_PASSWORD:
                    aProtectData.maPassword = it.toString();
                break;
                case XML_TOK_TABLE_PASSHASH:
                    aProtectData.meHash1 = ScPassHashHelper::getHashTypeFromURI( it.toString() );
                break;
                case XML_TOK_TABLE_PASSHASH_2:
                    aProtectData.meHash2 = ScPassHashHelper::getHashTypeFromURI( it.toString() );
                break;
                case XML_TOK_TABLE_PRINT:
                    {
                        if (IsXMLToken( it.toCString(), XML_FALSE) )
                            bPrintEntireSheet = false;
                    }
                    break;
            }

        }
    }

    OUString aExtUrl, aExtTabName;
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
            pExternalRefInfo->mpCacheTable = pRefMgr->getCacheTable(pExternalRefInfo->mnFileId, aExtTabName, true,
                    nullptr, &aExtUrl);
            pExternalRefInfo->mpCacheTable->setWholeTableCached();
        }
    }
    else
    {
        // This is a regular table.
        GetScImport().GetTables().NewSheet(sName, sStyleName, aProtectData);
    }
}

ScXMLTableContext::~ScXMLTableContext()
{
}

SvXMLImportContext *ScXMLTableContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
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

    SvXMLImportContext *pContext(nullptr);

    switch (nToken)
    {
    case XML_TOK_TABLE_NAMED_EXPRESSIONS:
    {
        SCTAB nTab = GetScImport().GetTables().GetCurrentSheet();
        pContext = new ScXMLNamedExpressionsContext(
            GetScImport(), nPrefix, rLName, xAttrList,
            new ScXMLNamedExpressionsContext::SheetLocalInserter(GetScImport(), nTab));
    }
        break;
    case XML_TOK_TABLE_COL_GROUP:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, true );
        break;
    case XML_TOK_TABLE_HEADER_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   true, false );
        break;
    case XML_TOK_TABLE_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, false );
        break;
    case XML_TOK_TABLE_COL:
            pContext = new ScXMLTableColContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList );
        break;
    case XML_TOK_TABLE_PROTECTION:
    case XML_TOK_TABLE_PROTECTION_EXT:
        pContext = new ScXMLTableProtectionContext( GetScImport(), nPrefix, rLName, xAttrList );
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
            bStartFormPage = true;
            pContext = xmloff::OFormLayerXMLImport::createOfficeFormsContext( GetScImport(), nPrefix, rLName );
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
    case XML_TOK_TABLE_CONDFORMATS:
        pContext = new ScXMLConditionalFormatsContext( GetScImport(), nPrefix, rLName );
        break;
    default:
        ;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
        ScXMLTableContext::createFastChildContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    const SvXMLTokenMap& rTokenMap(GetScImport().GetTableElemTokenMap());
    sal_uInt16 nToken = rTokenMap.Get( nElement );
    if (pExternalRefInfo.get())
    {
        return new SvXMLImportContext( GetImport() );
    }

    SvXMLImportContext *pContext(nullptr);

    switch (nToken)
    {
    case XML_TOK_TABLE_ROW_GROUP:
        pContext = new ScXMLTableRowsContext( GetScImport(), nElement, xAttrList,
                                                   false, true );
        break;
    case XML_TOK_TABLE_HEADER_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nElement, xAttrList,
                                                   true, false );
        break;
    case XML_TOK_TABLE_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nElement, xAttrList,
                                                   false, false );
        break;
    case XML_TOK_TABLE_ROW:
            pContext = new ScXMLTableRowContext( GetScImport(), nElement,
                                                      xAttrList//,
                                                      //this
                                                      );
        break;
    default:
        pContext = new SvXMLImportContext( GetImport() );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLTableContext::endFastElement(sal_Int32 /*nElement*/)
{
    ScXMLImport::MutexGuard aMutexGuard(GetScImport());
    ScXMLImport& rImport = GetScImport();
    rImport.GetStylesImportHelper()->EndTable();
    ScDocument* pDoc(rImport.GetDocument());
    if (!pDoc)
        return;

    ScMyTables& rTables = rImport.GetTables();
    SCTAB nCurTab = rTables.GetCurrentSheet();
    if (!sPrintRanges.isEmpty())
    {
         ScRangeList aRangeList;
        ScRangeStringConverter::GetRangeListFromString( aRangeList, sPrintRanges, pDoc, ::formula::FormulaGrammar::CONV_OOO );
        size_t nCount = aRangeList.size();
        for (size_t i=0; i< nCount; i++ )
        {
            pDoc->AddPrintRange( nCurTab, *aRangeList[i] );
        }
    }
    else if (!bPrintEntireSheet)
        // Sheet has "print entire sheet" option by default.  Remove it.
        pDoc->ClearPrintRanges(nCurTab);

    ScOutlineTable* pOutlineTable(pDoc->GetOutlineTable(nCurTab));
    if (pOutlineTable)
    {
        ScOutlineArray& rColArray(pOutlineTable->GetColArray());
        size_t nDepth = rColArray.GetDepth();
        for (size_t i = 0; i < nDepth; ++i)
        {
            size_t nCount = rColArray.GetCount(i);
            for (size_t j = 0; j < nCount; ++j)
            {
                const ScOutlineEntry* pEntry = rColArray.GetEntry(i, j);
                if (pEntry->IsHidden())
                    rColArray.SetVisibleBelow(i, j, false);
            }
        }
        ScOutlineArray& rRowArray(pOutlineTable->GetRowArray());
        nDepth = rRowArray.GetDepth();
        for (size_t i = 0; i < nDepth; ++i)
        {
            size_t nCount = rRowArray.GetCount(i);
            for (size_t j = 0; j < nCount; ++j)
            {
                const ScOutlineEntry* pEntry = rRowArray.GetEntry(i, j);
                if (pEntry->IsHidden())
                    rRowArray.SetVisibleBelow(i, j, false);
            }
        }
    }
    if (rTables.HasDrawPage())
    {
        if (rTables.HasXShapes())
        {
            rImport.GetShapeImport()->popGroupAndSort();
            uno::Reference < drawing::XShapes > xTempShapes(rTables.GetCurrentXShapes());
            rImport.GetShapeImport()->endPage(xTempShapes);
        }
        if (bStartFormPage)
            rImport.GetFormImport()->endPage();
    }

    rTables.DeleteTable();
    rImport.ProgressBarIncrement();

    // store stream positions
    if (!pExternalRefInfo.get() && nStartOffset >= 0 /* && nEndOffset >= 0 */)
    {
        ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rImport.GetModel())->GetSheetSaveData();
        SCTAB nTab = rTables.GetCurrentSheet();
        // pSheetData->AddStreamPos( nTab, nStartOffset, nEndOffset );
        pSheetData->StartStreamPos( nTab, nStartOffset );
    }
}

ScXMLTableProtectionContext::ScXMLTableProtectionContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList ) :
    ScXMLImportContext( rImport, nPrefix, rLName )
{
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableProtectionAttrTokenMap();
    bool bSelectProtectedCells = false;
    bool bSelectUnprotectedCells = false;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        const OUString& aAttrName = xAttrList->getNameByIndex(i);
        const OUString aValue = xAttrList->getValueByIndex(i);

        OUString aLocalName;
        sal_uInt16 nLocalPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
            aAttrName, &aLocalName);

        switch (rAttrTokenMap.Get(nLocalPrefix, aLocalName))
        {
            case XML_TOK_TABLE_SELECT_PROTECTED_CELLS:
            case XML_TOK_TABLE_SELECT_PROTECTED_CELLS_EXT:
                bSelectProtectedCells = IsXMLToken(aValue, XML_TRUE);
            break;
            case XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS:
            case XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS_EXT:
                bSelectUnprotectedCells = IsXMLToken(aValue, XML_TRUE);
            break;
            default:
                SAL_WARN("sc", "unknown attribute: " << aAttrName);
        }
    }

    ScXMLTabProtectionData& rProtectData = GetScImport().GetTables().GetCurrentProtectionData();
    rProtectData.mbSelectProtectedCells   = bSelectProtectedCells;
    rProtectData.mbSelectUnprotectedCells = bSelectUnprotectedCells;
}

ScXMLTableProtectionContext::~ScXMLTableProtectionContext()
{
}

SvXMLImportContext* ScXMLTableProtectionContext::CreateChildContext(
    sal_uInt16 /*nPrefix*/, const OUString& /*rLocalName*/, const Reference<XAttributeList>& /*xAttrList*/ )
{
    return nullptr;
}

void ScXMLTableProtectionContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
