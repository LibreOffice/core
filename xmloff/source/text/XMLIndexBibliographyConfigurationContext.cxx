/*************************************************************************
 *
 *  $RCSfile: XMLIndexBibliographyConfigurationContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-21 10:52:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYCONFIGURATIONCONTEXT_HXX_
#include "XMLIndexBibliographyConfigurationContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYENTRYCONTEXT_HXX_
#include "XMLIndexBibliographyEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

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
        sSuffix(),
        sPrefix(),
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

        if (nPrefix == XML_NAMESPACE_TEXT)
        {
            ProcessAttribute(sLocalName, xAttrList->getValueByIndex(nAttr));
        }
        // else: ignore
    }
}

void XMLIndexBibliographyConfigurationContext::ProcessAttribute(
    OUString sLocalName,
    OUString sValue)
{
    if (sLocalName.equalsAsciiL(sXML_prefix, sizeof(sXML_prefix)-1))
    {
        sPrefix = sValue;
    }
    else if (sLocalName.equalsAsciiL(sXML_suffix, sizeof(sXML_suffix)-1))
    {
        sSuffix = sValue;
    }
    else if (sLocalName.equalsAsciiL(sXML_numbered_entries,
                                     sizeof(sXML_numbered_entries)-1))
    {
        sal_Bool bTmp;
        if (SvXMLUnitConverter::convertBool(bTmp, sValue))
        {
            bNumberedEntries = bTmp;
        }
    }
    else if (sLocalName.equalsAsciiL(sXML_sort_by_position,
                                     sizeof(sXML_sort_by_position)-1))
    {
        sal_Bool bTmp;
        if (SvXMLUnitConverter::convertBool(bTmp, sValue))
        {
            bSortByPosition = bTmp;
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
    if ((nPrefix == XML_NAMESPACE_TEXT) &&
        rLocalName.equalsAsciiL(sXML_sort_key, sizeof(sXML_sort_key)-1))
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
                if (sLocalName.equalsAsciiL(sXML_key, sizeof(sXML_key)-1))
                {
                    sKey = xAttrList->getValueByIndex(nAttr);
                }
                else if (sLocalName.equalsAsciiL(sXML_sort_ascending,
                                                sizeof(sXML_sort_ascending)-1))
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

                if (sSuffix.getLength() > 0)
                {
                    aAny <<= sSuffix;
                    xPropSet->setPropertyValue(sBracketAfter, aAny);
                }

                if (sPrefix.getLength() > 0)
                {
                    aAny <<= sPrefix;
                    xPropSet->setPropertyValue(sBracketBefore, aAny);
                }

                aAny.setValue(&bNumberedEntries, ::getBooleanCppuType());
                xPropSet->setPropertyValue(sIsNumberEntries, aAny);

                aAny.setValue(&bSortByPosition, ::getBooleanCppuType());
                xPropSet->setPropertyValue(sIsSortByPosition, aAny);

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
