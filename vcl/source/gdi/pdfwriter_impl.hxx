/*************************************************************************
 *
 *  $RCSfile: pdfwriter_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2002-07-15 12:02:21 $
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

#include <vector>
#include <map>
#include <list>

namespace rtl { class OStringBuffer; }
class SalLayout;
struct ImplFontData;

namespace vcl
{

class PDFWriterImpl
{
public:
    // definition of structs

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
        void appendPoint( const Point& rPoint, rtl::OStringBuffer& rBuffer, bool bNeg = false );
        // appends a rectangle
        void appendRect( const Rectangle& rRect, rtl::OStringBuffer& rBuffer );
        // appends a polygon optionally closing it
        void appendPolygon( const Polygon& rPoly, rtl::OStringBuffer& rBuffer, bool bClose = true );
        // appends a polypolygon optionally closing the subpaths
        void appendPolyPolygon( const PolyPolygon& rPolyPoly, rtl::OStringBuffer& rBuffer, bool bClose = true );
        // converts a length (either vertical or horizontal; this
        // can be important if the source MapMode is not
        // symmetrical) to page length and appends it to the buffer
        void appendMappedLength( sal_Int32 nLength, rtl::OStringBuffer& rBuffer, bool bVertical = true );
        // appends LineInfo
        void appendLineInfo( const LineInfo& rInfo, rtl::OStringBuffer& rBuffer );
    };

    friend struct PDFPage;

    struct BitmapEmit
    {
        BitmapEx    m_aBitmap;
        sal_Int32   m_nObject;
    };

    struct GradientEmit
    {
        Gradient    m_aGradient;
        Size        m_aSize;
        sal_Int32   m_nObject;
    };

    struct HatchEmit
    {
        Hatch       m_aHatch;
        sal_Int32   m_nObject;
    };

    // for bitmap tilings (drawWallpaper)
    struct BitmapPatternEmit
    {
        sal_Int32   m_nObject;
        sal_Int32   m_nBitmapObject;
        Rectangle   m_aRectangle;
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

private:
    OutputDevice*                   m_pReferenceDevice;

    MapMode                         m_aMapMode; // PDFWriterImpl scaled units
    std::list< PDFPage >            m_aPages;
    /* maps object numbers to file offsets (needed for xref) */
    std::vector< sal_uInt64 >       m_aObjects;
    /* contains Bitmaps until they are written to the
     *  file stream as XObjects*/
    std::list< BitmapEmit >         m_aBitmaps;
    /* contains Bitmaps for gradient functions until they are written
     *  to the file stream */
    std::list< GradientEmit >       m_aGradients;
    /*  contains hatches to be emitted for the whole document
     */
    std::list< HatchEmit >          m_aHatches;
    /* contains bitmap tiling patterns */
    std::list< BitmapPatternEmit >  m_aTilings;
    /*  contains all font subsets in use */
    FontSubsetData                  m_aSubsets;
    sal_Int32                       m_nNextFID;

    sal_Int32                       m_nInheritedPageWidth;  // in inch/72
    sal_Int32                       m_nInheritedPageHeight; // in inch/72
    PDFWriter::Orientation          m_eInheritedOrientation;
    sal_Int32                       m_nCurrentPage;

    sal_Int32                       m_nCatalogObject;

    PDFWriter::PDFVersion           m_eVersion;
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
        TextAlign       m_eTextAlign;
        sal_Int32       m_nTransparentPercent;

        GraphicsState() :
                m_aLineColor( COL_TRANSPARENT ),
                m_aFillColor( COL_TRANSPARENT ),
                m_aTextLineColor( COL_BLACK ),
                m_nLayoutMode( 0 ),
                m_nTransparentPercent( 0 ) {}
        GraphicsState( const GraphicsState& rState ) :
                m_aFont( rState.m_aFont ),
                m_aMapMode( rState.m_aMapMode ),
                m_aLineColor( rState.m_aLineColor ),
                m_aFillColor( rState.m_aFillColor ),
                m_aTextLineColor( rState.m_aTextLineColor ),
                m_aClipRegion( rState.m_aClipRegion ),
                m_nAntiAlias( rState.m_nAntiAlias ),
                m_nLayoutMode( rState.m_nLayoutMode ),
                m_eTextAlign( rState.m_eTextAlign ),
                m_nTransparentPercent( rState.m_nTransparentPercent )
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
            m_eTextAlign            = rState.m_eTextAlign;
            m_nTransparentPercent   = rState.m_nTransparentPercent;
            return *this;
        }
    };
    std::list< GraphicsState >          m_aGraphicsStack;
    GraphicsState                       m_aCurrentPDFState;

    /* creates fonts and subsets that will be emitted later */
    void registerGlyphs( int nGlyphs, long* pGlyphs, sal_Unicode* pUnicodes, sal_uInt8* pMappedGlyphs, sal_Int32* pMappedFontObjects );

    /*  emits a text object according to the passed layout */
    void drawLayout( const SalLayout& rLayout );

    /*  writes differences between graphics stack and current real PDF
     *   state to the file
     */
    void updateGraphicsState();

    /* writes an XObject of type image, may create
       a second for the mask
     */
    bool writeBitmapObject( BitmapEmit& rObject, bool bMask = false );
    /* writes the Do operation inside the content stream */
    void drawBitmap( const Point& rDestPt, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor );
    /* write the function object for a Gradient */
    bool writeGradientFunction( GradientEmit& rObject );
    /* creates a GradientEmit and returns its object number */
    sal_Int32 createGradient(  const Gradient& rGradient, const Size& rSize );
    /* creates a HatchEmit and returns its object number */
    sal_Int32 createHatch(  const Hatch& rHatch );

    /* writes all tilings */
    bool emitTilings();
    /* writes all gradient patterns */
    bool emitGradients();
    /* writes all hatch patterns */
    bool emitHatches();
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

    /* adds an entry to m_aObjects and returns its index+1,
     * sets the offset to ~0
     */
    sal_Int32 createObject();
    /* sets the offset of object n to the current position of output file+1
     */
    bool updateObject( sal_Int32 n );

    bool writeBuffer( const void* pBuffer, sal_uInt64 nBytes );
    void endPage();
