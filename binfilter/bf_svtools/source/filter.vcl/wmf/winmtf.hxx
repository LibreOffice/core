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

#ifndef _WINMTF_HXX
#define _WINMTF_HXX

#ifdef DBG_UTIL
#define WIN_MTF_ASSERT
#endif

#include <vector>
// #include <math.h>
#include <stdlib.h>
#include <sot/object.hxx>
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#ifndef _TOOL_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STACK_HXX
#include <tools/stack.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _DYNARY_HXX
#include <tools/dynary.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <vcl/lineinfo.hxx>
#endif
#ifndef _FLTCALL_HXX
#include <bf_svtools/fltcall.hxx>
#endif

namespace binfilter
{


#define ERROR                   0
#define NULLREGION              1
#define SIMPLEREGION            2
#define COMPLEXREGION           3

#define RGN_AND					1
#define RGN_OR					2
#define RGN_XOR					3
#define RGN_DIFF				4
#define RGN_COPY				5

#define TRANSPARENT				1
#define OPAQUE					2
#define BKMODE_LAST				2

/* xform stuff */
#define MWT_IDENTITY			1
#define MWT_LEFTMULTIPLY		2
#define MWT_RIGHTMULTIPLY		3

#define ENHMETA_STOCK_OBJECT    0x80000000

/* Stock Logical Objects */
#define WHITE_BRUSH				0
#define LTGRAY_BRUSH			1
#define GRAY_BRUSH				2
#define DKGRAY_BRUSH			3
#define BLACK_BRUSH				4
#define NULL_BRUSH				5
#define HOLLOW_BRUSH			NULL_BRUSH
#define WHITE_PEN				6
#define BLACK_PEN				7
#define NULL_PEN				8
#define OEM_FIXED_FONT			10
#define ANSI_FIXED_FONT			11
#define ANSI_VAR_FONT			12
#define SYSTEM_FONT				13
#define DEVICE_DEFAULT_FONT		14
#define DEFAULT_PALETTE			15
#define SYSTEM_FIXED_FONT		16


#define R2_BLACK				1
#define R2_NOTMERGEPEN			2
#define R2_MASKNOTPEN			3
#define R2_NOTCOPYPEN			4
#define R2_MASKPENNOT			5
#define R2_NOT					6
#define R2_XORPEN				7
#define R2_NOTMASKPEN			8
#define R2_MASKPEN				9
#define R2_NOTXORPEN			10
#define R2_NOP					11
#define R2_MERGENOTPEN			12
#define R2_COPYPEN				13
#define R2_MERGEPENNOT			14
#define R2_MERGEPEN				15
#define R2_WHITE				16

/* Mapping Modes */
#define MM_TEXT					1
#define MM_LOMETRIC				2
#define MM_HIMETRIC				3
#define MM_LOENGLISH			4
#define MM_HIENGLISH			5
#define MM_TWIPS				6
#define MM_ISOTROPIC			7
#define MM_ANISOTROPIC			8


/* Graphics Modes */
#define GM_COMPATIBLE			1
#define GM_ADVANCED				2
#define GM_LAST					2

/* StretchBlt() Modes */
#define BLACKONWHITE			1
#define WHITEONBLACK			2
#define COLORONCOLOR			3
#define HALFTONE				4
#define MAXSTRETCHBLTMODE		4
#define STRETCH_ANDSCANS		BLACKONWHITE
#define STRETCH_ORSCANS			WHITEONBLACK
#define STRETCH_DELETESCANS		COLORONCOLOR
#define STRETCH_HALFTONE		HALFTONE

#define LF_FACESIZE				32

struct LOGFONTW
{
    INT32		lfHeight;
    INT32		lfWidth;
    INT32		lfEscapement;
    INT32		lfOrientation;
    INT32		lfWeight;
    BYTE		lfItalic;
    BYTE		lfUnderline;
    BYTE		lfStrikeOut;
    BYTE		lfCharSet;
    BYTE		lfOutPrecision;
    BYTE		lfClipPrecision;
    BYTE		lfQuality;
    BYTE		lfPitchAndFamily;
    String		alfFaceName;
};

#define TA_NOUPDATECP			0x0000
#define TA_UPDATECP				0x0001
#define TA_LEFT					0x0000
#define TA_RIGHT				0x0002
#define TA_CENTER				0x0006
#define TA_RIGHT_CENTER	 (TA_RIGHT | TA_CENTER)
#define TA_TOP					0x0000
#define TA_BOTTOM				0x0008
#define TA_BASELINE				0x0018

#define SRCCOPY					0x00CC0020L
#define SRCPAINT				0x00EE0086L
#define SRCAND					0x008800C6L
#define SRCINVERT				0x00660046L
#define SRCERASE				0x00440328L
#define NOTSRCCOPY				0x00330008L
#define NOTSRCERASE				0x001100A6L
#define MERGECOPY				0x00C000CAL
#define MERGEPAINT				0x00BB0226L
#define PATCOPY					0x00F00021L
#define PATPAINT				0x00FB0A09L
#define PATINVERT				0x005A0049L
#define DSTINVERT				0x00550009L
#define BLACKNESS				0x00000042L
#define WHITENESS				0x00FF0062L

#define PS_SOLID				0
#define PS_DASH					1
#define PS_DOT					2
#define PS_DASHDOT				3
#define PS_DASHDOTDOT			4
#define PS_NULL					5
#define PS_INSIDEFRAME			6
#define PS_USERSTYLE            7
#define PS_ALTERNATE            8
#define PS_STYLE_MASK           15

#define PS_ENDCAP_ROUND     0x000
#define PS_ENDCAP_SQUARE    0x100
#define PS_ENDCAP_FLAT      0x200
#define PS_ENDCAP_MASK      0xF00

#define PS_JOIN_ROUND       0x0000
#define PS_JOIN_BEVEL       0x1000
#define PS_JOIN_MITER       0x2000
#define PS_JOIN_MASK        0xF000

#define PS_COSMETIC         0x00000
#define PS_GEOMETRIC        0x10000
#define PS_TYPE_MASK        0xF0000

#define ANSI_CHARSET			0
#define DEFAULT_CHARSET			1
#define SYMBOL_CHARSET			2
#define SHIFTJIS_CHARSET		128
#define HANGEUL_CHARSET			129
#define GB2312_CHARSET			134
#define CHINESEBIG5_CHARSET		136
#define OEM_CHARSET				255
/*WINVER >= 0x0400*/
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204
#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#define ETO_OPAQUE				0x0002
#define ETO_CLIPPED				0x0004
/*WINVER >= 0x0400*/
#define ETO_GLYPH_INDEX			0x0010
#define ETO_RTLREADING			0x0080
#define ETO_NUMERICSLOCAL		0x0400
#define ETO_NUMERICSLATIN		0x0800
#define ETO_IGNORELANGUAGE		0x1000
/*_WIN32_WINNT >= 0x0500*/
#define ETO_PDY					0x2000


#define DEFAULT_PITCH			0x00
#define FIXED_PITCH				0x01
#define VARIABLE_PITCH			0x02

/* Font Families */
#define FF_DONTCARE				0x00
#define FF_ROMAN				0x10
#define FF_SWISS				0x20
#define FF_MODERN				0x30
#define FF_SCRIPT				0x40
#define FF_DECORATIVE			0x50

#define FW_DONTCARE				0
#define FW_THIN					100
#define FW_EXTRALIGHT			200
#define FW_LIGHT				300
#define FW_NORMAL				400
#define FW_MEDIUM				500
#define FW_SEMIBOLD				600
#define FW_BOLD					700
#define FW_EXTRABOLD			800
#define FW_HEAVY				900
#define FW_ULTRALIGHT			200
#define FW_REGULAR				400
#define FW_DEMIBOLD				600
#define FW_ULTRABOLD			800
#define FW_BLACK				900

#define BS_SOLID				0
#define BS_NULL					1
#define BS_HOLLOW				1
#define BS_HATCHED				2
#define BS_PATTERN				3
#define BS_INDEXED				4
#define BS_DIBPATTERN			5
#define BS_DIBPATTERNPT			6
#define BS_PATTERN8X8			7
#define BS_DIBPATTERN8X8		8
#define BS_MONOPATTERN			9

#define W_HS_HORIZONTAL			0
#define W_HS_VERTICAL			1
#define W_HS_FDIAGONAL			2
#define W_HS_BDIAGONAL			3
#define W_HS_CROSS				4
#define W_HS_DIAGCROSS			5

#define RDH_RECTANGLES  1

#define W_MFCOMMENT				15

#define PRIVATE_ESCAPE_UNICODE	2

//============================ WMFReader ==================================


#ifdef WIN_MTF_ASSERT
#define WIN_MTF_ASSERT_INIT     0x80000000
#define WIN_MTF_ASSERT_ONCE     0x40000000
#define WIN_MTF_ASSERT_MIFE     0x20000000

void WinMtfAssertHandler( const sal_Char*, sal_uInt32 nFlags = WIN_MTF_ASSERT_MIFE );
#endif 

enum WinMtfClipPathType{ EMPTY, RECTANGLE, COMPLEX };

class WinMtfClipPath
{
        PolyPolygon			aPolyPoly;
        WinMtfClipPathType	eType;

