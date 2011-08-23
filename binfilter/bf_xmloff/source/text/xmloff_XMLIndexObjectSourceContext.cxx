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


#include "XMLIndexObjectSourceContext.hxx"



#include "XMLIndexTemplateContext.hxx"






#include "xmlnmspe.hxx"



#include "xmluconv.hxx"


namespace binfilter {


using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_OBJECT_INDEX_ENTRY_TEMPLATE;
using ::binfilter::xmloff::token::XML_TOKEN_INVALID;

const sal_Char sAPI_CreateFromStarCalc[] = "CreateFromStarCalc";
const sal_Char sAPI_CreateFromStarChart[] = "CreateFromStarChart";
const sal_Char sAPI_CreateFromStarDraw[] = "CreateFromStarDraw";
const sal_Char sAPI_CreateFromStarImage[] = "CreateFromStarImage";
const sal_Char sAPI_CreateFromStarMath[] = "CreateFromStarMath";
const sal_Char sAPI_CreateFromOtherEmbeddedObjects[] = "CreateFromOtherEmbeddedObjects";


TYPEINIT1( XMLIndexObjectSourceContext, XMLIndexSourceBaseContext );

XMLIndexObjectSourceContext::XMLIndexObjectSourceContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName, 
                                  rPropSet, sal_False),
        sCreateFromStarCalc(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_CreateFromStarCalc)),
        sCreateFromStarChart(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_CreateFromStarChart)),
        sCreateFromStarDraw(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_CreateFromStarDraw)),
        sCreateFromStarMath(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_CreateFromStarMath)),
        sCreateFromOtherEmbeddedObjects(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_CreateFromOtherEmbeddedObjects)),
        bUseCalc(sal_False),
        bUseChart(sal_False),
        bUseDraw(sal_False),
        bUseMath(sal_False),
        bUseOtherObjects(sal_False)
{
}

XMLIndexObjectSourceContext::~XMLIndexObjectSourceContext()
{
}

void XMLIndexObjectSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam, 
    const OUString& rValue)
{
    switch (eParam)
    {
        sal_Bool bTmp;

        case XML_TOK_INDEXSOURCE_USE_OTHER_OBJECTS:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseOtherObjects = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_SHEET:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseCalc = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_CHART:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseChart = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_DRAW:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseDraw = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_MATH:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseMath = bTmp;
            }
            break;

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(eParam, rValue);
            break;
    }
}

void XMLIndexObjectSourceContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bUseCalc, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromStarCalc, aAny);

    aAny.setValue(&bUseChart, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromStarChart, aAny);

    aAny.setValue(&bUseDraw, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromStarDraw, aAny);

    aAny.setValue(&bUseMath, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromStarMath, aAny);

    aAny.setValue(&bUseOtherObjects, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromOtherEmbeddedObjects, aAny);

    XMLIndexSourceBaseContext::EndElement();
}

SvXMLImportContext* XMLIndexObjectSourceContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         (IsXMLToken(rLocalName, XML_OBJECT_INDEX_ENTRY_TEMPLATE)) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet, 
                                           nPrefix, rLocalName,
                                           aLevelNameTableMap,
                                           XML_TOKEN_INVALID, // no outline-level attr
                                           aLevelStylePropNameTableMap,
                                           aAllowedTokenTypesTable);
    }
    else 
    {
        return XMLIndexSourceBaseContext::CreateChildContext(nPrefix, 
                                                             rLocalName,
                                                             xAttrList);
    }

}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
