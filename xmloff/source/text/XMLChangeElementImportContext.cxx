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

#include "XMLChangeElementImportContext.hxx"
#include "XMLChangedRegionImportContext.hxx"
#include "XMLChangeInfoContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>



using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_P;
using ::xmloff::token::XML_CHANGE_INFO;

TYPEINIT1( XMLChangeElementImportContext, SvXMLImportContext );

XMLChangeElementImportContext::XMLChangeElementImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    bool bAccContent,
    XMLChangedRegionImportContext& rParent) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bAcceptContent(bAccContent),
        rChangedRegion(rParent)
{
}

SvXMLImportContext* XMLChangeElementImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken( rLocalName, XML_CHANGE_INFO) )
    {
        pContext = new XMLChangeInfoContext(GetImport(), nPrefix, rLocalName,
                                            rChangedRegion, GetLocalName());
    }
    else
    {
        // import into redline -> create XText
        rChangedRegion.UseRedlineText();

        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
            XML_TEXT_TYPE_CHANGED_REGION);

        if (NULL == pContext)
        {
            // no text element
            // illegal element content! TODO: discard this redline!
            // for the moment -> use default
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);
        }
    }

    return pContext;
}

void XMLChangeElementImportContext::StartElement( const Reference< XAttributeList >& )
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(true);
    }
}

void XMLChangeElementImportContext::EndElement()
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
