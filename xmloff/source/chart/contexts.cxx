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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlstyle.hxx>
#include <SchXMLImport.hxx>
#include "SchXMLCalculationSettingsContext.hxx"

#include "contexts.hxx"

#include <sal/log.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

namespace {

class SchXMLBodyContext_Impl : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:

    SchXMLBodyContext_Impl( SchXMLImportHelper& rImpHelper,
                SvXMLImport& rImport );

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

}

SchXMLBodyContext_Impl::SchXMLBodyContext_Impl(
        SchXMLImportHelper& rImpHelper, SvXMLImport& rImport ) :
    SvXMLImportContext( rImport ),
    mrImportHelper( rImpHelper )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SchXMLBodyContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SchXMLBodyContext( mrImportHelper, GetImport(), nElement );
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
    const uno::Reference< xml::sax::XAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContextRef xContext;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetDocElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_DOC_META:
            // we come here in the flat ODF file format,
            // if XDocumentPropertiesSupplier is not supported at the model
            break;
    }

    return xContext;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SchXMLDocContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportFlags nFlags = GetImport().getImportFlags();
    switch (nElement)
    {
        case XML_ELEMENT(OFFICE, XML_BODY):
            if( nFlags & SvXMLImportFlags::CONTENT )
                return new SchXMLBodyContext_Impl( mrImportHelper, GetImport() );
            break;
        case XML_ELEMENT(OFFICE, XML_STYLES):
            // for draw styles containing gradients/hatches/markers and dashes
            if( nFlags & SvXMLImportFlags::STYLES )
                return new SvXMLStylesContext( GetImport() );
            break;
        case  XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
            if( nFlags & SvXMLImportFlags::AUTOSTYLES )
                // not nice, but this is safe, as the SchXMLDocContext class can only by
                // instantiated by the chart import class SchXMLImport (header is not exported)
                return
                    static_cast< SchXMLImport& >( GetImport() ).CreateStylesContext();
            break;
    }
    return nullptr;
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
                                      sal_Int32 nElement ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper )
{
    SAL_WARN_IF( nElement != XML_ELEMENT(OFFICE, XML_CHART), "xmloff.chart", "SchXMLBodyContext instantiated with no <office:chart> element" );
}

SchXMLBodyContext::~SchXMLBodyContext()
{}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLBodyContext::createFastChildContext(
        sal_Int32 /*nElement*/, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return nullptr;
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

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
