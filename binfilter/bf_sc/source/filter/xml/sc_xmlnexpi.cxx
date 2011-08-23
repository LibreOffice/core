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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "xmlnexpi.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"
//#include "document.hxx"
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <bf_xmloff/nmspmap.hxx>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#ifndef _COM_SUN_STAR_SHEET_XNAMEDRANGE_HPP_
#include <com/sun/star/sheet/XNamedRange.hpp>
#endif
namespace binfilter {

#define SC_NAMEDRANGES "NamedRanges"
#define SC_REPEAT_COLUMN "repeat-column"
#define SC_REPEAT_ROW "repeat-row"
#define SC_FILTER "filter"
#define SC_PRINT_RANGE "print-range"

using namespace ::com::sun::star;

//------------------------------------------------------------------

ScXMLNamedExpressionsContext::ScXMLNamedExpressionsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
/*	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetNamedRangeAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
        }
    }*/
    rImport.LockSolarMutex();
}

ScXMLNamedExpressionsContext::~ScXMLNamedExpressionsContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLNamedExpressionsContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetNamedExpressionsElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE:
        pContext = new ScXMLNamedRangeContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    case XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION:
        pContext = new ScXMLNamedExpressionContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

sal_Int32 ScXMLNamedExpressionsContext::GetRangeType(const ::rtl::OUString sRangeType) const
{
    sal_Int32 nRangeType = 0;
    ::rtl::OUStringBuffer sBuffer;
    sal_Int16 i = 0;
    while (i <= sRangeType.getLength())
    {
        if ((sRangeType[i] == ' ') || (i == sRangeType.getLength()))
        {
            ::rtl::OUString sTemp = sBuffer.makeStringAndClear();
            if (sTemp.compareToAscii(SC_REPEAT_COLUMN) == 0)
                nRangeType |= sheet::NamedRangeFlag::COLUMN_HEADER;
            else if (sTemp.compareToAscii(SC_REPEAT_ROW) == 0)
                nRangeType |= sheet::NamedRangeFlag::ROW_HEADER;
            else if (sTemp.compareToAscii(SC_FILTER) == 0)
                 nRangeType |= sheet::NamedRangeFlag::FILTER_CRITERIA;
            else if (sTemp.compareToAscii(SC_PRINT_RANGE) == 0)
                 nRangeType |= sheet::NamedRangeFlag::PRINT_AREA;
        }
        else if (i < sRangeType.getLength())
            sBuffer.append(sRangeType[i]);
        i++;
    }
    return nRangeType;
}

