/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLChartPropertyContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:16:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLChartPropertyContext.hxx"
#include "PropertyMap.hxx"

#ifndef _XMLOFF_SYMBOLIMAGECONTEXT_HXX_
#include "XMLSymbolImageContext.hxx"
#endif

TYPEINIT1( XMLChartPropertyContext, SvXMLImportContext );

using namespace ::com::sun::star;

XMLChartPropertyContext::XMLChartPropertyContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    sal_uInt32 nFamily,
    ::std::vector< XMLPropertyState >& rProps,
    const UniReference< SvXMLImportPropertyMapper >& rMapper ) :
        SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, nFamily, rProps, rMapper )
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

    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
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