        void		ImpUpdateType();

    public :

        sal_Bool	bNeedsUpdate;

                    WinMtfClipPath(): eType(EMPTY), bNeedsUpdate( sal_False ){};

        void		SetClipPath( const PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode );
        void		IntersectClipRect( const Rectangle& rRect );
        void		ExcludeClipRect( const Rectangle& rRect );
        void		MoveClipRegion( const Size& rSize );

        WinMtfClipPathType GetType() const { return eType; };
        const PolyPolygon& GetClipPath() const { return aPolyPoly; };

        sal_Bool operator==( const WinMtfClipPath& rPath )
        {
            return  ( rPath.eType == eType ) &&
                    ( rPath.aPolyPoly == aPolyPoly );
        };
};

class WinMtfPathObj : public PolyPolygon
{
        sal_Bool	bClosed;

    public :

                WinMtfPathObj() { bClosed = sal_True; }
        void    Init() { Clear(); bClosed = sal_True; };
        void	ClosePath();

        void	AddPoint( const Point& rPoint );
        void	AddPolygon( const Polygon& rPoly );
        void	AddPolyLine( const Polygon& rPoly );
        void	AddPolyPolygon( const PolyPolygon& rPolyPolygon );
};

struct WinMtfFontStyle
{
    Font		aFont;

