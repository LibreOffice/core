/*************************************************************************
 *
 *  $RCSfile: winmtf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#ifndef _WINMTF_HXX
#define _WINMTF_HXX

#include <math.h>
#include <stdlib.h>
#include <sot/object.hxx>
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
#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
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
#include <fltcall.hxx>
#endif


#define TRANSPARENT             1
#define OPAQUE                  2
#define BKMODE_LAST             2

/* xform stuff */
#define MWT_IDENTITY            1
#define MWT_LEFTMULTIPLY        2
#define MWT_RIGHTMULTIPLY       3

#define ENHMETA_STOCK_OBJECT    0x80000000

/* Stock Logical Objects */
#define WHITE_BRUSH             0
#define LTGRAY_BRUSH            1
#define GRAY_BRUSH              2
#define DKGRAY_BRUSH            3
#define BLACK_BRUSH             4
#define NULL_BRUSH              5
#define HOLLOW_BRUSH            NULL_BRUSH
#define WHITE_PEN               6
#define BLACK_PEN               7
#define NULL_PEN                8
#define OEM_FIXED_FONT          10
#define ANSI_FIXED_FONT         11
#define ANSI_VAR_FONT           12
#define SYSTEM_FONT             13
#define DEVICE_DEFAULT_FONT     14
#define DEFAULT_PALETTE         15
#define SYSTEM_FIXED_FONT       16


#define R2_BLACK                1
#define R2_NOTMERGEPEN          2
#define R2_MASKNOTPEN           3
#define R2_NOTCOPYPEN           4
#define R2_MASKPENNOT           5
#define R2_NOT                  6
#define R2_XORPEN               7
#define R2_NOTMASKPEN           8
#define R2_MASKPEN              9
#define R2_NOTXORPEN            10
#define R2_NOP                  11
#define R2_MERGENOTPEN          12
#define R2_COPYPEN              13
#define R2_MERGEPENNOT          14
#define R2_MERGEPEN             15
#define R2_WHITE                16

/* Mapping Modes */
#define MM_TEXT                 1
#define MM_LOMETRIC             2
#define MM_HIMETRIC             3
#define MM_LOENGLISH            4
#define MM_HIENGLISH            5
#define MM_TWIPS                6
#define MM_ISOTROPIC            7
#define MM_ANISOTROPIC          8


/* Graphics Modes */
#define GM_COMPATIBLE           1
#define GM_ADVANCED             2
#define GM_LAST                 2

/* StretchBlt() Modes */
#define BLACKONWHITE            1
#define WHITEONBLACK            2
#define COLORONCOLOR            3
#define HALFTONE                4
#define MAXSTRETCHBLTMODE       4
#define STRETCH_ANDSCANS        BLACKONWHITE
#define STRETCH_ORSCANS         WHITEONBLACK
#define STRETCH_DELETESCANS     COLORONCOLOR
#define STRETCH_HALFTONE        HALFTONE

#define LF_FACESIZE             32

struct LOGFONTW
{
    INT32       lfHeight;
    INT32       lfWidth;
    INT32       lfEscapement;
    INT32       lfOrientation;
    INT32       lfWeight;
    BYTE        lfItalic;
    BYTE        lfUnderline;
    BYTE        lfStrikeOut;
    BYTE        lfCharSet;
    BYTE        lfOutPrecision;
    BYTE        lfClipPrecision;
    BYTE        lfQuality;
    BYTE        lfPitchAndFamily;
    BYTE        lfFaceName[ LF_FACESIZE ];
};

#define TA_NOUPDATECP           0x0000
#define TA_UPDATECP             0x0001
#define TA_LEFT                 0x0000
#define TA_RIGHT                0x0002
#define TA_CENTER               0x0006
#define TA_RIGHT_CENTER  (TA_RIGHT | TA_CENTER)
#define TA_TOP                  0x0000
#define TA_BOTTOM               0x0008
#define TA_BASELINE             0x0018

#define SRCCOPY                 0x00CC0020L
#define SRCPAINT                0x00EE0086L
#define SRCAND                  0x008800C6L
#define SRCINVERT               0x00660046L
#define SRCERASE                0x00440328L
#define NOTSRCCOPY              0x00330008L
#define NOTSRCERASE             0x001100A6L
#define MERGECOPY               0x00C000CAL
#define MERGEPAINT              0x00BB0226L
#define PATCOPY                 0x00F00021L
#define PATPAINT                0x00FB0A09L
#define PATINVERT               0x005A0049L
#define DSTINVERT               0x00550009L
#define BLACKNESS               0x00000042L
#define WHITENESS               0x00FF0062L

#define PS_SOLID                0
#define PS_DASH                 1
#define PS_DOT                  2
#define PS_DASHDOT              3
#define PS_DASHDOTDOT           4
#define PS_NULL                 5
#define PS_INSIDEFRAME          6

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

