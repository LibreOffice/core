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

#include "xmlconti.hxx"
#include "xmlimprt.hxx"

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

using namespace xmloff::token;

ScXMLContentContext::ScXMLContentContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      OUStringBuffer& sTempValue) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    sOUText(),
    sValue(sTempValue)
{
}

ScXMLContentContext::~ScXMLContentContext()
{
}

SvXMLImportContextRef ScXMLContentContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    if ((nPrefix == XML_NAMESPACE_TEXT) && IsXMLToken(rLName, XML_S))
    {
        sal_Int32 nRepeat(0);
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const OUString& sAttrName(xAttrList->getNameByIndex( i ));
            const OUString& sAttrValue(xAttrList->getValueByIndex( i ));
            OUString aLocalName;
            sal_uInt16 nPrfx = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            if ((nPrfx == XML_NAMESPACE_TEXT) && IsXMLToken(aLocalName, XML_C))
                nRepeat = sAttrValue.toInt32();
        }
        if (nRepeat)
            for (sal_Int32 j = 0; j < nRepeat; ++j)
                sOUText.append(' ');
        else
            sOUText.append(' ');
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLContentContext::Characters( const OUString& rChars )
{
    sOUText.append(rChars);
}

void ScXMLContentContext::EndElement()
{
    sValue.append(sOUText.toString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
