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


#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYCONFIGURATIONCONTEXT_HXX_
#include "XMLIndexBibliographyConfigurationContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYENTRYCONTEXT_HXX_
#include "XMLIndexBibliographyEntryContext.hxx"
#endif


#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif


#ifndef _XMLOFF_NMSPMAP_HXX 
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif


#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif




namespace binfilter {

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;

const sal_Char sAPI_FieldMaster_Bibliography[] = 
                                "com.sun.star.text.FieldMaster.Bibliography";


TYPEINIT1( XMLIndexBibliographyConfigurationContext, SvXMLStyleContext );

XMLIndexBibliographyConfigurationContext::XMLIndexBibliographyConfigurationContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList) :
        SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, XML_STYLE_FAMILY_TEXT_BIBLIOGRAPHYCONFIG),
        sFieldMaster_Bibliography(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_FieldMaster_Bibliography)),
        sBracketBefore(RTL_CONSTASCII_USTRINGPARAM("BracketBefore")),
        sBracketAfter(RTL_CONSTASCII_USTRINGPARAM("BracketAfter")),
        sIsNumberEntries(RTL_CONSTASCII_USTRINGPARAM("IsNumberEntries")),
        sIsSortByPosition(RTL_CONSTASCII_USTRINGPARAM("IsSortByPosition")),
        sSortKeys(RTL_CONSTASCII_USTRINGPARAM("SortKeys")),
        sSortKey(RTL_CONSTASCII_USTRINGPARAM("SortKey")),
        sIsSortAscending(RTL_CONSTASCII_USTRINGPARAM("IsSortAscending")),
        sSortAlgorithm(RTL_CONSTASCII_USTRINGPARAM("SortAlgorithm")),
        sLocale(RTL_CONSTASCII_USTRINGPARAM("Locale")),
        sSuffix(),
        sPrefix(),
        sAlgorithm(),
        aLocale(),
        bNumberedEntries(sal_False),
        bSortByPosition(sal_True)
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
    OUString sLocalName,
    OUString sValue)
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
            sal_Bool bTmp;
            if( SvXMLUnitConverter::convertBool(bTmp, sValue) )
            {
                bNumberedEntries = bTmp;
            }
        }
        else if( IsXMLToken(sLocalName, XML_SORT_BY_POSITION) )
        {
            sal_Bool bTmp;
            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
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
            aLocale.Language = sValue;
        }
        else if( IsXMLToken(sLocalName, XML_COUNTRY) )
        {
            aLocale.Country = sValue;
        }
    }
}


SvXMLImportContext *XMLIndexBibliographyConfigurationContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    OUString sKey;
    sal_Bool bSort(sal_True);

    // process children here and use default context!
    if ( ( nPrefix == XML_NAMESPACE_TEXT ) && 
         IsXMLToken( rLocalName, XML_SORT_KEY ) )
    {
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
                    sal_Bool bTmp;
                    if (SvXMLUnitConverter::convertBool(
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

            Any aAny;
            Sequence<PropertyValue> aKey(2);
        
            PropertyValue aNameValue;
            aNameValue.Name = sSortKey;
            aAny <<= (sal_Int16)nKey;
            aNameValue.Value = aAny;
            aKey[0] = aNameValue;

            PropertyValue aSortValue;
            aSortValue.Name = sIsSortAscending;
            aAny.setValue(&bSort, ::getBooleanCppuType());
            aSortValue.Value = aAny;
            aKey[1] = aSortValue;
            
            aSortKeys.push_back(aKey);
        }
    }

    return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, 
                                                  xAttrList);
}

void XMLIndexBibliographyConfigurationContext::CreateAndInsert(
    sal_Bool bOverwrite)
{
    // (code almost the same as export...)

    // insert and block mode is handled in insertStyleFamily

    // first: get field master
    // (we'll create one, and get the only master for this type)
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Sequence<rtl::OUString> aServices = xFactory->getAvailableServiceNames();
        sal_Bool bFound(sal_False);
        sal_Int32 i(0);
        sal_Int32 nCount(aServices.getLength());
        while (i < nCount && !bFound)
        {
            if (aServices[i].equals(sFieldMaster_Bibliography))
            // here we should use a methode which compares in reverse order if available
            // #85282#
                bFound = sal_True;
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

                aAny <<= sSuffix;
                xPropSet->setPropertyValue(sBracketAfter, aAny);

                aAny <<= sPrefix;
                xPropSet->setPropertyValue(sBracketBefore, aAny);
        
                aAny.setValue(&bNumberedEntries, ::getBooleanCppuType());
                xPropSet->setPropertyValue(sIsNumberEntries, aAny);

                aAny.setValue(&bSortByPosition, ::getBooleanCppuType());
                xPropSet->setPropertyValue(sIsSortByPosition, aAny);

                if( (aLocale.Language.getLength() > 0) &&
                    (aLocale.Country.getLength() > 0)     )
                {
                    aAny <<= aLocale;
                    xPropSet->setPropertyValue(sLocale, aAny);
                }

                if( sAlgorithm.getLength() > 0 )
                {
                    aAny <<= sAlgorithm;
                    xPropSet->setPropertyValue(sSortAlgorithm, aAny);
                }

                sal_Int32 nCount = aSortKeys.size();
                Sequence<Sequence<PropertyValue> > aKeysSeq(nCount);
                for(sal_Int32 i = 0; i < nCount; i++)
                {
                    aKeysSeq[i] = aSortKeys[i];
                }
                aAny <<= aKeysSeq;
                xPropSet->setPropertyValue(sSortKeys, aAny);
            }
            // else: can't get FieldMaster -> ignore
        }
    }
    // else: can't even get Factory -> ignore
}
}//end of namespace binfilter
