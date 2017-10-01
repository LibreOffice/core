/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
};

SchXMLBodyContext_Impl::SchXMLBodyContext_Impl(
        SchXMLImportHelper& rImpHelper, SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrImportHelper( rImpHelper )
{
}

SvXMLImportContextRef SchXMLBodyContext_Impl::CreateChildContext(
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
          !IsXMLToken( rLName, XML_DOCUMENT_CONTENT) ), "xmloff.chart", "SchXMLDocContext instantiated with no <office:document> element" );
}

SchXMLDocContext::SchXMLDocContext( SchXMLImportHelper& rImpHelper,
                                    SvXMLImport& rImport,
                                    sal_Int32 nElement ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper )
{
    SAL_WARN_IF(( nElement != XML_ELEMENT( OFFICE, XML_DOCUMENT ) &&
          nElement != XML_ELEMENT( OFFICE, XML_DOCUMENT_META ) &&
          nElement != XML_ELEMENT( OFFICE, XML_DOCUMENT_STYLES ) &&
          nElement != XML_ELEMENT( OFFICE, XML_DOCUMENT_CONTENT ) ), "xmloff.chart", "SchXMLDocContext instantiated with no <office:document> element" );
}

SchXMLDocContext::~SchXMLDocContext()
{}


SvXMLImportContextRef SchXMLDocContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetDocElemTokenMap();
    SvXMLImportFlags nFlags = GetImport().getImportFlags();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_DOC_AUTOSTYLES:
            if( nFlags & SvXMLImportFlags::AUTOSTYLES )
                // not nice, but this is safe, as the SchXMLDocContext class can only by
                // instantiated by the chart import class SchXMLImport (header is not exported)
                xContext =
                    static_cast< SchXMLImport& >( GetImport() ).CreateStylesContext( rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_STYLES:
            // for draw styles containing gradients/hatches/markers and dashes
            if( nFlags & SvXMLImportFlags::STYLES )
                xContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_META:
            // we come here in the flat ODF file format,
            // if XDocumentPropertiesSupplier is not supported at the model
            xContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_BODY:
            if( nFlags & SvXMLImportFlags::CONTENT )
                xContext = new SchXMLBodyContext_Impl( mrImportHelper, GetImport(), nPrefix, rLocalName );
            break;
    }

    // call parent when no own context was created
    if (!xContext)
        xContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return xContext;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SchXMLDocContext::createFastChildContext(
    sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport() );
}

SchXMLFlatDocContext_Impl::SchXMLFlatDocContext_Impl(
        SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        sal_Int32 i_nElement,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
    SvXMLImportContext(i_rImport),
    SchXMLDocContext(i_rImpHelper, i_rImport, i_nElement),
    SvXMLMetaDocumentContext(i_rImport, i_xDocProps)
{
}

void SAL_CALL SchXMLFlatDocContext_Impl::startFastElement( sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLMetaDocumentContext::startFastElement(nElement, xAttrList);
}

void SAL_CALL SchXMLFlatDocContext_Impl::endFastElement( sal_Int32 nElement )
{
    SvXMLMetaDocumentContext::endFastElement(nElement);
}

void SAL_CALL SchXMLFlatDocContext_Impl::characters( const OUString& rChars )
{
    SvXMLMetaDocumentContext::characters(rChars);
}

SvXMLImportContextRef SchXMLFlatDocContext_Impl::CreateChildContext(
    sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& i_xAttrList)
{
    return SchXMLDocContext::CreateChildContext(
                i_nPrefix, i_rLocalName, i_xAttrList );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SchXMLFlatDocContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    // behave like meta base class iff we encounter office:meta
    if ( nElement == XML_ELEMENT( OFFICE, XML_META ) ) {
        return SvXMLMetaDocumentContext::createFastChildContext(
                    nElement, xAttrList );
    } else {
        return SchXMLDocContext::createFastChildContext(
                    nElement, xAttrList );
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
                !IsXMLToken( rLName, XML_CHART ), "xmloff.chart", "SchXMLBodyContext instantiated with no <office:chart> element" );
}

SchXMLBodyContext::~SchXMLBodyContext()
{}

void SchXMLBodyContext::EndElement()
{
}

SvXMLImportContextRef SchXMLBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    // <chart:chart> element
    if( nPrefix == XML_NAMESPACE_CHART &&
        IsXMLToken( rLocalName, XML_CHART ) )
    {
        xContext = mrImportHelper.CreateChartContext( GetImport(),
                                                      nPrefix, rLocalName,
                                                      GetImport().GetModel(),
                                                      xAttrList );
    }
    else if(nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( rLocalName, XML_CALCULATION_SETTINGS ))
    {
        // i99104 handle null date correctly
        xContext = new SchXMLCalculationSettingsContext ( GetImport(), nPrefix, rLocalName, xAttrList);
    }
    else
    {
        xContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    }

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
