/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "XMLIndexSimpleEntryContext.hxx"

#include "XMLIndexTemplateContext.hxx"


#include "xmlimp.hxx"


#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


namespace binfilter {


//using namespace ::com::sun::star::text;

using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_STYLE_NAME;

const sal_Char sAPI_TokenType[] = "TokenType";
const sal_Char sAPI_CharacterStyleName[] = "CharacterStyleName";

TYPEINIT1( XMLIndexSimpleEntryContext, SvXMLImportContext);

XMLIndexSimpleEntryContext::XMLIndexSimpleEntryContext(
    SvXMLImport& rImport, 
    const OUString& rEntry,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rTemplateContext(rTemplate),
        rEntryType(rEntry),
        sCharStyleName(),
        bCharStyleNameOK(sal_False),
        nValues(1)
{
}

XMLIndexSimpleEntryContext::~XMLIndexSimpleEntryContext()
{
}

void XMLIndexSimpleEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // we know only one attribute: style-name
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
            bCharStyleNameOK = sal_True;
        }
    }

    // if we have a style name, set it!
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
    // due to the limited number of subclasses, we fill the values
    // directly into the slots. Subclasses will have to know they can
    // only use slot so-and-so.

    Any aAny;

    // token type
    rValues[0].Name = rTemplateContext.sTokenType;
    aAny <<= rEntryType;
    rValues[0].Value = aAny;

    // char style
    if (bCharStyleNameOK)
    {
        rValues[1].Name = rTemplateContext.sCharacterStyleName;
        aAny <<= sCharStyleName;
        rValues[1].Value = aAny;
    }

}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
