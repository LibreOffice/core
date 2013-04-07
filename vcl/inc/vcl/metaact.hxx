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

#ifndef _SV_METAACT_HXX
#define _SV_METAACT_HXX

#include <vcl/dllapi.h>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/wall.hxx>
#include <vcl/font.hxx>
#include <tools/poly.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/region.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/lineinfo.hxx>

class SvStream;

#define META_NULL_ACTION                    (0)
#define META_PIXEL_ACTION                   (100)
#define META_POINT_ACTION                   (101)
#define META_LINE_ACTION                    (102)
#define META_RECT_ACTION                    (103)
#define META_ROUNDRECT_ACTION               (104)
#define META_ELLIPSE_ACTION                 (105)
#define META_ARC_ACTION                     (106)
#define META_PIE_ACTION                     (107)
#define META_CHORD_ACTION                   (108)
#define META_POLYLINE_ACTION                (109)
#define META_POLYGON_ACTION                 (110)
#define META_POLYPOLYGON_ACTION             (111)
#define META_TEXT_ACTION                    (112)
#define META_TEXTARRAY_ACTION               (113)
#define META_STRETCHTEXT_ACTION             (114)
#define META_TEXTRECT_ACTION                (115)
#define META_BMP_ACTION                     (116)
#define META_BMPSCALE_ACTION                (117)
#define META_BMPSCALEPART_ACTION            (118)
#define META_BMPEX_ACTION                   (119)
#define META_BMPEXSCALE_ACTION              (120)
#define META_BMPEXSCALEPART_ACTION          (121)
#define META_MASK_ACTION                    (122)
#define META_MASKSCALE_ACTION               (123)
#define META_MASKSCALEPART_ACTION           (124)
#define META_GRADIENT_ACTION                (125)
#define META_HATCH_ACTION                   (126)
#define META_WALLPAPER_ACTION               (127)
#define META_CLIPREGION_ACTION              (128)
#define META_ISECTRECTCLIPREGION_ACTION     (129)
#define META_ISECTREGIONCLIPREGION_ACTION   (130)
#define META_MOVECLIPREGION_ACTION          (131)
#define META_LINECOLOR_ACTION               (132)
#define META_FILLCOLOR_ACTION               (133)
#define META_TEXTCOLOR_ACTION               (134)
#define META_TEXTFILLCOLOR_ACTION           (135)
#define META_TEXTALIGN_ACTION               (136)
#define META_MAPMODE_ACTION                 (137)
#define META_FONT_ACTION                    (138)
#define META_PUSH_ACTION                    (139)
#define META_POP_ACTION                     (140)
#define META_RASTEROP_ACTION                (141)
#define META_TRANSPARENT_ACTION             (142)
#define META_EPS_ACTION                     (143)
#define META_REFPOINT_ACTION                (144)
#define META_TEXTLINECOLOR_ACTION           (145)
#define META_TEXTLINE_ACTION                (146)
#define META_FLOATTRANSPARENT_ACTION        (147)
#define META_GRADIENTEX_ACTION              (148)
#define META_LAYOUTMODE_ACTION              (149)
#define META_TEXTLANGUAGE_ACTION            (150)
#define META_OVERLINECOLOR_ACTION           (151)

#define META_COMMENT_ACTION                 (512)

struct ImplMetaReadData
{
    rtl_TextEncoding        meActualCharSet;

                            ImplMetaReadData() :
                                meActualCharSet( RTL_TEXTENCODING_ASCII_US )
                            {
                            }
};

struct ImplMetaWriteData
{
    rtl_TextEncoding        meActualCharSet;

                            ImplMetaWriteData() :
                                meActualCharSet( RTL_TEXTENCODING_ASCII_US )
                            {
                            }
};

#define DECL_META_ACTION( Name, nType )                                     \
                        Meta##Name##Action();                               \
protected:                                                                  \
    virtual             ~Meta##Name##Action();                              \
