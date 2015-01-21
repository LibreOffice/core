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

#ifndef INCLUDED_VCL_INC_SALLAYOUT_HXX
#define INCLUDED_VCL_INC_SALLAYOUT_HXX

#include <iostream>
#include <list>
#include <vector>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx> // for typedef sal_UCS4
#include <vcl/devicecoordinate.hxx>

#ifndef _TOOLS_LANG_HXX
typedef unsigned short LanguageType;
#endif

#include "magic.h"
#include "salglyphid.hxx"

class SalGraphics;
class PhysicalFontFace;


// used for managing runs e.g. for BiDi, glyph and script fallback
class VCL_PLUGIN_PUBLIC ImplLayoutRuns
{
private:
    int                 mnRunIndex;
    std::vector<int>    maRuns;

public:
            ImplLayoutRuns() { mnRunIndex = 0; maRuns.reserve(8); }

    void    Clear()             { maRuns.clear(); }
    bool    AddPos( int nCharPos, bool bRTL );
    bool    AddRun( int nMinRunPos, int nEndRunPos, bool bRTL );

    bool    IsEmpty() const     { return maRuns.empty(); }
    void    ResetPos()          { mnRunIndex = 0; }
    void    NextRun()           { mnRunIndex += 2; }
    bool    GetRun( int* nMinRunPos, int* nEndRunPos, bool* bRTL ) const;
    bool    GetNextPos( int* nCharPos, bool* bRTL );
    bool    PosIsInRun( int nCharPos ) const;
    bool    PosIsInAnyRun( int nCharPos ) const;
};

class ImplLayoutArgs
{
public:
    // string related inputs
    LanguageTag         maLanguageTag;
    int                 mnFlags;
    int                 mnLength;
    int                 mnMinCharPos;
    int                 mnEndCharPos;
    const sal_Unicode*  mpStr;

    // positioning related inputs
    const DeviceCoordinate* mpDXArray;     // in pixel units
    DeviceCoordinate    mnLayoutWidth;      // in pixel units
    int                 mnOrientation;      // in 0-3600 system

    // data for bidi and glyph+script fallback
    ImplLayoutRuns      maRuns;
    ImplLayoutRuns      maFallbackRuns;

public:
                ImplLayoutArgs( const sal_Unicode* pStr, int nLength,
                                int nMinCharPos, int nEndCharPos, int nFlags,
                                const LanguageTag& rLanguageTag );

    void        SetLayoutWidth( DeviceCoordinate nWidth )       { mnLayoutWidth = nWidth; }
    void        SetDXArray( const DeviceCoordinate* pDXArray )  { mpDXArray = pDXArray; }
    void        SetOrientation( int nOrientation )  { mnOrientation = nOrientation; }

    void        ResetPos()
                    { maRuns.ResetPos(); }
    bool        GetNextPos( int* nCharPos, bool* bRTL )
                    { return maRuns.GetNextPos( nCharPos, bRTL ); }
    bool        GetNextRun( int* nMinRunPos, int* nEndRunPos, bool* bRTL );
    bool        NeedFallback( int nCharPos, bool bRTL )
                    { return maFallbackRuns.AddPos( nCharPos, bRTL ); }
    bool        NeedFallback( int nMinRunPos, int nEndRunPos, bool bRTL )
                    { return maFallbackRuns.AddRun( nMinRunPos, nEndRunPos, bRTL ); }
    // methods used by BiDi and glyph fallback
    bool        NeedFallback() const
                    { return !maFallbackRuns.IsEmpty(); }
    bool        PrepareFallback();

protected:
    void        AddRun( int nMinCharPos, int nEndCharPos, bool bRTL );
};

// For nice SAL_INFO logging of ImplLayoutArgs values
std::ostream &operator <<(std::ostream& s, ImplLayoutArgs &rArgs);

// helper functions often used with ImplLayoutArgs
bool IsDiacritic( sal_UCS4 );
int GetVerticalFlags( sal_UCS4 );
sal_UCS4 GetVerticalChar( sal_UCS4 );

// all positions/widths are in font units
// one exception: drawposition is in pixel units

// Unfortunately there is little documentation to help implementors of
// new classes derived from SalLayout ("layout engines"), and the code
// and data structures are far from obvious.

// For instance, I *think* the important virtual functions in the
// layout engines are called in this order:

// * InitFont()
// * LayoutText()
// * AdjustLayout(), any number of times (but presumably
// usually not at all or just once)
// * Optionally, DrawText()

// Functions that just return information like GetTexWidth() and
// FillDXArray() are called after LayoutText() and before DrawText().

