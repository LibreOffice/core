/*************************************************************************
 *
 *  $RCSfile: pdfwriter_impl.hxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:29:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _VCL_PDFWRITER_IMPL_HXX
#define _VCL_PDFWRITER_IMPL_HXX

#ifndef _VCL_PDFWRITER_HXX
#include <pdfwriter.hxx>
#endif

#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_FILE_H
#include <osl/file.h>
#endif
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _SV_HATCH_HXX
#include <hatch.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <wall.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _RTL_STRBUF_HXX
#include <rtl/strbuf.hxx>
#endif

#include <vector>
#include <map>
#include <list>

class SalLayout;
class ImplLayoutArgs;
class ImplFontData;
class ImplFontSelectData;
class ImplFontMetricData;
struct FontSubsetInfo;
class ZCodec;
class SvMemoryStream;

namespace vcl
{

class PDFSalLayout;

class PDFWriterImpl
{
    friend class PDFSalLayout;
public:
    // definition of structs
    struct BuiltinFont
    {
        const char *                m_pName;                     // Name
        const char *                m_pStyleName;                // StyleName
        const char *                m_pPSName;                   // PSName
        int                         m_nAscent;
        int                         m_nDescent;
        FontFamily                  m_eFamily;                   // Family
        CharSet                     m_eCharSet;                  // CharSet
        FontPitch                   m_ePitch;                    // Pitch
        FontWidth                   m_eWidthType;                // WidthType
        FontWeight                  m_eWeight;                   // Weight
        FontItalic                  m_eItalic;                   // Italic
        int                         m_aWidths[256];              // character metrics
    };

    struct PDFPage
    {
        PDFWriterImpl*              m_pWriter;
        sal_Int32                   m_nPageWidth;           // in inch/72
        sal_Int32                   m_nPageHeight;          // in inch/72
        PDFWriter::Orientation      m_eOrientation;
        sal_Int32                   m_nPageObject;
        sal_Int32                   m_nStreamObject;
        sal_Int32                   m_nStreamLengthObject;
        sal_uInt64                  m_nBeginStreamPos;

        PDFPage( PDFWriterImpl* pWriter, sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation );
        ~PDFPage();

        void beginStream();
        void endStream();
        bool emit( sal_Int32 nParentPage );

        // converts point from ref device coordinates to
        // page coordinates and appends the point to the buffer
        // if bNeg is true, the coordinates are inverted AFTER transformation
        // to page (useful for transformation matrices
        // if pOutPoint is set it will be updated to the emitted point
        // (in PDF map mode, that is 10th of point)
        void appendPoint( const Point& rPoint, rtl::OStringBuffer& rBuffer, bool bNeg = false, Point* pOutPoint = NULL );
        // appends a rectangle
        void appendRect( const Rectangle& rRect, rtl::OStringBuffer& rBuffer );
        // converts a rectangle to 10th points page space
        void convertRect( Rectangle& rRect );
        // appends a polygon optionally closing it
        void appendPolygon( const Polygon& rPoly, rtl::OStringBuffer& rBuffer, bool bClose = true );
        // appends a polypolygon optionally closing the subpaths
        void appendPolyPolygon( const PolyPolygon& rPolyPoly, rtl::OStringBuffer& rBuffer, bool bClose = true );
        // converts a length (either vertical or horizontal; this
        // can be important if the source MapMode is not
        // symmetrical) to page length and appends it to the buffer
        // if pOutLength is set it will be updated to the emitted length
        // (in PDF map mode, that is 10th of point)
        void appendMappedLength( sal_Int32 nLength, rtl::OStringBuffer& rBuffer, bool bVertical = true, sal_Int32* pOutLength = NULL );
        // the same for double values
        void appendMappedLength( double fLength, rtl::OStringBuffer& rBuffer, bool bVertical = true, sal_Int32* pOutLength = NULL );
        // appends LineInfo
        void appendLineInfo( const LineInfo& rInfo, rtl::OStringBuffer& rBuffer );
        // appends a horizontal waveline with vertical offset (helper for drawWaveLine)
        void appendWaveLine( sal_Int32 nLength, sal_Int32 nYOffset, sal_Int32 nDelta, rtl::OStringBuffer& rBuffer );
    };

    friend struct PDFPage;

    struct BitmapID
    {
        Size        m_aPixelSize;
        sal_Int32   m_nSize;
        sal_Int32   m_nChecksum;
        sal_Int32   m_nMaskChecksum;

        BitmapID() : m_nSize( 0 ), m_nChecksum( 0 ), m_nMaskChecksum( 0 ) {}

        BitmapID& operator=( const BitmapID& rCopy )
        {
            m_aPixelSize    = rCopy.m_aPixelSize;
            m_nSize         = rCopy.m_nSize;
            m_nChecksum     = rCopy.m_nChecksum;
            m_nMaskChecksum = rCopy.m_nMaskChecksum;
            return *this;
        }

        bool operator==( const BitmapID& rComp )
        {
            return (m_aPixelSize == rComp.m_aPixelSize &&
                    m_nSize == rComp.m_nSize &&
                    m_nChecksum == rComp.m_nChecksum &&
                    m_nMaskChecksum == rComp.m_nMaskChecksum );
        }
    };

    struct BitmapEmit
    {
        BitmapID    m_aID;
        BitmapEx    m_aBitmap;
        sal_Int32   m_nObject;
        bool        m_bDrawMask;

        BitmapEmit() : m_bDrawMask( false ) {}
    };

    struct JPGEmit
    {
        BitmapID            m_aID;
        SvMemoryStream*     m_pStream;
        Bitmap              m_aMask;
        sal_Int32           m_nObject;

        JPGEmit() : m_pStream( NULL ) {}
        ~JPGEmit() { delete m_pStream; }
    };

    struct GradientEmit
    {
        Gradient    m_aGradient;
        Size        m_aSize;
        sal_Int32   m_nObject;
    };

    // for bitmap tilings (drawWallpaper)
    struct BitmapPatternEmit
    {
        sal_Int32   m_nObject;
        sal_Int32   m_nBitmapObject;
        Rectangle   m_aRectangle;
    };

    // for transparency group XObjects
    struct TransparencyEmit
    {
        sal_Int32           m_nObject;
        double              m_fAlpha;
        Rectangle           m_aBoundRect;
        rtl::OStringBuffer  m_aContentStream;
    };

    // font subsets
    struct GlyphEmit
    {
        sal_uInt8       m_nSubsetGlyphID;
        sal_Unicode     m_aUnicode;
    };
    typedef std::map< long, GlyphEmit > FontEmitMapping;
    struct FontEmit
    {
        sal_Int32           m_nFontID;
        FontEmitMapping     m_aMapping;

        FontEmit( sal_Int32 nID ) : m_nFontID( nID ) {}
    };
    typedef std::list< FontEmit > FontEmitList;
    struct Glyph
    {
        sal_Int32   m_nFontID;
        sal_uInt8   m_nSubsetGlyphID;
    };
    typedef std::map< long, Glyph > FontMapping;

    struct FontSubset
    {
        FontEmitList        m_aSubsets;
        FontMapping         m_aMapping;
    };
    typedef std::map< ImplFontData*, FontSubset > FontSubsetData;

    struct EmbedCode
    {
        sal_Unicode         m_aUnicode;
        rtl::OString        m_aName;
    };
    struct EmbedEncoding
    {
        sal_Int32                               m_nFontID;
        std::vector< EmbedCode >                m_aEncVector;
        std::map< sal_Unicode, sal_Int8 >       m_aCMap;
    };
    struct EmbedFont
    {
        sal_Int32                       m_nNormalFontID;
        std::list< EmbedEncoding >      m_aExtendedEncodings;
    };
    typedef std::map< ImplFontData*, EmbedFont > FontEmbedData;
private:
    static const BuiltinFont m_aBuiltinFonts[14];

    OutputDevice*                   m_pReferenceDevice;

    MapMode                         m_aMapMode; // PDFWriterImpl scaled units
    std::list< PDFPage >            m_aPages;
    PDFDocInfo                      m_aDocInfo;
    /* maps object numbers to file offsets (needed for xref) */
    std::vector< sal_uInt64 >       m_aObjects;
    /* contains Bitmaps until they are written to the
     *  file stream as XObjects*/
    std::list< BitmapEmit >         m_aBitmaps;
    /* contains JPG streams until written to file
     */
    std::list<JPGEmit>              m_aJPGs;

    /* contains Bitmaps for gradient functions until they are written
     *  to the file stream */
    std::list< GradientEmit >       m_aGradients;
    /* contains bitmap tiling patterns */
    std::list< BitmapPatternEmit >  m_aTilings;
    std::list< TransparencyEmit >   m_aTransparentObjects;
    /*  contains all font subsets in use */
    FontSubsetData                  m_aSubsets;
    FontEmbedData                   m_aEmbeddedFonts;
    sal_Int32                       m_nNextFID;

    sal_Int32                       m_nInheritedPageWidth;  // in inch/72
    sal_Int32                       m_nInheritedPageHeight; // in inch/72
    PDFWriter::Orientation          m_eInheritedOrientation;
    sal_Int32                       m_nCurrentPage;

    sal_Int32                       m_nCatalogObject;

    PDFWriter::PDFVersion           m_eVersion;
    PDFWriter::Compression          m_eCompression;
    rtl::OUString                   m_aFileName;
    oslFileHandle                   m_aFile;
    bool                            m_bOpen;

    // graphics state
    struct GraphicsState
    {
        Font            m_aFont;
        MapMode         m_aMapMode;
        Color           m_aLineColor;
        Color           m_aFillColor;
        Color           m_aTextLineColor;
        Region          m_aClipRegion;
        sal_Int32       m_nAntiAlias;
        sal_Int32       m_nLayoutMode;
        sal_Int32       m_nTransparentPercent;
        sal_uInt16      m_nFlags;

        GraphicsState() :
                m_aLineColor( COL_TRANSPARENT ),
                m_aFillColor( COL_TRANSPARENT ),
                m_aTextLineColor( COL_TRANSPARENT ),
                m_nAntiAlias( 1 ),
                m_nLayoutMode( 0 ),
                m_nTransparentPercent( 0 ),
                m_nFlags( 0xffff )
        {}
        GraphicsState( const GraphicsState& rState ) :
                m_aFont( rState.m_aFont ),
                m_aMapMode( rState.m_aMapMode ),
                m_aLineColor( rState.m_aLineColor ),
                m_aFillColor( rState.m_aFillColor ),
                m_aTextLineColor( rState.m_aTextLineColor ),
                m_aClipRegion( rState.m_aClipRegion ),
                m_nAntiAlias( rState.m_nAntiAlias ),
                m_nLayoutMode( rState.m_nLayoutMode ),
                m_nTransparentPercent( rState.m_nTransparentPercent ),
                m_nFlags( rState.m_nFlags )
        {
        }

        GraphicsState& operator=(const GraphicsState& rState )
        {
            m_aFont                 = rState.m_aFont;
            m_aMapMode              = rState.m_aMapMode;
            m_aLineColor            = rState.m_aLineColor;
            m_aFillColor            = rState.m_aFillColor;
            m_aTextLineColor        = rState.m_aTextLineColor;
            m_aClipRegion           = rState.m_aClipRegion;
            m_nAntiAlias            = rState.m_nAntiAlias;
            m_nLayoutMode           = rState.m_nLayoutMode;
            m_nTransparentPercent   = rState.m_nTransparentPercent;
            m_nFlags                = rState.m_nFlags;
            return *this;
        }
    };
    std::list< GraphicsState >          m_aGraphicsStack;
    GraphicsState                       m_aCurrentPDFState;

    ZCodec*                             m_pCodec;
    SvMemoryStream*                     m_pMemStream;

    /* creates fonts and subsets that will be emitted later */
    void registerGlyphs( int nGlyphs, sal_Int32* pGlyphs, sal_Unicode* pUnicodes, sal_uInt8* pMappedGlyphs, sal_Int32* pMappedFontObjects, ImplFontData* pFallbackFonts[] );

    /*  emits a text object according to the passed layout */
    /* TODO: remove rText as soon as SalLayout will change so that rText is not necessary anymore */
    void drawLayout( SalLayout& rLayout, const String& rText, bool bTextLines );
    void drawRelief( SalLayout& rLayout, const String& rText, bool bTextLines );
    void drawShadow( SalLayout& rLayout, const String& rText, bool bTextLines );

    /*  writes differences between graphics stack and current real PDF
     *   state to the file
     */
    void updateGraphicsState();

    /* writes a transparency group object */
    bool writeTransparentObject( TransparencyEmit& rObject );

    /* writes an XObject of type image, may create
       a second for the mask
     */
    bool writeBitmapObject( BitmapEmit& rObject, bool bMask = false );

    bool writeJPG( JPGEmit& rEmit );

    /* tries to find the bitmap by its id and returns its emit data if exists,
       else creates a new emit data block */
    const BitmapEmit& createBitmapEmit( const BitmapEx& rBitmapEx, bool bDrawMask = false );

    /* writes the Do operation inside the content stream */
    void drawBitmap( const Point& rDestPt, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor );
    /* write the function object for a Gradient */
    bool writeGradientFunction( GradientEmit& rObject );
    /* creates a GradientEmit and returns its object number */
    sal_Int32 createGradient(  const Gradient& rGradient, const Size& rSize );

    /* writes all tilings */
    bool emitTilings();
    /* writes all gradient patterns */
    bool emitGradients();
    /* writes a builtin font object and returns its objectid (or 0 in case of failure ) */
    sal_Int32 emitBuiltinFont( ImplFontData* pFont );
    /* writes a type1 embedded font object and returns its mapping from font ids to object ids (or 0 in case of failure ) */
    std::map< sal_Int32, sal_Int32 > emitEmbeddedFont( ImplFontData* pFont, EmbedFont& rEmbed );
    /* writes a font descriptor and returns its object id (or 0) */
    sal_Int32 emitFontDescriptor( ImplFontData* pFont, FontSubsetInfo& rInfo, sal_Int32 nSubsetID, sal_Int32 nStream );
    /* writes a ToUnicode cmap, returns the corresponding stream object */
    sal_Int32 createToUnicodeCMap( sal_uInt8* pEncoding, sal_Unicode* pUnicodes, int nGlyphs );
    /* writes a the font dictionary and emits all font objects
     * returns object id of font directory (or 0 on error)
     */
    sal_Int32 emitFonts();
    /* writes the Resource dictionary;
     * returns dict object id (or 0 on error)
     */
    sal_Int32 emitResources();
    // writes page tree and catalog
    bool emitCatalog();
    // writes xref and trailer
    bool emitTrailer();
    // emits info dict (if applicable)
    sal_Int32 emitInfoDict();

    /* adds an entry to m_aObjects and returns its index+1,
     * sets the offset to ~0
     */
    sal_Int32 createObject();
    /* sets the offset of object n to the current position of output file+1
     */
    bool updateObject( sal_Int32 n );

    bool writeBuffer( const void* pBuffer, sal_uInt64 nBytes );
    void beginCompression();
    void endCompression();
    void endPage();

    /* draws an emphasis mark */
    void drawEmphasisMark(  long nX, long nY, const PolyPolygon& rPolyPoly, BOOL bPolyLine, const Rectangle& rRect1, const Rectangle& rRect2 );
