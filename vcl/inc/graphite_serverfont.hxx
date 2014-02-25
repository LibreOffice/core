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

// Modules

class VCL_PLUGIN_PUBLIC GraphiteLayoutImpl : public GraphiteLayout
{
public:
    GraphiteLayoutImpl(const gr_face * pFace,
                       ServerFont & rServerFont) throw()
    : GraphiteLayout(pFace), mrServerFont(rServerFont) {};
    virtual ~GraphiteLayoutImpl() throw() {};
    virtual sal_GlyphId getKashidaGlyph(int & width);
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
        const sal_Unicode * mpStr;
public:
        GraphiteServerFontLayout(ServerFont& pServerFont) throw();

        virtual bool  LayoutText( ImplLayoutArgs& rArgs)
        {
            mpStr = rArgs.mpStr;
            SalLayout::AdjustLayout(rArgs);
            return maImpl.LayoutText(rArgs);
        };    // first step of layout
        virtual void  AdjustLayout( ImplLayoutArgs& rArgs)
        {
            SalLayout::AdjustLayout(rArgs);
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            maImpl.AdjustLayout(rArgs);
        };
        virtual long    GetTextWidth() const                           { return maImpl.GetTextWidth(); }
        virtual long    FillDXArray( sal_Int32* dxa ) const                 { return maImpl.FillDXArray(dxa); }
        virtual sal_Int32 GetTextBreak(long mw, long ce, int f) const SAL_OVERRIDE
            { return maImpl.GetTextBreak(mw, ce, f); }
        virtual void    GetCaretPositions( int as, sal_Int32* cxa ) const   { maImpl.GetCaretPositions(as, cxa); }

        // used by display layers
        virtual int     GetNextGlyphs( int l, sal_GlyphId* gia, Point& p, int& s,
                        sal_Int32* gaa = NULL, int* cpa = NULL,
                        const PhysicalFontFace** pFallbackFonts = NULL ) const
        {
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            return maImpl.GetNextGlyphs(l, gia, p, s, gaa, cpa, pFallbackFonts);
        }

        virtual void    MoveGlyph( int nStart, long nNewXPos ) { maImpl.MoveGlyph(nStart, nNewXPos); };
        virtual void    DropGlyph( int nStart ) { maImpl.DropGlyph(nStart); };
        virtual void    Simplify( bool bIsBase ) { maImpl.Simplify(bIsBase); };

        virtual ~GraphiteServerFontLayout() throw();

        static bool IsGraphiteEnabledFont(ServerFont& rServerFont);
// For use with PspGraphics
        const sal_Unicode* getTextPtr() const { return mpStr; };
        int getMinCharPos() const { return mnMinCharPos; }
        int getMaxCharPos() const { return mnEndCharPos; }
};

#endif
#endif // INCLUDED_VCL_INC_GRAPHITE_SERVERFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
