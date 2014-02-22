/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/debug.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlstyle.hxx>
#include "SchXMLImport.hxx"
#include "SchXMLCalculationSettingsContext.hxx"

#include "contexts.hxx"
#include "SchXMLChartContext.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

class SchXMLBodyContext_Impl : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:

    SchXMLBodyContext_Impl( SchXMLImportHelper& rImpHelper,
                SvXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName );
    virtual ~SchXMLBodyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );
};

SchXMLBodyContext_Impl::SchXMLBodyContext_Impl(
        SchXMLImportHelper& rImpHelper, SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrImportHelper( rImpHelper )
{
}

SchXMLBodyContext_Impl::~SchXMLBodyContext_Impl()
{
}

SvXMLImportContext *SchXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & )
{
    return new SchXMLBodyContext( mrImportHelper, GetImport(), nPrefix,
                                  rLocalName );
}

SchXMLDocContext::SchXMLDocContext( SchXMLImportHelper& rImpHelper,
                                    SvXMLImport& rImport,
                                    sal_uInt16 nPrefix,
                                    const OUString& rLName ) :
        SvXMLImportContext( rImport, nPrefix, rLName ),
        mrImportHelper( rImpHelper )
{
    SAL_WARN_IF( (XML_NAMESPACE_OFFICE != nPrefix) ||
        ( !IsXMLToken( rLName, XML_DOCUMENT ) &&
          !IsXMLToken( rLName, XML_DOCUMENT_META) &&
          !IsXMLToken( rLName, XML_DOCUMENT_STYLES) &&
          !IsXMLToken( rLName, XML_DOCUMENT_CONTENT) ), "xmloff.chart", "SchXMLDocContext instanciated with no <office:document> element" );
}

SchXMLDocContext::~SchXMLDocContext()
{}

TYPEINIT1( SchXMLDocContext, SvXMLImportContext );

SvXMLImportContext* SchXMLDocContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetDocElemTokenMap();
    sal_uInt16 nFlags = GetImport().getImportFlags();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_DOC_AUTOSTYLES:
            if( nFlags & IMPORT_AUTOSTYLES )
                
                
                pContext =
                    static_cast< SchXMLImport& >( GetImport() ).CreateStylesContext( rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_STYLES:
            
            if( nFlags & IMPORT_STYLES )
                pContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_META:
          
          

            pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_BODY:
            if( nFlags & IMPORT_CONTENT )
                pContext = new SchXMLBodyContext_Impl( mrImportHelper, GetImport(), nPrefix, rLocalName );
            break;
    }

    
    if( ! pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

SchXMLFlatDocContext_Impl::SchXMLFlatDocContext_Impl(
        SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const OUString & i_rLName,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
    SvXMLImportContext(i_rImport, i_nPrefix, i_rLName),
    SchXMLDocContext(i_rImpHelper, i_rImport, i_nPrefix, i_rLName),
    SvXMLMetaDocumentContext(i_rImport, i_nPrefix, i_rLName,
        i_xDocProps)
{
}

SchXMLFlatDocContext_Impl::~SchXMLFlatDocContext_Impl() { }

SvXMLImportContext *SchXMLFlatDocContext_Impl::CreateChildContext(
    sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& i_xAttrList)
{
    
    const SvXMLTokenMap& rTokenMap =
        mrImportHelper.GetDocElemTokenMap();
    if ( XML_TOK_DOC_META == rTokenMap.Get( i_nPrefix, i_rLocalName ) ) {
        return SvXMLMetaDocumentContext::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    } else {
        return SchXMLDocContext::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    }
}

SchXMLBodyContext::SchXMLBodyContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport,
                                      sal_uInt16 nPrefix,
                                      const OUString& rLName ) :
        SvXMLImportContext( rImport, nPrefix, rLName ),
        mrImportHelper( rImpHelper )
{
    SAL_WARN_IF( (XML_NAMESPACE_OFFICE != nPrefix) ||
                !IsXMLToken( rLName, XML_CHART ), "xmloff.chart", "SchXMLBodyContext instanciated with no <office:chart> element" );
}

SchXMLBodyContext::~SchXMLBodyContext()
{}

void SchXMLBodyContext::EndElement()
{
}

SvXMLImportContext* SchXMLBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    
    if( nPrefix == XML_NAMESPACE_CHART &&
        IsXMLToken( rLocalName, XML_CHART ) )
    {
        pContext = mrImportHelper.CreateChartContext( GetImport(),
                                                      nPrefix, rLocalName,
                                                      GetImport().GetModel(),
                                                      xAttrList );
    }
    else if(nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( rLocalName, XML_CALCULATION_SETTINGS ))
    {
        
        pContext = new SchXMLCalculationSettingsContext ( GetImport(), nPrefix, rLocalName, xAttrList);
    }
    else
    {
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