/* Font Families */
#define FF_DONTCARE             0x00
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

#define FW_DONTCARE             0
#define FW_THIN                 100
#define FW_EXTRALIGHT           200
#define FW_LIGHT                300
#define FW_NORMAL               400
#define FW_MEDIUM               500
#define FW_SEMIBOLD             600
#define FW_BOLD                 700
#define FW_EXTRABOLD            800
#define FW_HEAVY                900
#define FW_ULTRALIGHT           200
#define FW_REGULAR              400
#define FW_DEMIBOLD             600
#define FW_ULTRABOLD            800
#define FW_BLACK                900

#define BS_SOLID                0
#define BS_NULL                 1
#define BS_HOLLOW               1
#define BS_HATCHED              2
#define BS_PATTERN              3
#define BS_INDEXED              4
#define BS_DIBPATTERN           5
#define BS_DIBPATTERNPT         6
#define BS_PATTERN8X8           7
#define BS_DIBPATTERN8X8        8
#define BS_MONOPATTERN          9

#define W_HS_HORIZONTAL         0
#define W_HS_VERTICAL           1
#define W_HS_FDIAGONAL          2
#define W_HS_BDIAGONAL          3
#define W_HS_CROSS              4
#define W_HS_DIAGCROSS          5

//============================ WMFReader ==================================

// -----------------------------------------------------------------------------

struct WinMtfFontStyle
{
    Font        aFont;

    WinMtfFontStyle( LOGFONTW& rLogFont );
};

// -----------------------------------------------------------------------------

struct WinMtfFillStyle
{
    Color   aFillColor;
    BOOL    bTransparent;

    WinMtfFillStyle() :
        aFillColor  ( Color( COL_BLACK ) ),
        bTransparent( FALSE )
    {
    };

    WinMtfFillStyle( const Color& rColor, BOOL bTrans = FALSE ) :
        aFillColor  ( rColor ),
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
    Color       aLineColor;
    LineInfo    aLineInfo;
    BOOL        bTransparent;

    WinMtfLineStyle() :
        aLineColor  ( COL_BLACK ),
        bTransparent( FALSE ) {};

    WinMtfLineStyle( const Color& rColor, BOOL bTrans = FALSE ) :
        aLineColor  ( rColor ),
        bTransparent( bTrans ) {};

    WinMtfLineStyle( const Color& rColor, const LineInfo rStyle, BOOL bTrans = FALSE ) :
        aLineColor  ( rColor ),
        aLineInfo   ( rStyle ),
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

struct SaveStruct
{
    UINT32              nBkMode;
    BOOL                bWinExtSet;
    long                nWinOrgX, nWinOrgY, nWinExtX, nWinExtY;
    long                nDevOrgX, nDevOrgY, nDevWidth, nDevHeight;
    WinMtfLineStyle     aLineStyle;
    WinMtfFillStyle     aFillStyle;
    Color               aBkColor;
    Color               aTextColor;
    Point               aActPos;
    BOOL                bFontChanged;
    Font                aFont;
    UINT32              nActTextAlign;
};

DECLARE_STACK( SaveStack, SaveStruct* );

// -----------------------------------------------------------------------------

struct BSaveStruct
{
    Bitmap      aBmp;
    Rectangle   aOutRect;
    UINT32      nWinRop;

                BSaveStruct( const Bitmap& rBmp, const Rectangle& rOutRect, UINT32 nRop ) :
                    aBmp( rBmp ), aOutRect( rOutRect ), nWinRop( nRop ){};
};

// -----------------------------------------------------------------------------

enum GDIObjectType { GDI_DUMMY = 0, GDI_PEN = 1, GDI_BRUSH = 2, GDI_FONT = 3, GDI_PALETTE = 4, GDI_BITMAP = 5, GDI_REGION = 6 };

struct GDIObj
{
    void*           pStyle;
    GDIObjectType   eType;

    GDIObj() :
        pStyle  ( NULL ),
        eType   ( GDI_DUMMY )
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
                    delete pStyle;
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

struct XForm
{
    float   eM11;
    float   eM12;
    float   eM21;
    float   eM22;
    float   eDx;
    float   eDy;
    XForm()
    {
        eM11 = eM12 = eM21 = eM22 = 1.0f;
        eDx = eDx = 0.0f;
    };
};

// -----------------------------------------------------------------------------

class WinMtfOutput
{
    protected:

        GDIObj**            mpGDIObj;
        UINT32              mnEntrys;
        UINT32              mnActTextAlign;         // Aktuelle Textausrichtung (im MS-Windows-Format)
        UINT32              mnBkMode;               // Aktueller Modus, wie der Hintergrund uebermalt
        Point               maActPos;               // wird. (ist gleich TRANSPARENT oder nicht)