    WinMtfFontStyle( LOGFONTW& rLogFont );
};

// -----------------------------------------------------------------------------

struct WinMtfFillStyle
{
    Color	aFillColor;
    BOOL	bTransparent;

    WinMtfFillStyle() :
        aFillColor	( Color( COL_BLACK ) ),
        bTransparent( FALSE )
    {
    };

    WinMtfFillStyle( const Color& rColor, BOOL bTrans = FALSE ) :
        aFillColor	( rColor ),
        bTransparent( bTrans )
    {
    };

    BOOL operator==( const WinMtfFillStyle& rStyle )
        { return ( ( aFillColor == rStyle.aFillColor ) && ( bTransparent == rStyle.bTransparent ) ); };
    BOOL operator==( WinMtfFillStyle* pStyle )
        { return ( ( aFillColor == pStyle->aFillColor ) && ( bTransparent == pStyle->bTransparent ) ); };
    void operator=( const WinMtfFillStyle& rStyle ) { aFillColor = rStyle.aFillColor; bTransparent = rStyle.bTransparent; };
    void operator=( WinMtfFillStyle* pStyle ) { aFillColor = pStyle->aFillColor; bTransparent = pStyle->bTransparent; };
};

// -----------------------------------------------------------------------------

struct WinMtfLineStyle
{
    Color		aLineColor;
    LineInfo	aLineInfo;
    BOOL		bTransparent;

    WinMtfLineStyle() :
        aLineColor	( COL_BLACK ),
        bTransparent( FALSE ) {};

    WinMtfLineStyle( const Color& rColor, BOOL bTrans = FALSE ) :
        aLineColor	( rColor ),
        bTransparent( bTrans ) {};

    WinMtfLineStyle( const Color& rColor, const LineInfo rStyle, BOOL bTrans = FALSE ) :
        aLineColor	( rColor ),
        aLineInfo	( rStyle ),
        bTransparent( bTrans ) {};

    BOOL operator==( const WinMtfLineStyle& rStyle ) { return ( ( aLineColor == rStyle.aLineColor ) && ( bTransparent == rStyle.bTransparent ) && ( aLineInfo == rStyle.aLineInfo ) ); };
    BOOL operator==( WinMtfLineStyle* pStyle ) { return ( ( aLineColor == pStyle->aLineColor ) && ( bTransparent == pStyle->bTransparent ) && ( aLineInfo == pStyle->aLineInfo ) ); };
    void operator=( const WinMtfLineStyle& rStyle )
    {
        aLineColor = rStyle.aLineColor;
        bTransparent = rStyle.bTransparent;
        aLineInfo = rStyle.aLineInfo;
    };

