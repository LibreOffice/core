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

#ifndef _SWF_WRITER_HXX_
#define _SWF_WRITER_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <vcl/font.hxx>
#include <vcl/gradient.hxx>
#include <unotools/tempfile.hxx>
#include <tools/color.hxx>
#include <tools/poly.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

// #i73264#
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <osl/file.hxx>

#include <vector>
#include <stack>
#include <map>

#ifdef AUGUSTUS
#include "lame.h"
#include "sndfile.h"
#endif

#include <stdio.h>

class GDIMetaFile;
class BitmapEx;
class PolyPolygon;
class Gradient;
class SvtGraphicFill;
class SvtGraphicStroke;
class LineInfo;
namespace basegfx { class B2DPolygon; }

inline sal_uInt16 _uInt16( sal_Int32 nValue )
{
    OSL_ENSURE( (nValue >= 0) && ((sal_uInt32)nValue <= 0xffff), "overflow while converting sal_Int32 to sal_uInt16" );
    return (sal_uInt16)nValue;
}

inline sal_Int16 _Int16( sal_Int32 nValue )
{
    OSL_ENSURE( (nValue >= -32768) && (nValue <= 32767), "overflow while converting sal_Int32 to sal_Int16" );
    return (sal_Int16)nValue;
}

class VirtualDevice;

namespace swf {

const sal_uInt8 TAG_END             = 0;
const sal_uInt8 TAG_SHOWFRAME       = 1;

const sal_uInt8 TAG_DEFINEBUTTON    = 7;

const sal_uInt8 TAG_BACKGROUNDCOLOR = 9;

const sal_uInt8 TAG_DOACTION        = 12;
const sal_uInt8 TAG_STARTSOUND      = 15;

const sal_uInt8 TAG_SOUNDSTREAMBLOCK = 19;
const sal_uInt8 TAG_SOUNDSTREAMHEAD = 18;
const sal_uInt8 TAG_SOUNDSTREAMHEAD2 = 45;

const sal_uInt8 TAG_JPEGTABLES      = 8;
const sal_uInt8 TAG_DEFINEBITS      = 6;
const sal_uInt8 TAG_DEFINEBITSLOSSLESS = 20;
const sal_uInt8 TAG_DEFINEBITSJPEG2 = 21;
const sal_uInt8 TAG_DEFINEBITSJPEG3 = 35;
const sal_uInt8 TAG_DEFINEBITSLOSSLESS2 = 36;
const sal_uInt8 TAG_DEFINEEDITTEXT= 37;
const sal_uInt8 TAG_PLACEOBJECT     = 4;
const sal_uInt8 TAG_PLACEOBJECT2    = 26;
const sal_uInt8 TAG_REMOVEOBJECT2   = 28;

const sal_uInt8 TAG_DEFINEFONT      = 10;
const sal_uInt8 TAG_DEFINETEXT      = 11;
const sal_uInt8 TAG_DEFINESHAPE3    = 32;
const sal_uInt8 TAG_DEFINESPRITE    = 39;

const sal_uInt8 TAG_FRAMELABEL      = 43;

const sal_uInt8 TAG_HEADER          = 0xff;

///////////////////////////////////////////////////////////////////////

/** converts a double to a 16.16 flash fixed value */
sal_uInt32 getFixed( double fValue );

///////////////////////////////////////////////////////////////////////

typedef ::std::map<sal_uInt32, sal_uInt16> ChecksumCache;

/** unsigned int 16 compare operation for stl */
struct ltuint16
{
  bool operator()(sal_uInt16 s1, sal_uInt16 s2) const
  {
    return s1 < s2;
  }
};

///////////////////////////////////////////////////////////////////////

/** container class to create bit structures */
class BitStream
{
public:
    BitStream();

    void writeUB( sal_uInt32 nValue, sal_uInt16 nBits );
    void writeSB( sal_Int32 nValue, sal_uInt16 nBits );
    void writeFB( sal_uInt32 nValue, sal_uInt16 nBits );

