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

#include "xmldrani.hxx"
#include "xmlsorti.hxx"
#include "xmlimprt.hxx"
#include <convuno.hxx>
#include <unonames.hxx>
#include <rangeutl.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include <com/sun/star/util/SortField.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLSortContext::ScXMLSortContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    ScXMLImportContext( rImport ),
    pDatabaseRangeContext(pTempDatabaseRangeContext),
    sAlgorithm(),
    nUserListIndex(0),
    bCopyOutputData(false),
    bBindFormatsToContent(true),
    bIsCaseSensitive(false),
    bEnabledUserList(false)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_BIND_STYLES_TO_CONTENT ):
                {
                    bBindFormatsToContent = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_TARGET_RANGE_ADDRESS ):
                {
                    ScRange aScRange;
                    sal_Int32 nOffset(0);
                    if (ScRangeStringConverter::GetRangeFromString( aScRange, aIter.toString(), GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                    {
                        ScUnoConversion::FillApiAddress( aOutputPosition, aScRange.aStart );
                        bCopyOutputData = true;
                    }
                }
                break;
                case XML_ELEMENT( TABLE, XML_CASE_SENSITIVE ):
                {
                    bIsCaseSensitive = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_RFC_LANGUAGE_TAG ):
                {
                    maLanguageTagODF.maRfcLanguageTag = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_LANGUAGE ):
                {
                    maLanguageTagODF.maLanguage = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_SCRIPT ):
                {
                    maLanguageTagODF.maScript = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_COUNTRY ):
                {
                    maLanguageTagODF.maCountry = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_ALGORITHM ):
                {
                    sAlgorithm = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLSortContext::~ScXMLSortContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLSortContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_SORT_BY ):
        {
            pContext = new ScXMLSortByContext( GetScImport(), nElement, pAttribList, this );
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLSortContext::endFastElement( sal_Int32 /*nElement*/ )
{
    sal_Int32 nAlgoLength(sAlgorithm.getLength());
    sal_uInt8 i (0);
    if (!maLanguageTagODF.isEmpty())
        ++i;
    if (nAlgoLength)
        ++i;
    uno::Sequence <beans::PropertyValue> aSortDescriptor(7 + i);
    aSortDescriptor[0].Name = SC_UNONAME_BINDFMT;
    aSortDescriptor[0].Value <<= bBindFormatsToContent;
    aSortDescriptor[1].Name = SC_UNONAME_COPYOUT;
    aSortDescriptor[1].Value <<= bCopyOutputData;
    aSortDescriptor[2].Name = SC_UNONAME_ISCASE;
    aSortDescriptor[2].Value <<= bIsCaseSensitive;
    aSortDescriptor[3].Name = SC_UNONAME_ISULIST;
    aSortDescriptor[3].Value <<= bEnabledUserList;
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
                                      sal_Int32 /*nElement*/,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLSortContext* pTempSortContext) :
    ScXMLImportContext( rImport ),
    pSortContext(pTempSortContext),
    sDataType(GetXMLToken(XML_AUTOMATIC)),
    sOrder(GetXMLToken(XML_ASCENDING))
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_FIELD_NUMBER ):
                {
                    sFieldNumber = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_DATA_TYPE ):
                {
                    sDataType = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_ORDER ):
                {
                    sOrder = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLSortByContext::~ScXMLSortByContext()
{
}

void SAL_CALL ScXMLSortByContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pSortContext->AddSortField(sFieldNumber, sDataType, sOrder);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
