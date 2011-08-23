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

#ifndef _XMLOFF_XMLCHANGEINFOCONTEXT_HXX
#include "XMLChangeInfoContext.hxx"
#endif

#ifndef _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX
#include "XMLChangedRegionImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX
#include "XMLStringBufferImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif


#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
namespace binfilter {



using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1(XMLChangeInfoContext, SvXMLImportContext);

XMLChangeInfoContext::XMLChangeInfoContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    XMLChangedRegionImportContext& rPParent,
    const OUString& rChangeType) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rChangedRegion(rPParent),
        rType(rChangeType)
{
}

XMLChangeInfoContext::~XMLChangeInfoContext()
{
}

void XMLChangeInfoContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process attributes: chg-author, chg-date-time
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );
        OUString sValue = xAttrList->getValueByIndex(nAttr);
        if (XML_NAMESPACE_OFFICE == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_CHG_AUTHOR ) )
            {
                sAuthor = sValue;
            }
            else if ( IsXMLToken( sLocalName, XML_CHG_DATE_TIME ) )
            {
                sDateTime = sValue;
            }
            // else: unknown attribute
        }
        // else: unknown namespace
    }

}

SvXMLImportContext* XMLChangeInfoContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( ( XML_NAMESPACE_TEXT == nPrefix ) && 
         IsXMLToken( rLocalName, XML_P )       )
    {
        pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                                   rLocalName, sCommentBuffer);
    }
    else
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}

void XMLChangeInfoContext::EndElement()
{
    // set values at changed region context
    rChangedRegion.SetChangeInfo(rType, sAuthor, 
                                 sCommentBuffer.makeStringAndClear(), 
                                 sDateTime);
}
}//end of namespace binfilter