void ScXMLNamedExpressionsContext::EndElement()
{
    if (GetScImport().GetModel().is())
    {
        uno::Reference <beans::XPropertySet> xPropertySet (GetScImport().GetModel(), uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aNamedRanges = xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NAMEDRANGES)));
            uno::Reference <sheet::XNamedRanges> xNamedRanges;
            if (aNamedRanges >>= xNamedRanges)
            {
                ScMyNamedExpressions* pNamedExpressions = GetScImport().GetNamedExpressions();
                ScMyNamedExpressions::iterator aItr = pNamedExpressions->begin();
                ScMyNamedExpressions::const_iterator aEndItr = pNamedExpressions->end();
                table::CellAddress aCellAddress;
                ::rtl::OUString sTempContent(RTL_CONSTASCII_USTRINGPARAM("0"));
                while (aItr != aEndItr)
                {
                    sal_Int32 nOffset(0);
                    if (ScXMLConverter::GetAddressFromString(
                        aCellAddress, (*aItr)->sBaseCellAddress, GetScImport().GetDocument(), nOffset ))
                    {
                        try
                        {
                            xNamedRanges->addNewByName((*aItr)->sName, sTempContent, aCellAddress, GetRangeType((*aItr)->sRangeType));
                        }
                        catch( uno::RuntimeException& r )
                        {
                            DBG_ERROR("here are some Named Ranges with the same name");
                            uno::Reference < container::XIndexAccess > xIndex(xNamedRanges, uno::UNO_QUERY);
                            if (xIndex.is())
                            {
                                sal_Int32 nMax(xIndex->getCount());
                                sal_Bool bInserted(sal_False);
                                sal_Int32 nCount(1);
                                ::rtl::OUStringBuffer sName((*aItr)->sName);
                                sName.append(sal_Unicode('_'));
                                while (!bInserted && nCount <= nMax)
                                {
                                    ::rtl::OUStringBuffer sTemp(sName);
                                    sTemp.append(::rtl::OUString::valueOf(nCount));
                                    try
                                    {
                                        xNamedRanges->addNewByName(sTemp.makeStringAndClear(), sTempContent, aCellAddress, GetRangeType((*aItr)->sRangeType));
                                        bInserted = sal_True;
                                    }
                                    catch( uno::RuntimeException& rE )
                                    {
                                        ++nCount;
                                    }
                                }
                            }
                        }
                    }
                    aItr++;
                }
                aItr = pNamedExpressions->begin();
                while (aItr != aEndItr)
                {
                    sal_Int32 nOffset(0);
                    if (ScXMLConverter::GetAddressFromString(
                        aCellAddress, (*aItr)->sBaseCellAddress, GetScImport().GetDocument(), nOffset ))
                    {
                        sTempContent = (*aItr)->sContent;
                        ScXMLConverter::ParseFormula(sTempContent, (*aItr)->bIsExpression);
                        uno::Any aNamedRange = xNamedRanges->getByName((*aItr)->sName);
                        uno::Reference <sheet::XNamedRange> xNamedRange;
                        if (aNamedRange >>= xNamedRange)
                            xNamedRange->setContent(sTempContent);
                    }
                    delete *aItr;
                    aItr = pNamedExpressions->erase(aItr);
                }
            }
        }
    }
}

ScXMLNamedRangeContext::ScXMLNamedRangeContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    ScMyNamedExpression* pNamedExpression = new ScMyNamedExpression;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetNamedRangeAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_NAMED_RANGE_ATTR_NAME :
            {
                pNamedExpression->sName = sValue;
            }
            break;
            case XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS :
            {
                pNamedExpression->sContent = sValue;
            }
            break;
            case XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS :
            {
                pNamedExpression->sBaseCellAddress = sValue;
            }
            break;
            case XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS :
            {
                pNamedExpression->sRangeType = sValue;
            }
            break;
        }
    }
    pNamedExpression->bIsExpression = sal_False;
    GetScImport().AddNamedExpression(pNamedExpression);
}

ScXMLNamedRangeContext::~ScXMLNamedRangeContext()
{
}

SvXMLImportContext *ScXMLNamedRangeContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

/*	const SvXMLTokenMap& rTokenMap = GetScImport().GetTableElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    }*/

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLNamedRangeContext::EndElement()
{
}

ScXMLNamedExpressionContext::ScXMLNamedExpressionContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    ScMyNamedExpression* pNamedExpression = new ScMyNamedExpression;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetNamedExpressionAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_NAMED_EXPRESSION_ATTR_NAME :
            {
                pNamedExpression->sName = sValue;
            }
            break;
            case XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION :
            {
                pNamedExpression->sContent = sValue;
            }
            break;
            case XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS :
            {
                pNamedExpression->sBaseCellAddress = sValue;
            }
            break;
        }
    }
    pNamedExpression->bIsExpression = sal_True;
    GetScImport().AddNamedExpression(pNamedExpression);
}

ScXMLNamedExpressionContext::~ScXMLNamedExpressionContext()
{
}

SvXMLImportContext *ScXMLNamedExpressionContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

/*	const SvXMLTokenMap& rTokenMap = GetScImport().GetTableElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    }*/

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLNamedExpressionContext::EndElement()
{
}

}
