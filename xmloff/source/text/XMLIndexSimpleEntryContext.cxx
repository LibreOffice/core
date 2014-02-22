/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "XMLIndexSimpleEntryContext.hxx"
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_STYLE_NAME;

TYPEINIT1( XMLIndexSimpleEntryContext, SvXMLImportContext);

XMLIndexSimpleEntryContext::XMLIndexSimpleEntryContext(
    SvXMLImport& rImport,
    const OUString& rEntry,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   rEntryType(rEntry)
,   bCharStyleNameOK(sal_False)
,   rTemplateContext(rTemplate)
,   nValues(1)
{
}

XMLIndexSimpleEntryContext::~XMLIndexSimpleEntryContext()
{
}

void XMLIndexSimpleEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken(sLocalName, XML_STYLE_NAME) )
        {
            sCharStyleName = xAttrList->getValueByIndex(nAttr);
            OUString sDisplayStyleName = GetImport().GetStyleDisplayName(
                XML_STYLE_FAMILY_TEXT_TEXT, sCharStyleName );
            
            const Reference < ::com::sun::star::container::XNameContainer > & rStyles =
                GetImport().GetTextImport()->GetTextStyles();
            if( rStyles.is() && rStyles->hasByName( sDisplayStyleName ) )
                bCharStyleNameOK = sal_True;
            else
                bCharStyleNameOK = sal_False;
        }
    }

    
    if (bCharStyleNameOK)
    {
        nValues++;
    }

}

void XMLIndexSimpleEntryContext::EndElement()
{
    Sequence<PropertyValue> aValues(nValues);

    FillPropertyValues(aValues);
    rTemplateContext.addTemplateEntry(aValues);
}

void XMLIndexSimpleEntryContext::FillPropertyValues(
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rValues)
{
    
    
    

    Any aAny;

    
    rValues[0].Name = rTemplateContext.sTokenType;
    aAny <<= rEntryType;
    rValues[0].Value = aAny;

    
    if (bCharStyleNameOK)
    {
        rValues[1].Name = rTemplateContext.sCharacterStyleName;
        aAny <<= GetImport().GetStyleDisplayName(
                                    XML_STYLE_FAMILY_TEXT_TEXT,
                                    sCharStyleName );
        rValues[1].Value = aAny;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
