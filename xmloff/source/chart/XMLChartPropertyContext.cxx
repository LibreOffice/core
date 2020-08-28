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
#include <xmloff/xmlprmap.hxx>

using namespace ::com::sun::star;

XMLChartPropertyContext::XMLChartPropertyContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
    sal_uInt32 nFamily,
    ::std::vector< XMLPropertyState >& rProps,
    const rtl::Reference< SvXMLImportPropertyMapper >& rMapper ) :
        SvXMLPropertySetContext( rImport, nElement, xAttrList, nFamily, rProps, rMapper )
{
}

XMLChartPropertyContext::~XMLChartPropertyContext()
{}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLChartPropertyContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    SvXMLImportContextRef xContext;

    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
        case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE:
            return new XMLSymbolImageContext( GetImport(), nElement, rProp, rProperties );
            break;
        case XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR:
            return new XMLLabelSeparatorContext( GetImport(), nElement, rProp, rProperties );
            break;
    }

    // default / no context yet: create child context by base class
    return SvXMLPropertySetContext::createFastChildContext(
            nElement, xAttrList, rProperties, rProp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