    void operator=( WinMtfLineStyle* pStyle )
    {
        aLineColor = pStyle->aLineColor;
        bTransparent = pStyle->bTransparent;
        aLineInfo = pStyle->aLineInfo;
    };
};

// -----------------------------------------------------------------------------

struct XForm
{
    float	eM11;
    float	eM12;
    float	eM21;
    float	eM22;
    float	eDx;
    float	eDy;
    XForm()
    {
        eM11 =  eM22 = 1.0f;
        eDx = eDy = eM12 = eM21 = 0.0f;
    };

    friend SvStream& operator>>( SvStream& rIn, XForm& rXForm );
};

// -----------------------------------------------------------------------------

struct SaveStruct
{
    sal_uInt32          nBkMode, nMapMode, nGfxMode, nTextLayoutMode;
    sal_Int32			nWinOrgX, nWinOrgY, nWinExtX, nWinExtY;
    sal_Int32			nDevOrgX, nDevOrgY, nDevWidth, nDevHeight;

    WinMtfLineStyle		aLineStyle;
    WinMtfFillStyle		aFillStyle;

    Font				aFont;
    Color				aBkColor;
    Color				aTextColor;
    sal_uInt32          nTextAlign;
    RasterOp            eRasterOp;

    Point				aActPos;
    WinMtfPathObj		aPathObj;
    WinMtfClipPath		aClipPath;
    XForm				aXForm;

    sal_Bool			bRecordPath;
    sal_Bool			bFillStyleSelected;
};

typedef ::boost::shared_ptr< SaveStruct > SaveStructPtr;

// -----------------------------------------------------------------------------

struct BSaveStruct
{
    Bitmap		aBmp;
    Rectangle	aOutRect;
    UINT32		nWinRop;

                BSaveStruct( const Bitmap& rBmp, const Rectangle& rOutRect, UINT32 nRop ) :
                    aBmp( rBmp ), aOutRect( rOutRect ), nWinRop( nRop ){};
};

// -----------------------------------------------------------------------------

enum GDIObjectType { GDI_DUMMY = 0, GDI_PEN = 1, GDI_BRUSH = 2, GDI_FONT = 3, GDI_PALETTE = 4, GDI_BITMAP = 5, GDI_REGION = 6 };

struct GDIObj
{
    void*			pStyle;
    GDIObjectType 	eType;

    GDIObj() :
        pStyle	( NULL ),
        eType	( GDI_DUMMY )
    {
    };

    GDIObj( GDIObjectType eT, void* pS ) { pStyle = pS; eType = eT; };
    void Set( GDIObjectType eT, void* pS ) { pStyle = pS; eType = eT; };
    void Delete()
    {
        if ( pStyle )
        {
            switch ( eType )
            {
                case GDI_PEN :
                    delete (WinMtfLineStyle*)pStyle;
                break;
                case GDI_BRUSH :
                    delete (WinMtfFillStyle*)pStyle;
                break;
                case GDI_FONT :
                    delete (WinMtfFontStyle*)pStyle;
                break;

                default:
                    DBG_ERROR( "unsupported style deleted" );
                    break;
            }
            pStyle = NULL;
        }
    };

    ~GDIObj()
    {
        Delete();
    }
};

// -----------------------------------------------------------------------------

class WinMtfOutput
{

        WinMtfPathObj		aPathObj;
        WinMtfClipPath		aClipPath;

        WinMtfLineStyle		maLatestLineStyle;
        WinMtfLineStyle		maLineStyle;
        WinMtfFillStyle		maLatestFillStyle;
        WinMtfFillStyle		maFillStyle;
        Font                maLatestFont;
        Font				maFont;
        sal_uInt32          mnLatestTextAlign;
        sal_uInt32          mnTextAlign;
        Color               maLatestTextColor;
        Color				maTextColor;
        Color               maLatestBkColor;
        Color				maBkColor;
        sal_uInt32			mnLatestTextLayoutMode;
        sal_uInt32			mnTextLayoutMode;
        sal_uInt32          mnLatestBkMode;
        sal_uInt32          mnBkMode;
        RasterOp            meLatestRasterOp;
        RasterOp			meRasterOp;

