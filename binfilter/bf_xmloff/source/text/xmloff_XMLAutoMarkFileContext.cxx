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

#ifndef _XMLOFF_XMLAUTOMARKFILECONTEXT_HXX_
#include "XMLAutoMarkFileContext.hxx"
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




namespace binfilter {


using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::XPropertySet;

using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_HREF;


TYPEINIT1( XMLAutoMarkFileContext, SvXMLImportContext );

XMLAutoMarkFileContext::XMLAutoMarkFileContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        sIndexAutoMarkFileURL(
            RTL_CONSTASCII_USTRINGPARAM("IndexAutoMarkFileURL"))
{
}

XMLAutoMarkFileContext::~XMLAutoMarkFileContext()
{
}


void XMLAutoMarkFileContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // scan for text:alphabetical-index-auto-mark-file attribute, and if 
    // found set value with the document

    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nLength; i++ )
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        if ( ( XML_NAMESPACE_XLINK == nPrefix ) && 
             IsXMLToken(sLocalName, XML_HREF) )
        {
            Any aAny;
            aAny <<= GetImport().GetAbsoluteReference( xAttrList->getValueByIndex(i) );
            Reference<XPropertySet> xPropertySet(
                GetImport().GetModel(), UNO_QUERY );
            if (xPropertySet.is())
            {
                xPropertySet->setPropertyValue( sIndexAutoMarkFileURL, aAny );
            }
        }
    }
}
}//end of namespace binfilter