public:
    PDFWriterImpl( const rtl::OUString& rTargetFile, PDFWriter::PDFVersion eVersion = PDFWriter::PDF_1_4, PDFWriter::Compression eCompression = PDFWriter::Screen );
    ~PDFWriterImpl();

    /*  for OutputDevice so the reference device can have a list
     *  that contains only suitable fonts (subsettable or builtin)
     *  produces a new font list
     */
    ImplDevFontList* filterDevFontList( ImplDevFontList* pFontList );
    /*  for OutputDevice: get layout for builtin fonts
     */
    bool isBuiltinFont( ImplFontData* pFont ) const;
    SalLayout* GetTextLayout( ImplLayoutArgs& rArgs, ImplFontSelectData* pFont );
    void getFontMetric( ImplFontSelectData* pFont, ImplFontMetricData* pMetric ) const;


    /* for documentation of public functions please see pdfwriter.hxx */

    OutputDevice* getReferenceDevice();

    /* document structure */
    sal_Int32 newPage( sal_Int32 nPageWidth , sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation );
    bool emit();

    PDFWriter::PDFVersion getVersion() const { return m_eVersion; }
    void setDocInfo( const PDFDocInfo& rInfo );
    const PDFDocInfo& getDocInfo() const { return m_aDocInfo; }


    /* graphics state */
    void push( sal_uInt16 nFlags );
    void pop();

    void setFont( const Font& rFont )
    { m_aGraphicsStack.front().m_aFont = rFont; }

    void setMapMode( const MapMode& rMapMode );
    void setMapMode() { setMapMode( m_aMapMode ); }


    const MapMode& getMapMode() { return m_aGraphicsStack.front().m_aMapMode; }

    void setLineColor( const Color& rColor )
    { m_aGraphicsStack.front().m_aLineColor = rColor; }

    void setFillColor( const Color& rColor )
    { m_aGraphicsStack.front().m_aFillColor = rColor; }

    void setTextLineColor()
    { m_aGraphicsStack.front().m_aTextLineColor = Color( COL_TRANSPARENT ); }

    void setTextLineColor( const Color& rColor )
    { m_aGraphicsStack.front().m_aTextLineColor = rColor; }

    void setTextFillColor( const Color& rColor )
    {
        m_aGraphicsStack.front().m_aFont.SetFillColor( rColor );
        m_aGraphicsStack.front().m_aFont.SetTransparent( ImplIsColorTransparent( rColor ) );
    }
    void setTextFillColor()
    {
        m_aGraphicsStack.front().m_aFont.SetFillColor( Color( COL_TRANSPARENT ) );
        m_aGraphicsStack.front().m_aFont.SetTransparent( TRUE );
    }
    void setTextColor( const Color& rColor )
    { m_aGraphicsStack.front().m_aFont.SetColor( rColor ); }

    void clearClipRegion()
    { m_aGraphicsStack.front().m_aClipRegion.SetNull(); }

    void setClipRegion( const Region& rRegion );

    void moveClipRegion( sal_Int32 nX, sal_Int32 nY );

    bool intersectClipRegion( const Rectangle& rRect );

    bool intersectClipRegion( const Region& rRegion );

    void setLayoutMode( sal_Int32 nLayoutMode )
    { m_aGraphicsStack.front().m_nLayoutMode = nLayoutMode; }

    void setTextAlign( TextAlign eAlign )
    { m_aGraphicsStack.front().m_aFont.SetAlign( eAlign ); }

    void setAntiAlias( sal_Int32 nAntiAlias )
    { m_aGraphicsStack.front().m_nAntiAlias = nAntiAlias; }

    /* actual drawing functions */
    void drawText( const Point& rPos, const String& rText, xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN, bool bTextLines = true );
    void drawTextArray( const Point& rPos, const String& rText, const sal_Int32* pDXArray = NULL, xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN, bool bTextLines = true );
    void drawStretchText( const Point& rPos, ULONG nWidth, const String& rText,
                          xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN,
                          bool bTextLines = true  );
    void drawText( const Rectangle& rRect, const String& rOrigStr, USHORT nStyle, bool bTextLines = true  );
    void drawTextLine( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, bool bUnderlineAbove );

    void drawLine( const Point& rStart, const Point& rStop );
    void drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo );
    void drawPolygon( const Polygon& rPoly );
    void drawPolyPolygon( const PolyPolygon& rPolyPoly );
    void drawPolyLine( const Polygon& rPoly );
    void drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo );
    void drawWaveLine( const Point& rStart, const Point& rStop, sal_Int32 nDelta, sal_Int32 nLineWidth );

    void drawPixel( const Point& rPt, const Color& rColor );
    void drawPixel( const Polygon& rPts, const Color* pColors = NULL );

    void drawRectangle( const Rectangle& rRect );
    void drawRectangle( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound );
    void drawEllipse( const Rectangle& rRect );
    void drawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWidthChord );

    void drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap );
    void drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap );
    void drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor );
    void drawJPGBitmap( SvStream& rDCTData, const Size& rSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask );

    void drawGradient( const Rectangle& rRect, const Gradient& rGradient );
    void drawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch );
    void drawWallpaper( const Rectangle& rRect, const Wallpaper& rWall );
    void drawTransparent( const PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent );

    void emitComment( const rtl::OString& rComment );
};

}

#endif //_VCL_PDFEXPORT_HXX