        std::vector< GDIObj* > vGDIObj;

        Point				maActPos;

        sal_uInt32          mnRop;
        sal_Bool            mbNopMode;
        sal_Bool			mbFillStyleSelected;

        std::vector< SaveStructPtr > vSaveStack;

        sal_uInt32			mnGfxMode;
        sal_uInt32          mnMapMode;
        XForm				maXForm;
        sal_Int32			mnDevOrgX, mnDevOrgY;
        sal_Int32			mnDevWidth, mnDevHeight;
        sal_Int32			mnWinOrgX, mnWinOrgY;		// aktuelles Window-Origin
        sal_Int32			mnWinExtX, mnWinExtY;		// aktuelles Window-Extent
    
        sal_Int32			mnPixX, mnPixY;				// Reference Device in pixel
        sal_Int32			mnMillX, mnMillY;			// Reference Device in Mill
        Rectangle			mrclFrame;					// rectangle in logical units 1/100th mm
        Rectangle			mrclBounds;
        
        GDIMetaFile*		mpGDIMetaFile;

        void				UpdateLineStyle();
        void				UpdateFillStyle();

        Point				ImplMap( const Point& rPt );
        Size				ImplMap( const Size& rSz );
        Rectangle			ImplMap( const Rectangle& rRectangle );
        void				ImplMap( Font& rFont );
        Polygon&			ImplMap( Polygon& rPolygon );
        PolyPolygon&		ImplMap( PolyPolygon& rPolyPolygon );
        void				ImplResizeObjectArry( UINT32 nNewEntry );
        void				ImplSetNonPersistentLineColorTransparenz();
        void				ImplDrawClippedPolyPolygon( const PolyPolygon& rPolyPoly );
        void				ImplDrawBitmap( const Point& rPos, const Size& rSize, const BitmapEx rBitmap );

    public:

        void				SetDevOrg( const Point& rPoint );
        void				SetDevOrgOffset( INT32 nXAdd, INT32 nYAdd );
        void				SetDevExt( const Size& rSize );
        void				ScaleDevExt( double fX, double fY );

        void				SetWinOrg( const Point& rPoint );
        void				SetWinOrgOffset( INT32 nX, INT32 nY );
        void				SetWinExt( const Size& rSize );
        void				ScaleWinExt( double fX, double fY );

        void				SetrclBounds( const Rectangle& rRect );
        void				SetrclFrame( const Rectangle& rRect );
        void				SetRefPix( const Size& rSize );
        void				SetRefMill( const Size& rSize );

        sal_uInt32			GetMapMode() const { return mnMapMode; };
        void                SetMapMode( sal_uInt32 mnMapMode );
        void				SetWorldTransform( const XForm& rXForm );
        void				ModifyWorldTransform( const XForm& rXForm, UINT32 nMode );

        void				Push();
        void				Pop();

        UINT32				SetRasterOp( UINT32 nRasterOp );
        void				StrokeAndFillPath( sal_Bool bStroke, sal_Bool bFill );

        void				SetGfxMode( sal_Int32 nGfxMode ){ mnGfxMode = nGfxMode; };
        sal_Int32			GetGfxMode() const { return mnGfxMode; };
        void				SetBkMode( UINT32 nMode );
        void				SetBkColor( const Color& rColor );
        void				SetTextColor( const Color& rColor );
        void				SetTextAlign( UINT32 nAlign );
        void				CreateObject( GDIObjectType, void* pStyle = NULL );
        void				CreateObject( INT32 nIndex, GDIObjectType, void* pStyle = NULL );
        void				DeleteObject( INT32 nIndex );
        void				SelectObject( INT32 nIndex );
        CharSet				GetCharSet(){ return maFont.GetCharSet(); };
        const Font&			GetFont() const;
        void				SetTextLayoutMode( const sal_uInt32 nLayoutMode );

        void				ClearPath(){ aPathObj.Init(); };
        void				ClosePath(){ aPathObj.ClosePath(); };
        const PolyPolygon&	GetPathObj(){ return aPathObj; };