// Another important questions is which parts of an ImplLayoutArgs can
// be changed by callers between LayoutText() and AdjustLayout()
// calls. It probably makes sense only if one assumes that the "string
// related inputs" part are not changed after LayoutText().

// But why use the same ImplLayoutArgs structure as parameter for both
// LayoutText() and AdjustLayout() in the first place? And why
// duplicate some of the fields in both SalLayout and ImplLayoutArgs
// (mnMinCharPos, mnEndCharPos, mnLayoutFlags==mnFlags,
// mnOrientation)? Lost in history...

class VCL_PLUGIN_PUBLIC SalLayout
{
public:
    // used by upper layers
    Point&          DrawBase()                              { return maDrawBase; }
    const Point&    DrawBase() const                        { return maDrawBase; }
    Point&          DrawOffset()                            { return maDrawOffset; }
    const Point&    DrawOffset() const                      { return maDrawOffset; }
    Point           GetDrawPosition( const Point& rRelative = Point(0,0) ) const;

    virtual bool    LayoutText( ImplLayoutArgs& ) = 0;  // first step of layouting
    virtual void    AdjustLayout( ImplLayoutArgs& );    // adjusting after fallback etc.
    virtual void    InitFont() const {}
    virtual void    DrawText( SalGraphics& ) const = 0;
    virtual bool    DrawTextSpecial( SalGraphics& /* rGraphics */, sal_uInt32 /* flags */ ) const { return false; }
#define DRAWTEXT_F_OUTLINE ((sal_uInt32)(1<<0))

    int             GetUnitsPerPixel() const                { return mnUnitsPerPixel; }
    int             GetOrientation() const                  { return mnOrientation; }

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra=0, int nFactor=1) const = 0;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const = 0;
    virtual DeviceCoordinate GetTextWidth() const { return FillDXArray( NULL ); }
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const = 0;
    bool            IsKashidaPosValid ( int /*nCharPos*/ ) const { return true; } // i60594

    // methods using glyph indexing
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIdAry, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
                                   const PhysicalFontFace** pFallbackFonts = NULL ) const = 0;
    virtual bool    GetOutline( SalGraphics&, ::basegfx::B2DPolyPolygonVector& ) const;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const;

    bool    IsSpacingGlyph( sal_GlyphId ) const;

    // reference counting
    void            Release() const;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) = 0;
    virtual void    DropGlyph( int nStart ) = 0;
    virtual void    Simplify( bool bIsBase ) = 0;
    void            DisableGlyphInjection( bool /*bDisable*/ ) {}

protected:
    // used by layout engines
                    SalLayout();
    virtual         ~SalLayout();

    // used by layout layers
    void            SetUnitsPerPixel( int n )               { mnUnitsPerPixel = n; }
    void            SetOrientation( int nOrientation )      // in 0-3600 system
                    { mnOrientation = nOrientation; }

    static int      CalcAsianKerning( sal_UCS4, bool bLeft, bool bVertical );

private:
    // enforce proper copy semantic
    SAL_DLLPRIVATE  SalLayout( const SalLayout& );
    SAL_DLLPRIVATE  SalLayout& operator=( const SalLayout& );

protected:
    int             mnMinCharPos;
    int             mnEndCharPos;
    int             mnLayoutFlags;

    int             mnUnitsPerPixel;
    int             mnOrientation;

    mutable int     mnRefCount;
    mutable Point   maDrawOffset;
    Point           maDrawBase;
};

class VCL_PLUGIN_PUBLIC MultiSalLayout : public SalLayout
{
public:
    virtual void    DrawText( SalGraphics& ) const SAL_OVERRIDE;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const SAL_OVERRIDE;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const SAL_OVERRIDE;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const SAL_OVERRIDE;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIdxAry, Point& rPos,
                                   int&, DeviceCoordinate* pGlyphAdvAry, int* pCharPosAry,
                                   const PhysicalFontFace** pFallbackFonts ) const SAL_OVERRIDE;
    virtual bool    GetOutline( SalGraphics&, ::basegfx::B2DPolyPolygonVector& ) const SAL_OVERRIDE;

    // used only by OutputDevice::ImplLayout, TODO: make friend
    explicit        MultiSalLayout( SalLayout& rBaseLayout,
                                    const PhysicalFontFace* pBaseFont = NULL );
    bool            AddFallback( SalLayout& rFallbackLayout,
                                 ImplLayoutRuns&, const PhysicalFontFace* pFallbackFont );
    virtual bool    LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    InitFont() const SAL_OVERRIDE;

    void SetInComplete(bool bInComplete = true);

