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

#include <xmloff/xmlstyle.hxx>

namespace com::sun::star::io {
    class XOutputStream;
}

/// Handles <style:font-face>
class XMLFontStyleContextFontFace : public SvXMLStyleContext
{
    css::uno::Any aFamilyName;
    css::uno::Any aStyleName;
    css::uno::Any aFamily;
    css::uno::Any aPitch;
    css::uno::Any aEnc;

    rtl::Reference<XMLFontStylesContext> xStyles;

    XMLFontStylesContext *GetStyles()
    {
        return xStyles.get();
    }

public:


    XMLFontStyleContextFontFace( SvXMLImport& rImport,
            XMLFontStylesContext& rStyles );
    virtual ~XMLFontStyleContextFontFace() override;

    void SetAttribute( sal_Int32 nElement,
                       const OUString& rValue ) override;

    void FillProperties( ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const;

    OUString familyName() const;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

/// Handles <style:font-face-src>
class XMLFontStyleContextFontFaceSrc : public SvXMLImportContext
{
    const XMLFontStyleContextFontFace& font;
public:


    XMLFontStyleContextFontFaceSrc( SvXMLImport& rImport,
            const XMLFontStyleContextFontFace& font );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

/// Handles <style:font-face-uri>
class XMLFontStyleContextFontFaceUri : public SvXMLStyleContext
{
    const XMLFontStyleContextFontFace& font;
    OUString format;
    OUString linkPath;
    ::css::uno::Sequence< sal_Int8 > maFontData;
    ::css::uno::Reference< ::css::io::XOutputStream > mxBase64Stream;

    void handleEmbeddedFont( const OUString& url, bool eot );
    void handleEmbeddedFont( const ::css::uno::Sequence< sal_Int8 >& rData, bool eot );
public:


    XMLFontStyleContextFontFaceUri( SvXMLImport& rImport,
            const XMLFontStyleContextFontFace& font );

    virtual void SetAttribute( sal_Int32 nElement,
        const OUString& rValue ) override;
    void SetFormat( const OUString& rFormat );
    void SAL_CALL endFastElement(sal_Int32 nElement) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

/// Handles <svg:font-face-format>
class XMLFontStyleContextFontFaceFormat : public SvXMLStyleContext
{
    XMLFontStyleContextFontFaceUri& uri;
public:

    XMLFontStyleContextFontFaceFormat( SvXMLImport& rImport,
            XMLFontStyleContextFontFaceUri& uri );

    void SetAttribute( sal_Int32 nElement,
        const OUString& rValue ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