public:                                                                     \
    virtual void        Execute( OutputDevice* pOut );                      \
    virtual MetaAction* Clone();                                            \
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ); \
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

#define IMPL_META_ACTION( Name, nType )                                 \
Meta##Name##Action::Meta##Name##Action() :                              \
    MetaAction( nType ) {}                                              \
Meta##Name##Action::~Meta##Name##Action() {}

class VCL_DLLPUBLIC MetaAction
{
private:
    sal_uLong               mnRefCount;
    sal_uInt16              mnType;

    virtual sal_Bool    Compare( const MetaAction& ) const;

protected:
    virtual             ~MetaAction();

public:
                        MetaAction();
                        MetaAction( sal_uInt16 nType );

    virtual void        Execute( OutputDevice* pOut );

    virtual MetaAction* Clone();

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData );
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

    sal_uInt16              GetType() const { return mnType; }
    sal_uLong               GetRefCount() const { return mnRefCount; }
    void                ResetRefCount() { mnRefCount = 1; }
    void                Duplicate()  { mnRefCount++; }
    void                Delete() { if ( 0 == --mnRefCount ) delete this; }

public:
    static MetaAction*  ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData );
};

class VCL_DLLPUBLIC MetaPixelAction : public MetaAction
{
private:
    Point               maPt;
    Color               maColor;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Pixel, META_PIXEL_ACTION )

                        MetaPixelAction( const Point& rPt, const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Point&        GetPoint() const { return maPt; }
    const Color&        GetColor() const { return maColor; }
};

class VCL_DLLPUBLIC MetaPointAction : public MetaAction
{
private:
    Point               maPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Point, META_POINT_ACTION )

                        MetaPointAction( const Point& rPt );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaLineAction : public MetaAction
{
private:

    LineInfo            maLineInfo;
    Point               maStartPt;
    Point               maEndPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Line, META_LINE_ACTION )

                        MetaLineAction( const Point& rStart, const Point& rEnd );
                        MetaLineAction( const Point& rStart, const Point& rEnd,
                                        const LineInfo& rLineInfo );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

class VCL_DLLPUBLIC MetaRectAction : public MetaAction
{
private:

    Rectangle           maRect;

    virtual sal_Bool    Compare( const MetaAction& ) const;
public:
                        DECL_META_ACTION( Rect, META_RECT_ACTION )

                        MetaRectAction( const Rectangle& rRect );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaRoundRectAction : public MetaAction
{
private:

    Rectangle           maRect;
    sal_uInt32          mnHorzRound;
    sal_uInt32          mnVertRound;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( RoundRect, META_ROUNDRECT_ACTION )

                        MetaRoundRectAction( const Rectangle& rRect,
                                             sal_uInt32 nHorzRound, sal_uInt32 nVertRound );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    sal_uInt32          GetHorzRound() const { return mnHorzRound; }
    sal_uInt32          GetVertRound() const { return mnVertRound; }
};

class VCL_DLLPUBLIC MetaEllipseAction : public MetaAction
{
private:

    Rectangle           maRect;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Ellipse, META_ELLIPSE_ACTION )

                        MetaEllipseAction( const Rectangle& rRect );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaArcAction : public MetaAction
{
private:

    Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Arc, META_ARC_ACTION )

