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

#ifndef INCLUDED_XMLOFF_XMLPRCON_HXX
#define INCLUDED_XMLOFF_XMLPRCON_HXX

#include <xmloff/xmlictxt.hxx>

#include <vector>

class SvXMLImportPropertyMapper;
struct XMLPropertyState;

class XMLOFF_DLLPUBLIC SvXMLPropertySetContext : public SvXMLImportContext
{
protected:
    sal_Int32 const mnStartIdx;
    sal_Int32 const mnEndIdx;
    sal_uInt32 const mnFamily;
    ::std::vector< XMLPropertyState > &mrProperties;
    rtl::Reference < SvXMLImportPropertyMapper >   mxMapper;

public:

    SvXMLPropertySetContext(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
            sal_uInt32 nFamily,
            ::std::vector< XMLPropertyState > &rProps,
            const rtl::Reference < SvXMLImportPropertyMapper > &rMap,
              sal_Int32 nStartIdx = -1, sal_Int32 nEndIdx = -1 );

    virtual ~SvXMLPropertySetContext() override;

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    // This method is called from this instance implementation of
    // CreateChildContext if the element matches an entry in the
    // SvXMLImportItemMapper with the mid flag MID_FLAG_ELEMENT_ITEM_IMPORT
    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                                   ::std::vector< XMLPropertyState > &rProperties,
                                   const XMLPropertyState& rProp );

};

#endif // INCLUDED_XMLOFF_XMLPRCON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
