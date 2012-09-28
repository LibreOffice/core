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

#include "xmlexternaltabi.hxx"
#include "xmlimprt.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"

#include "token.hxx"
#include "document.hxx"

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

#include <sax/tools/converter.hxx>

#include <com/sun/star/util/NumberFormat.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

// ============================================================================

ScXMLExternalRefTabSourceContext::ScXMLExternalRefTabSourceContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList, ScXMLExternalTabData& rRefInfo ) :
    SvXMLImportContext( rImport, nPrefix, rLName ),
    mrScImport(rImport),
    mrExternalRefInfo(rRefInfo)
{
    using namespace ::xmloff::token;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        const rtl::OUString& sAttrName = xAttrList->getNameByIndex(i);
        rtl::OUString aLocalName;
        sal_uInt16 nAttrPrefix = mrScImport.GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        const rtl::OUString& sValue = xAttrList->getValueByIndex(i);
        if (nAttrPrefix == XML_NAMESPACE_XLINK)
        {
            if (IsXMLToken(aLocalName, XML_HREF))
                maRelativeUrl = sValue;
        }
        else if (nAttrPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_TABLE_NAME))
                maTableName = sValue;
            else if (IsXMLToken(aLocalName, XML_FILTER_NAME))
                maFilterName = sValue;
            else if (IsXMLToken(aLocalName, XML_FILTER_OPTIONS))
                maFilterOptions = sValue;
        }
    }
}

ScXMLExternalRefTabSourceContext::~ScXMLExternalRefTabSourceContext()
{
}

SvXMLImportContext* ScXMLExternalRefTabSourceContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
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

void ScXMLExternalRefTabSourceContext::EndElement()
{
    ScDocument* pDoc = mrScImport.GetDocument();
    if (!pDoc)
        return;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    if (lcl_isValidRelativeURL(maRelativeUrl))
        pRefMgr->setRelativeFileName(mrExternalRefInfo.mnFileId, maRelativeUrl);
    pRefMgr->setFilterData(mrExternalRefInfo.mnFileId, maFilterName, maFilterOptions);
}

// ============================================================================

ScXMLExternalRefRowsContext::ScXMLExternalRefRowsContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& /* xAttrList */, ScXMLExternalTabData& rRefInfo ) :
    SvXMLImportContext( rImport, nPrefix, rLName ),
    mrScImport(rImport),
    mrExternalRefInfo(rRefInfo)
{
}

ScXMLExternalRefRowsContext::~ScXMLExternalRefRowsContext()
{
}

SvXMLImportContext* ScXMLExternalRefRowsContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& xAttrList )
{
    // #i101319# row elements inside group, rows or header-rows
    // are treated like row elements directly in the table element

    const SvXMLTokenMap& rTokenMap = mrScImport.GetTableRowsElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    switch (nToken)
    {
        case XML_TOK_TABLE_ROWS_ROW_GROUP:
        case XML_TOK_TABLE_ROWS_HEADER_ROWS:
        case XML_TOK_TABLE_ROWS_ROWS:
            return new ScXMLExternalRefRowsContext(
                mrScImport, nPrefix, rLocalName, xAttrList, mrExternalRefInfo);
        case XML_TOK_TABLE_ROWS_ROW:
            return new ScXMLExternalRefRowContext(
                mrScImport, nPrefix, rLocalName, xAttrList, mrExternalRefInfo);
        default:
            ;
    }
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLExternalRefRowsContext::EndElement()
{
}

// ============================================================================

ScXMLExternalRefRowContext::ScXMLExternalRefRowContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList, ScXMLExternalTabData& rRefInfo ) :
    SvXMLImportContext( rImport, nPrefix, rLName ),
    mrScImport(rImport),
    mrExternalRefInfo(rRefInfo),
    mnRepeatRowCount(1)
{
    mrExternalRefInfo.mnCol = 0;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = mrScImport.GetTableRowAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName = xAttrList->getNameByIndex(i);
        rtl::OUString aLocalName;
        sal_uInt16 nAttrPrefix = mrScImport.GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        const rtl::OUString& sValue = xAttrList->getValueByIndex(i);

        switch (rAttrTokenMap.Get(nAttrPrefix, aLocalName))
        {
            case XML_TOK_TABLE_ROW_ATTR_REPEATED:
            {
                mnRepeatRowCount = std::max(sValue.toInt32(), static_cast<sal_Int32>(1));
            }
            break;
        }
    }
}

