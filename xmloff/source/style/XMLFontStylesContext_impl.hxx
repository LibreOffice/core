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

#ifndef INCLUDED_XMLOFF_SOURCE_STYLE_XMLFONTSTYLESCONTEXT_IMPL_HXX
#define INCLUDED_XMLOFF_SOURCE_STYLE_XMLFONTSTYLESCONTEXT_IMPL_HXX

#include <xmloff/xmlstyle.hxx>

namespace com { namespace sun { namespace star { namespace io {
    class XOutputStream;
} } } }

/// Handles <style:font-face>
class XMLFontStyleContextFontFace : public SvXMLStyleContext
{
    css::uno::Any aFamilyName;
    css::uno::Any aStyleName;
    css::uno::Any aFamily;
    css::uno::Any aPitch;
    css::uno::Any aEnc;

    SvXMLImportContextRef xStyles;

    XMLFontStylesContext *GetStyles()
    {
        return static_cast<XMLFontStylesContext *>(&xStyles);
    }

public:


    XMLFontStyleContextFontFace( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference<
                css::xml::sax::XAttributeList > & xAttrList,
            XMLFontStylesContext& rStyles );
    virtual ~XMLFontStyleContextFontFace();

    void SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
                       const OUString& rValue ) override;

    void FillProperties( ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const;

    OUString familyName() const;

    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;
};

/// Handles <style:font-face-src>
class XMLFontStyleContextFontFaceSrc : public SvXMLImportContext
{
    const XMLFontStyleContextFontFace& font;
public:


    XMLFontStyleContextFontFaceSrc( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const XMLFontStyleContextFontFace& font );

    virtual SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;
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


    XMLFontStyleContextFontFaceUri( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference<
                css::xml::sax::XAttributeList > & xAttrList,
            const XMLFontStyleContextFontFace& font );

    virtual void SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
        const OUString& rValue ) override;
    void SetFormat( const OUString& rFormat );
    void EndElement() override;
    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;
};

/// Handles <svg:font-face-format>
class XMLFontStyleContextFontFaceFormat : public SvXMLStyleContext
{
    XMLFontStyleContextFontFaceUri& uri;
public:

    XMLFontStyleContextFontFaceFormat( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference<
                css::xml::sax::XAttributeList > & xAttrList,
            XMLFontStyleContextFontFaceUri& uri );

    void SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
        const OUString& rValue ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
