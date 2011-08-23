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

#ifndef _XMLOFF_XMLFOOTNOTEBODYIMPORTCONTEXT_HXX_
#include "XMLFootnoteBodyImportContext.hxx"
#endif

#include "rtl/ustring.hxx"


#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif




namespace binfilter {

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1( XMLFootnoteBodyImportContext, SvXMLImportContext );

XMLFootnoteBodyImportContext::XMLFootnoteBodyImportContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName)
{
}

SvXMLImportContext* XMLFootnoteBodyImportContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // return text context
    SvXMLImportContext *pContext = 
        GetImport().GetTextImport()->CreateTextChildContext(GetImport(), 
                                                       nPrefix, 
                                                       rLocalName, 
                                                       xAttrList, 
                                                       XML_TEXT_TYPE_FOOTNOTE);
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
}//end of namespace binfilter
