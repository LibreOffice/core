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
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmlimprt.hxx"

using namespace ::com::sun::star;
using namespace xmloff::token;

ScXMLLabelRangesContext::ScXMLLabelRangesContext(
        ScXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& /* xAttrList */ ):
    SvXMLImportContext( rImport, nPrefix, rLName )
{
    rImport.LockSolarMutex();
}

ScXMLLabelRangesContext::~ScXMLLabelRangesContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext* ScXMLLabelRangesContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext*     pContext(nullptr);
    const SvXMLTokenMap&    rTokenMap(GetScImport().GetLabelRangesElemTokenMap());

    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_LABEL_RANGE_ELEM:
            pContext = new ScXMLLabelRangeContext( GetScImport(), nPrefix, rLName, xAttrList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLLabelRangesContext::EndElement()
{
}

ScXMLLabelRangeContext::ScXMLLabelRangeContext(
        ScXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bColumnOrientation( false )
{
    sal_Int16               nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap&    rAttrTokenMap(GetScImport().GetLabelRangeAttrTokenMap());

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; ++nIndex )
    {
        const OUString& sAttrName  (xAttrList->getNameByIndex( nIndex ));
        const OUString& sValue     (xAttrList->getValueByIndex( nIndex ));
        OUString    aLocalName;
        sal_uInt16      nPrefix     (GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE:
                sLabelRangeStr = sValue;
            break;
            case XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE:
                sDataRangeStr = sValue;
            break;
            case XML_TOK_LABEL_RANGE_ATTR_ORIENTATION:
                bColumnOrientation = IsXMLToken(sValue, XML_COLUMN );
            break;
        }
    }
}

ScXMLLabelRangeContext::~ScXMLLabelRangeContext()
{
}

SvXMLImportContext* ScXMLLabelRangeContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLLabelRangeContext::EndElement()
{
    //  Label ranges must be stored as strings until all sheets are loaded
    //  (like named expressions).

    ScMyLabelRange* pLabelRange = new ScMyLabelRange;

    pLabelRange->sLabelRangeStr = sLabelRangeStr;
    pLabelRange->sDataRangeStr = sDataRangeStr;
    pLabelRange->bColumnOrientation = bColumnOrientation;

    GetScImport().AddLabelRange(pLabelRange);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
