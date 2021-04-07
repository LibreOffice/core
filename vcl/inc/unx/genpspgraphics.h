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

#ifndef INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H
#define INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H

#include <vcl/vclenum.hxx>
#include <config_cairo_canvas.h>

#include <unx/fontmanager.hxx>
#include <salgdi.hxx>
#include <sallayout.hxx>

#include <unx/GenPspGfxBackend.hxx>

class PhysicalFontFace;
class PhysicalFontCollection;

namespace psp { struct JobData; class PrinterGfx; }

class FreetypeFontInstance;
class FontAttributes;
class SalInfoPrinter;
class ImplFontMetricData;

class VCL_DLLPUBLIC GenPspGraphics final : public SalGraphicsAutoDelegateToImpl
{
    std::unique_ptr<GenPspGfxBackend> m_pBackend;

    psp::JobData*           m_pJobData;
    psp::PrinterGfx*        m_pPrinterGfx;

    rtl::Reference<FreetypeFontInstance>
                            m_pFreetypeFont[ MAX_FALLBACK ];
public:
                            GenPspGraphics();
    virtual                ~GenPspGraphics() override;

    void                    Init( psp::JobData* pJob, psp::PrinterGfx* pGfx );

    // helper methods
    static const void *     DoGetEmbedFontData(psp::fontID aFont, tools::Long* pDataLen);
    static void             DoFreeEmbedFontData( const void* pData, tools::Long nLen );

    // helper methods for sharing with X11SalGraphics
    static void             DoGetGlyphWidths( psp::fontID aFont,
                                              bool bVertical,
                                              std::vector< sal_Int32 >& rWidths,
                                              Ucs2UIntMap& rUnicodeEnc );

    static FontAttributes Info2FontAttributes( const psp::FastPrintFontInfo& );
    static void             AnnounceFonts( PhysicalFontCollection*,
                                           const psp::FastPrintFontInfo& );

    // override all pure virtual methods
    virtual SalGraphicsImpl* GetImpl() const override
    {
        return m_pBackend.get();
    }

    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;

    virtual void            SetTextColor( Color nColor ) override;
    virtual void            SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void            GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    virtual FontCharMapRef  GetFontCharMap() const override;
    virtual bool            GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void            GetDevFontList( PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() override;
    virtual bool            AddTempDevFont( PhysicalFontCollection*,
                                            const OUString& rFileURL,
                                            const OUString& rFontName ) override;
    static bool             AddTempDevFontHelper( PhysicalFontCollection* pFontCollection,
                                                  const OUString& rFileURL,
                                                  const OUString& rFontName);

    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              const sal_GlyphId* pGlyphIDs,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo ) override;
    virtual const void*     GetEmbedFontData(const PhysicalFontFace*, tools::Long* pDataLen) override;
    virtual void            FreeEmbedFontData( const void* pData, tools::Long nDataLen ) override;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;
    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    virtual bool            SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
#endif // ENABLE_CAIRO_CANVAS
};

#endif // INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
