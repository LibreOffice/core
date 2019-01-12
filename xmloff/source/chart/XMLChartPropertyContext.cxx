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

#include "XMLChartPropertyContext.hxx"
#include "PropertyMap.hxx"

#include "XMLSymbolImageContext.hxx"
#include "XMLLabelSeparatorContext.hxx"
#include <xmloff/xmlimppr.hxx>


using namespace ::com::sun::star;

XMLChartPropertyContext::XMLChartPropertyContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    sal_uInt32 nFamily,
    ::std::vector< XMLPropertyState >& rProps,
    const rtl::Reference< SvXMLImportPropertyMapper >& rMapper ) :
        SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, nFamily, rProps, rMapper )
{
}

XMLChartPropertyContext::~XMLChartPropertyContext()
{}

SvXMLImportContextRef XMLChartPropertyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    SvXMLImportContextRef xContext;

    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
        case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE:
            xContext = new XMLSymbolImageContext( GetImport(), nPrefix, rLocalName, rProp, rProperties );
            break;
        case XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR:
            xContext = new XMLLabelSeparatorContext( GetImport(), nPrefix, rLocalName, rProp, rProperties );
            break;
    }

    // default / no context yet: create child context by base class
    if (!xContext)
    {
        xContext = SvXMLPropertySetContext::CreateChildContext(
            nPrefix, rLocalName, xAttrList, rProperties, rProp );
    }

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
