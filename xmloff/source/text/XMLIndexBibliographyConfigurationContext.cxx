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

#include <XMLIndexBibliographyConfigurationContext.hxx>
#include "XMLIndexBibliographyEntryContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;


const OUStringLiteral gsFieldMaster_Bibliography("com.sun.star.text.FieldMaster.Bibliography");
const OUStringLiteral gsBracketBefore("BracketBefore");
const OUStringLiteral gsBracketAfter("BracketAfter");
const OUStringLiteral gsIsNumberEntries("IsNumberEntries");
const OUStringLiteral gsIsSortByPosition("IsSortByPosition");
const OUStringLiteral gsSortKeys("SortKeys");
const OUStringLiteral gsSortKey("SortKey");
const OUStringLiteral gsIsSortAscending("IsSortAscending");
const OUStringLiteral gsSortAlgorithm("SortAlgorithm");
const OUStringLiteral gsLocale("Locale");

XMLIndexBibliographyConfigurationContext::XMLIndexBibliographyConfigurationContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList) :
        SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, XmlStyleFamily::TEXT_BIBLIOGRAPHYCONFIG),
        sSuffix(),
        sPrefix(),
        sAlgorithm(),
        maLanguageTagODF(),
        bNumberedEntries(false),
        bSortByPosition(true)
{
}

XMLIndexBibliographyConfigurationContext::~XMLIndexBibliographyConfigurationContext()
{
}

void XMLIndexBibliographyConfigurationContext::SetAttribute(
    sal_uInt16 nPrefix,
    const OUString& sLocalName,
    const OUString& sValue)
{
    if( XML_NAMESPACE_TEXT == nPrefix )
    {
        if( IsXMLToken(sLocalName, XML_PREFIX) )
        {
            sPrefix = sValue;
        }
        else if( IsXMLToken(sLocalName, XML_SUFFIX) )
        {
            sSuffix = sValue;
        }
        else if( IsXMLToken(sLocalName, XML_NUMBERED_ENTRIES) )
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bNumberedEntries = bTmp;
            }
        }
        else if( IsXMLToken(sLocalName, XML_SORT_BY_POSITION) )
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bSortByPosition = bTmp;
            }
        }
        else if( IsXMLToken(sLocalName, XML_SORT_ALGORITHM) )
        {
            sAlgorithm = sValue;
        }
    }
    else if( XML_NAMESPACE_FO == nPrefix )
    {
        if( IsXMLToken(sLocalName, XML_LANGUAGE) )
        {
            maLanguageTagODF.maLanguage = sValue;
        }
        else if( IsXMLToken(sLocalName, XML_SCRIPT) )
        {
            maLanguageTagODF.maScript = sValue;
        }
        else if( IsXMLToken(sLocalName, XML_COUNTRY) )
        {
            maLanguageTagODF.maCountry = sValue;
        }
    }
    else if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( IsXMLToken(sLocalName, XML_RFC_LANGUAGE_TAG) )
        {
            maLanguageTagODF.maRfcLanguageTag = sValue;
        }
    }
}

SvXMLImportContextRef XMLIndexBibliographyConfigurationContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // process children here and use default context!
    if ( ( nPrefix == XML_NAMESPACE_TEXT ) &&
         IsXMLToken( rLocalName, XML_SORT_KEY ) )
    {
        OUString sKey;
        bool bSort(true);

        sal_Int16 nLength = xAttrList->getLength();
        for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
        {
            OUString sLocalName;
            sal_uInt16 nPrfx = GetImport().GetNamespaceMap().
                GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                                  &sLocalName );

            if (nPrfx == XML_NAMESPACE_TEXT)
            {
                if ( IsXMLToken( sLocalName, XML_KEY ) )
                {
                    sKey = xAttrList->getValueByIndex(nAttr);
                }
                else if ( IsXMLToken( sLocalName, XML_SORT_ASCENDING ) )
                {
                    bool bTmp(false);
                    if (::sax::Converter::convertBool(
                        bTmp, xAttrList->getValueByIndex(nAttr)))
                    {
                        bSort = bTmp;
                    }
                }
            }
        }

        // valid data?
        sal_uInt16 nKey;
        if (SvXMLUnitConverter::convertEnum(nKey, sKey,
                                            aBibliographyDataFieldMap))
        {
            Sequence<PropertyValue> aKey(2);

            PropertyValue aNameValue;
            aNameValue.Name = gsSortKey;
            aNameValue.Value <<= static_cast<sal_Int16>(nKey);
            aKey[0] = aNameValue;

            PropertyValue aSortValue;
            aSortValue.Name = gsIsSortAscending;
            aSortValue.Value <<= bSort;
            aKey[1] = aSortValue;

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
