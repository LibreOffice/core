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

#include "xmllabri.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmlimprt.hxx"
#include <xmloff/xmlnmspe.hxx>

using namespace ::com::sun::star;
using namespace xmloff::token;

ScXMLLabelRangesContext::ScXMLLabelRangesContext(
        ScXMLImport& rImport ):
    ScXMLImportContext( rImport )
{
    rImport.LockSolarMutex();
}

ScXMLLabelRangesContext::~ScXMLLabelRangesContext()
{
    GetScImport().UnlockSolarMutex();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLLabelRangesContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext*     pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_LABEL_RANGE ):
            pContext = new ScXMLLabelRangeContext( GetScImport(), pAttribList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLLabelRangeContext::ScXMLLabelRangeContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    bColumnOrientation( false )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_LABEL_CELL_RANGE_ADDRESS ):
                sLabelRangeStr = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_DATA_CELL_RANGE_ADDRESS ):
                sDataRangeStr = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_ORIENTATION ):
                bColumnOrientation = IsXMLToken(aIter, XML_COLUMN );
                break;
            }
        }
    }
}

ScXMLLabelRangeContext::~ScXMLLabelRangeContext()
{
}

void SAL_CALL ScXMLLabelRangeContext::endFastElement( sal_Int32 /*nElement*/ )
{
    //  Label ranges must be stored as strings until all sheets are loaded
    //  (like named expressions).

    auto pLabelRange = std::make_unique<ScMyLabelRange>(
                ScMyLabelRange{sLabelRangeStr, sDataRangeStr, bColumnOrientation});

    GetScImport().AddLabelRange(std::move(pLabelRange));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
