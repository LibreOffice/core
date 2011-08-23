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

#include "xmlnmspe.hxx"
#include "xmlmetai.hxx"
#include "xmlstyle.hxx"

#include "contexts.hxx"

namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

// ==================================================

SchXMLDocContext::SchXMLDocContext( SchXMLImportHelper& rImpHelper,
                                    SvXMLImport& rImport,
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLName ) :
        SvXMLImportContext( rImport, nPrefix, rLName ),
        mrImportHelper( rImpHelper )
{
    DBG_ASSERT( XML_NAMESPACE_OFFICE == nPrefix &&
        ( IsXMLToken( rLName, XML_DOCUMENT ) ||
          IsXMLToken( rLName, XML_DOCUMENT_META) ||
          IsXMLToken( rLName, XML_DOCUMENT_STYLES) ||
          IsXMLToken( rLName, XML_DOCUMENT_CONTENT) ),
                "SchXMLDocContext instanciated with no <office:document> element" );
}

SchXMLDocContext::~SchXMLDocContext()
{}

SvXMLImportContext* SchXMLDocContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetDocElemTokenMap();
    sal_uInt16 nFlags = GetImport().getImportFlags();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_DOC_AUTOSTYLES:
            if( nFlags & IMPORT_AUTOSTYLES )
                // not nice, but this is safe, as the SchXMLDocContext class can only by
                // instantiated by the chart import class SchXMLImport (header is not exported)
                pContext =
                    static_cast< SchXMLImport& >( GetImport() ).CreateStylesContext( rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_STYLES:
            // for draw styles containing gradients/hatches/markers and dashes
            if( nFlags & IMPORT_STYLES )
                pContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_META:
            if( nFlags & IMPORT_META )
                pContext = new SfxXMLMetaContext( GetImport(), nPrefix, rLocalName, GetImport().GetModel());
            break;
        case XML_TOK_DOC_BODY:
            if( nFlags & IMPORT_CONTENT )
                pContext = new SchXMLBodyContext( mrImportHelper, GetImport(), nPrefix, rLocalName );
            break;
    }

    // call parent when no own context was created
    if( ! pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;	
}

// ----------------------------------------

SchXMLBodyContext::SchXMLBodyContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport,
                                      USHORT nPrefix,
                                      const ::rtl::OUString& rLName ) :
        SvXMLImportContext( rImport, nPrefix, rLName ),
        mrImportHelper( rImpHelper )
{
    DBG_ASSERT( XML_NAMESPACE_OFFICE == nPrefix &&
                IsXMLToken( rLName, XML_BODY ),
                "SchXMLBodyContext instanciated with no <office:body> element" );
}

SchXMLBodyContext::~SchXMLBodyContext()
{}

void SchXMLBodyContext::EndElement()
{
}

SvXMLImportContext* SchXMLBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    // <chart:chart> element
    if( nPrefix == XML_NAMESPACE_CHART &&
        IsXMLToken( rLocalName, XML_CHART ) )
    {
        pContext = mrImportHelper.CreateChartContext( GetImport(),
                                                      nPrefix, rLocalName,
                                                      GetImport().GetModel(),
                                                      xAttrList );
    }
    else
    {
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;	
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