protected:
    virtual         ~MultiSalLayout();

private:
    // dummy implementations
    virtual void    MoveGlyph( int, long ) SAL_OVERRIDE {}
    virtual void    DropGlyph( int ) SAL_OVERRIDE {}
    virtual void    Simplify( bool ) SAL_OVERRIDE {}

    // enforce proper copy semantic
    SAL_DLLPRIVATE  MultiSalLayout( const MultiSalLayout& );
    SAL_DLLPRIVATE  MultiSalLayout& operator=( const MultiSalLayout& );

private:
    SalLayout*      mpLayouts[ MAX_FALLBACK ];
    const PhysicalFontFace* mpFallbackFonts[ MAX_FALLBACK ];
    ImplLayoutRuns  maFallbackRuns[ MAX_FALLBACK ];
    int             mnLevel;
    bool            mbInComplete;
};

struct GlyphItem
{
    int     mnFlags;
    int     mnCharPos;      // index in string
    int     mnOrigWidth;    // original glyph width
    int     mnNewWidth;     // width after adjustments
    int     mnXOffset;
    sal_GlyphId maGlyphId;
    Point   maLinearPos;    // absolute position of non rotated string

public:
            GlyphItem()
                : mnFlags(0)
                , mnCharPos(0)
                , mnOrigWidth(0)
                , mnNewWidth(0)
                , mnXOffset(0)
                , maGlyphId(0)
            {}

            GlyphItem( int nCharPos, sal_GlyphId aGlyphId, const Point& rLinearPos,
                long nFlags, int nOrigWidth )
            :   mnFlags(nFlags), mnCharPos(nCharPos),
                mnOrigWidth(nOrigWidth), mnNewWidth(nOrigWidth),
                mnXOffset(0),
                maGlyphId(aGlyphId), maLinearPos(rLinearPos)
            {}

            GlyphItem( int nCharPos, sal_GlyphId aGlyphId, const Point& rLinearPos,
                long nFlags, int nOrigWidth, int nXOffset )
            :   mnFlags(nFlags), mnCharPos(nCharPos),
                mnOrigWidth(nOrigWidth), mnNewWidth(nOrigWidth),
                mnXOffset(nXOffset),
                maGlyphId(aGlyphId), maLinearPos(rLinearPos)
            {}

    enum{ FALLBACK_MASK=0xFF, IS_IN_CLUSTER=0x100, IS_RTL_GLYPH=0x200, IS_DIACRITIC=0x400 };

    bool    IsClusterStart() const  { return ((mnFlags & IS_IN_CLUSTER) == 0); }
    bool    IsRTLGlyph() const      { return ((mnFlags & IS_RTL_GLYPH) != 0); }
    bool    IsDiacritic() const     { return ((mnFlags & IS_DIACRITIC) != 0); }
};

typedef std::list<GlyphItem> GlyphList;
typedef std::vector<GlyphItem> GlyphVector;

class VCL_PLUGIN_PUBLIC GenericSalLayout : public SalLayout
{
public:
    // used by layout engines
    void            AppendGlyph( const GlyphItem& );
    void            Reserve(int size) { m_GlyphItems.reserve(size + 1); }
    virtual void    AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;
    void    ApplyDXArray( ImplLayoutArgs& );
    void    Justify( DeviceCoordinate nNewWidth );
    void            KashidaJustify( long nIndex, int nWidth );
    void            ApplyAsianKerning( const sal_Unicode*, int nLength );
    void            SortGlyphItems();

    // used by upper layers
    virtual DeviceCoordinate GetTextWidth() const SAL_OVERRIDE;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const SAL_OVERRIDE;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const SAL_OVERRIDE;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const SAL_OVERRIDE;

    // used by display layers
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIdxAry, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
                                   const PhysicalFontFace** pFallbackFonts = NULL ) const SAL_OVERRIDE;

protected:
                    GenericSalLayout();
    virtual         ~GenericSalLayout();

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) SAL_OVERRIDE;
    virtual void    DropGlyph( int nStart ) SAL_OVERRIDE;
    virtual void    Simplify( bool bIsBase ) SAL_OVERRIDE;

    bool            GetCharWidths( DeviceCoordinate* pCharWidths ) const;

    GlyphVector     m_GlyphItems;

private:
    mutable Point   maBasePoint;

    // enforce proper copy semantic
    SAL_DLLPRIVATE  GenericSalLayout( const GenericSalLayout& );
    SAL_DLLPRIVATE  GenericSalLayout& operator=( const GenericSalLayout& );
};

#undef SalGraphics

#endif // INCLUDED_VCL_INC_SALLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
