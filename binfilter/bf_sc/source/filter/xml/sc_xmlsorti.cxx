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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "xmlsorti.hxx"
#include "xmlimprt.hxx"

#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "unonames.hxx"

#include <bf_xmloff/nmspmap.hxx>
#include <comphelper/extract.hxx>
namespace binfilter {

#define SC_USERLIST "UserList"

using namespace ::com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLSortContext::ScXMLSortContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    bEnabledUserList(sal_False),
    bBindFormatsToContent(sal_True),
    bIsCaseSensitive(sal_False),
    bCopyOutputData(sal_False),
    sCountry(),
    sLanguage(),
    sAlgorithm(),
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    nUserListIndex = 0;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSortAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT :
            {
                bBindFormatsToContent = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                sal_Int32 nOffset(0);
                if (ScXMLConverter::GetRangeFromString( aScRange, sValue, GetScImport().GetDocument(), nOffset ))
                {
                    ScUnoConversion::FillApiAddress( aOutputPosition, aScRange.aStart );
                    bCopyOutputData = sal_True;
                }
            }
            break;
            case XML_TOK_SORT_ATTR_CASE_SENSITIVE :
            {
                bIsCaseSensitive = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_SORT_ATTR_LANGUAGE :
                sLanguage = sValue;
            break;
            case XML_TOK_SORT_ATTR_COUNTRY :
                sCountry = sValue;
            break;
            case XML_TOK_SORT_ATTR_ALGORITHM :
                sAlgorithm = sValue;
            break;
        }
    }
}

ScXMLSortContext::~ScXMLSortContext()
{
}

SvXMLImportContext *ScXMLSortContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetSortElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SORT_SORT_BY :
        {
            pContext = new ScXMLSortByContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSortContext::EndElement()
{
    sal_Int32 nLangLength(sLanguage.getLength());
    sal_Int32 nCountryLength(sCountry.getLength());
    sal_Int32 nAlgoLength(sAlgorithm.getLength());
    sal_uInt8 i (0);
    if (nLangLength || nCountryLength)
        i++;
    if (nAlgoLength)
        i++;
    uno::Sequence <beans::PropertyValue> aSortDescriptor(7 + i);
    uno::Any aTemp;
    beans::PropertyValue aPropertyValue;
    aTemp = ::cppu::bool2any(bBindFormatsToContent);
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_BINDFMT);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[0] = aPropertyValue;
    aTemp = ::cppu::bool2any(bCopyOutputData);
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_COPYOUT);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[1] = aPropertyValue;
    aTemp = ::cppu::bool2any(bIsCaseSensitive);
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_ISCASE);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[2] = aPropertyValue;
    aTemp = ::cppu::bool2any(bEnabledUserList);
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_ISULIST);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[3] = aPropertyValue;
    aTemp <<= aOutputPosition;
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_OUTPOS);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[4] = aPropertyValue;
    aTemp <<= nUserListIndex;
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_UINDEX);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[5] = aPropertyValue;
    aTemp <<= aSortFields;
    aPropertyValue.Name = ::rtl::OUString::createFromAscii(SC_UNONAME_SORTFLD);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[6] = aPropertyValue;
    if (nLangLength || nCountryLength)
    {
        lang::Locale aLocale;
        aLocale.Language = sLanguage;
        aLocale.Country = sCountry;
        aTemp <<= aLocale;
        aPropertyValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COLLLOC));
        aPropertyValue.Value = aTemp;
        aSortDescriptor[7] = aPropertyValue;
    }
    if (nAlgoLength)
    {
        aTemp <<= sAlgorithm;
        aPropertyValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COLLALG));
        aPropertyValue.Value = aTemp;
        aSortDescriptor[6 + i] = aPropertyValue;
    }
    pDatabaseRangeContext->SetSortSequence(aSortDescriptor);
}

void ScXMLSortContext::AddSortField(const ::rtl::OUString& sFieldNumber, const ::rtl::OUString& sDataType, const ::rtl::OUString& sOrder)
{
    util::SortField aSortField;
    aSortField.Field = sFieldNumber.toInt32();
    if (IsXMLToken(sOrder, XML_ASCENDING))
        aSortField.SortAscending = sal_True;
    else
        aSortField.SortAscending = sal_False;
    if (sDataType.getLength() > 8)
    {
        ::rtl::OUString sTemp = sDataType.copy(0, 8);
        if (sTemp.compareToAscii(SC_USERLIST) == 0)
        {
            bEnabledUserList = sal_True;
            sTemp = sDataType.copy(8);
            nUserListIndex = static_cast<sal_Int16>(sTemp.toInt32());
        }
        else
        {
            if (IsXMLToken(sDataType, XML_AUTOMATIC))
                aSortField.FieldType = util::SortFieldType_AUTOMATIC;
        }
    }
    else
    {
        if (IsXMLToken(sDataType, XML_TEXT))
            aSortField.FieldType = util::SortFieldType_ALPHANUMERIC;
        else if (IsXMLToken(sDataType, XML_NUMBER))
            aSortField.FieldType = util::SortFieldType_NUMERIC;
    }
    aSortFields.realloc(aSortFields.getLength() + 1);
    aSortFields[aSortFields.getLength() - 1] = aSortField;
}

ScXMLSortByContext::ScXMLSortByContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLSortContext* pTempSortContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sOrder(GetXMLToken(XML_ASCENDING)),
    sDataType(GetXMLToken(XML_AUTOMATIC))
{
    pSortContext = pTempSortContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSortSortByAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SORT_BY_ATTR_FIELD_NUMBER :
            {
                sFieldNumber = sValue;
            }
            break;
            case XML_TOK_SORT_BY_ATTR_DATA_TYPE :
            {
                sDataType = sValue;
            }
            break;
            case XML_TOK_SORT_BY_ATTR_ORDER :
            {
                sOrder = sValue;
            }
            break;
        }
    }
}

ScXMLSortByContext::~ScXMLSortByContext()
{
}

SvXMLImportContext *ScXMLSortByContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSortByContext::EndElement()
{
    pSortContext->AddSortField(sFieldNumber, sDataType, sOrder);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
