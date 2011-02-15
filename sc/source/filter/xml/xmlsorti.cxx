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

#include "xmlsorti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "unonames.hxx"
#include "rangeutl.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <comphelper/extract.hxx>
#include <xmloff/xmltoken.hxx>

#define SC_USERLIST "UserList"

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLSortContext::ScXMLSortContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext),
    sCountry(),
    sLanguage(),
    sAlgorithm(),
    nUserListIndex(0),
    bCopyOutputData(sal_False),
    bBindFormatsToContent(sal_True),
    bIsCaseSensitive(sal_False),
    bEnabledUserList(sal_False)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetSortAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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
                if (ScRangeStringConverter::GetRangeFromString( aScRange, sValue, GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset ))
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

SvXMLImportContext *ScXMLSortContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetSortElemTokenMap());
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
        ++i;
    if (nAlgoLength)
        ++i;
    uno::Sequence <beans::PropertyValue> aSortDescriptor(7 + i);
    aSortDescriptor[0].Name = rtl::OUString::createFromAscii(SC_UNONAME_BINDFMT);
    aSortDescriptor[0].Value = ::cppu::bool2any(bBindFormatsToContent);
    aSortDescriptor[1].Name = rtl::OUString::createFromAscii(SC_UNONAME_COPYOUT);
    aSortDescriptor[1].Value = ::cppu::bool2any(bCopyOutputData);
    aSortDescriptor[2].Name = rtl::OUString::createFromAscii(SC_UNONAME_ISCASE);
    aSortDescriptor[2].Value = ::cppu::bool2any(bIsCaseSensitive);
    aSortDescriptor[3].Name = rtl::OUString::createFromAscii(SC_UNONAME_ISULIST);
    aSortDescriptor[3].Value = ::cppu::bool2any(bEnabledUserList);
    aSortDescriptor[4].Name = rtl::OUString::createFromAscii(SC_UNONAME_OUTPOS);
    aSortDescriptor[4].Value <<= aOutputPosition;
    aSortDescriptor[5].Name = rtl::OUString::createFromAscii(SC_UNONAME_UINDEX);
    aSortDescriptor[5].Value <<= nUserListIndex;
    aSortDescriptor[6].Name = rtl::OUString::createFromAscii(SC_UNONAME_SORTFLD);
    aSortDescriptor[6].Value <<= aSortFields;
    if (nLangLength || nCountryLength)
    {
        lang::Locale aLocale;
        aLocale.Language = sLanguage;
        aLocale.Country = sCountry;
        aSortDescriptor[7].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COLLLOC));
        aSortDescriptor[7].Value <<= aLocale;
    }
    if (nAlgoLength)
    {
        aSortDescriptor[6 + i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COLLALG));
        aSortDescriptor[6 + i].Value <<= sAlgorithm;
    }
    pDatabaseRangeContext->SetSortSequence(aSortDescriptor);
}

void ScXMLSortContext::AddSortField(const rtl::OUString& sFieldNumber, const rtl::OUString& sDataType, const rtl::OUString& sOrder)
{
    util::SortField aSortField;
    aSortField.Field = sFieldNumber.toInt32();
    if (IsXMLToken(sOrder, XML_ASCENDING))
        aSortField.SortAscending = sal_True;
    else
        aSortField.SortAscending = sal_False;
    if (sDataType.getLength() > 8)
    {
        rtl::OUString sTemp = sDataType.copy(0, 8);
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLSortContext* pTempSortContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pSortContext(pTempSortContext),
    sDataType(GetXMLToken(XML_AUTOMATIC)),
    sOrder(GetXMLToken(XML_ASCENDING))
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetSortSortByAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLSortByContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLSortByContext::EndElement()
{
    pSortContext->AddSortField(sFieldNumber, sDataType, sOrder);
}

