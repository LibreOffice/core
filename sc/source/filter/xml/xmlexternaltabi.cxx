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

#include "xmlexternaltabi.hxx"
#include "xmlimprt.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"

#include <document.hxx>
#include <documentimport.hxx>

#include <svl/sharedstringpool.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

#include <sax/tools/converter.hxx>

#include <com/sun/star/util/NumberFormat.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;

ScXMLExternalRefTabSourceContext::ScXMLExternalRefTabSourceContext(
    ScXMLImport& rImport,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList, ScXMLExternalTabData& rRefInfo ) :
    ScXMLImportContext( rImport ),
    mrExternalRefInfo(rRefInfo)
{
    using namespace ::xmloff::token;

    if ( !rAttrList.is() )
        return;

    for (auto &it : *rAttrList)
    {
        sal_Int32 nAttrToken = it.getToken();
        if ( nAttrToken == XML_ELEMENT( XLINK, XML_HREF ) )
            maRelativeUrl = it.toString();
        else if ( nAttrToken == XML_ELEMENT( TABLE, XML_TABLE_NAME ) )
            // todo
            ;
        else if ( nAttrToken == XML_ELEMENT( TABLE, XML_FILTER_NAME ) )
            maFilterName = it.toString();
        else if ( nAttrToken == XML_ELEMENT( TABLE, XML_FILTER_OPTIONS ) )
            maFilterOptions = it.toString();
    }
}

ScXMLExternalRefTabSourceContext::~ScXMLExternalRefTabSourceContext()
{
}

/**
 * Make sure the URL is a valid relative URL, mainly to avoid storing
 * absolute URL as relative URL by accident.  For now, we only check the first
 * three characters which are assumed to be always '../', because the relative
 * URL for an external document is always in reference to the content.xml
 * fragment of the original document.
 */
static bool lcl_isValidRelativeURL(const OUString& rUrl)
{
    sal_Int32 n = ::std::min( rUrl.getLength(), static_cast<sal_Int32>(3));
    if (n < 3)
        return false;
    const sal_Unicode* p = rUrl.getStr();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Unicode c = p[i];
        if (i < 2 && c != '.')
            // the path must begin with '..'
            return false;
        else if (i == 2 && c != '/')
            // a '/' path separator must follow
            return false;
    }
    return true;
}

void SAL_CALL ScXMLExternalRefTabSourceContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScDocument* pDoc = GetScImport().GetDocument();
    if (!pDoc)
        return;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    if (lcl_isValidRelativeURL(maRelativeUrl))
        pRefMgr->setRelativeFileName(mrExternalRefInfo.mnFileId, maRelativeUrl);
    pRefMgr->setFilterData(mrExternalRefInfo.mnFileId, maFilterName, maFilterOptions);
}

ScXMLExternalRefRowsContext::ScXMLExternalRefRowsContext(
    ScXMLImport& rImport, ScXMLExternalTabData& rRefInfo ) :
    ScXMLImportContext( rImport ),
    mrExternalRefInfo(rRefInfo)
{
}

ScXMLExternalRefRowsContext::~ScXMLExternalRefRowsContext()
{
}

Reference< XFastContextHandler > SAL_CALL ScXMLExternalRefRowsContext::createFastChildContext(
    sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList )
{
    // #i101319# row elements inside group, rows or header-rows
    // are treated like row elements directly in the table element

    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT(TABLE, XML_TABLE_ROW_GROUP):
        case XML_ELEMENT(TABLE, XML_TABLE_HEADER_ROWS):
        case XML_ELEMENT(TABLE, XML_TABLE_ROWS):
            return new ScXMLExternalRefRowsContext(
                GetScImport(), mrExternalRefInfo);
        case XML_ELEMENT(TABLE, XML_TABLE_ROW):
            return new ScXMLExternalRefRowContext(
                GetScImport(), pAttribList, mrExternalRefInfo);
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("sc", nElement);
    }
    return nullptr;
}

ScXMLExternalRefRowContext::ScXMLExternalRefRowContext(
    ScXMLImport& rImport,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList, ScXMLExternalTabData& rRefInfo ) :
    ScXMLImportContext( rImport ),
    mrScImport(rImport),
    mrExternalRefInfo(rRefInfo),
    mnRepeatRowCount(1)
{
    mrExternalRefInfo.mnCol = 0;

    for (auto &it : *rAttrList)
    {
        switch (it.getToken())
        {
            case XML_ELEMENT(TABLE, XML_NUMBER_ROWS_REPEATED):
            {
                mnRepeatRowCount = std::max(it.toInt32(), static_cast<sal_Int32>(1));
            }
            break;
        }
    }
}

ScXMLExternalRefRowContext::~ScXMLExternalRefRowContext()
{
}

Reference< XFastContextHandler > SAL_CALL ScXMLExternalRefRowContext::createFastChildContext(
    sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList )
{
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    if (nElement == XML_ELEMENT(TABLE, XML_TABLE_CELL) || nElement == XML_ELEMENT(TABLE, XML_COVERED_TABLE_CELL))
        return new ScXMLExternalRefCellContext(mrScImport, pAttribList, mrExternalRefInfo);

    return nullptr;
}