                        MetaArcAction( const Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaPieAction : public MetaAction
{
private:

    Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Pie, META_PIE_ACTION )

                        MetaPieAction( const Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaChordAction : public MetaAction
{
private:

    Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Chord, META_CHORD_ACTION )

                        MetaChordAction( const Rectangle& rRect,
                                         const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaPolyLineAction : public MetaAction
{
private:

    LineInfo            maLineInfo;
    Polygon             maPoly;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( PolyLine, META_POLYLINE_ACTION )

                        MetaPolyLineAction( const Polygon& rPoly );
                        MetaPolyLineAction( const Polygon& rPoly, const LineInfo& rLineInfo );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Polygon&      GetPolygon() const { return maPoly; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

class VCL_DLLPUBLIC MetaPolygonAction : public MetaAction
{
private:

    Polygon             maPoly;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Polygon, META_POLYGON_ACTION )

                        MetaPolygonAction( const Polygon& rPoly );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Polygon&      GetPolygon() const { return maPoly; }
};

class VCL_DLLPUBLIC MetaPolyPolygonAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( PolyPolygon, META_POLYPOLYGON_ACTION )

                        MetaPolyPolygonAction( const PolyPolygon& rPolyPoly );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
};

class VCL_DLLPUBLIC MetaTextAction : public MetaAction
{
private:

    Point               maPt;
    OUString       maStr;
    sal_uInt16          mnIndex;
    sal_uInt16          mnLen;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Text, META_TEXT_ACTION )

    MetaTextAction( const Point& rPt, const OUString& rStr,
                    sal_uInt16 nIndex, sal_uInt16 nLen );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Point&        GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_uInt16              GetIndex() const { return mnIndex; }
    sal_uInt16              GetLen() const { return mnLen; }
};

class VCL_DLLPUBLIC MetaTextArrayAction : public MetaAction
{
private:

    Point               maStartPt;
    OUString       maStr;
    sal_Int32*          mpDXAry;
    sal_uInt16          mnIndex;
    sal_uInt16          mnLen;

    virtual sal_Bool    Compare( const MetaAction& ) const;

protected:
    virtual             ~MetaTextArrayAction();

public:
                        MetaTextArrayAction();
                        MetaTextArrayAction( const MetaTextArrayAction& rAction );
    MetaTextArrayAction( const Point& rStartPt, const OUString& rStr,
                         const sal_Int32* pDXAry, sal_uInt16 nIndex,
                         sal_uInt16 nLen );

    virtual void        Execute( OutputDevice* pOut );

    virtual MetaAction* Clone();

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData );
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

    const Point&        GetPoint() const { return maStartPt; }
    const OUString& GetText() const { return maStr; }
    sal_uInt16              GetIndex() const { return mnIndex; }
    sal_uInt16              GetLen() const { return mnLen; }
    sal_Int32*          GetDXArray() const { return mpDXAry; }
};

class VCL_DLLPUBLIC MetaStretchTextAction : public MetaAction
{
private:

    Point               maPt;
    OUString       maStr;
    sal_uInt32          mnWidth;
    sal_uInt16          mnIndex;
    sal_uInt16          mnLen;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( StretchText, META_STRETCHTEXT_ACTION )

    MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                           const OUString& rStr,
                           sal_uInt16 nIndex, sal_uInt16 nLen );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Point&        GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_uInt32          GetWidth() const { return mnWidth; }
    sal_uInt16              GetIndex() const { return mnIndex; }
    sal_uInt16              GetLen() const { return mnLen; }
};

class VCL_DLLPUBLIC MetaTextRectAction : public MetaAction
{
private:

    Rectangle           maRect;
    OUString       maStr;
    sal_uInt16          mnStyle;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextRect, META_TEXTRECT_ACTION )

    MetaTextRectAction( const Rectangle& rRect,
                        const OUString& rStr, sal_uInt16 nStyle );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    const OUString& GetText() const { return maStr; }
    sal_uInt16              GetStyle() const { return mnStyle; }
};

class VCL_DLLPUBLIC MetaTextLineAction : public MetaAction
{
private:

    Point               maPos;
    long                mnWidth;
    FontStrikeout       meStrikeout;
    FontUnderline       meUnderline;
    FontUnderline       meOverline;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextLine, META_TEXTLINE_ACTION )

                        MetaTextLineAction( const Point& rPos, long nWidth,
                                            FontStrikeout eStrikeout,
                                            FontUnderline eUnderline,
                                            FontUnderline eOverline );
    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Point&        GetStartPoint() const { return maPos; }
    long                GetWidth() const { return mnWidth; }
    FontStrikeout       GetStrikeout() const { return meStrikeout; }
    FontUnderline       GetUnderline() const { return meUnderline; }
    FontUnderline       GetOverline()  const { return meOverline; }
};

