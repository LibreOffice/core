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

#include <memory>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <xmloff/xmlictxt.hxx>

class SvXMLImport;
class SvXMLUnitConverter;
class SvxBrushItem;

namespace com::sun::star {
    namespace io { class XOutputStream; }
}

class SwXMLBrushItemImportContext : public SvXMLImportContext
{
private:
    css::uno::Reference<css::io::XOutputStream> m_xBase64Stream;
    css::uno::Reference<css::graphic::XGraphic> m_xGraphic;

    std::unique_ptr<SvxBrushItem> pItem;

    void ProcessAttrs(
               const css::uno::Reference<css::xml::sax::XFastAttributeList > & xAttrList,
               const SvXMLUnitConverter& rUnitConv );

public:

    SwXMLBrushItemImportContext(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const css::uno::Reference<css::xml::sax::XFastAttributeList > & xAttrList,
            const SvXMLUnitConverter& rUnitConv,
            const SvxBrushItem& rItem    );

    SwXMLBrushItemImportContext(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const css::uno::Reference<css::xml::sax::XFastAttributeList > & xAttrList,
            const SvXMLUnitConverter& rUnitConv,
            sal_uInt16 nWhich   );

    virtual ~SwXMLBrushItemImportContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    const SvxBrushItem& GetItem() const { return *pItem; }
};

#endif  //  _XMLBRSHI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
