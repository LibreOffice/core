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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTPROPERTYSETCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTPROPERTYSETCONTEXT_HXX

#include <xmloff/xmlprcon.hxx>


class XMLTextPropertySetContext : public SvXMLPropertySetContext
{
//  SvXMLImportContextRef xTabStop;
//  SvXMLImportContextRef xBackground;
//  SvXMLImportContextRef xDropCap;
    OUString& rDropCapTextStyleName;

public:
    XMLTextPropertySetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const css::uno::Reference<css::xml::sax::XAttributeList >& xAttrList,
                sal_uInt32 nFamily,
                ::std::vector< XMLPropertyState > &rProps,
                const rtl::Reference < SvXMLImportPropertyMapper > &rMap,
                OUString& rDopCapTextStyleName );

    XMLTextPropertySetContext( SvXMLImport& rImport, sal_Int32 nElement,
                const css::uno::Reference<css::xml::sax::XFastAttributeList >& xAttrList,
                sal_uInt32 nFamily,
                ::std::vector< XMLPropertyState > &rProps,
                const rtl::Reference < SvXMLImportPropertyMapper > &rMap,
                OUString& rDopCapTextStyleName );

    virtual ~XMLTextPropertySetContext() override;

    using SvXMLPropertySetContext::CreateChildContext;
    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                                   ::std::vector< XMLPropertyState > &rProperties,
                                   const XMLPropertyState& rProp) override;
};


#endif // INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTPROPERTYSETCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
