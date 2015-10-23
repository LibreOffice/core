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
#ifndef INCLUDED_XMLOFF_XMLFONTSTYLESCONTEXT_HXX
#define INCLUDED_XMLOFF_XMLFONTSTYLESCONTEXT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>

#include <vector>

#include <xmloff/xmlstyle.hxx>

struct XMLPropertyState;
class SvXMLTokenMap;
class XMLFontFamilyNamePropHdl;
class XMLFontFamilyPropHdl;
class XMLFontPitchPropHdl;
class XMLFontEncodingPropHdl;

class XMLOFF_DLLPUBLIC XMLFontStylesContext : public SvXMLStylesContext
{
    XMLFontFamilyNamePropHdl    *pFamilyNameHdl;
    XMLFontFamilyPropHdl        *pFamilyHdl;
    XMLFontPitchPropHdl         *pPitchHdl;
    XMLFontEncodingPropHdl      *pEncHdl;

    SvXMLTokenMap           *pFontStyleAttrTokenMap;

    rtl_TextEncoding        eDfltEncoding;

protected:

    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

public:

    TYPEINFO_OVERRIDE();

    XMLFontStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            rtl_TextEncoding eDfltEnc );
    virtual ~XMLFontStylesContext();

    const SvXMLTokenMap& GetFontStyleAttrTokenMap() const
    {
        return *pFontStyleAttrTokenMap;
    }

    bool FillProperties( const OUString& rName,
                         ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const;

    rtl_TextEncoding GetDfltCharset() const { return eDfltEncoding; }

    XMLFontFamilyNamePropHdl& GetFamilyNameHdl() const { return *pFamilyNameHdl; }
    XMLFontFamilyPropHdl& GetFamilyHdl() const { return *pFamilyHdl; }
    XMLFontPitchPropHdl& GetPitchHdl() const { return *pPitchHdl; }
    XMLFontEncodingPropHdl& GetEncodingHdl() const { return *pEncHdl; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
