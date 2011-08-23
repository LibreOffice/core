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

#ifndef _XMLOFF_XMLSECTIONSOURCEIMPORTCONTEXT_HXX_
#include "XMLSectionSourceImportContext.hxx"
#endif


#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
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


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

namespace binfilter {


using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;


TYPEINIT1(XMLSectionSourceImportContext, SvXMLImportContext);

XMLSectionSourceImportContext::XMLSectionSourceImportContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rSectionPropertySet(rSectPropSet)
{
}

XMLSectionSourceImportContext::~XMLSectionSourceImportContext()
{
}

enum XMLSectionSourceToken 
{
    XML_TOK_SECTION_XLINK_HREF,
    XML_TOK_SECTION_TEXT_FILTER_NAME,
    XML_TOK_SECTION_TEXT_SECTION_NAME
};

static __FAR_DATA SvXMLTokenMapEntry aSectionSourceTokenMap[] =
{
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_SECTION_XLINK_HREF },
    { XML_NAMESPACE_TEXT, XML_FILTER_NAME, XML_TOK_SECTION_TEXT_FILTER_NAME },
    { XML_NAMESPACE_TEXT, XML_SECTION_NAME, 
                                        XML_TOK_SECTION_TEXT_SECTION_NAME },
    XML_TOKEN_MAP_END
};


void XMLSectionSourceImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aSectionSourceTokenMap);
    OUString sURL;
    OUString sFilterName;
    OUString sSectionName;
    
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_SECTION_XLINK_HREF:
                sURL = xAttrList->getValueByIndex(nAttr);
                break;

            case XML_TOK_SECTION_TEXT_FILTER_NAME:
                sFilterName = xAttrList->getValueByIndex(nAttr);
                break;

            case XML_TOK_SECTION_TEXT_SECTION_NAME:
                sSectionName = xAttrList->getValueByIndex(nAttr);
                break;

            default:
                ; // ignore
                break;
        }
    }

    // we only need them once
    const OUString sFileLink(RTL_CONSTASCII_USTRINGPARAM("FileLink"));
    const OUString sLinkRegion(RTL_CONSTASCII_USTRINGPARAM("LinkRegion"));

    Any aAny;
    if ((sURL.getLength() > 0) || (sFilterName.getLength() > 0))
    {
        SectionFileLink aFileLink;
        aFileLink.FileURL = GetImport().GetAbsoluteReference( sURL );
        aFileLink.FilterName = sFilterName;

        aAny <<= aFileLink;
        rSectionPropertySet->setPropertyValue(sFileLink, aAny);
    }

    if (sSectionName.getLength() > 0)
    {
        aAny <<= sSectionName;
        rSectionPropertySet->setPropertyValue(sLinkRegion, aAny);
    }
}

void XMLSectionSourceImportContext::EndElement()
{
    // this space intentionally left blank.
}

SvXMLImportContext* XMLSectionSourceImportContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // ignore -> default context
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
}//end of namespace binfilter
