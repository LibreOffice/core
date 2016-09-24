/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "datastreamimport.hxx"
#include "xmlimprt.hxx"

#include <rangeutl.hxx>
#include <importfilterdata.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <formula/grammar.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLDataStreamContext::ScXMLDataStreamContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) :
    ScXMLImportContext(rImport, nPrefix, rLocalName),
    mbRefreshOnEmpty(false),
    meInsertPos(sc::ImportPostProcessData::DataStream::InsertBottom)
{
    if (!xAttrList.is())
        return;

    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataStreamAttrTokenMap();

    for (sal_Int32 i = 0; i < xAttrList->getLength(); ++i)
    {
        const OUString& rName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nLocalPrefix =
            GetScImport().GetNamespaceMap().GetKeyByAttrName(rName, &aLocalName);

        const OUString& rVal = xAttrList->getValueByIndex(i);
        switch (rAttrTokenMap.Get(nLocalPrefix, aLocalName))
        {
            case XML_TOK_DATA_STREAM_ATTR_URL:
                maURL = GetScImport().GetAbsoluteReference(rVal);
            break;
            case XML_TOK_DATA_STREAM_ATTR_RANGE:
            {
                ScDocument* pDoc = GetScImport().GetDocument();
                sal_Int32 nOffset = 0;
                if (!ScRangeStringConverter::GetRangeFromString(
                    maRange, rVal, pDoc, formula::FormulaGrammar::CONV_OOO, nOffset))
                    maRange.SetInvalid();
            }
            break;
            case XML_TOK_DATA_STREAM_ATTR_EMPTY_LINE_REFRESH:
                mbRefreshOnEmpty = IsXMLToken(rVal, XML_TRUE);
            break;
            case XML_TOK_DATA_STREAM_ATTR_INSERTION_POSITION:
                meInsertPos = IsXMLToken(rVal, XML_TOP) ?
                    sc::ImportPostProcessData::DataStream::InsertTop :
                    sc::ImportPostProcessData::DataStream::InsertBottom;
            break;
            default:
                ;
        }
    }
}

ScXMLDataStreamContext::~ScXMLDataStreamContext() {}

void ScXMLDataStreamContext::EndElement()
{
    if (!maRange.IsValid())
        // Range must be valid.
        return;

    sc::ImportPostProcessData* pData = GetScImport().GetPostProcessData();
    if (!pData)
        return;

    pData->mpDataStream.reset(new sc::ImportPostProcessData::DataStream);
    sc::ImportPostProcessData::DataStream& rData = *pData->mpDataStream;

    rData.maURL = maURL;
    rData.maRange = maRange;
    rData.mbRefreshOnEmpty = mbRefreshOnEmpty;
    rData.meInsertPos = meInsertPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