class VCL_DLLPUBLIC MetaBmpAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Bmp, META_BMP_ACTION )

                        MetaBmpAction( const Point& rPt, const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaBmpScaleAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maPt;
    Size                maSz;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( BmpScale, META_BMPSCALE_ACTION )

                        MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                            const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

class VCL_DLLPUBLIC MetaBmpScalePartAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( BmpScalePart, META_BMPSCALEPART_ACTION )

                        MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

class VCL_DLLPUBLIC MetaBmpExAction : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( BmpEx, META_BMPEX_ACTION )

                        MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaBmpExScaleAction : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maPt;
    Size                maSz;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( BmpExScale, META_BMPEXSCALE_ACTION )

                        MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                              const BitmapEx& rBmpEx ) ;

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

class VCL_DLLPUBLIC MetaBmpExScalePartAction : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( BmpExScalePart, META_BMPEXSCALEPART_ACTION )

                        MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                  const Point& rSrcPt, const Size& rSrcSz,
                                                  const BitmapEx& rBmpEx );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

class VCL_DLLPUBLIC MetaMaskAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maPt;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Mask, META_MASK_ACTION )

                        MetaMaskAction( const Point& rPt,
                                        const Bitmap& rBmp,
                                        const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaMaskScaleAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maPt;
    Size                maSz;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( MaskScale, META_MASKSCALE_ACTION )

                        MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                             const Bitmap& rBmp,
                                             const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

class VCL_DLLPUBLIC MetaMaskScalePartAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( MaskScalePart, META_MASKSCALEPART_ACTION )

                        MetaMaskScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                 const Point& rSrcPt, const Size& rSrcSz,
                                                 const Bitmap& rBmp,
                                                 const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

class VCL_DLLPUBLIC MetaGradientAction : public MetaAction
{
private:

    Rectangle           maRect;
    Gradient            maGradient;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Gradient, META_GRADIENT_ACTION )

                        MetaGradientAction( const Rectangle& rRect, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaGradientExAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;
    Gradient            maGradient;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( GradientEx, META_GRADIENTEX_ACTION )

                        MetaGradientExAction( const PolyPolygon& rPolyPoly, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaHatchAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;
    Hatch               maHatch;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Hatch, META_HATCH_ACTION )

                        MetaHatchAction( const PolyPolygon& rPolyPoly, const Hatch& rHatch );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Hatch&        GetHatch() const { return maHatch; }
};

class VCL_DLLPUBLIC MetaWallpaperAction : public MetaAction
{
private:

    Rectangle           maRect;
    Wallpaper           maWallpaper;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Wallpaper, META_WALLPAPER_ACTION )

                        MetaWallpaperAction( const Rectangle& rRect,
                                             const Wallpaper& rPaper );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
    const Wallpaper&    GetWallpaper() const { return maWallpaper; }
};

class VCL_DLLPUBLIC MetaClipRegionAction : public MetaAction
{
private:

    Region              maRegion;
    sal_Bool                mbClip;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( ClipRegion, META_CLIPREGION_ACTION )

                        MetaClipRegionAction( const Region& rRegion, sal_Bool bClip );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Region&       GetRegion() const { return maRegion; }
    sal_Bool                IsClipping() const { return mbClip; }
};

class VCL_DLLPUBLIC MetaISectRectClipRegionAction : public MetaAction
{
private:

    Rectangle           maRect;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( ISectRectClipRegion, META_ISECTRECTCLIPREGION_ACTION )

