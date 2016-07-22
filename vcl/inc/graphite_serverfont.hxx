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

#ifndef INCLUDED_VCL_INC_GRAPHITE_SERVERFONT_HXX
#define INCLUDED_VCL_INC_GRAPHITE_SERVERFONT_HXX

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

#ifndef _MSC_VER
#include <graphite_layout.hxx>

#include "unx/glyphcache.hxx"

class PhysicalFontFace;

// Modules

class VCL_PLUGIN_PUBLIC GraphiteLayoutImpl : public GraphiteLayout
{
public:
    GraphiteLayoutImpl(const gr_face * pFace,
                       ServerFont & rServerFont) throw()
    : GraphiteLayout(pFace), mrServerFont(rServerFont) {};
    virtual ~GraphiteLayoutImpl() throw() {};
    virtual sal_GlyphId getKashidaGlyph(int & width) override;
private:
    ServerFont & mrServerFont;
};

// This class implments the server font specific parts.
// @author tse

class VCL_PLUGIN_PUBLIC GraphiteServerFontLayout : public ServerFontLayout
{
private:
        // mutable so that the DrawOffset/DrawBase can be set
        mutable GraphiteLayoutImpl maImpl;
        grutils::GrFeatureParser * mpFeatures;
public:
        GraphiteServerFontLayout(ServerFont& pServerFont) throw();

        virtual bool  LayoutText( ImplLayoutArgs& rArgs) override
        {
            SalLayout::AdjustLayout(rArgs);
            return maImpl.LayoutText(rArgs);
        };    // first step of layout
        virtual void  AdjustLayout( ImplLayoutArgs& rArgs) override
        {
            SalLayout::AdjustLayout(rArgs);
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            maImpl.AdjustLayout(rArgs);
        };
        virtual DeviceCoordinate GetTextWidth() const override
        {
            return maImpl.GetTextWidth();
        }
        virtual DeviceCoordinate FillDXArray( DeviceCoordinate* dxa ) const override
        {
            return maImpl.FillDXArray(dxa);
        }
        virtual sal_Int32 GetTextBreak(DeviceCoordinate max_width, DeviceCoordinate extra=0, int factor=1) const override
        {
            return maImpl.GetTextBreak(max_width, extra, factor);
        }
        virtual void    GetCaretPositions( int as, long* cxa ) const override
        {
            maImpl.GetCaretPositions(as, cxa);
        }

        // used by display layers
        virtual int     GetNextGlyphs( int l, sal_GlyphId* gia, Point& p, int& s,
                        long* gaa = nullptr, int* cpa = nullptr,
                        const PhysicalFontFace** pFallbackFonts = nullptr ) const override
        {
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            return maImpl.GetNextGlyphs(l, gia, p, s, gaa, cpa, pFallbackFonts);
        }

        virtual void    MoveGlyph( int nStart, long nNewXPos ) override { maImpl.MoveGlyph(nStart, nNewXPos); };
        virtual void    DropGlyph( int nStart ) override { maImpl.DropGlyph(nStart); };
        virtual void    Simplify( bool bIsBase ) override { maImpl.Simplify(bIsBase); };

        virtual ~GraphiteServerFontLayout() throw();

        static bool IsGraphiteEnabledFont(ServerFont& rServerFont);
};

#endif
#endif // INCLUDED_VCL_INC_GRAPHITE_SERVERFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
