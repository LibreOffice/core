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

#include <comphelper/propertyvalue.hxx>
#include <XMLIndexBibliographyConfigurationContext.hxx>
#include "XMLIndexBibliographyEntryContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;


constexpr OUString gsFieldMaster_Bibliography(u"com.sun.star.text.FieldMaster.Bibliography"_ustr);
constexpr OUStringLiteral gsBracketBefore(u"BracketBefore");
constexpr OUStringLiteral gsBracketAfter(u"BracketAfter");
constexpr OUStringLiteral gsIsNumberEntries(u"IsNumberEntries");
constexpr OUStringLiteral gsIsSortByPosition(u"IsSortByPosition");
constexpr OUStringLiteral gsSortKeys(u"SortKeys");
constexpr OUString gsSortKey(u"SortKey"_ustr);
constexpr OUString gsIsSortAscending(u"IsSortAscending"_ustr);
constexpr OUStringLiteral gsSortAlgorithm(u"SortAlgorithm");
constexpr OUStringLiteral gsLocale(u"Locale");

XMLIndexBibliographyConfigurationContext::XMLIndexBibliographyConfigurationContext(
    SvXMLImport& rImport) :
        SvXMLStyleContext(rImport, XmlStyleFamily::TEXT_BIBLIOGRAPHYCONFIG),
        maLanguageTagODF(),
        bNumberedEntries(false),
        bSortByPosition(true)
{
}

XMLIndexBibliographyConfigurationContext::~XMLIndexBibliographyConfigurationContext()
{
}

void XMLIndexBibliographyConfigurationContext::SetAttribute(
    sal_Int32 nElement,
    const OUString& sValue)
{
    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_PREFIX):
            sPrefix = sValue;
            break;
        case XML_ELEMENT(TEXT, XML_SUFFIX):
            sSuffix = sValue;
            break;
        case XML_ELEMENT(TEXT, XML_NUMBERED_ENTRIES):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bNumberedEntries = bTmp;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_SORT_BY_POSITION):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bSortByPosition = bTmp;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_SORT_ALGORITHM):
            sAlgorithm = sValue;
            break;
        case XML_ELEMENT(FO, XML_LANGUAGE):
            maLanguageTagODF.maLanguage = sValue;
            break;
        case XML_ELEMENT(FO, XML_SCRIPT):
            maLanguageTagODF.maScript = sValue;
            break;
        case XML_ELEMENT(FO, XML_COUNTRY):
            maLanguageTagODF.maCountry = sValue;
            break;
        case XML_ELEMENT(STYLE, XML_RFC_LANGUAGE_TAG):
            maLanguageTagODF.maRfcLanguageTag = sValue;
            break;
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexBibliographyConfigurationContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // process children here and use default context!
    if ( nElement == XML_ELEMENT(TEXT, XML_SORT_KEY) )
    {
        std::string_view sKey;
        bool bSort(true);

        for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(TEXT, XML_KEY):
                    sKey = aIter.toView();
                    break;
                case XML_ELEMENT(TEXT, XML_SORT_ASCENDING):
                {
                    bool bTmp(false);
                    if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                        bSort = bTmp;
                    break;
                }
                default:
                    XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                    break;
            }
        }

        // valid data?
        sal_uInt16 nKey;
        if (SvXMLUnitConverter::convertEnum(nKey, sKey,
                                            aBibliographyDataFieldMap))
        {
            Sequence<PropertyValue> aKey
            {
                comphelper::makePropertyValue(gsSortKey, static_cast<sal_Int16>(nKey)),
                comphelper::makePropertyValue(gsIsSortAscending, bSort)
            };

            aSortKeys.push_back(aKey);
        }
    }

    return nullptr;
}

void XMLIndexBibliographyConfigurationContext::CreateAndInsert(bool)
{
    // (code almost the same as export...)

    // insert and block mode is handled in insertStyleFamily

    // first: get field master
    // (we'll create one, and get the only master for this type)
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( !xFactory.is() )
        return;

    Sequence<OUString> aServices = xFactory->getAvailableServiceNames();
    // here we should use a method which compares in reverse order if available
    if (comphelper::findValue(aServices, gsFieldMaster_Bibliography) == -1)
        return;

    Reference<XInterface> xIfc =
        xFactory->createInstance(gsFieldMaster_Bibliography);
    if( !xIfc.is() )
        return;

    Reference<XPropertySet> xPropSet( xIfc, UNO_QUERY );
    Any aAny;

    xPropSet->setPropertyValue(gsBracketAfter, Any(sSuffix));
    xPropSet->setPropertyValue(gsBracketBefore, Any(sPrefix));
    xPropSet->setPropertyValue(gsIsNumberEntries, Any(bNumberedEntries));
    xPropSet->setPropertyValue(gsIsSortByPosition, Any(bSortByPosition));

    if( !maLanguageTagODF.isEmpty() )
    {
        aAny <<= maLanguageTagODF.getLanguageTag().getLocale( false);
        xPropSet->setPropertyValue(gsLocale, aAny);
    }

    if( !sAlgorithm.isEmpty() )
    {
        xPropSet->setPropertyValue(gsSortAlgorithm, Any(sAlgorithm));
    }

    Sequence<Sequence<PropertyValue> > aKeysSeq = comphelper::containerToSequence(aSortKeys);
    xPropSet->setPropertyValue(gsSortKeys, Any(aKeysSeq));
    // else: can't get FieldMaster -> ignore
    // else: can't even get Factory -> ignore
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