    void pad();
    void writeTo( SvStream& out );

    sal_uInt32 getOffset() const;
private:

    std::vector< sal_uInt8 > maData;
    sal_uInt8 mnBitPos;
    sal_uInt8 mnCurrentByte;
};

///////////////////////////////////////////////////////////////////////

/** this class collects all used glyphs for a given fonts and maps
    characters to glyph ids.
*/
class FlashFont
{
public:
    FlashFont( const Font& rFont, sal_uInt16 nId );
    ~FlashFont();

    sal_uInt16 getGlyph( sal_uInt16 nChar, VirtualDevice* pVDev );

    void write( SvStream& out );

    sal_uInt16 getID() const { return mnId; }
    const Font& getFont() { return maFont; }

private:
    const Font  maFont;
    std::map<sal_uInt16, sal_uInt16, ltuint16> maGlyphIndex;
    sal_uInt16 mnNextIndex;
    sal_uInt16 mnId;
    BitStream maGlyphData;
    std::vector< sal_uInt16 > maGlyphOffsets;
};

typedef std::vector<FlashFont*> FontMap;

///////////////////////////////////////////////////////////////////////

/** this class helps creating flash tags */
class Tag : public SvMemoryStream
{
public:
    Tag( sal_uInt8 nTagId );

    sal_uInt8 getTagId() const { return mnTagId; }

    void write( SvStream& out );

    void addUI32( sal_uInt32 nValue );
    void addUI16( sal_uInt16 nValue );
    void addUI8( sal_uInt8 nValue );
    void addBits( BitStream& rIn );

    void addRGBA( const Color& rColor );
    void addRGB( const Color& rColor );
    void addRect( const Rectangle& rRect );
    void addMatrix( const ::basegfx::B2DHomMatrix& rMatrix ); // #i73264#
    void addString( const char* pString );
    void addStream( SvStream& rIn );

    static void writeMatrix( SvStream& rOut, const ::basegfx::B2DHomMatrix& rMatrix ); // #i73264#
    static void writeRect( SvStream& rOut, const Rectangle& rRect );

private:
    sal_uInt8 mnTagId;
};

///////////////////////////////////////////////////////////////////////

/** this class helps to define flash sprites */
class Sprite
{
public:
    Sprite( sal_uInt16 nId );
    ~Sprite();

    void write( SvStream& out );

    sal_uInt16 getId() const { return mnId; }

    void addTag( Tag* pNewTag );

private:
    std::vector< Tag* > maTags;
    sal_uInt16  mnId;
    sal_uInt32  mnFrames;
};

///////////////////////////////////////////////////////////////////////

/** this class stores a flash fill style for shapes */
class FillStyle
{
public:
    enum FillStyleType { solid = 0x00, linear_gradient = 0x10, radial_gradient = 0x12, tiled_bitmap = 0x40, clipped_bitmap = 0x41 };

    /** this c'tor creates a solid fill style */
    FillStyle( const Color& rSolidColor );

    /** this c'tor creates a linear or radial gradient fill style */
    FillStyle( const Rectangle& rBoundRect, const Gradient& rGradient );

    /** this c'tor creates a tiled or clipped bitmap fill style */
    FillStyle( sal_uInt16 nBitmapId, bool bClipped, const ::basegfx::B2DHomMatrix& rMatrix ); // #i73264#

    void addTo( Tag* pTag ) const;

private:
    void Impl_addGradient( Tag* pTag ) const;

    FillStyleType   meType;
    ::basegfx::B2DHomMatrix     maMatrix; // #i73264#
    sal_uInt16      mnBitmapId;
    Color           maColor;
    Gradient        maGradient;
    Rectangle       maBoundRect;
};

///////////////////////////////////////////////////////////////////////

/** this class creates a flash movie from vcl geometry */
class Writer
{
    friend class FlashFont;

public:
    /** creates a writer for a new flash movie.
        nDocWidth and nDocHeight are the dimensions of the movie.
        They must be in 100th/mm.

        An invisible shape with the size of the document is placed at depth 1
        and it clips all shapes on depth 2 and 3.
    */
    Writer( sal_Int32 nDocWidthInput, sal_Int32 nDocHeightInput, sal_Int32 nDocWidth, sal_Int32 nDocHeight, sal_Int32 nJPEGcompressMode = -1 );
    ~Writer();

