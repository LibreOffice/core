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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_CHANGE_INFO;


XMLChangeElementImportContext::XMLChangeElementImportContext(
    SvXMLImport& rImport,
    bool bAccContent,
    XMLChangedRegionImportContext& rParent,
    OUString const & rType) :
        SvXMLImportContext(rImport),
        bAcceptContent(bAccContent),
        maType(rType),
        rChangedRegion(rParent)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLChangeElementImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if ( nElement == XML_ELEMENT(OFFICE, XML_CHANGE_INFO) )
    {
        xContext = new XMLChangeInfoContext(GetImport(),
                                            rChangedRegion, maType);
    }
    else
    {
        // import into redline -> create XText
        rChangedRegion.UseRedlineText();

        xContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nElement, xAttrList,
            XMLTextType::ChangedRegion);

        if (!xContext)
        {
            // no text element
            // illegal element content! TODO: discard this redline!
            // for the moment -> use default
        }
    }

    return xContext;
}

void XMLChangeElementImportContext::startFastElement( sal_Int32, const Reference< css::xml::sax::XFastAttributeList >& )
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(true);
    }
}

void XMLChangeElementImportContext::endFastElement(sal_Int32 )
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
