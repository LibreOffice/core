/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/debug.hxx>

#include "xmlimp.hxx"
#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "txtparai.hxx"
#include "XMLTextListBlockContext.hxx"


#include "XMLTextListItemContext.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;

TYPEINIT1( XMLTextListItemContext, SvXMLImportContext );

XMLTextListItemContext::XMLTextListItemContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bIsHeader ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rTxtImport( rTxtImp ),
    nStartValue( -1 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( !bIsHeader && XML_NAMESPACE_TEXT == nPrefix &&
            IsXMLToken( aLocalName, XML_START_VALUE ) )
        {
            sal_Int32 nTmp = rValue.toInt32();
            if( nTmp >= 0 && nTmp <= SHRT_MAX )
                nStartValue = (sal_Int16)nTmp;
        }
    }

    DBG_ASSERT( !rTxtImport.GetListItem(),
        "SwXMLListItemContext::SwXMLListItemContext: list item is existing" );

    // If this is a <text:list-item> element, then remember it as a sign
    // that a bullet has to be generated.
    if( !bIsHeader )
        rTxtImport.SetListItem( this );
}

XMLTextListItemContext::~XMLTextListItemContext()
{
}

void XMLTextListItemContext::EndElement()
{
    // finish current list item
    rTxtImport.SetListItem( 0 );
}

SvXMLImportContext *XMLTextListItemContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = rTxtImport.GetTextElemTokenMap();
    sal_Bool bOrdered = sal_False;
    sal_Bool bHeading = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_H:
        bHeading = sal_True;
    case XML_TOK_TEXT_P:
        pContext = new XMLParaContext( GetImport(),
                                       nPrefix, rLocalName,
                                       xAttrList, bHeading );
        if (rTxtImport.IsProgress())
            GetImport().GetProgressBarHelper()->Increment();

        break;
    case XML_TOK_TEXT_ORDERED_LIST:
        bOrdered = sal_True;
    case XML_TOK_TEXT_UNORDERED_LIST:
        pContext = new XMLTextListBlockContext( GetImport(), rTxtImport,
                                            nPrefix, rLocalName,
                                            xAttrList, bOrdered );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