public:
    PDFWriterImpl( const rtl::OUString& rTargetFile, PDFWriter::PDFVersion eVersion = PDFWriter::PDF_1_4 );
    ~PDFWriterImpl();

    /*  for OutputDevice so the reference device can have a list
     *  that contains only suitable fonts (subsettable or builtin)
     *  produces a new font list
     */
    ImplDevFontList* filterDevFontList( ImplDevFontList* pFontList );


    /* for documentation of public fucntions please see pdfwriter.hxx */

    OutputDevice* getReferenceDevice();

    /* document structure */
    sal_Int32 newPage( sal_Int32 nPageWidth , sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation );
    bool emit();

    PDFWriter::PDFVersion getVersion() const { return m_eVersion; }

    /* graphics state */
    void push()
    { m_aGraphicsStack.push_front( m_aGraphicsStack.front() ); }

    void pop()
    { m_aGraphicsStack.pop_front(); }

    void setFont( const Font& rFont )
    { m_aGraphicsStack.front().m_aFont = rFont; }

    void setMapMode() { m_aGraphicsStack.front().m_aMapMode = m_aMapMode; }

    void setMapMode( const MapMode& rMapMode )
    { m_aGraphicsStack.front().m_aMapMode = rMapMode; }

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
    { m_aGraphicsStack.front().m_aClipRegion.SetEmpty(); }

    void setClipRegion( const Region& rRegion )
    { m_aGraphicsStack.front().m_aClipRegion = rRegion; }

    void moveClipRegion( sal_Int32 nX, sal_Int32 nY )
    { m_aGraphicsStack.front().m_aClipRegion.Move( nX, nY ); }

    bool intersectClipRegion( const Rectangle& rRect )
    { return m_aGraphicsStack.front().m_aClipRegion.Intersect( rRect ); }

    bool intersectClipRegion( const Region& rRegion )
    { return m_aGraphicsStack.front().m_aClipRegion.Intersect( rRegion ); }

    void setLayoutMode( sal_Int32 nLayoutMode )
    { m_aGraphicsStack.front().m_nLayoutMode = nLayoutMode; }

    void setTextAlign( TextAlign eAlign )
    { m_aGraphicsStack.front().m_eTextAlign = eAlign; }

    void setAntiAlias( sal_Int32 nAntiAlias )
    { m_aGraphicsStack.front().m_nAntiAlias = nAntiAlias; }

    /* actual drawing functions */
    void drawText( const Point& rPos, const String& rText );

    void drawLine( const Point& rStart, const Point& rStop );
    void drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo );
    void drawPolygon( const Polygon& rPoly );
    void drawPolyPolygon( const PolyPolygon& rPolyPoly );
    void drawPolyLine( const Polygon& rPoly );
    void drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo );

    void drawPixel( const Point& rPt, const Color& rColor );
    void drawPixel( const Polygon& rPts, const Color* pColors = NULL );

    void drawRectangle( const Rectangle& rRect );
    void drawRectangle( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound );
    void drawEllipse( const Rectangle& rRect );
    void drawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWidthChord );

    void drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap );
    void drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap );
    void drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor );
    void drawGradient( const Rectangle& rRect, const Gradient& rGradient );
    void drawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch );
    void drawWallpaper( const Rectangle& rRect, const Wallpaper& rWall );
    void drawTransparent( const PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent );


    void emitComment( const rtl::OString& rComment );
};

}

#endif //_VCL_PDFEXPORT_HXX
