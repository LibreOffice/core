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

#include "XMLIndexTitleTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <sal/log.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::xmloff::token::XML_STYLE_NAME;


XMLIndexTitleTemplateContext::XMLIndexTitleTemplateContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet)
:   SvXMLImportContext(rImport)
,   bStyleNameOK(false)
,   rTOCPropertySet(rPropSet)
{
}


XMLIndexTitleTemplateContext::~XMLIndexTitleTemplateContext()
{
}

void XMLIndexTitleTemplateContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // there's only one attribute: style-name
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME) )
        {
            sStyleName = aIter.toString();
            OUString sDisplayStyleName = GetImport().GetStyleDisplayName(
                XmlStyleFamily::TEXT_PARAGRAPH, sStyleName );
            const Reference < css::container::XNameContainer >&
                rStyles = GetImport().GetTextImport()->GetParaStyles();
            bStyleNameOK = rStyles.is() && rStyles->hasByName( sDisplayStyleName );
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

void XMLIndexTitleTemplateContext::endFastElement(sal_Int32 )
{
    Any aAny;

    aAny <<= sContent.makeStringAndClear();
    rTOCPropertySet->setPropertyValue("Title", aAny);

    if (bStyleNameOK)
    {
        aAny <<= GetImport().GetStyleDisplayName(
                                XmlStyleFamily::TEXT_PARAGRAPH,
                                sStyleName );
        rTOCPropertySet->setPropertyValue("ParaStyleHeading", aAny);
    }
}

void XMLIndexTitleTemplateContext::characters(
    const OUString& sString)
{
    sContent.append(sString);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
