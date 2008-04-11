/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: printergfx.hxx,v $
 * $Revision: 1.19 $
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

#ifndef _PSPRINT_PRINTERGFX_HXX_
#define _PSPRINT_PRINTERGFX_HXX_

#include <psprint/helper.hxx>
#include <osl/file.hxx>
#include <tools/gen.hxx>
#ifndef __SGI_STL_LIST
#include <list>
#endif
#include <hash_map>

namespace psp {

// forwards
class JobData;

/*
 * lightweight container to handle RGB values
 */

class PrinterColor
{
public:

    typedef enum    ColorSpace { eInvalid, eRGB };

private:

    sal_uInt8       mnRed;
    sal_uInt8       mnGreen;
    sal_uInt8       mnBlue;
    ColorSpace      meColorspace;

public:

    PrinterColor () :
            meColorspace(eInvalid)
    {}
    PrinterColor (sal_uInt16 nRed, sal_uInt16 nGreen,
                  sal_uInt16 nBlue) :
            mnRed   (nRed),
            mnGreen (nGreen),
            mnBlue  (nBlue),
            meColorspace (eRGB)
    {}
    PrinterColor (sal_uInt32 nRGB) :
            mnRed   ((nRGB & 0x00ff0000) >> 16),
            mnGreen ((nRGB & 0x0000ff00) >>  8),
            mnBlue  ((nRGB & 0x000000ff)      ),
            meColorspace (eRGB)
    {}
    ~PrinterColor ()
    {}

    sal_Bool        Is () const
    { return meColorspace != eInvalid; }

    ColorSpace      GetColorSpace () const
    { return meColorspace; }
    sal_uInt16      GetRed () const
    { return mnRed; }
    sal_uInt16      GetGreen () const
    { return mnGreen; }
    sal_uInt16      GetBlue () const
    { return mnBlue; }
    sal_Bool        operator== (const PrinterColor& aColor) const
    {
        return aColor.Is() && this->Is()
            && mnRed   == aColor.mnRed
            && mnGreen == aColor.mnGreen
            && mnBlue  == aColor.mnBlue;
    }
    sal_Bool        operator!= (const PrinterColor& aColor) const
    { return ! (aColor==*this); }
    PrinterColor&   operator= (const PrinterColor& aColor)
    {
        meColorspace = aColor.meColorspace;
        mnRed   = aColor.mnRed;
        mnGreen = aColor.mnGreen;
        mnBlue  = aColor.mnBlue;

        return *this;
    }

    PrinterColor&   operator= (sal_uInt32 nRGB)
    {
        meColorspace = eRGB;
        mnBlue  = (nRGB & 0x000000ff);
        mnGreen = (nRGB & 0x0000ff00) >>  8;
        mnRed   = (nRGB & 0x00ff0000) >> 16;

        return *this;
    }
};

/*
 * forward declarations
 */

class Font3;
class GlyphSet;
class PrinterJob;
class PrintFontManager;
class KernPair;
struct CharacterMetric;

/*
 * Bitmap Interface, this has to be filled with your actual bitmap implementation
 * sample implementations can be found in:
 *      psprint/workben/cui/pspdem.cxx
 *      vcl/unx/source/gdi/salgdi2.cxx
 */

class PrinterBmp
{
public:

    virtual             ~PrinterBmp ()  = 0;
    virtual sal_uInt32  GetPaletteColor (sal_uInt32 nIdx) const = 0;
    virtual sal_uInt32  GetPaletteEntryCount ()           const = 0;
    virtual sal_uInt32  GetPixelRGB  (sal_uInt32 nRow, sal_uInt32 nColumn) const = 0;
    virtual sal_uInt8   GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const = 0;
    virtual sal_uInt8   GetPixelIdx  (sal_uInt32 nRow, sal_uInt32 nColumn) const = 0;
    virtual sal_uInt32  GetWidth ()     const = 0;
    virtual sal_uInt32  GetHeight ()    const = 0;
    virtual sal_uInt32  GetDepth ()     const = 0;
};

typedef enum {
    InvalidType = 0,
    TrueColorImage,
    MonochromeImage,
    PaletteImage,
    GrayScaleImage
} ImageType;

/*
 * printer raster operations
 */

struct GraphicsStatus
{
    rtl::OString        maFont;
    rtl_TextEncoding    maEncoding;
    bool                mbArtItalic;
    bool                mbArtBold;
    sal_Int32           mnTextHeight;
    sal_Int32           mnTextWidth;
    PrinterColor        maColor;
    double             mfLineWidth;

