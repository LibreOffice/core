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

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_XIMP3DSCENE_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_XIMP3DSCENE_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <ximpshap.hxx>

// dr3d:3dscene context

class SdXML3DSceneShapeContext : public SdXMLShapeContext, public SdXML3DSceneAttributesHelper
{
    // the shape group this group is working on
    // this is the scene at the same time
    css::uno::Reference< css::drawing::XShapes > mxChildren;

public:

    SdXML3DSceneShapeContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXML3DSceneShapeContext();

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_XIMP3DSCENE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
