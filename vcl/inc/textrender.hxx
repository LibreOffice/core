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

#ifndef INCLUDED_VCL_INC_UNX_CAIROFONTIMPL_HXX
#define INCLUDED_VCL_INC_UNX_CAIROFONTIMPL_HXX

#include <sal/types.h>
#include <vcl/salgtype.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/metric.hxx>
#include <basebmp/bitmapdevice.hxx>
#include "salgdi.hxx"
#include "salglyphid.hxx"
#include "fontsubset.hxx"
#include <config_cairo_canvas.h>

class PspSalPrinter;
class PspSalInfoPrinter;
class ServerFont;
class ImplLayoutArgs;
class ServerFontLayout;
class PhysicalFontCollection;
class PhysicalFontFace;

class TextRenderImpl
{
public:
    virtual ~TextRenderImpl() {}

    virtual void                    SetTextColor( SalColor nSalColor ) = 0;
    virtual sal_uInt16              SetFont( FontSelectPattern*, int nFallbackLevel ) = 0;
    virtual void                    GetFontAttributes( ImplFontAttributes*, int nFallbackLevel ) = 0;
    virtual const FontCharMapPtr    GetFontCharMap() const = 0;
    virtual bool                    GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const = 0;
    virtual void                    GetDevFontList( PhysicalFontCollection* ) = 0;
    virtual void                    ClearDevFontCache() = 0;
    virtual bool                    AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) = 0;
    virtual bool                    CreateFontSubset(
                                        const OUString& rToFile,
                                        const PhysicalFontFace*,
                                        const sal_GlyphId* pGlyphIDs,
                                        const sal_uInt8* pEncoding,
                                        sal_Int32* pWidths,
                                        int nGlyphs,
                                        FontSubsetInfo& rInfo) = 0;

    virtual const Ucs2SIntMap*      GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const** ppPriority) = 0;
    virtual const void*             GetEmbedFontData(
                                        const PhysicalFontFace*,
                                        const sal_Ucs* pUnicodes,
                                        sal_Int32* pWidths,
                                        size_t nLen,
                                        FontSubsetInfo& rInfo,
                                        long* pDataLen ) = 0;

    virtual void                    FreeEmbedFontData( const void* pData, long nDataLen ) = 0;
    virtual void                    GetGlyphWidths(
                                        const PhysicalFontFace*,
                                        bool bVertical,
                                        Int32Vector& rWidths,
                                        Ucs2UIntMap& rUnicodeEnc ) = 0;

    virtual bool                    GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) = 0;
    virtual bool                    GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& ) = 0;
    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) = 0;
    virtual void                    DrawServerFontLayout( const ServerFontLayout& ) = 0;
#if ENABLE_CAIRO_CANVAS
    virtual SystemFontData          GetSysFontData( int nFallbackLevel ) const = 0;
#endif // ENABLE_CAIRO_CANVAS
};

#endif

/* vim:set tabstop=4 shiftwidth=4 softtabstop=4 expandtab: */