    void storeTo( com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutStream );

    // geometry
    void setClipping( const PolyPolygon* pClipPolyPolygon );

    /** defines a flash shape from a filled polygon.
        The coordinates must be in twips */
    sal_uInt16 defineShape( const Polygon& rPoly, const FillStyle& rFillStyle );

    /** defines a flash shape from a filled polypolygon.
        The coordinates must be in twips */
    sal_uInt16 defineShape( const PolyPolygon& rPolyPoly, const FillStyle& rFillStyle );

    /** defines a flash shape from a outlined polypolygon.
        The coordinates must be in twips */
    sal_uInt16 defineShape( const PolyPolygon& rPolyPoly, sal_uInt16 nLineWidth, const Color& rLineColor );

    /** defines a flash shape from a vcl metafile.
        The mapmode of the metafile is used to map all coordinates to twips.
        A character id of a flash sprite is returned that contains all geometry
        from the metafile.
    */
    sal_uInt16 defineShape( const GDIMetaFile& rMtf, sal_Int16 x = 0, sal_Int16 y = 0 );

    /** defines a bitmap and returns its flash id.
    */
    sal_uInt16 defineBitmap( const BitmapEx& bmpSource, sal_Int32 nJPEGQualityLevel  );

    // control tags

    /** inserts a place shape tag into the movie stream or the current sprite */
    void placeShape( sal_uInt16 nID, sal_uInt16 nDepth, sal_Int32 x, sal_Int32 y, sal_uInt16 nClipDepth = 0, const char* pName = NULL );

#ifdef THEFUTURE
    /** inserts a move shape tag into the movie stream or the current sprite */
    void moveShape( sal_uInt16 nDepth, sal_Int32 x, sal_Int32 y );
#endif

    /** inserts a remove shape tag into the movie stream or the current sprite */
    void removeShape( sal_uInt16 nDepth );

    /** inserts a show frame tag into the movie stream or the current sprite */
    void showFrame();

    /** creates a new sprite and sets it as the current sprite for editing.
        Only one sprite can be edited at one time */
    sal_uInt16 startSprite();

    /** ends editing of the curent sprites and adds it to the movie stream */
    void endSprite();

    /** inserts a doaction tag with an ActionStop */
    void stop();

    /** inserts a doaction tag with an ActionStop, place a button on depth nDepth that
        continues playback on click */
    void waitOnClick( sal_uInt16 nDepth );

    /** inserts a doaction tag with an ActionGotoFrame */
    void gotoFrame( sal_uInt16 nFrame );

#ifdef AUGUSTUS
    /** stream out a sound.  Should make it more intelligent so it interleaves with other items.*/
    sal_Bool streamSound( const char * filename );
#endif

private:
    Point                   map( const Point& rPoint ) const;
    Size                    map( const Size& rSize ) const;
    void                    map( PolyPolygon& rPolyPolygon ) const;
    sal_Int32               mapRelative( sal_Int32 n100thMM ) const;

    void startTag( sal_uInt8 nTagId );
    void endTag();
    sal_uInt16 createID();

