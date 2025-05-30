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

#include "PageHeaderFooterContext.hxx"
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlimppr.hxx>
#include "PagePropertySetContext.hxx"
#include <sal/log.hxx>

using namespace com::sun::star;
using ::xmloff::token::XML_HEADER_FOOTER_PROPERTIES;

PageHeaderFooterContext::PageHeaderFooterContext( SvXMLImport& rImport,
                                      ::std::vector< XMLPropertyState > & rTempProperties,
                                      SvXMLImportPropertyMapper* pTempMap,
                                      sal_Int32 nStart, sal_Int32 nEnd,
                                      const bool bTempHeader ) :
    SvXMLImportContext( rImport ),
    rProperties(rTempProperties),
    nStartIndex(nStart),
    nEndIndex(nEnd),
    rMap(pTempMap)
{
    bHeader = bTempHeader;
}

PageHeaderFooterContext::~PageHeaderFooterContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > PageHeaderFooterContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_HEADER_FOOTER_PROPERTIES) )
    {
        PageContextType aType = Header;
        if (!bHeader)
            aType = Footer;
        return new PagePropertySetContext( GetImport(), nElement,
                                           xAttrList,
                                           XML_TYPE_PROP_HEADER_FOOTER,
                                           rProperties,
                                           rMap,  nStartIndex, nEndIndex, aType);
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
