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

#include "XMLIndexBibliographyConfigurationContext.hxx"
#include "XMLIndexBibliographyEntryContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;

const sal_Char sAPI_FieldMaster_Bibliography[] =
                                "com.sun.star.text.FieldMaster.Bibliography";


XMLIndexBibliographyConfigurationContext::XMLIndexBibliographyConfigurationContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList) :
        SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, XML_STYLE_FAMILY_TEXT_BIBLIOGRAPHYCONFIG),
        sFieldMaster_Bibliography(
            sAPI_FieldMaster_Bibliography),
        sBracketBefore("BracketBefore"),
        sBracketAfter("BracketAfter"),
        sIsNumberEntries("IsNumberEntries"),
        sIsSortByPosition("IsSortByPosition"),
        sSortKeys("SortKeys"),
        sSortKey("SortKey"),
        sIsSortAscending("IsSortAscending"),
        sSortAlgorithm("SortAlgorithm"),
        sLocale("Locale"),
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

void XMLIndexBibliographyConfigurationContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        ProcessAttribute(nPrefix, sLocalName,
                         xAttrList->getValueByIndex(nAttr));
        // else: ignore
    }
}

void XMLIndexBibliographyConfigurationContext::ProcessAttribute(
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


SvXMLImportContext *XMLIndexBibliographyConfigurationContext::CreateChildContext(
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
            aNameValue.Name = sSortKey;
            aNameValue.Value = Any((sal_Int16)nKey);
            aKey[0] = aNameValue;

            PropertyValue aSortValue;
            aSortValue.Name = sIsSortAscending;
            aSortValue.Value = Any(bSort);
            aKey[1] = aSortValue;

            aSortKeys.push_back(aKey);
        }
    }

    return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                  xAttrList);
}

void XMLIndexBibliographyConfigurationContext::CreateAndInsert(bool)
{
    // (code almost the same as export...)

    // insert and block mode is handled in insertStyleFamily

    // first: get field master
    // (we'll create one, and get the only master for this type)
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Sequence<OUString> aServices = xFactory->getAvailableServiceNames();
        bool bFound(false);
        sal_Int32 i(0);
        sal_Int32 nServiceCount(aServices.getLength());
        while (i < nServiceCount && !bFound)
        {
            if (aServices[i].equals(sFieldMaster_Bibliography))
            // here we should use a method which compares in reverse order if available
                bFound = true;
            else
                i++;
        }
        if (bFound)
        {
            Reference<XInterface> xIfc =
                xFactory->createInstance(sFieldMaster_Bibliography);
            if( xIfc.is() )
            {
                Reference<XPropertySet> xPropSet( xIfc, UNO_QUERY );
                Any aAny;

                xPropSet->setPropertyValue(sBracketAfter, Any(sSuffix));
                xPropSet->setPropertyValue(sBracketBefore, Any(sPrefix));
                xPropSet->setPropertyValue(sIsNumberEntries, Any(bNumberedEntries));
                xPropSet->setPropertyValue(sIsSortByPosition, Any(bSortByPosition));

                if( !maLanguageTagODF.isEmpty() )
                {
                    aAny <<= maLanguageTagODF.getLanguageTag().getLocale( false);
                    xPropSet->setPropertyValue(sLocale, aAny);
                }

                if( !sAlgorithm.isEmpty() )
                {
                    xPropSet->setPropertyValue(sSortAlgorithm, Any(sAlgorithm));
                }

                sal_Int32 nCount = aSortKeys.size();
                Sequence<Sequence<PropertyValue> > aKeysSeq(nCount);
                for(i = 0; i < nCount; i++)
                {
                    aKeysSeq[i] = aSortKeys[i];
                }
                xPropSet->setPropertyValue(sSortKeys, Any(aKeysSeq));
            }
            // else: can't get FieldMaster -> ignore
        }
    }
    // else: can't even get Factory -> ignore
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