    void Impl_writeBmp( sal_uInt16 nBitmapId, sal_uInt32 width, sal_uInt32 height, sal_uInt8 *pCompressed, sal_uInt32 compressed_size );
    void Impl_writeImage( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& rSrcPt, const Size& rSrcSz, const Rectangle& rClipRect, bool bMap );
    void Impl_writeJPEG(sal_uInt16 nBitmapId, const sal_uInt8* pJpgData, sal_uInt32 nJpgDataLength, sal_uInt8 *pCompressed, sal_uInt32 compressed_size );
    void Impl_handleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon);
    void Impl_writeActions( const GDIMetaFile& rMtf );
    void Impl_writePolygon( const Polygon& rPoly, sal_Bool bFilled );
    void Impl_writePolygon( const Polygon& rPoly, sal_Bool bFilled, const Color& rFillColor, const Color& rLineColor );
    void Impl_writePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bFilled, sal_uInt8 nTransparence = 0);
    void Impl_writePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bFilled, const Color& rFillColor, const Color& rLineColor );
    void Impl_writeText( const Point& rPos, const String& rText, const sal_Int32* pDXArray, long nWidth );
    void Impl_writeText( const Point& rPos, const String& rText, const sal_Int32* pDXArray, long nWidth, Color aTextColor );
    void Impl_writeGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void Impl_writeLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor = NULL );
    void Impl_writeRect( const Rectangle& rRect, long nRadX, long nRadY );
    void Impl_writeEllipse( const Point& rCenter, long nRadX, long nRadY );
    bool Impl_writeFilling( SvtGraphicFill& rFilling );
    bool Impl_writeStroke( SvtGraphicStroke& rStroke );

    FlashFont& Impl_getFont( const Font& rFont );

    static void Impl_addPolygon( BitStream& rBits, const Polygon& rPoly, sal_Bool bFilled );

    static void Impl_addShapeRecordChange( BitStream& rBits, sal_Int16 dx, sal_Int16 dy, sal_Bool bFilled );
    static void Impl_addStraightEdgeRecord( BitStream& rBits, sal_Int16 dx, sal_Int16 dy );
    static void Impl_addCurvedEdgeRecord( BitStream& rBits, sal_Int16 control_dx, sal_Int16 control_dy, sal_Int16 anchor_dx, sal_Int16 anchor_dy );
    static void Impl_addEndShapeRecord( BitStream& rBits );

    static void Impl_addStraightLine( BitStream& rBits,
                                  Point& rLastPoint,
                                  const double P2x, const double P2y );
    static void Impl_addQuadBezier( BitStream& rBits,
                                Point& rLastPoint,
                                const double P2x, const double P2y,
                                const double P3x, const double P3y );
    static void Impl_quadBezierApprox( BitStream& rBits,
                                   Point& rLastPoint,
                                   const double d2,
                                   const double P1x, const double P1y,
                                   const double P2x, const double P2y,
                                   const double P3x, const double P3y,
                                   const double P4x, const double P4y );

    com::sun::star::uno::Reference < com::sun::star::i18n::XBreakIterator > Impl_GetBreakIterator();

private:
    com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > mxBreakIterator;

    FontMap                 maFonts;

    sal_Int32 mnDocWidth;
    sal_Int32 mnDocHeight;

    // AS: Scaling factor for output.
    double mnDocXScale;
    double mnDocYScale;

    sal_uInt16 mnWhiteBackgroundShapeId;
    sal_uInt16 mnPageButtonId;

    VirtualDevice*  mpVDev;

    const PolyPolygon* mpClipPolyPolygon;

    /** holds the informations of the objects defined in the movie stream
        while executing defineShape
    */
    typedef std::vector<sal_uInt16> CharacterIdVector;
    CharacterIdVector       maShapeIds;

    Tag* mpTag;
    Sprite* mpSprite;
    std::stack<Sprite*> mvSpriteStack;
    ChecksumCache mBitmapCache;

    sal_uInt16 mnNextId;
    sal_uInt32  mnFrames;

//  com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > mxOutStream;
    oslFileHandle mxOutStream;

    utl::TempFile maMovieTempFile;
    utl::TempFile maFontsTempFile;

    SvStream* mpMovieStream;
    SvStream* mpFontsStream;

#ifdef AUGUSTUS
    lame_global_flags *m_lame_flags;
#endif

    sal_uInt8 mnGlobalTransparency;
    sal_Int32 mnJPEGCompressMode;
};

///////////////////////////////////////////////////////////////////////

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
