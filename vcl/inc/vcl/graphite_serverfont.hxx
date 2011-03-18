/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_GRAPHITESERVERFONT_HXX
#define _SV_GRAPHITESERVERFONT_HXX

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

#ifndef MSC
#include "vcl/graphite_layout.hxx"

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
//
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
        virtual int     GetTextBreak( long mw, long ce, int f ) const  { return maImpl.GetTextBreak(mw, ce, f); }
        virtual void    GetCaretPositions( int as, sal_Int32* cxa ) const   { maImpl.GetCaretPositions(as, cxa); }

        // used by display layers
        virtual int     GetNextGlyphs( int l, sal_GlyphId* gia, Point& p, int& s,
                        sal_Int32* gaa = NULL, int* cpa = NULL ) const
        {
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            return maImpl.GetNextGlyphs(l, gia, p, s, gaa, cpa);
        }

        virtual void    MoveGlyph( int nStart, long nNewXPos ) { maImpl.MoveGlyph(nStart, nNewXPos); };
        virtual void    DropGlyph( int nStart ) { maImpl.DropGlyph(nStart); };
        virtual void    Simplify( bool bIsBase ) { maImpl.Simplify(bIsBase); };

        virtual ~GraphiteServerFontLayout() throw();

        static bool IsGraphiteEnabledFont(ServerFont * pServerFont);
// For use with PspGraphics
        const sal_Unicode* getTextPtr() const { return mpStr; };
        int getMinCharPos() const { return mnMinCharPos; }
        int getMaxCharPos() const { return mnEndCharPos; }
};

#endif
#endif //_SV_GRAPHITESERVERFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