    GraphicsStatus();
};

class Font3;

class PrinterGfx
{
private:

    /* common settings */

    double          mfScaleX;
    double          mfScaleY;

    sal_uInt32      mnDpi;
    sal_uInt16      mnDepth;

    sal_uInt16      mnPSLevel;
    sal_Bool        mbColor;
    sal_Bool        mbUploadPS42Fonts;

    osl::File*      mpPageHeader;
    osl::File*      mpPageBody;

    void            TranslateCoordinates (sal_Int32 &rXOut, sal_Int32 &rYOut,
                                          sal_Int32 nXIn, sal_Int32 nYIn )
    { rXOut = nXIn; rYOut = nYIn; }
    void            TranslateCoordinates (Point& rOut, const Point& rIn)
    { rOut = rIn; }

    /* text/font related data, for a type1 font it has to be checked
       whether this font has already been downloaded. A TrueType font
       will be converted into one or more Type3 fonts, containing glyphs
       in no particular order. In addition to the existence of the
       glyph in one of the subfonts, the mapping from unicode to the
       glyph has to be remembered */

    std::list< sal_Int32 > maPS1Font;
    std::list< GlyphSet > maPS3Font;

    sal_Int32       mnFontID;
    sal_Int32       mnFallbackID;
    sal_Int32       mnTextAngle;
    bool           mbTextVertical;
    PrintFontManager& mrFontMgr;

    /* bitmap drawing implementation */

    sal_Bool    mbCompressBmp;

    void    DrawPS1GrayImage      (const PrinterBmp& rBitmap, const Rectangle& rArea);
    void    writePS2ImageHeader   (const Rectangle& rArea, psp::ImageType nType);
    void    writePS2Colorspace    (const PrinterBmp& rBitmap, psp::ImageType nType);
    void    DrawPS2GrayImage      (const PrinterBmp& rBitmap, const Rectangle& rArea);
    void    DrawPS2PaletteImage   (const PrinterBmp& rBitmap, const Rectangle& rArea);
    void    DrawPS2TrueColorImage (const PrinterBmp& rBitmap, const Rectangle& rArea);
    void    DrawPS2MonoImage      (const PrinterBmp& rBitmap, const Rectangle& rArea);

    /* clip region */

    std::list< Rectangle > maClipRegion;
    sal_Bool JoinVerticalClipRectangles( std::list< Rectangle >::iterator& it,
                                         Point& aOldPoint, sal_Int32& nColumn );

    /* color settings */
    PrinterColor    maFillColor;
    PrinterColor    maTextColor;
    PrinterColor    maLineColor;

    /* graphics state */
    GraphicsStatus                  maVirtualStatus;
    std::list< GraphicsStatus >     maGraphicsStack;
    GraphicsStatus& currentState() { return maGraphicsStack.front(); }

    /* font / font substitution */
    friend class Font3;
    const ::std::hash_map< fontID, fontID >*    mpFontSubstitutes;
    int             getCharWidth (sal_Bool b_vert, sal_Unicode n_char,
                                  CharacterMetric *p_bbox);
    fontID          getCharMetric (const Font3 &rFont, sal_Unicode n_char,
                                   CharacterMetric *p_bbox);
    fontID          getFontSubstitute () const;
    fontID          getFallbackID () const { return mnFallbackID; }

    bool            mbStrictSO52Compatibility;
public:
    /* grahics status update */
    void            PSSetColor ();
    void            PSSetLineWidth ();
    void            PSSetFont ();

    /* graphics status functions */
    void            PSSetColor (const PrinterColor& rColor)
    { maVirtualStatus.maColor = rColor; }

