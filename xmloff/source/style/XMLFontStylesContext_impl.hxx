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

#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_IMPL_HXX_
#define _XMLOFF_XMLFONTSTYLESCONTEXT_IMPL_HXX_

#include <xmloff/xmlstyle.hxx>

/// Handles <style:font-face>
class XMLFontStyleContextFontFace : public SvXMLStyleContext
{
    ::com::sun::star::uno::Any aFamilyName;
    ::com::sun::star::uno::Any aStyleName;
    ::com::sun::star::uno::Any aFamily;
    ::com::sun::star::uno::Any aPitch;
    ::com::sun::star::uno::Any aEnc;

    SvXMLImportContextRef xStyles;

    XMLFontStylesContext *GetStyles()
    {
        return ((XMLFontStylesContext *)&xStyles);
    }

public:

    TYPEINFO();

    XMLFontStyleContextFontFace( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            XMLFontStylesContext& rStyles );
    virtual ~XMLFontStyleContextFontFace();

    void SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
                       const OUString& rValue );

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
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
};

/// Handles <style:font-face-src>
class XMLFontStyleContextFontFaceSrc : public SvXMLImportContext
{
    const XMLFontStyleContextFontFace& font;
public:

    TYPEINFO();

    XMLFontStyleContextFontFaceSrc( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const XMLFontStyleContextFontFace& font );

    virtual SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
};

/// Handles <style:font-face-uri>
class XMLFontStyleContextFontFaceUri : public SvXMLStyleContext
{
    const XMLFontStyleContextFontFace& font;
    void handleEmbeddedFont( const OUString& url );
public:

    TYPEINFO();

    XMLFontStyleContextFontFaceUri( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const XMLFontStyleContextFontFace& font );

    virtual void SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
        const OUString& rValue );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
