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
#include "XMLChartPropertyContext.hxx"
#include "PropertyMap.hxx"

#ifndef _XMLOFF_SYMBOLIMAGECONTEXT_HXX_
#include "XMLSymbolImageContext.hxx"
#endif
namespace binfilter {

TYPEINIT1( XMLChartPropertyContext, SvXMLImportContext );

using namespace ::com::sun::star;

XMLChartPropertyContext::XMLChartPropertyContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    ::std::vector< XMLPropertyState >& rProps,
    const UniReference< SvXMLImportPropertyMapper >& rMapper ) :
        SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, rProps, rMapper )
{
}

XMLChartPropertyContext::~XMLChartPropertyContext()
{}

SvXMLImportContext* XMLChartPropertyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

    switch( xMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
        case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE:
            pContext = new XMLSymbolImageContext( GetImport(), nPrefix, rLocalName, rProp, rProperties );
            break;
    }

    // default / no context yet: create child context by base class
    if( !pContext )
    {
        pContext = SvXMLPropertySetContext::CreateChildContext(
            nPrefix, rLocalName, xAttrList, rProperties, rProp );
    }
    
    return pContext;
}
}//end of namespace binfilter