        void				MoveTo( const Point& rPoint, sal_Bool bRecordPath = sal_False );
        void				LineTo( const Point& rPoint, sal_Bool bRecordPath = sal_False );
        void				DrawPixel( const Point& rSource, const Color& rColor );
        void				DrawRect( const Rectangle& rRect, BOOL bEdge = TRUE );
        void				DrawRoundRect( const Rectangle& rRect, const Size& rSize );
        void				DrawEllipse( const Rectangle& rRect );
        void				DrawArc( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle, BOOL bDrawTo = FALSE );
        void				DrawPie( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle );
        void				DrawChord( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle );
        void				DrawPolygon( Polygon& rPolygon, sal_Bool bRecordPath = sal_False );
        void				DrawPolyPolygon( PolyPolygon& rPolyPolygon, sal_Bool bRecordPath = sal_False );
        void				DrawPolyLine( Polygon& rPolygon, sal_Bool bDrawTo = sal_False, sal_Bool bRecordPath = sal_False );
        void				DrawPolyBezier( Polygon& rPolygin, sal_Bool bDrawTo = sal_False, sal_Bool bRecordPath = sal_False );
        void				DrawText( Point& rPosition, String& rString, sal_Int32* pDXArry = NULL, sal_Bool bRecordPath = sal_False,
                                        sal_Int32 nGraphicsMode = GM_COMPATIBLE );
        void				ResolveBitmapActions( List& rSaveList );

        void				IntersectClipRect( const Rectangle& rRect );
        void				ExcludeClipRect( const Rectangle& rRect );
        void				MoveClipRegion( const Size& rSize );
        void				SetClipPath( const PolyPolygon& rPolyPoly, sal_Int32 nClippingMode, sal_Bool bIsMapped );
        void				UpdateClipRegion();

                            WinMtfOutput( GDIMetaFile& rGDIMetaFile );
        virtual				~WinMtfOutput();
};

// -----------------------------------------------------------------------------

class WinMtf
{
    protected:

    WinMtfOutput*		pOut;					//
    SvStream*			pWMF;					// Die einzulesende WMF/EMF-Datei

    UINT32				nStartPos, nEndPos;
    List				aBmpSaveList;

    FilterConfigItem*	pFilterConfigItem;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    // Sorgt dafuer, das aSampledBrush der aktuelle Brush des GDIMetaFiles ist.

    Color				ReadColor();
    void				Callback( USHORT nPercent );

                        WinMtf( WinMtfOutput* pOut, SvStream& rStreamWMF, FilterConfigItem* pConfigItem = NULL );
                        ~WinMtf();

    public:

};

//============================ EMFReader ==================================

class EnhWMFReader : public WinMtf
{
    sal_Bool		bRecordPath;
    sal_Int32		nRecordCount;

    BOOL			ReadHeader();
    Rectangle		ReadRectangle( INT32, INT32, INT32, INT32 );			// Liesst und konvertiert ein Rechteck
    void			ImplExtTextOut( BOOL bWideCharakter );

public:
                    EnhWMFReader( SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile, FilterConfigItem* pConfigItem = NULL )
                                    : WinMtf( new WinMtfOutput( rGDIMetaFile ), rStreamWMF, pConfigItem ), bRecordPath( sal_False ) {};
                    ~EnhWMFReader();

    BOOL			ReadEnhWMF();
};

//============================ WMFReader ==================================

class WMFReader : public WinMtf
{
private:

    VirtualDevice	aVDev;	// just for the purpose of "IsFontAvailable"
    UINT16			nUnitsPerInch;
    sal_uInt32		nRecSize;

    sal_uInt32		nSkipActions;
    sal_uInt32		nCurrentAction;
    sal_uInt32		nUnicodeEscapeAction;

    // Liesst den Kopf der WMF-Datei
    BOOL			ReadHeader();

    // Liesst die Parameter des Rocords mit der Funktionsnummer nFunction.
    void			ReadRecordParams( USHORT nFunction );

    Point			ReadPoint();				// Liesst und konvertiert einen Punkt (erst X dann Y)
    Point			ReadYX();					// Liesst und konvertiert einen Punkt (erst Y dann X)
    Rectangle		ReadRectangle();			// Liesst und konvertiert ein Rechteck
    Size			ReadYXExt();
    sal_Bool        GetPlaceableBound( Rectangle& rSize, SvStream* pStrm );

public:

                    WMFReader( SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile, FilterConfigItem* pConfigItem = NULL )
                        : WinMtf( new WinMtfOutput( rGDIMetaFile ), rStreamWMF, pConfigItem ) {};

    // Liesst aus dem Stream eine WMF-Datei und fuellt das GDIMetaFile
    void			ReadWMF();
};

}

#endif


