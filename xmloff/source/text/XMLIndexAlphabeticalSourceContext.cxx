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


#include "XMLIndexAlphabeticalSourceContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <sax/tools/converter.hxx>

#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using namespace ::xmloff::token;

XMLIndexAlphabeticalSourceContext::XMLIndexAlphabeticalSourceContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet)
:   XMLIndexSourceBaseContext(rImport, rPropSet, UseStyles::None)
,   bMainEntryStyleNameOK(false)
,   bSeparators(false)
,   bCombineEntries(true)
,   bCaseSensitive(true)
,   bEntry(false)
,   bUpperCase(false)
,   bCombineDash(false)
,   bCombinePP(true)
,   bCommaSeparated(false)
{
}

XMLIndexAlphabeticalSourceContext::~XMLIndexAlphabeticalSourceContext()
{
}

void XMLIndexAlphabeticalSourceContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    bool bTmp(false);

    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_MAIN_ENTRY_STYLE_NAME):
            {
                sMainEntryStyleName = aIter.toString();
                OUString sDisplayStyleName = GetImport().GetStyleDisplayName(
                    XmlStyleFamily::TEXT_TEXT, sMainEntryStyleName );
                const Reference < css::container::XNameContainer >&
                    rStyles = GetImport().GetTextImport()->GetTextStyles();
                bMainEntryStyleNameOK = rStyles.is() && rStyles->hasByName( sDisplayStyleName );
            }
            break;

        case XML_ELEMENT(TEXT, XML_IGNORE_CASE):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bCaseSensitive = !bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_SEPARATORS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bSeparators = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COMBINE_ENTRIES):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bCombineEntries = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COMBINE_ENTRIES_WITH_DASH):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bCombineDash = bTmp;
            }
            break;
        case XML_ELEMENT(TEXT, XML_USE_KEYS_AS_ENTRIES):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bEntry = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COMBINE_ENTRIES_WITH_PP):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bCombinePP = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_CAPITALIZE_ENTRIES):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUpperCase = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COMMA_SEPARATED):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bCommaSeparated = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_SORT_ALGORITHM):
            sAlgorithm = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_RFC_LANGUAGE_TAG):
            maLanguageTagODF.maRfcLanguageTag = aIter.toString();
            break;
        case XML_ELEMENT(FO, XML_LANGUAGE):
            maLanguageTagODF.maLanguage = aIter.toString();
            break;
        case XML_ELEMENT(FO, XML_SCRIPT):
            maLanguageTagODF.maScript = aIter.toString();
            break;
        case XML_ELEMENT(FO, XML_COUNTRY):
            maLanguageTagODF.maCountry = aIter.toString();
            break;

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(aIter);
            break;
    }
}

void XMLIndexAlphabeticalSourceContext::endFastElement(sal_Int32 nElement)
{

    Any aAny;

    if (bMainEntryStyleNameOK)
    {
        aAny <<= GetImport().GetStyleDisplayName(
                            XmlStyleFamily::TEXT_TEXT, sMainEntryStyleName );
        rIndexPropertySet->setPropertyValue(u"MainEntryCharacterStyleName"_ustr,aAny);
    }

    rIndexPropertySet->setPropertyValue(u"UseAlphabeticalSeparators"_ustr, css::uno::Any(bSeparators));
    rIndexPropertySet->setPropertyValue(u"UseCombinedEntries"_ustr, css::uno::Any(bCombineEntries));
    rIndexPropertySet->setPropertyValue(u"IsCaseSensitive"_ustr, css::uno::Any(bCaseSensitive));
    rIndexPropertySet->setPropertyValue(u"UseKeyAsEntry"_ustr, css::uno::Any(bEntry));
    rIndexPropertySet->setPropertyValue(u"UseUpperCase"_ustr, css::uno::Any(bUpperCase));
    rIndexPropertySet->setPropertyValue(u"UseDash"_ustr, css::uno::Any(bCombineDash));
    rIndexPropertySet->setPropertyValue(u"UsePP"_ustr, css::uno::Any(bCombinePP));
    rIndexPropertySet->setPropertyValue(u"IsCommaSeparated"_ustr, css::uno::Any(bCommaSeparated));


    if (!sAlgorithm.isEmpty())
    {
        rIndexPropertySet->setPropertyValue(u"SortAlgorithm"_ustr, css::uno::Any(sAlgorithm));
    }

    if ( !maLanguageTagODF.isEmpty() )
    {
        aAny <<= maLanguageTagODF.getLanguageTag().getLocale( false);
        rIndexPropertySet->setPropertyValue(u"Locale"_ustr, aAny);
    }

    XMLIndexSourceBaseContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexAlphabeticalSourceContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_ENTRY_TEMPLATE) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           aLevelNameAlphaMap,
                                           XML_OUTLINE_LEVEL,
                                           aLevelStylePropNameAlphaMap,
                                           aAllowedTokenTypesAlpha);
    }
    else
    {
        return XMLIndexSourceBaseContext::createFastChildContext(nElement,
                                                             xAttrList);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
