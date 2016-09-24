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

#include "xmlsorti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "unonames.hxx"
#include "rangeutl.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLSortContext::ScXMLSortContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<
                                      css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext),
    sAlgorithm(),
    nUserListIndex(0),
    bCopyOutputData(false),
    bBindFormatsToContent(true),
    bIsCaseSensitive(false),
    bEnabledUserList(false)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetSortAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                    bCopyOutputData = true;
                }
            }
            break;
            case XML_TOK_SORT_ATTR_CASE_SENSITIVE :
            {
                bIsCaseSensitive = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_SORT_ATTR_RFC_LANGUAGE_TAG :
                maLanguageTagODF.maRfcLanguageTag = sValue;
            break;
            case XML_TOK_SORT_ATTR_LANGUAGE :
                maLanguageTagODF.maLanguage = sValue;
            break;
            case XML_TOK_SORT_ATTR_SCRIPT :
                maLanguageTagODF.maScript = sValue;
            break;
            case XML_TOK_SORT_ATTR_COUNTRY :
                maLanguageTagODF.maCountry = sValue;
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
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
    sal_Int32 nAlgoLength(sAlgorithm.getLength());
    sal_uInt8 i (0);
    if (!maLanguageTagODF.isEmpty())
        ++i;
    if (nAlgoLength)
        ++i;
    uno::Sequence <beans::PropertyValue> aSortDescriptor(7 + i);
    aSortDescriptor[0].Name = SC_UNONAME_BINDFMT;
    aSortDescriptor[0].Value = css::uno::makeAny(bBindFormatsToContent);
    aSortDescriptor[1].Name = SC_UNONAME_COPYOUT;
    aSortDescriptor[1].Value = css::uno::makeAny(bCopyOutputData);
    aSortDescriptor[2].Name = SC_UNONAME_ISCASE;
    aSortDescriptor[2].Value = css::uno::makeAny(bIsCaseSensitive);
    aSortDescriptor[3].Name = SC_UNONAME_ISULIST;
    aSortDescriptor[3].Value = css::uno::makeAny(bEnabledUserList);
    aSortDescriptor[4].Name = SC_UNONAME_OUTPOS;
    aSortDescriptor[4].Value <<= aOutputPosition;
    aSortDescriptor[5].Name = SC_UNONAME_UINDEX;
    aSortDescriptor[5].Value <<= nUserListIndex;
    aSortDescriptor[6].Name = SC_UNONAME_SORTFLD;
    aSortDescriptor[6].Value <<= aSortFields;
    if (!maLanguageTagODF.isEmpty())
    {
        aSortDescriptor[7].Name = SC_UNONAME_COLLLOC;
        aSortDescriptor[7].Value <<= maLanguageTagODF.getLanguageTag().getLocale( false);
    }
    if (nAlgoLength)
    {
        aSortDescriptor[6 + i].Name = SC_UNONAME_COLLALG;
        aSortDescriptor[6 + i].Value <<= sAlgorithm;
    }
    pDatabaseRangeContext->SetSortSequence(aSortDescriptor);
}

void ScXMLSortContext::AddSortField(const OUString& sFieldNumber, const OUString& sDataType, const OUString& sOrder)
{
    util::SortField aSortField;
    aSortField.Field = sFieldNumber.toInt32();
    if (IsXMLToken(sOrder, XML_ASCENDING))
        aSortField.SortAscending = true;
    else
        aSortField.SortAscending = false;
    if (sDataType.getLength() > 8)
    {
        OUString sTemp = sDataType.copy(0, 8);
        if (sTemp == "UserList")
        {
            bEnabledUserList = true;
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
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
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
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLSortByContext::EndElement()
{
    pSortContext->AddSortField(sFieldNumber, sDataType, sOrder);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