void SAL_CALL ScXMLExternalRefRowContext::endFastElement( sal_Int32 /* nElement */ )
{
    ScExternalRefCache::TableTypeRef pTab = mrExternalRefInfo.mpCacheTable;

    for (sal_Int32 i = 1; i < mnRepeatRowCount; ++i)
    {
        // Performance: duplicates of a non-existent row will still not exist.
        // Don't find that out for every cell.
        // External references often are a sparse matrix.
        if (i == 1 && !pTab->hasRow( mrExternalRefInfo.mnRow))
        {
            mrExternalRefInfo.mnRow += mnRepeatRowCount;
            return;
        }

        for (sal_Int32 j = 0; j < mrExternalRefInfo.mnCol; ++j)
        {
            ScExternalRefCache::TokenRef pToken = pTab->getCell(
                static_cast<SCCOL>(j), static_cast<SCROW>(mrExternalRefInfo.mnRow));

            if (pToken)
            {
                pTab->setCell(static_cast<SCCOL>(j),
                              static_cast<SCROW>(mrExternalRefInfo.mnRow+i), pToken);
            }
        }
    }
    mrExternalRefInfo.mnRow += mnRepeatRowCount;
}

ScXMLExternalRefCellContext::ScXMLExternalRefCellContext(
    ScXMLImport& rImport,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList, ScXMLExternalTabData& rRefInfo ) :
    ScXMLImportContext( rImport ),
    mrScImport(rImport),
    mrExternalRefInfo(rRefInfo),
    mfCellValue(0.0),
    mnRepeatCount(1),
    mnNumberFormat(-1),
    mnCellType(css::util::NumberFormat::UNDEFINED),
    mbIsNumeric(false),
    mbIsEmpty(true)
{
    using namespace ::xmloff::token;

    for (auto &it : *rAttrList)
    {
        switch ( it.getToken() )
        {
            case XML_ELEMENT(TABLE, XML_STYLE_NAME):
            {
                XMLTableStylesContext* pStyles = static_cast<XMLTableStylesContext*>(mrScImport.GetAutoStyles());
                const XMLTableStyleContext* pStyle = static_cast<const XMLTableStyleContext*>(
                    pStyles->FindStyleChildContext(XmlStyleFamily::TABLE_CELL, it.toString(), true));
                if (pStyle)
                    mnNumberFormat = const_cast<XMLTableStyleContext*>(pStyle)->GetNumberFormat();
            }
            break;
            case XML_ELEMENT(TABLE, XML_NUMBER_COLUMNS_REPEATED):
            {
                mnRepeatCount = ::std::max( it.toInt32(), static_cast<sal_Int32>(1) );
            }
            break;
            case XML_ELEMENT(OFFICE, XML_VALUE_TYPE):
            {
                mnCellType = ScXMLImport::GetCellType( it.toCString(), it.getLength() );
            }
            break;
            case XML_ELEMENT(OFFICE, XML_VALUE):
            {
                if ( !it.isEmpty() )
                {
                    mfCellValue = it.toDouble();
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_ELEMENT(OFFICE, XML_DATE_VALUE):
            {
                if ( !it.isEmpty() && mrScImport.SetNullDateOnUnitConverter() )
                {
                    mrScImport.GetMM100UnitConverter().convertDateTime( mfCellValue, it.toString() );
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_ELEMENT(OFFICE, XML_TIME_VALUE):
            {
                if ( !it.isEmpty() )
                {
                    ::sax::Converter::convertDuration( mfCellValue, it.toString() );
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
            {
                if ( !it.isEmpty() )
                {
                    maCellString = it.toString();
                    mbIsNumeric = false;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_ELEMENT(OFFICE, XML_BOOLEAN_VALUE):
            {
                if ( !it.isEmpty() )
                {
                    mfCellValue = IsXMLToken( it, XML_TRUE ) ? 1.0 : 0.0;
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            default:
                ;
        }
    }
}

ScXMLExternalRefCellContext::~ScXMLExternalRefCellContext()
{
}

Reference< XFastContextHandler > SAL_CALL ScXMLExternalRefCellContext::createFastChildContext(
    sal_Int32 nElement, const Reference< XFastAttributeList >& /*xAttrList*/ )
{
    if (nElement == XML_ELEMENT(TEXT, XML_P))
        return new ScXMLExternalRefCellTextContext(mrScImport, *this);

    return nullptr;
}

void SAL_CALL ScXMLExternalRefCellContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!maCellString.isEmpty())
        mbIsEmpty = false;

    for (sal_Int32 i = 0; i < mnRepeatCount; ++i, ++mrExternalRefInfo.mnCol)
    {
        if (mbIsEmpty)
            continue;

        ScExternalRefCache::TokenRef aToken;
        if (mbIsNumeric)
            aToken.reset(new formula::FormulaDoubleToken(mfCellValue));
        else
        {
            ScDocument& rDoc = mrScImport.GetDoc().getDoc();
            svl::SharedString aSS = rDoc.GetSharedStringPool().intern(maCellString);
            aToken.reset(new formula::FormulaStringToken(aSS));
        }

        sal_uInt32 nNumFmt = mnNumberFormat >= 0 ? static_cast<sal_uInt32>(mnNumberFormat) : 0;
        mrExternalRefInfo.mpCacheTable->setCell(
            static_cast<SCCOL>(mrExternalRefInfo.mnCol),
            static_cast<SCROW>(mrExternalRefInfo.mnRow),
            aToken, nNumFmt);
    }
}

void ScXMLExternalRefCellContext::SetCellString(const OUString& rStr)
{
    maCellString = rStr;
}

ScXMLExternalRefCellTextContext::ScXMLExternalRefCellTextContext(
    ScXMLImport& rImport,
    ScXMLExternalRefCellContext& rParent ) :
    ScXMLImportContext( rImport ),
    mrParent(rParent)
{
}

ScXMLExternalRefCellTextContext::~ScXMLExternalRefCellTextContext()
{
}

void SAL_CALL ScXMLExternalRefCellTextContext::characters( const OUString& rChars )
{
    maCellStrBuf.append( rChars );
}

void SAL_CALL ScXMLExternalRefCellTextContext::endFastElement( sal_Int32 /*nElement*/ )
{
    mrParent.SetCellString( maCellStrBuf.makeStringAndClear() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
