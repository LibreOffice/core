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

#include "XMLIndexTitleTemplateContext.hxx"


#include "xmlimp.hxx"

#include "nmspmap.hxx"

#include "xmlnmspe.hxx"

namespace binfilter {


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_STYLE_NAME;


const sal_Char sAPI_Title[] = "Title";
const sal_Char sAPI_ParaStyleHeading[] = "ParaStyleHeading";


TYPEINIT1( XMLIndexTitleTemplateContext, SvXMLImportContext );

XMLIndexTitleTemplateContext::XMLIndexTitleTemplateContext(
    SvXMLImport& rImport, 
    Reference<XPropertySet> & rPropSet,
    sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        sStyleName(),
        bStyleNameOK(sal_False),
        sContent(),
        rTOCPropertySet(rPropSet),
        sTitle(RTL_CONSTASCII_USTRINGPARAM(sAPI_Title)),
        sParaStyleHeading(RTL_CONSTASCII_USTRINGPARAM(sAPI_ParaStyleHeading))
{
}


XMLIndexTitleTemplateContext::~XMLIndexTitleTemplateContext()
{
}

void XMLIndexTitleTemplateContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // there's only one attribute: style-name
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             (IsXMLToken(sLocalName, XML_STYLE_NAME)) )
        {
            sStyleName = xAttrList->getValueByIndex(nAttr);
            bStyleNameOK = sal_True;
        }
    }
}

void XMLIndexTitleTemplateContext::EndElement()
{
    Any aAny;

    aAny <<= sContent.makeStringAndClear();
    rTOCPropertySet->setPropertyValue(sTitle, aAny);

    if (bStyleNameOK)
    {
        aAny <<= sStyleName;
        rTOCPropertySet->setPropertyValue(sParaStyleHeading, aAny);
    }
}

void XMLIndexTitleTemplateContext::Characters(
    const OUString& sString)
{
    sContent.append(sString);
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
