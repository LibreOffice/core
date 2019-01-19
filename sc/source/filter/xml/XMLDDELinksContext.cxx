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

#include "XMLDDELinksContext.hxx"
#include "xmlimprt.hxx"
#include <document.hxx>
#include <scmatrix.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <svl/sharedstringpool.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLDDELinksContext::ScXMLDDELinksContext( ScXMLImport& rImport ) :
    ScXMLImportContext( rImport )
{
    // here are no attributes
    rImport.LockSolarMutex();
}

ScXMLDDELinksContext::~ScXMLDDELinksContext()
{
    GetScImport().UnlockSolarMutex();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDDELinksContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    if ( nElement == XML_ELEMENT( TABLE, XML_DDE_LINK) )
        pContext = new ScXMLDDELinkContext(GetScImport());

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDDELinkContext::ScXMLDDELinkContext( ScXMLImport& rImport ) :
    ScXMLImportContext( rImport ),
    aDDELinkTable(),
    aDDELinkRow(),
    sApplication(),
    sTopic(),
    sItem(),
    nPosition(-1),
    nColumns(0),
    nRows(0),
    nMode(SC_DDE_DEFAULT)
{
    // here are no attributes
}

ScXMLDDELinkContext::~ScXMLDDELinkContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDDELinkContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( OFFICE, XML_DDE_SOURCE ):
            pContext = new ScXMLDDESourceContext(GetScImport(), pAttribList, this);
        break;
        case XML_ELEMENT( TABLE, XML_TABLE ):
            pContext = new ScXMLDDETableContext(GetScImport(), this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void ScXMLDDELinkContext::CreateDDELink()
{
    if (GetScImport().GetDocument() &&
        !sApplication.isEmpty() &&
        !sTopic.isEmpty() &&
        !sItem.isEmpty())
    {
        GetScImport().GetDocument()->CreateDdeLink(sApplication, sTopic, sItem, nMode, ScMatrixRef());
        size_t nPos;
        if(GetScImport().GetDocument()->FindDdeLink(sApplication, sTopic, sItem, nMode, nPos))
            nPosition = nPos;
        else
        {
            nPosition = -1;
            SAL_WARN("sc" , "DDE Link not inserted");
        }
    }
}

void ScXMLDDELinkContext::AddCellToRow(const ScDDELinkCell& aCell)
{
    aDDELinkRow.push_back(aCell);
}

void ScXMLDDELinkContext::AddRowsToTable(const sal_Int32 nRowsP)
{
    for (sal_Int32 i = 0; i < nRowsP; ++i)
        aDDELinkTable.insert(aDDELinkTable.end(), aDDELinkRow.begin(), aDDELinkRow.end());
    aDDELinkRow.clear();
}

void SAL_CALL ScXMLDDELinkContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScDocument* pDoc = GetScImport().GetDocument();
    if (nPosition > -1 && nColumns && nRows)
    {
        bool bSizeMatch = (static_cast<size_t>(nColumns * nRows) == aDDELinkTable.size());
        OSL_ENSURE( bSizeMatch, "ScXMLDDELinkContext::EndElement: matrix dimension doesn't match cells count");
        // Excel writes bad ODF in that it does not write the
        // table:number-columns-repeated attribute of the
        // <table:table-column> element, but apparently uses the number of
        // <table:table-cell> elements within a <table:table-row> element to
        // determine the column count instead. Be lenient ...
        if (!bSizeMatch && nColumns == 1)
        {
            nColumns = aDDELinkTable.size() / nRows;
            OSL_ENSURE( static_cast<size_t>(nColumns * nRows) == aDDELinkTable.size(),
                    "ScXMLDDELinkContext::EndElement: adapted matrix dimension doesn't match either");
        }
        ScMatrixRef pMatrix = new ScMatrix(static_cast<SCSIZE>(nColumns), static_cast<SCSIZE>(nRows), 0.0);
        sal_Int32 nCol(0);
        sal_Int32 nRow(-1);
        sal_Int32 nIndex(0);

        svl::SharedStringPool& rPool = pDoc->GetSharedStringPool();
        for (const auto& rDDELinkCell : aDDELinkTable)
        {
            if (nIndex % nColumns == 0)
            {
                ++nRow;
                nCol = 0;
            }
            else
                ++nCol;

            SCSIZE nScCol( static_cast< SCSIZE >( nCol ) );
            SCSIZE nScRow( static_cast< SCSIZE >( nRow ) );
            if( rDDELinkCell.bEmpty )
                pMatrix->PutEmpty( nScCol, nScRow );
            else if( rDDELinkCell.bString )
                pMatrix->PutString(rPool.intern(rDDELinkCell.sValue), nScCol, nScRow);
            else
                pMatrix->PutDouble( rDDELinkCell.fValue, nScCol, nScRow );

            ++nIndex;
        }

        GetScImport().GetDocument()->SetDdeLinkResultMatrix( static_cast< sal_uInt16 >( nPosition ), pMatrix );
    }
}

ScXMLDDESourceContext::ScXMLDDESourceContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDDELinkContext* pTempDDELink) :
    ScXMLImportContext( rImport ),
    pDDELink(pTempDDELink)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( OFFICE, XML_DDE_APPLICATION ):
                    pDDELink->SetApplication(aIter.toString());
                break;
                case XML_ELEMENT( OFFICE, XML_DDE_TOPIC ):
                    pDDELink->SetTopic(aIter.toString());
                break;
                case XML_ELEMENT( OFFICE, XML_DDE_ITEM ):
                    pDDELink->SetItem(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_CONVERSION_MODE ):
                    if (IsXMLToken(aIter, XML_INTO_ENGLISH_NUMBER))
                        pDDELink->SetMode(SC_DDE_ENGLISH);
                    else if (IsXMLToken(aIter, XML_KEEP_TEXT))
                        pDDELink->SetMode(SC_DDE_TEXT);
                    else
                        pDDELink->SetMode(SC_DDE_DEFAULT);
                break;
            }
        }
    }
}