                        MetaISectRectClipRegionAction( const Rectangle& rRect );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaISectRegionClipRegionAction : public MetaAction
{
private:

    Region              maRegion;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( ISectRegionClipRegion, META_ISECTREGIONCLIPREGION_ACTION )

                        MetaISectRegionClipRegionAction( const Region& rRegion );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const Region&       GetRegion() const { return maRegion; }
};

class VCL_DLLPUBLIC MetaMoveClipRegionAction : public MetaAction
{
private:

    long                mnHorzMove;
    long                mnVertMove;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( MoveClipRegion, META_MOVECLIPREGION_ACTION )

                        MetaMoveClipRegionAction( long nHorzMove, long nVertMove );

    virtual void        Scale( double fScaleX, double fScaleY );

    long                GetHorzMove() const { return mnHorzMove; }
    long                GetVertMove() const { return mnVertMove; }
};

class VCL_DLLPUBLIC MetaLineColorAction : public MetaAction
{
private:

    Color               maColor;
    sal_Bool                mbSet;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( LineColor, META_LINECOLOR_ACTION )

                        MetaLineColorAction( const Color& rColor, sal_Bool bSet );

    const Color&        GetColor() const { return maColor; }
    sal_Bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaFillColorAction : public MetaAction
{
private:

    Color               maColor;
    sal_Bool                mbSet;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( FillColor, META_FILLCOLOR_ACTION )

                        MetaFillColorAction( const Color& rColor, sal_Bool bSet );

    const Color&        GetColor() const { return maColor; }
    sal_Bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextColorAction : public MetaAction
{
private:

    Color               maColor;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextColor, META_TEXTCOLOR_ACTION )

                        MetaTextColorAction( const Color& rColor );

    const Color&        GetColor() const { return maColor; }
};

class VCL_DLLPUBLIC MetaTextFillColorAction : public MetaAction
{
private:

    Color               maColor;
    sal_Bool                mbSet;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextFillColor, META_TEXTFILLCOLOR_ACTION )

                        MetaTextFillColorAction( const Color& rColor, sal_Bool bSet );

    const Color&        GetColor() const { return maColor; }
    sal_Bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextLineColorAction : public MetaAction
{
private:

    Color               maColor;
    sal_Bool                mbSet;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextLineColor, META_TEXTLINECOLOR_ACTION )

                        MetaTextLineColorAction( const Color& rColor, sal_Bool bSet );

    const Color&        GetColor() const { return maColor; }
    sal_Bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaOverlineColorAction : public MetaAction
{
private:

    Color               maColor;
    sal_Bool                mbSet;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( OverlineColor, META_OVERLINECOLOR_ACTION )

                        MetaOverlineColorAction( const Color& rColor, sal_Bool bSet );

    const Color&        GetColor() const { return maColor; }
    sal_Bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextAlignAction : public MetaAction
{
private:

    TextAlign           maAlign;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextAlign, META_TEXTALIGN_ACTION )

                        MetaTextAlignAction( TextAlign aAlign );

    TextAlign           GetTextAlign() const { return maAlign; }
};

class VCL_DLLPUBLIC MetaMapModeAction : public MetaAction
{
private:

    MapMode             maMapMode;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( MapMode, META_MAPMODE_ACTION )

                        MetaMapModeAction( const MapMode& rMapMode );

    virtual void        Scale( double fScaleX, double fScaleY );

    const MapMode&      GetMapMode() const { return maMapMode; }
};

class VCL_DLLPUBLIC MetaFontAction : public MetaAction
{
private:

    Font                maFont;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Font, META_FONT_ACTION )

                        MetaFontAction( const Font& rFont );

    virtual void        Scale( double fScaleX, double fScaleY );

    const Font&         GetFont() const { return maFont; }
};

class VCL_DLLPUBLIC MetaPushAction : public MetaAction
{
private:

    sal_uInt16              mnFlags;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Push, META_PUSH_ACTION )

                        MetaPushAction( sal_uInt16 nFlags );

    sal_uInt16              GetFlags() const { return mnFlags; }
};

class VCL_DLLPUBLIC MetaPopAction : public MetaAction
{
public:

        DECL_META_ACTION( Pop, META_POP_ACTION )
};

class VCL_DLLPUBLIC MetaRasterOpAction : public MetaAction
{
private:

    RasterOp            meRasterOp;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( RasterOp, META_RASTEROP_ACTION )

                        MetaRasterOpAction( RasterOp eRasterOp );

    RasterOp            GetRasterOp() const { return meRasterOp; }
};

class VCL_DLLPUBLIC MetaTransparentAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;
    sal_uInt16              mnTransPercent;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( Transparent, META_TRANSPARENT_ACTION )

                        MetaTransparentAction( const PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    sal_uInt16              GetTransparence() const { return mnTransPercent; }
};

class VCL_DLLPUBLIC MetaFloatTransparentAction : public MetaAction
{
private:

    GDIMetaFile         maMtf;
    Point               maPoint;
    Size                maSize;
    Gradient            maGradient;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( FloatTransparent, META_FLOATTRANSPARENT_ACTION )

                        MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                    const Size& rSize, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const GDIMetaFile&  GetGDIMetaFile() const { return maMtf; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaEPSAction : public MetaAction
{
private:

    GfxLink             maGfxLink;
    GDIMetaFile         maSubst;
    Point               maPoint;
    Size                maSize;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( EPS, META_EPS_ACTION )

                        MetaEPSAction( const Point& rPoint, const Size& rSize,
                                       const GfxLink& rGfxLink, const GDIMetaFile& rSubst );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    const GfxLink&      GetLink() const { return maGfxLink; }
    const GDIMetaFile&  GetSubstitute() const { return maSubst; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
};

class VCL_DLLPUBLIC MetaRefPointAction : public MetaAction
{
private:

    Point               maRefPoint;
    sal_Bool                mbSet;

    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( RefPoint, META_REFPOINT_ACTION )

                        MetaRefPointAction( const Point& rRefPoint, sal_Bool bSet );

    const Point&        GetRefPoint() const { return maRefPoint; }
    sal_Bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaCommentAction : public MetaAction
{
private:

    OString        maComment;
    sal_Int32           mnValue;
    sal_uInt32          mnDataSize;
    sal_uInt8*          mpData;

    SAL_DLLPRIVATE void ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize );
    virtual sal_Bool    Compare( const MetaAction& ) const;

protected:
                        ~MetaCommentAction();

public:
                        MetaCommentAction( sal_Int32 nValue = 0L );
                        MetaCommentAction( const MetaCommentAction& rAct );
                        MetaCommentAction( const OString& rComment, sal_Int32 nValue = 0L, const sal_uInt8* pData = NULL, sal_uInt32 nDataSize = 0UL );

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    virtual void        Execute( OutputDevice* pOut );
    virtual MetaAction* Clone();
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData );
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

    const OString& GetComment() const { return maComment; }
    sal_Int32           GetValue() const { return mnValue; }
    sal_uInt32          GetDataSize() const { return mnDataSize; }
    const sal_uInt8*        GetData() const { return mpData; }
};

class VCL_DLLPUBLIC MetaLayoutModeAction : public MetaAction
{
private:

    sal_uInt32          mnLayoutMode;
    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( LayoutMode, META_LAYOUTMODE_ACTION )

                        MetaLayoutModeAction( sal_uInt32 nLayoutMode );

    sal_uInt32          GetLayoutMode() const { return mnLayoutMode; }
};

class VCL_DLLPUBLIC MetaTextLanguageAction : public MetaAction
{
private:

    LanguageType        meTextLanguage;
    virtual sal_Bool    Compare( const MetaAction& ) const;

public:
                        DECL_META_ACTION( TextLanguage, META_TEXTLANGUAGE_ACTION )

                        MetaTextLanguageAction( LanguageType );

    LanguageType        GetTextLanguage() const { return meTextLanguage; }
};

#endif // _SV_METAACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
