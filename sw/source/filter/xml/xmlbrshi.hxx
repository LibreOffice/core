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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLBRSHI_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLBRSHI_HXX

#include <com/sun/star/io/XOutputStream.hpp>

#include <xmloff/xmlictxt.hxx>

class SvXMLImport;
class SvXMLUnitConverter;
class SvxBrushItem;

namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }

class SwXMLBrushItemImportContext : public SvXMLImportContext
{
private:
    css::uno::Reference < css::io::XOutputStream > xBase64Stream;
    SvxBrushItem                *pItem;

    void ProcessAttrs(
               const css::uno::Reference<css::xml::sax::XAttributeList > & xAttrList,
               const SvXMLUnitConverter& rUnitConv );

public:
    TYPEINFO_OVERRIDE();

    SwXMLBrushItemImportContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference<css::xml::sax::XAttributeList > & xAttrList,
            const SvXMLUnitConverter& rUnitConv,
            const SvxBrushItem& rItem    );

    SwXMLBrushItemImportContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference<css::xml::sax::XAttributeList > & xAttrList,
            const SvXMLUnitConverter& rUnitConv,
            sal_uInt16 nWhich   );

    virtual ~SwXMLBrushItemImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                 const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void EndElement() override;

    const SvxBrushItem& GetItem() const { return *pItem; }
};

#endif  //  _XMLBRSHI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
