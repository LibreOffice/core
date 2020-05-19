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

#ifndef INCLUDED_VCL_INC_UNX_FREETYPETEXTRENDER_HXX
#define INCLUDED_VCL_INC_UNX_FREETYPETEXTRENDER_HXX

#include <textrender.hxx>

class FreetypeFontInstance;

// Generic implementation that uses freetype, but DrawTextLayout()
// still needs implementing (e.g. by Cairo or Skia).
class VCL_DLLPUBLIC FreeTypeTextRenderImpl : public TextRenderImpl
{
protected:
    rtl::Reference<FreetypeFontInstance>
                            mpFreetypeFont[ MAX_FALLBACK ];

    Color           mnTextColor;

public:
    FreeTypeTextRenderImpl();
    virtual ~FreeTypeTextRenderImpl() override;

    virtual void                SetTextColor( Color nColor ) override;
    virtual void                SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void                GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    virtual FontCharMapRef      GetFontCharMap() const override;
    virtual bool                GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void                GetDevFontList( PhysicalFontCollection* ) override;
    virtual void                ClearDevFontCache() override;
    virtual bool                AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
    virtual bool                CreateFontSubset(
                                    const OUString& rToFile,
                                    const PhysicalFontFace*,
                                    const sal_GlyphId* pGlyphIDs,
                                    const sal_uInt8* pEncoding,
                                    sal_Int32* pWidths,
                                    int nGlyphs,
                                    FontSubsetInfo& rInfo) override;

    virtual const void*         GetEmbedFontData(const PhysicalFontFace*, long* pDataLen) override;
    virtual void                FreeEmbedFontData( const void* pData, long nDataLen ) override;
    virtual void                GetGlyphWidths(
                                    const PhysicalFontFace*,
                                    bool bVertical,
                                    std::vector< sal_Int32 >& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc ) override;

    virtual std::unique_ptr<GenericSalLayout>
                                GetTextLayout(int nFallbackLevel) override;
#if ENABLE_CAIRO_CANVAS
    virtual SystemFontData      GetSysFontData( int nFallbackLevel ) const override;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