    void            PSUploadPS1Font (sal_Int32 nFontID);
    void            PSSetFont (const rtl::OString& rName,
                               rtl_TextEncoding nEncoding = RTL_TEXTENCODING_DONTKNOW)
    { maVirtualStatus.maFont = rName; maVirtualStatus.maEncoding = nEncoding; }

    /* graphics status stack */
    void            PSGSave ();
    void            PSGRestore ();


    /* PS helpers */
    enum pspath_t { moveto = 0, lineto = 1 };
    void            PSBinLineTo (const Point& rCurrent, Point& rOld,
                                 sal_Int32& nColumn);
    void            PSBinMoveTo (const Point& rCurrent, Point& rOld,
                                 sal_Int32& nColumn);
    void            PSBinStartPath ();
    void            PSBinEndPath ();
    void            PSBinCurrentPath (sal_uInt32 nPoints, const Point* pPath);
    void            PSBinPath (const Point& rCurrent, Point& rOld,
                               pspath_t eType, sal_Int32& nColumn);

    void            PSRotate (sal_Int32 nAngle);
    void            PSTranslate (const Point& rPoint);
    void            PSMoveTo (const Point& rPoint);
    void            PSRMoveTo (sal_Int32 nDx, sal_Int32 nDy = 0);
    void            PSScale (double fScaleX, double fScaleY);
    void            PSLineTo(const Point& rPoint );
    void            PSPointOp (const Point& rPoint, const sal_Char* pOperator);
    void            PSHexString (const sal_uChar* pString, sal_Int16 nLen);
    void            PSDeltaArray (const sal_Int32 *pArray, sal_Int16 nEntries);
    void            PSShowText (const sal_uChar* pString,
                                sal_Int16 nGlyphs, sal_Int16 nBytes,
                                const sal_Int32* pDeltaArray = NULL);
    void            PSComment (const sal_Char* pComment );
    void            LicenseWarning (const Point& rPoint, const sal_Unicode* pStr,
                                    sal_Int16 nLen, const sal_Int32* pDeltaArray);

    void            OnEndPage ();
    void            OnEndJob ();
    void            writeResources( osl::File* pFile, std::list< rtl::OString >& rSuppliedFonts, std::list< rtl::OString >& rNeededFonts );
    PrintFontManager& GetFontMgr () { return mrFontMgr; }

    void            drawVerticalizedText (const Point& rPoint,
                                          const sal_Unicode* pStr,
                                          sal_Int16 nLen,
                                          const sal_Int32* pDeltaArray );
    void            drawText (const Point& rPoint,
                              const sal_Unicode* pStr, sal_Int16 nLen,
                              const sal_Int32* pDeltaArray = NULL);

    void            drawGlyphs( const Point& rPoint,
                                sal_uInt32* pGlyphIds,
                                sal_Unicode* pUnicodes,
                                sal_Int16 nLen,
                                sal_Int32* pDeltaArray );
public:
    PrinterGfx();
    ~PrinterGfx();
    sal_Bool        Init (PrinterJob &rPrinterSpec);
    sal_Bool        Init (const JobData& rData);
    void            Clear();

    // query depth and size
    void            GetResolution (sal_Int32 &rDpiX, sal_Int32 &rDpiY) const;
    void            GetScreenFontResolution (sal_Int32 &rDpiX, sal_Int32 &rDpiY) const;
    sal_uInt16      GetBitCount ();

    // clip region
    void            ResetClipRegion ();
    void            BeginSetClipRegion (sal_uInt32);
    sal_Bool        UnionClipRegion (sal_Int32 nX, sal_Int32 nY,
                                     sal_Int32 nDX, sal_Int32 nDY);
    void            EndSetClipRegion ();

    // set xy color
    void            SetLineColor (const PrinterColor& rLineColor = PrinterColor())
    { maLineColor = rLineColor; }
    void            SetFillColor (const PrinterColor& rFillColor = PrinterColor())
    { maFillColor = rFillColor; }