        BOOL                mbFontChanged;
        Font                maFont;
        WinMtfLineStyle     maLineStyle;
        WinMtfFillStyle     maFillStyle;

        Color               maTextColor;
        Color               maBkColor;

        UINT32              mnRop;
        RasterOp            meRasterOp;
        BOOL                mbNopMode;

        SaveStack           maSaveStack;                // Stapel fuer aktuelle Zustaende bzw. DCs (Drawing-Contexts)

        XForm               maXForm;
        long                mnDevOrgX, mnDevOrgY;
        long                mnDevWidth, mnDevHeight;
        long                mnWinOrgX, mnWinOrgY;       // aktuelles Window-Origin
        long                mnWinExtX, mnWinExtY;       // aktuelles Window-Extent

        Point               ImplMap( const Point& rPt );
        Size                ImplMap( const Size& rSz );
        Rectangle           ImplMap( const Rectangle& rRectangle );
        void                ImplMap( Font& rFont );
        Polygon&            ImplMap( Polygon& rPolygon );
        PolyPolygon&        ImplMap( PolyPolygon& rPolyPolygon );
        void                ImplResizeObjectArry( UINT32 nNewEntry );

    public:

        virtual void        SetDevOrg( const Point& rPoint ) {};
        virtual void        SetDevOrgOffset( INT32 nXAdd, INT32 nYAdd ){};
        virtual void        SetDevExt( const Size& rSize ){};
        virtual void        ScaleDevExt( double fX, double fY ){};

        virtual void        SetWinOrg( const Point& rPoint ){};
        virtual void        SetWinOrgOffset( INT32 nX, INT32 nY ){};
        virtual void        SetWinExt( const Size& rSize ){};
        virtual void        ScaleWinExt( double fX, double fY ){};

        virtual void        SetWorldTransform( const XForm& rXForm ){};
        virtual void        ModifyWorldTransform( const XForm& rXForm, UINT32 nMode ){};

        virtual void        Push( BOOL bWinExtSet = TRUE );
        virtual void        Pop();

        void                SetBkMode( UINT32 nMode );
        void                SetBkColor( const Color& rColor );
        void                SetTextColor( const Color& rColor );
        void                SetTextAlign( UINT32 nAlign );
        virtual UINT32      SetRasterOp( UINT32 nRasterOp );
        void                CreateObject( GDIObjectType, void* pStyle = NULL );
        void                CreateObject( INT32 nIndex, GDIObjectType, void* pStyle = NULL );
        void                DeleteObject( INT32 nIndex );
        void                SelectObject( INT32 nIndex );
        CharSet             GetCharSet(){ return maFont.GetCharSet(); };

        virtual void        DrawPixel( const Point& rSource, const Color& rColor ){};
        void                MoveTo( const Point& rPoint ) { maActPos = ImplMap( rPoint ); };
        virtual void        LineTo( const Point& rPoint ){};
        virtual void        DrawLine( const Point& rSource, const Point& rDest ){};
        virtual void        DrawRect( const Rectangle& rRect, BOOL bEdge = TRUE ){};
        virtual void        DrawRoundRect( const Rectangle& rRect, const Size& rSize ){};
        virtual void        DrawEllipse( const Rectangle& rRect ){};
        virtual void        DrawArc( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle, BOOL bDrawTo = FALSE ){};
        virtual void        DrawPie( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle ){};
        virtual void        DrawChord( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle ){};
        virtual void        DrawPolygon( Polygon& rPolygon ){};
        virtual void        DrawPolyPolygon( PolyPolygon& rPolyPolygon ){};
        virtual void        DrawPolyLine( Polygon& rPolygon, BOOL bDrawTo = FALSE ){};
        virtual void        DrawPolyBezier( Polygon& rPolygin, BOOL bDrawTo = FALSE ){};
        virtual void        DrawText( Point& rPosition, String& rString, INT32* pDXArry = NULL );
        virtual void        ResolveBitmapActions( List& rSaveList ){};
        virtual void        IntersectClipRect( const Rectangle& rRectangle ){};
        virtual void        MoveClipRegion( const Size& rSize ){};

                            WinMtfOutput();
        virtual             ~WinMtfOutput();
};

// -----------------------------------------------------------------------------

class WinMtfMetaOutput : public WinMtfOutput
{
        UINT32              mnPushPopCount;             // hoehe des Stapels
        GDIMetaFile*        mpGDIMetaFile;
        WinMtfLineStyle     maLatestLineStyle;
        WinMtfFillStyle     maLatestFillStyle;

        void                UpdateLineStyle();
        void                UpdateFillStyle();

    public:

        virtual void        SetDevOrg( const Point& rPoint );
        virtual void        SetDevOrgOffset( INT32 nXAdd, INT32 nYAdd );
        virtual void        SetDevExt( const Size& rSize );
        virtual void        ScaleDevExt( double fX, double fY );

        virtual void        SetWinOrg( const Point& rPoint );
        virtual void        SetWinOrgOffset( INT32 nX, INT32 nY );
        virtual void        SetWinExt( const Size& rSize );
        virtual void        ScaleWinExt( double fX, double fY );

        virtual void        SetWorldTransform( const XForm& rXForm );
        virtual void        ModifyWorldTransform( const XForm& rXForm, UINT32 nMode );

        virtual void        Push( BOOL bWinExtSet = TRUE );
        virtual void        Pop();

        virtual UINT32      SetRasterOp( UINT32 nRasterOp );

        virtual void        LineTo( const Point& rPoint );
        virtual void        DrawPixel( const Point& rSource, const Color& rColor );
        virtual void        DrawLine( const Point& rSource, const Point& rDest );
        virtual void        DrawRect( const Rectangle& rRect, BOOL bEdge = TRUE );
        virtual void        DrawRoundRect( const Rectangle& rRect, const Size& rSize );
        virtual void        DrawEllipse( const Rectangle& rRect );
        virtual void        DrawArc( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle, BOOL bDrawTo = FALSE );
        virtual void        DrawPie( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle );
        virtual void        DrawChord( const Rectangle& rRect, const Point& rStartAngle, const Point& rEndAngle );
        virtual void        DrawPolygon( Polygon& rPolygon );
        virtual void        DrawPolyPolygon( PolyPolygon& rPolyPolygon );
        virtual void        DrawPolyLine( Polygon& rPolygon, BOOL bDrawTo = FALSE );
        virtual void        DrawPolyBezier( Polygon& rPolygin, BOOL bDrawTo = FALSE );
        virtual void        DrawText( Point& rPosition, String& rString, INT32* pDXArry = NULL );
        virtual void        ResolveBitmapActions( List& rSaveList );
        virtual void        IntersectClipRect( const Rectangle& rRectangle );
        virtual void        MoveClipRegion( const Size& rSize );


                            WinMtfMetaOutput( GDIMetaFile& rGDIMetaFile );
        virtual             ~WinMtfMetaOutput();
};

// -----------------------------------------------------------------------------

class WinMtf
{
    protected:

    WinMtfOutput*   pOut;                   //
    SvStream*       pWMF;                   // Die einzulesende WMF/EMF-Datei

    UINT32          nStartPos, nEndPos;
    List            aBmpSaveList;

    PFilterCallback pCallback;
    void*           pCallerData;

    // Sorgt dafuer, das aSampledBrush der aktuelle Brush des GDIMetaFiles ist.

    Color           ReadColor();
    BOOL            Callback( USHORT nPercent );

                    WinMtf( WinMtfOutput* pOut, SvStream& rStreamWMF, PFilterCallback pcallback, void * pcallerdata );
                    ~WinMtf();

    public:

};

//============================ EMFReader ==================================

class EnhWMFReader : public WinMtf
{

private:

    INT32           nRecordCount;

    BOOL            ReadHeader();
    Rectangle       ReadRectangle( INT32, INT32, INT32, INT32 );            // Liesst und konvertiert ein Rechteck
    void            ImplExtTextOut( BOOL bWideCharakter );

public:
                    EnhWMFReader( SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile,
                                    PFilterCallback pcallback, void * pcallerdata ) : WinMtf( new WinMtfMetaOutput( rGDIMetaFile ), rStreamWMF,
                                                                                        pcallback, pcallerdata ) {};

    BOOL            ReadEnhWMF();
};

//============================ WMFReader ==================================

class WMFReader : public WinMtf
{
private:

    BOOL            bWinExtSet;
    UINT16          nUnitsPerInch;

    // Liesst den Kopf der WMF-Datei
    BOOL            ReadHeader();

    // Liesst die Parameter des Rocords mit der Funktionsnummer nFunction.
    void            ReadRecordParams( USHORT nFunction );

    Point           ReadPoint();                // Liesst und konvertiert einen Punkt (erst X dann Y)
    Point           ReadYX();                   // Liesst und konvertiert einen Punkt (erst Y dann X)
    Rectangle       ReadRectangle();            // Liesst und konvertiert ein Rechteck
    Size            ReadYXExt();
    void            ImplSetWMFSize( const Size& rSize );

public:

                    WMFReader( SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile,
                                PFilterCallback pcallback, void * pcallerdata ) : WinMtf( new WinMtfMetaOutput( rGDIMetaFile ), rStreamWMF,
                                                                                    pcallback, pcallerdata ) {};

    // Liesst aus dem Stream eine WMF-Datei und fuellt das GDIMetaFile
    void            ReadWMF();
};

#endif


