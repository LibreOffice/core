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


#ifndef _XMLOFF_XMLINDEXILLUSTRATIONSOURCECONTEXT_HXX_
#include "XMLIndexIllustrationSourceContext.hxx"
#endif


#ifndef _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_
#include "XMLIndexTemplateContext.hxx"
#endif




#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif





namespace binfilter {

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_ILLUSTRATION_INDEX_ENTRY_TEMPLATE;
using ::binfilter::xmloff::token::XML_TOKEN_INVALID;

TYPEINIT1(XMLIndexIllustrationSourceContext, XMLIndexTableSourceContext);


XMLIndexIllustrationSourceContext::XMLIndexIllustrationSourceContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        XMLIndexTableSourceContext(rImport, nPrfx, rLocalName, rPropSet)
{
}

XMLIndexIllustrationSourceContext::~XMLIndexIllustrationSourceContext()
{
}

SvXMLImportContext* XMLIndexIllustrationSourceContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( ( XML_NAMESPACE_TEXT == nPrefix ) &&
         ( IsXMLToken( rLocalName, XML_ILLUSTRATION_INDEX_ENTRY_TEMPLATE ) ) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet, 
                                           nPrefix, rLocalName,
                                           aLevelNameTableMap,
                                           XML_TOKEN_INVALID, // no outline-level attr
                                           aLevelStylePropNameTableMap,
                                           aAllowedTokenTypesTable);
    }
    else 
    {
        return XMLIndexSourceBaseContext::CreateChildContext(nPrefix, 
                                                             rLocalName,
                                                             xAttrList);
    }

}
}//end of namespace binfilter
