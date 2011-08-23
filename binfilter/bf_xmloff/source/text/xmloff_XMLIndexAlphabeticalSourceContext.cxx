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


#include "XMLIndexAlphabeticalSourceContext.hxx"



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
using ::binfilter::xmloff::token::XML_ALPHABETICAL_INDEX_ENTRY_TEMPLATE;
using ::binfilter::xmloff::token::XML_OUTLINE_LEVEL;

const sal_Char sAPI_MainEntryCharacterStyleName[] = "MainEntryCharacterStyleName";
const sal_Char sAPI_UseAlphabeticalSeparators[] = "UseAlphabeticalSeparators";
const sal_Char sAPI_UseCombinedEntries[] = "UseCombinedEntries";
const sal_Char sAPI_IsCaseSensitive[] = "IsCaseSensitive";
const sal_Char sAPI_UseKeyAsEntry[] = "UseKeyAsEntry";
const sal_Char sAPI_UseUpperCase[] = "UseUpperCase";
const sal_Char sAPI_UseDash[] = "UseDash";
const sal_Char sAPI_UsePP[] = "UsePP";
const sal_Char sAPI_SortAlgorithm[] = "SortAlgorithm";
const sal_Char sAPI_Locale[] = "Locale";


TYPEINIT1( XMLIndexAlphabeticalSourceContext, XMLIndexSourceBaseContext );

XMLIndexAlphabeticalSourceContext::XMLIndexAlphabeticalSourceContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName, 
                                  rPropSet, sal_False),
        sMainEntryStyleName(),
        bMainEntryStyleNameOK(sal_False),
        bSeparators(sal_False),
        bCombineEntries(sal_True),
        bCaseSensitive(sal_True),
        bEntry(sal_False),
        bUpperCase(sal_False),
        bCombineDash(sal_False),
        bCombinePP(sal_True),
        bCommaSeparated(sal_False),
        sMainEntryCharacterStyleName(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_MainEntryCharacterStyleName)),
        sUseAlphabeticalSeparators(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_UseAlphabeticalSeparators)),
        sUseCombinedEntries(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_UseCombinedEntries)),
        sIsCaseSensitive(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsCaseSensitive)),
        sUseKeyAsEntry(RTL_CONSTASCII_USTRINGPARAM(sAPI_UseKeyAsEntry)),
        sUseUpperCase(RTL_CONSTASCII_USTRINGPARAM(sAPI_UseUpperCase)),
        sUseDash(RTL_CONSTASCII_USTRINGPARAM(sAPI_UseDash)),
        sUsePP(RTL_CONSTASCII_USTRINGPARAM(sAPI_UsePP)),
        sSortAlgorithm(RTL_CONSTASCII_USTRINGPARAM(sAPI_SortAlgorithm)),
        sLocale(RTL_CONSTASCII_USTRINGPARAM(sAPI_Locale)),
        sIsCommaSeparated(RTL_CONSTASCII_USTRINGPARAM("IsCommaSeparated"))
{
}

XMLIndexAlphabeticalSourceContext::~XMLIndexAlphabeticalSourceContext()
{
}

void XMLIndexAlphabeticalSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam, 
    const OUString& rValue)
{
    sal_Bool bTmp;

    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_MAIN_ENTRY_STYLE:
            sMainEntryStyleName = rValue;
            bMainEntryStyleNameOK = sal_True;
            break;

        case XML_TOK_INDEXSOURCE_IGNORE_CASE:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bCaseSensitive = !bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_SEPARATORS:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bSeparators = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_COMBINE_ENTRIES:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bCombineEntries = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_COMBINE_WITH_DASH:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bCombineDash = bTmp;
            }
            break;
        case XML_TOK_INDEXSOURCE_KEYS_AS_ENTRIES:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bEntry = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_COMBINE_WITH_PP:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bCombinePP = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_CAPITALIZE:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUpperCase = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_COMMA_SEPARATED:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bCommaSeparated = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_SORT_ALGORITHM:
            sAlgorithm = rValue;
            break;
        case XML_TOK_INDEXSOURCE_LANGUAGE:
            aLocale.Language = rValue;
            break;
        case XML_TOK_INDEXSOURCE_COUNTRY:
            aLocale.Country = rValue;
            break;

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(eParam, rValue);
            break;
    }
}

void XMLIndexAlphabeticalSourceContext::EndElement()
{

    Any aAny;

    if (bMainEntryStyleNameOK)
    {
        aAny <<= sMainEntryStyleName;
        rIndexPropertySet->setPropertyValue(sMainEntryCharacterStyleName,aAny);
    }

    aAny.setValue(&bSeparators, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sUseAlphabeticalSeparators, aAny);

    aAny.setValue(&bCombineEntries, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sUseCombinedEntries, aAny);

    aAny.setValue(&bCaseSensitive, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sIsCaseSensitive, aAny);

    aAny.setValue(&bEntry, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sUseKeyAsEntry, aAny);

    aAny.setValue(&bUpperCase, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sUseUpperCase, aAny);

    aAny.setValue(&bCombineDash, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sUseDash, aAny);

    aAny.setValue(&bCombinePP, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sUsePP, aAny);

    aAny.setValue(&bCommaSeparated, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sIsCommaSeparated, aAny);


    if (sAlgorithm.getLength() > 0)
    {
        aAny <<= sAlgorithm;
        rIndexPropertySet->setPropertyValue(sSortAlgorithm, aAny);
    }

    if ( (aLocale.Language.getLength() > 0) &&
         (aLocale.Country.getLength() > 0)      )
    {
        aAny <<= aLocale;
        rIndexPropertySet->setPropertyValue(sLocale, aAny);
    }

    XMLIndexSourceBaseContext::EndElement();
}

SvXMLImportContext* XMLIndexAlphabeticalSourceContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken( rLocalName, XML_ALPHABETICAL_INDEX_ENTRY_TEMPLATE ) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet, 
                                           nPrefix, rLocalName,
                                           aLevelNameAlphaMap,
                                           XML_OUTLINE_LEVEL,
                                           aLevelStylePropNameAlphaMap,
                                           aAllowedTokenTypesAlpha);
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
