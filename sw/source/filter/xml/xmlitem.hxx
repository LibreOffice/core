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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLITEM_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLITEM_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <svl/itemset.hxx>
#include <xmloff/xmlictxt.hxx>

class SfxItemSet;
class SvXMLImportItemMapper;
class SvXMLUnitConverter;
struct SvXMLItemMapEntry;

class SwXMLItemSetContext final : public SvXMLImportContext
{
    SfxItemSet                  &rItemSet;
    const SvXMLImportItemMapper &rIMapper;
    const SvXMLUnitConverter    &rUnitConv;
    SvXMLImportContextRef xBackground;

public:

    SwXMLItemSetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                         const OUString& rLName,
                         const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                         SfxItemSet&  rItemSet,
                         SvXMLImportItemMapper& rIMap,
                         const SvXMLUnitConverter& rUnitConv );

    virtual ~SwXMLItemSetContext() override;

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:
    // This method is called from this instance implementation of
    // CreateChildContext if the element matches an entry in the
    // SvXMLImportItemMapper with the mid flag MID_SW_FLAG_ELEMENT_ITEM_IMPORT
    SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                                   const SvXMLItemMapEntry& rEntry );
};

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
