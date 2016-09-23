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

#ifndef INCLUDED_VCL_INC_UNX_CAIROTEXTRENDER_HXX
#define INCLUDED_VCL_INC_UNX_CAIROTEXTRENDER_HXX

#include <vcl/region.hxx>

#include "textrender.hxx"

#include <deque>

class ServerFont;
class GlyphCache;
typedef struct _cairo cairo_t;

class VCL_DLLPUBLIC CairoTextRender : public TextRenderImpl
{
    ServerFont*     mpServerFont[ MAX_FALLBACK ];

    SalColor        mnTextColor;

    bool            bDisableGraphite_;

protected:
    virtual GlyphCache&         getPlatformGlyphCache() = 0;
    virtual cairo_t*            getCairoContext() = 0;
    virtual void                getSurfaceOffset(double& nDX, double& nDY) = 0;
    virtual void                releaseCairoContext(cairo_t* cr) = 0;

    bool                        setFont( const FontSelectPattern *pEntry, int nFallbackLevel );

    virtual void                clipRegion(cairo_t* cr) = 0;

public:
                                CairoTextRender();


    virtual void                SetTextColor( SalColor nSalColor ) override;
    virtual void                SetFont( FontSelectPattern*, int nFallbackLevel ) override;
    virtual void                GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    virtual const FontCharMapRef GetFontCharMap() const override;
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

    virtual const Ucs2SIntMap*  GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const**) override;
    virtual const void*         GetEmbedFontData(
                                    const PhysicalFontFace*,
                                    const sal_Ucs* pUnicodes,
                                    sal_Int32* pWidths,
                                    size_t nLen,
                                    FontSubsetInfo& rInfo,
                                    long* pDataLen ) override;

    virtual void                FreeEmbedFontData( const void* pData, long nDataLen ) override;
    virtual void                GetGlyphWidths(
                                    const PhysicalFontFace*,
                                    bool bVertical,
                                    std::vector< sal_Int32 >& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc ) override;

    virtual bool                GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) override;
    virtual bool                GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& ) override;
    virtual SalLayout*          GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) override;
    virtual void                DrawServerFontLayout( const GenericSalLayout&, const ServerFont& ) override;
#if ENABLE_CAIRO_CANVAS
    virtual SystemFontData      GetSysFontData( int nFallbackLevel ) const override;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
