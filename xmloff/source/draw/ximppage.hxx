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

#pragma once

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>

// draw:g context (RECURSIVE)

class SdXMLGenericPageContext : public SvXMLImportContext
{
    // the shape group this group is working on
    css::uno::Reference< css::drawing::XShapes > mxShapes;
    css::uno::Reference< css::office::XAnnotationAccess > mxAnnotationAccess;

protected:
    OUString               maPageLayoutName;
    OUString               maUseHeaderDeclName;
    OUString               maUseFooterDeclName;
    OUString               maUseDateTimeDeclName;
    OUString               msNavOrder;

    /** sets the page style on this page */
    void SetStyle( OUString const & rStyleName );

    /** sets the presentation layout at this page. It is used for drawing pages and for the handout master */
    void SetLayout();

    /** deletes all shapes on this drawing page */
    void DeleteAllShapes();

    const SdXMLImport& GetSdImport() const { return static_cast<const SdXMLImport&>(GetImport()); }
    SdXMLImport& GetSdImport() { return static_cast<SdXMLImport&>(GetImport()); }

    /** sets the properties from a page master style with the given name on this contexts page */
    void SetPageMaster( OUString const & rsPageMasterName );

    void SetNavigationOrder();

public:

    SdXMLGenericPageContext( SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);
    virtual ~SdXMLGenericPageContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    const css::uno::Reference< css::drawing::XShapes >& GetLocalShapesContext() const
        { return mxShapes; }
    css::uno::Reference< css::drawing::XShapes >& GetLocalShapesContext()
        { return mxShapes; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