    // drawing primitives
    void            DrawPixel (const Point& rPoint, const PrinterColor& rPixelColor);
    void            DrawPixel (const Point& rPoint)
    { DrawPixel (rPoint, maLineColor); }
    void            DrawLine  (const Point& rFrom, const Point& rTo);
    void            DrawRect  (const Rectangle& rRectangle);
    void            DrawPolyLine (sal_uInt32 nPoints, const Point* pPath );
    void            DrawPolygon  (sal_uInt32 nPoints, const Point* pPath);
    void            DrawPolyPolygon (sal_uInt32 nPoly,
                                     const sal_uInt32 *pPolygonSize,
                                     const Point** pPolygonList);
    void            DrawPolyLineBezier (sal_uInt32 nPoints,
                                     const Point* pPath,
                                     const BYTE* pFlgAry );
    void            DrawPolygonBezier  (sal_uInt32 nPoints,
                                     const Point* pPath,
                                     const BYTE* pFlgAry);
    void            DrawPolyPolygonBezier  (sal_uInt32 nPoly,
                                     const sal_uInt32* pPoints,
                                     const Point* const* pPtAry,
                                     const BYTE* const* pFlgAry);

    // eps
    sal_Bool        DrawEPS ( const Rectangle& rBoundingBox, void* pPtr, sal_uInt32 nSize);

    // image drawing
    void            DrawBitmap (const Rectangle& rDest, const Rectangle& rSrc,
                                const PrinterBmp& rBitmap);
    void            DrawBitmap (const Rectangle& rDest, const Rectangle& rSrc,
                                const PrinterBmp& rBitmap,
                                const PrinterBmp& rTransBitmap);
    void            DrawMask   (const Rectangle& rDest, const Rectangle& rSrc,
                                const PrinterBmp &rBitmap, PrinterColor& rMaskColor);

    // font and text handling
    sal_uInt16      SetFont (
                             sal_Int32 nFontID,
                             sal_Int32 nPointHeight,
                             sal_Int32 nPointWidth,
                             sal_Int32 nAngle,
                             bool bVertical,
                             bool bArtItalic,
                             bool bArtBold
                             );
    sal_uInt16      SetFallbackFont ( sal_Int32 nFontID );
    sal_Int32       GetFontAngle () const
    { return mnTextAngle; }
    sal_Int32       GetFontID () const
    { return mnFontID; }
    bool            GetFontVertical() const
    { return mbTextVertical; }
    sal_Int32       GetFontHeight () const
    { return maVirtualStatus.mnTextHeight; }
    sal_Int32       GetFontWidth () const
    { return maVirtualStatus.mnTextWidth; }
    bool            GetArtificialItalic() const
    { return maVirtualStatus.mbArtItalic; }
    bool            GetArtificialBold() const
    { return maVirtualStatus.mbArtBold; }
    void            DrawText (const Point& rPoint,
                              const sal_Unicode* pStr, sal_Int16 nLen,
                              const sal_Int32* pDeltaArray = NULL);
    void            SetTextColor (PrinterColor& rTextColor)
    { maTextColor = rTextColor; }
    sal_Int32       GetCharWidth (sal_uInt16 nFrom, sal_uInt16 nTo,
                                  long *pWidthArray);
    const ::std::list< KernPair >& getKernPairs( bool bVertical = false ) const;
    // advanced font handling
    sal_Bool        GetGlyphBoundRect (sal_Unicode c, Rectangle& rOutRect);
    sal_uInt32      GetGlyphOutline (sal_Unicode c,
                                     sal_uInt16 **ppPolySizes, Point **ppPoints,
                                     sal_uInt8 **ppFlags);

    // for CTL
    void            DrawGlyphs( const Point& rPoint,
                                sal_uInt32* pGlyphIds,
                                sal_Unicode* pUnicodes,
                                sal_Int16 nLen,
                                sal_Int32* pDeltaArray );

    bool getStrictSO52Compatibility() const;
    void setStrictSO52Compatibility( bool );
};

} /* namespace psp */


#endif /* _PSPRINT_PRINTERGFX_HXX_ */

