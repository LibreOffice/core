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

#include "XMLChangeElementImportContext.hxx"

#include "XMLChangedRegionImportContext.hxx"

#include "XMLChangeInfoContext.hxx"

#include <com/sun/star/uno/Reference.h>

#include "xmlimp.hxx"

#include "xmlnmspe.hxx"

namespace binfilter {



using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_P;
using ::binfilter::xmloff::token::XML_CHANGE_INFO;

TYPEINIT1( XMLChangeElementImportContext, SvXMLImportContext );

XMLChangeElementImportContext::XMLChangeElementImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    sal_Bool bAccContent,
    XMLChangedRegionImportContext& rParent) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bAcceptContent(bAccContent),
        rChangedRegion(rParent)
{
}

SvXMLImportContext* XMLChangeElementImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken( rLocalName, XML_CHANGE_INFO) )
    {
        pContext = new XMLChangeInfoContext(GetImport(), nPrefix, rLocalName,
                                            rChangedRegion, GetLocalName());
    }
    else 
    {
        // import into redline -> create XText
        rChangedRegion.UseRedlineText();

        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
            XML_TEXT_TYPE_CHANGED_REGION);

        if (NULL == pContext)
        {
            // no text element -> use default
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);

            // illegal element content! TODO: discard this redline!
        }
    }


    return pContext;
}

// #107848#
void XMLChangeElementImportContext::StartElement( const Reference< XAttributeList >& xAttrList )
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(sal_True);
    }
}

// #107848#
void XMLChangeElementImportContext::EndElement()
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(sal_False);
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
