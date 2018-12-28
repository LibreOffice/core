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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <formula/grammar.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLDataStreamContext::ScXMLDataStreamContext(
    ScXMLImport& rImport,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext(rImport),
    mbRefreshOnEmpty(false),
    meInsertPos(sc::ImportPostProcessData::DataStream::InsertBottom)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch ( aIter.getToken() )
            {
                case XML_ELEMENT( XLINK, XML_HREF ):
                    maURL = GetScImport().GetAbsoluteReference( aIter.toString() );
                break;
                case XML_ELEMENT( TABLE, XML_TARGET_RANGE_ADDRESS ):
                {
                    ScDocument* pDoc = GetScImport().GetDocument();
                    sal_Int32 nOffset = 0;
                    if (!ScRangeStringConverter::GetRangeFromString(
                        maRange, aIter.toString(), pDoc, formula::FormulaGrammar::CONV_OOO, nOffset))
                        maRange.SetInvalid();
                }
                break;
                case XML_ELEMENT( CALC_EXT, XML_EMPTY_LINE_REFRESH ):
                    mbRefreshOnEmpty = IsXMLToken( aIter, XML_TRUE );
                break;
                case XML_ELEMENT( CALC_EXT, XML_INSERTION_POSITION ):
                    meInsertPos = IsXMLToken( aIter, XML_TOP ) ?
                        sc::ImportPostProcessData::DataStream::InsertTop :
                        sc::ImportPostProcessData::DataStream::InsertBottom;
                break;
                default:
                    ;
            }
        }
    }
}

ScXMLDataStreamContext::~ScXMLDataStreamContext() {}

void SAL_CALL ScXMLDataStreamContext::endFastElement( sal_Int32 /*nElement*/ )
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