ScXMLDDESourceContext::~ScXMLDDESourceContext()
{
}

void SAL_CALL ScXMLDDESourceContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pDDELink->CreateDDELink();
}

ScXMLDDETableContext::ScXMLDDETableContext( ScXMLImport& rImport,
                                      ScXMLDDELinkContext* pTempDDELink) :
    ScXMLImportContext( rImport ),
    pDDELink(pTempDDELink)
{
    // here are no attributes
}

ScXMLDDETableContext::~ScXMLDDETableContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDDETableContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_TABLE_COLUMN ):
            pContext = new ScXMLDDEColumnContext(GetScImport(), pAttribList, pDDELink);
        break;
        case XML_ELEMENT( TABLE, XML_TABLE_ROW ):
            pContext = new ScXMLDDERowContext(GetScImport(), pAttribList, pDDELink);
        break;
    }

    if (!pContext)
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDDEColumnContext::ScXMLDDEColumnContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDDELinkContext* pDDELink) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        sal_Int32 nCols(1);
        auto &aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_NUMBER_COLUMNS_REPEATED ) ) );
        if (aIter != rAttrList->end())
            nCols = aIter.toInt32();

        pDDELink->AddColumns(nCols);
    }
}

ScXMLDDEColumnContext::~ScXMLDDEColumnContext()
{
}

ScXMLDDERowContext::ScXMLDDERowContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDDELinkContext* pTempDDELink) :
    ScXMLImportContext( rImport ),
    pDDELink(pTempDDELink),
    nRows(1)
{
    if ( rAttrList.is() )
    {
        auto &aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_NUMBER_ROWS_REPEATED ) ) );
        if (aIter != rAttrList->end())
            nRows = aIter.toInt32();

        pDDELink->AddRows(nRows);
    }
}

ScXMLDDERowContext::~ScXMLDDERowContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDDERowContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    if (nElement == XML_ELEMENT( TABLE, XML_TABLE_CELL ))
        pContext = new ScXMLDDECellContext(GetScImport(), pAttribList, pDDELink);

    if (!pContext)
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDDERowContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pDDELink->AddRowsToTable(nRows);
}

ScXMLDDECellContext::ScXMLDDECellContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDDELinkContext* pTempDDELink) :
    ScXMLImportContext( rImport ),
    sValue(),
    fValue(),
    nCells(1),
    bString(true),
    bString2(true),
    bEmpty(true),
    pDDELink(pTempDDELink)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( OFFICE, XML_VALUE_TYPE ):
                    if (IsXMLToken(aIter, XML_STRING))
                        bString = true;
                    else
                        bString = false;
                break;
                case XML_ELEMENT( OFFICE, XML_STRING_VALUE ):
                    sValue = aIter.toString();
                    bEmpty = false;
                    bString2 = true;
                break;
                case XML_ELEMENT( OFFICE, XML_VALUE ):
                    fValue = aIter.toDouble();
                    bEmpty = false;
                    bString2 = false;
                break;
                case XML_ELEMENT( TABLE, XML_NUMBER_COLUMNS_REPEATED ):
                    nCells = aIter.toInt32();
                break;
            }
        }
    }
}

ScXMLDDECellContext::~ScXMLDDECellContext()
{
}

void SAL_CALL ScXMLDDECellContext::endFastElement( sal_Int32 /*nElement*/ )
{
    OSL_ENSURE(bString == bString2, "something wrong with this type");
    ScDDELinkCell aCell;
    aCell.sValue = sValue;
    aCell.fValue = fValue;
    aCell.bEmpty = bEmpty;
    aCell.bString = bString2;
    for(sal_Int32 i = 0; i < nCells; ++i)
        pDDELink->AddCellToRow(aCell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