ScXMLExternalRefRowContext::~ScXMLExternalRefRowContext()
{
}

SvXMLImportContext* ScXMLExternalRefRowContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = mrScImport.GetTableRowElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    if (nToken == XML_TOK_TABLE_ROW_CELL || nToken == XML_TOK_TABLE_ROW_COVERED_CELL)
        return new ScXMLExternalRefCellContext(mrScImport, nPrefix, rLocalName, xAttrList, mrExternalRefInfo);

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLExternalRefRowContext::EndElement()
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

            if (pToken.get())
            {
                pTab->setCell(static_cast<SCCOL>(j),
                              static_cast<SCROW>(mrExternalRefInfo.mnRow+i), pToken);
            }
        }
    }
    mrExternalRefInfo.mnRow += mnRepeatRowCount;
}

// ============================================================================

ScXMLExternalRefCellContext::ScXMLExternalRefCellContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList, ScXMLExternalTabData& rRefInfo ) :
    SvXMLImportContext( rImport, nPrefix, rLName ),
    mrScImport(rImport),
    mrExternalRefInfo(rRefInfo),
    mfCellValue(0.0),
    mnRepeatCount(1),
    mnNumberFormat(-1),
    mnCellType(::com::sun::star::util::NumberFormat::UNDEFINED),
    mbIsNumeric(false),
    mbIsEmpty(true)
{
    using namespace ::xmloff::token;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rTokenMap = rImport.GetTableRowCellAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        OUString aLocalName;
        sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
            xAttrList->getNameByIndex(i), &aLocalName);

        const rtl::OUString& sValue = xAttrList->getValueByIndex(i);
        sal_uInt16 nToken = rTokenMap.Get(nAttrPrefix, aLocalName);

        switch (nToken)
        {
            case XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME:
            {
                XMLTableStylesContext* pStyles = static_cast<XMLTableStylesContext*>(mrScImport.GetAutoStyles());
                const XMLTableStyleContext* pStyle = static_cast<const XMLTableStyleContext*>(
                    pStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL, sValue, true));
                if (pStyle)
                    mnNumberFormat = const_cast<XMLTableStyleContext*>(pStyle)->GetNumberFormat();
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED:
            {
                mnRepeatCount = ::std::max(sValue.toInt32(), static_cast<sal_Int32>(1));
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE:
            {
                mnCellType = mrScImport.GetCellType(sValue);
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    ::sax::Converter::convertDouble(mfCellValue, sValue);
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE:
            {
                if (!sValue.isEmpty() && mrScImport.SetNullDateOnUnitConverter())
                {
                    mrScImport.GetMM100UnitConverter().convertDateTime(mfCellValue, sValue);
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    ::sax::Converter::convertDuration(mfCellValue, sValue);
                    mbIsNumeric = true;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    maCellString = sValue;
                    mbIsNumeric = false;
                    mbIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    mfCellValue = IsXMLToken(sValue, XML_TRUE) ? 1.0 : 0.0;
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

SvXMLImportContext* ScXMLExternalRefCellContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = mrScImport.GetTableRowCellElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    if (nToken == XML_TOK_TABLE_ROW_CELL_P)
        return new ScXMLExternalRefCellTextContext(mrScImport, nPrefix, rLocalName, xAttrList, *this);

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLExternalRefCellContext::EndElement()
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
            aToken.reset(new formula::FormulaStringToken(maCellString));

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

// ============================================================================

ScXMLExternalRefCellTextContext::ScXMLExternalRefCellTextContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& /*xAttrList*/,
    ScXMLExternalRefCellContext& rParent ) :
    SvXMLImportContext( rImport, nPrefix, rLName ),
    mrParent(rParent)
{
}

ScXMLExternalRefCellTextContext::~ScXMLExternalRefCellTextContext()
{
}

SvXMLImportContext* ScXMLExternalRefCellTextContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLExternalRefCellTextContext::Characters(const OUString& rChar)
{
    maCellStrBuf.append(rChar);
}

void ScXMLExternalRefCellTextContext::EndElement()
{
    mrParent.SetCellString(maCellStrBuf.makeStringAndClear());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
