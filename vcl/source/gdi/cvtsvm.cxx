/*************************************************************************
 *
 *  $RCSfile: cvtsvm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2000-11-15 13:01:09 $
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

#define _SV_CVTSVM_CXX
#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <string.h>

#ifndef _STACK_HXX //autogen
#include <tools/stack.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <graph.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#include <cvtsvm.hxx>

// -----------
// - Defines -
// -----------

#define CVTSVM_WRITE_SUBACTIONCOUNT 1

// -----------
// - Inlines -
// -----------

void ImplReadRect( SvStream& rIStm, Rectangle& rRect )
{
    Point aTL;
    Point aBR;

    rIStm >> aTL;
    rIStm >> aBR;

    rRect = Rectangle( aTL, aBR );
}

// ------------------------------------------------------------------------

void ImplWriteRect( SvStream& rOStm, const Rectangle& rRect )
{
    rOStm << rRect.TopLeft();
    rOStm << rRect.BottomRight();
}

// ------------------------------------------------------------------------

void ImplReadPoly( SvStream& rIStm, Polygon& rPoly )
{
    INT32   nSize;

    rIStm >> nSize;
    rPoly = Polygon( (USHORT) nSize );

    for( USHORT i = 0; i < (USHORT) nSize; i++ )
        rIStm >> rPoly[ i ];
}

// ------------------------------------------------------------------------

void ImplWritePoly( SvStream& rOStm, const Polygon& rPoly )
{
    INT32 nSize = rPoly.GetSize();

    rOStm << nSize;

    for( INT32 i = 0; i < nSize; i++ )
        rOStm << rPoly[ (USHORT) i ];
}

// ------------------------------------------------------------------------

void ImplReadPolyPoly( SvStream& rIStm, PolyPolygon& rPolyPoly )
{
    Polygon aPoly;
    INT32   nPolyCount;

    rIStm >> nPolyCount;

    for( USHORT i = 0; i < (USHORT) nPolyCount; i++ )
    {
        ImplReadPoly( rIStm, aPoly );
        rPolyPoly.Insert( aPoly );
    }
}

// ------------------------------------------------------------------------

void ImplWritePolyPolyAction( SvStream& rOStm, const PolyPolygon& rPolyPoly )
{
    const USHORT    nPoly = rPolyPoly.Count();
    USHORT          nPoints = 0;
    USHORT          n;

    for( n = 0; n < nPoly; n++ )
        nPoints += rPolyPoly[ n ].GetSize();

    rOStm << (INT16) GDI_POLYPOLYGON_ACTION;
    rOStm << (INT32) ( 8 + ( nPoly << 2 ) + ( nPoints << 3 ) );
    rOStm << (INT32) nPoly;

    for( n = 0; n < nPoly; n++ )
    {
        const Polygon&  rPoly = rPolyPoly[ n ];
        const USHORT    nSize = rPoly.GetSize();

        rOStm << (INT32) nSize;

        for( USHORT j = 0; j < nSize; j++ )
            rOStm << rPoly[ j ];
    }
}

// ------------------------------------------------------------------------

void ImplReadColor( SvStream& rIStm, Color& rColor )
{
    INT16 nVal;

    rIStm >> nVal; rColor.SetRed( (USHORT) nVal >> 8 );
    rIStm >> nVal; rColor.SetGreen( (USHORT) nVal >> 8 );
    rIStm >> nVal; rColor.SetBlue( (USHORT) nVal >> 8 );
}

// ------------------------------------------------------------------------

void ImplWriteColor( SvStream& rOStm, const Color& rColor )
{
    INT16 nVal;

    nVal = ( (INT16) rColor.GetRed() << 8 ) | rColor.GetRed();
    rOStm << nVal;

    nVal = ( (INT16) rColor.GetGreen() << 8 ) | rColor.GetGreen();
    rOStm << nVal;

    nVal = ( (INT16) rColor.GetBlue() << 8 ) | rColor.GetBlue();
    rOStm << nVal;
}

// ------------------------------------------------------------------------

void ImplReadMapMode( SvStream& rIStm, MapMode& rMapMode )
{
    Point   aOrg;
    INT32   nXNum;
    INT32   nXDenom;
    INT32   nYNum;
    INT32   nYDenom;
    INT16   nUnit;

    rIStm >> nUnit >> aOrg >> nXNum >> nXDenom >> nYNum >> nYDenom;
    rMapMode = MapMode( (MapUnit) nUnit, aOrg, Fraction( nXNum, nXDenom ), Fraction( nYNum, nYDenom ) );
}

// ------------------------------------------------------------------------

void ImplWriteMapMode( SvStream& rOStm, const MapMode& rMapMode )
{
    rOStm << (INT16) rMapMode.GetMapUnit();
    rOStm << rMapMode.GetOrigin();
    rOStm << (INT32) rMapMode.GetScaleX().GetNumerator();
    rOStm << (INT32) rMapMode.GetScaleX().GetDenominator();
    rOStm << (INT32) rMapMode.GetScaleY().GetNumerator();
    rOStm << (INT32) rMapMode.GetScaleY().GetDenominator();
}

// ------------------------------------------------------------------------

void ImplWritePushAction( SvStream& rOStm )
{
    rOStm << (INT16) GDI_PUSH_ACTION;
    rOStm << (INT32) 4;
}

// ------------------------------------------------------------------------

void ImplWritePopAction( SvStream& rOStm )
{
    rOStm << (INT16) GDI_POP_ACTION;
    rOStm << (INT32) 4;
}

// ------------------------------------------------------------------------

void ImplWriteLineColor( SvStream& rOStm, const Color& rColor, INT16 nStyle, INT32 nWidth = 0L )
{
    if( rColor.GetTransparency() > 127 )
        nStyle = 0;

    rOStm << (INT16) GDI_PEN_ACTION;
    rOStm << (INT32) 16;
    ImplWriteColor( rOStm, rColor );
    rOStm << nWidth;
    rOStm << nStyle;
}

// ------------------------------------------------------------------------

void ImplWriteFillColor( SvStream& rOStm, const Color& rColor, INT16 nStyle )
{
    rOStm << (INT16) GDI_FILLBRUSH_ACTION;
    rOStm << (INT32) 20;
    ImplWriteColor( rOStm, rColor );

    if( rColor.GetTransparency() > 127 )
        nStyle = 0;

    if( nStyle > 1 )
    {
        ImplWriteColor( rOStm, COL_WHITE );
        rOStm << nStyle;
        rOStm << (INT16) 1;
    }
    else
    {
        ImplWriteColor( rOStm, COL_BLACK );
        rOStm << nStyle;
        rOStm << (INT16) 0;
    }
}

// ------------------------------------------------------------------------

void ImplWriteFont( SvStream& rOStm, const Font& rFont,
                    rtl_TextEncoding& rActualCharSet )
{
    char    aName[32];
    short   nWeight;

    ByteString aByteName( rFont.GetName(), rOStm.GetStreamCharSet() );
    strncpy( aName, aByteName.GetBuffer(), 32 );

    switch ( rFont.GetWeight() )
    {
        case WEIGHT_THIN:
        case WEIGHT_ULTRALIGHT:
        case WEIGHT_LIGHT:
            nWeight = 1;
        break;

        case WEIGHT_NORMAL:
        case WEIGHT_MEDIUM:
            nWeight = 2;
        break;

        case WEIGHT_BOLD:
        case WEIGHT_ULTRABOLD:
        case WEIGHT_BLACK:
            nWeight = 3;
        break;

        default:
            nWeight = 0;
        break;
    }

    rOStm << (INT16) GDI_FONT_ACTION;
    rOStm << (INT32) 78;

    rActualCharSet = GetStoreCharSet( rFont.GetCharSet(), rOStm.GetVersion() );
    ImplWriteColor( rOStm, rFont.GetColor() );
    ImplWriteColor( rOStm, rFont.GetFillColor() );
    rOStm.Write( aName, 32 );
    rOStm << rFont.GetSize();
    rOStm << (INT16) 0; // no character orientation anymore
    rOStm << (INT16) rFont.GetOrientation();
    rOStm << (INT16) rActualCharSet;
    rOStm << (INT16) rFont.GetFamily();
    rOStm << (INT16) rFont.GetPitch();
    rOStm << (INT16) rFont.GetAlign();
    rOStm << (INT16) nWeight;
    rOStm << (INT16) rFont.GetUnderline();
    rOStm << (INT16) rFont.GetStrikeout();
    rOStm << (BOOL) ( rFont.GetItalic() != ITALIC_NONE );
    rOStm << rFont.IsOutline();
    rOStm << rFont.IsShadow();
    rOStm << rFont.IsTransparent();
    if ( rActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        rActualCharSet = gsl_getSystemTextEncoding();
}

// ------------------------------------------------------------------------

void ImplWriteRasterOpAction( SvStream& rOStm, INT16 nRasterOp )
{
    rOStm << (INT16) GDI_RASTEROP_ACTION << (INT32) 6 << nRasterOp;
}

// ------------------------------------------------------------------------

sal_Bool ImplWriteUnicodeComment( SvStream& rOStm, const String& rString )
{
    sal_uInt32 i, nStringLen = rString.Len();
    if ( nStringLen )
    {
        sal_uInt32  nSize = ( nStringLen << 1 ) + 4;
        sal_uInt16  nType = GDI_UNICODE_COMMENT;

        rOStm << nType << nSize;
        for ( i = 0; i < nStringLen; i++ )
        {
            sal_Unicode nUni = rString.GetChar( i );
            rOStm << nUni;
        }
    }
    return nStringLen != 0;
}

// ------------------------------------------------------------------------

void ImplReadUnicodeComment( sal_uInt32 nStrmPos, SvStream& rIStm, String& rString )
{
    sal_uInt32 nOld = rIStm.Tell();
    if ( nStrmPos )
    {
        sal_uInt16  nType;
        sal_uInt32  nStringLen, nActionSize;

        rIStm.Seek( nStrmPos );
        rIStm   >> nType
                >> nActionSize;

        nStringLen = ( nActionSize - 4 ) >> 1;

        if ( nStringLen && ( nType == GDI_UNICODE_COMMENT ) )
        {
            sal_Unicode* pBuffer = rString.AllocBuffer( nStringLen );
            while ( nStringLen-- )
                rIStm >> *pBuffer++;
        }
    }
    rIStm.Seek( nOld );
}

// ------------------------------------------------------------------------

void ImplSkipActions( SvStream& rIStm, ULONG nSkipCount )
{
    INT32 nActionSize;
    INT16 nType;

    for( ULONG i = 0UL; i < nSkipCount; i++ )
    {
        rIStm >> nType >> nActionSize;
        rIStm.SeekRel( nActionSize - 4L );
    }
}

// ----------------
// - SVMConverter -
// ----------------

SVMConverter::SVMConverter( SvStream& rStm, GDIMetaFile& rMtf, ULONG nConvertMode )
{
    if( !rStm.GetError() )
    {
        if( CONVERT_FROM_SVM1 == nConvertMode )
            ImplConvertFromSVM1( rStm, rMtf );
        else if( CONVERT_TO_SVM1 == nConvertMode )
            ImplConvertToSVM1( rStm, rMtf );
    }
}

// ------------------------------------------------------------------------

void SVMConverter::ImplConvertFromSVM1( SvStream& rIStm, GDIMetaFile& rMtf )
{
    LineInfo            aLineInfo( LINE_NONE, 0 );
    Stack               aLIStack;
    ULONG               nPos = rIStm.Tell();
    const USHORT        nOldFormat = rIStm.GetNumberFormatInt();
    rtl_TextEncoding    eActualCharSet = gsl_getSystemTextEncoding();
    BOOL                bFatLine = FALSE;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    char    aCode[ 5 ];
    MapMode aMapMode;
    Size    aPrefSz;
    INT32   nActions;
    INT16   nSize;
    INT16   nVersion;

    // Header lesen
    rIStm.Read( (char*) &aCode, sizeof( aCode ) );  // Kennung
    rIStm >> nSize;                                 // Size
    rIStm >> nVersion;                              // Version
    rIStm >> aPrefSz;                               // PrefSize
    ImplReadMapMode( rIStm, aMapMode );             // MapMode
    rIStm >> nActions;                              // Action count

    // Header-Kennung und Versionsnummer pruefen
    if( ( memcmp( aCode, "SVGDI", sizeof( aCode ) ) != 0 ) || ( nVersion != 200 ) )
    {
        rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        rIStm.SetNumberFormatInt( nOldFormat );
        rIStm.Seek( nPos );
    }
    else
    {
        Polygon     aPoly;
        Rectangle   aRect;
        Point       aPt, aPt1;
        Size        aSz;
        Color       aColor;
        INT32       nTmp, nTmp1, nActionSize;
        INT16       nType;

        sal_uInt32  nUnicodeCommentStreamPos = 0;
        sal_uInt32  nUnicodeCommentActionNumber = 0;

        rMtf.SetPrefSize( aPrefSz );
        rMtf.SetPrefMapMode( aMapMode );

        for( INT32 i = 0L; i < nActions; i++ )
        {
            rIStm >> nType;
            rIStm >> nActionSize;

            DBG_ASSERT( ( nType <= 33 ) || ( nType >= 1024 ), "Unknown GDIMetaAction while converting!" );

            switch( nType )
            {
                case( GDI_PIXEL_ACTION ):
                {
                    rIStm >> aPt;
                    ImplReadColor( rIStm, aColor );
                    rMtf.AddAction( new MetaPixelAction( aPt, aColor ) );
                }
                break;

                case( GDI_POINT_ACTION ):
                {
                    rIStm >> aPt;
                    rMtf.AddAction( new MetaPointAction( aPt ) );
                }
                break;

                case( GDI_LINE_ACTION ):
                {
                    rIStm >> aPt >> aPt1;
                    rMtf.AddAction( new MetaLineAction( aPt, aPt1, aLineInfo ) );
                }
                break;

                case( GDI_RECT_ACTION ):
                {
                    ImplReadRect( rIStm, aRect );
                    rIStm >> nTmp >> nTmp1;

                    if( nTmp || nTmp1 )
                        rMtf.AddAction( new MetaRoundRectAction( aRect, nTmp, nTmp1 ) );
                    else
                    {
                        rMtf.AddAction( new MetaRectAction( aRect ) );

                        if( bFatLine )
                            rMtf.AddAction( new MetaPolyLineAction( aRect, aLineInfo ) );
                    }
                }
                break;

                case( GDI_ELLIPSE_ACTION ):
                {
                    ImplReadRect( rIStm, aRect );

                    if( bFatLine )
                    {
                        const Polygon aPoly( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );

                        rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, FALSE ) );
                        rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                        rMtf.AddAction( new MetaPopAction() );
                        rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                    }
                    else
                        rMtf.AddAction( new MetaEllipseAction( aRect ) );
                }
                break;

                case( GDI_ARC_ACTION ):
                {
                    ImplReadRect( rIStm, aRect );
                    rIStm >> aPt >> aPt1;

                    if( bFatLine )
                    {
                        const Polygon aPoly( aRect, aPt, aPt1, POLY_ARC );

                        rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, FALSE ) );
                        rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                        rMtf.AddAction( new MetaPopAction() );
                        rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                    }
                    else
                        rMtf.AddAction( new MetaArcAction( aRect, aPt, aPt1 ) );
                }
                break;

                case( GDI_PIE_ACTION ):
                {
                    ImplReadRect( rIStm, aRect );
                    rIStm >> aPt >> aPt1;

                    if( bFatLine )
                    {
                        const Polygon aPoly( aRect, aPt, aPt1, POLY_PIE );

                        rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, FALSE ) );
                        rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                        rMtf.AddAction( new MetaPopAction() );
                        rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                    }
                    else
                        rMtf.AddAction( new MetaPieAction( aRect, aPt, aPt1 ) );
                }
                break;

                case( GDI_INVERTRECT_ACTION ):
                case( GDI_HIGHLIGHTRECT_ACTION ):
                {
                    ImplReadRect( rIStm, aRect );
                    rMtf.AddAction( new MetaPushAction( PUSH_RASTEROP ) );
                    rMtf.AddAction( new MetaRasterOpAction( ROP_INVERT ) );
                    rMtf.AddAction( new MetaRectAction( aRect ) );
                    rMtf.AddAction( new MetaPopAction() );
                }
                break;

                case( GDI_POLYLINE_ACTION ):
                {
                    ImplReadPoly( rIStm, aPoly );

                    if( bFatLine )
                        rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                    else
                        rMtf.AddAction( new MetaPolyLineAction( aPoly ) );
                }
                break;

                case( GDI_POLYGON_ACTION ):
                {
                    ImplReadPoly( rIStm, aPoly );

                    if( bFatLine )
                    {
                        rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, FALSE ) );
                        rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                        rMtf.AddAction( new MetaPopAction() );
                        rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                    }
                    else
                        rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                }
                break;

                case( GDI_POLYPOLYGON_ACTION ):
                {
                    PolyPolygon aPolyPoly;

                    ImplReadPolyPoly( rIStm, aPolyPoly );

                    if( bFatLine )
                    {
                        rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, FALSE ) );
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                        rMtf.AddAction( new MetaPopAction() );

                        for( USHORT nPoly = 0, nCount = aPolyPoly.Count(); nPoly < nCount; nPoly++ )
                            rMtf.AddAction( new MetaPolyLineAction( aPolyPoly[ nPoly ], aLineInfo ) );
                    }
                    else
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                }
                break;

                case( GDI_FONT_ACTION ):
                {
                    Font    aFont;
                    char    aName[ 32 ];
                    INT32   nWidth, nHeight;
                    INT16   nCharSet, nFamily, nPitch, nAlign, nWeight, nUnderline, nStrikeout;
                    INT16   nCharOrient, nLineOrient;
                    BOOL    bItalic, bOutline, bShadow, bTransparent;

                    ImplReadColor( rIStm, aColor ); aFont.SetColor( aColor );
                    ImplReadColor( rIStm, aColor ); aFont.SetFillColor( aColor );
                    rIStm.Read( aName, 32 );
                    aFont.SetName( UniString( aName, rIStm.GetStreamCharSet() ) );
                    rIStm >> nWidth >> nHeight;
                    rIStm >> nCharOrient >> nLineOrient;
                    rIStm >> nCharSet >> nFamily >> nPitch >> nAlign >> nWeight >> nUnderline >> nStrikeout;
                    rIStm >> bItalic >> bOutline >> bShadow >> bTransparent;

                    aFont.SetSize( Size( nWidth, nHeight ) );
                    aFont.SetCharSet( (CharSet) nCharSet );
                    aFont.SetFamily( (FontFamily) nFamily );
                    aFont.SetPitch( (FontPitch) nPitch );
                    aFont.SetAlign( (FontAlign) nAlign );
                    aFont.SetWeight( ( nWeight == 1 ) ? WEIGHT_LIGHT : ( nWeight == 2 ) ? WEIGHT_NORMAL :
                                     ( nWeight == 3 ) ? WEIGHT_BOLD : WEIGHT_DONTKNOW );
                    aFont.SetUnderline( (FontUnderline) nUnderline );
                    aFont.SetStrikeout( (FontStrikeout) nStrikeout );
                    aFont.SetItalic( bItalic ? ITALIC_NORMAL : ITALIC_NONE );
                    aFont.SetOutline( bOutline );
                    aFont.SetShadow( bShadow );
                    aFont.SetOrientation( nLineOrient );
                    aFont.SetTransparent( bTransparent );

                    eActualCharSet = aFont.GetCharSet();
                    if ( eActualCharSet == RTL_TEXTENCODING_DONTKNOW )
                        eActualCharSet = gsl_getSystemTextEncoding();
                    rMtf.AddAction( new MetaFontAction( aFont ) );
                }
                break;

                case( GDI_TEXT_ACTION ):
                {
                    ByteString  aByteStr;
                    INT32       nIndex, nLen;

                    rIStm >> aPt >> nIndex >> nLen >> nTmp;
                    rIStm.Read( aByteStr.AllocBuffer( (USHORT)nTmp ), nTmp + 1 );
                    UniString aStr( aByteStr, eActualCharSet );
                    if ( nUnicodeCommentActionNumber == i )
                        ImplReadUnicodeComment( nUnicodeCommentStreamPos, rIStm, aStr );
                    rMtf.AddAction( new MetaTextAction( aPt, aStr, (USHORT) nIndex, (USHORT) nLen ) );
                }
                break;

                case( GDI_TEXTARRAY_ACTION ):
                {
                    ByteString  aByteStr;
                    long*       pDXAry = NULL;
                    INT32       nIndex, nLen, nAryLen;

                    rIStm >> aPt >> nIndex >> nLen >> nTmp >> nAryLen;
                    rIStm.Read( aByteStr.AllocBuffer( (USHORT)nTmp ), nTmp + 1 );
                    UniString aStr( aByteStr, eActualCharSet );

                    if( nAryLen > 0L )
                    {
                        pDXAry = new long[ nAryLen ];

                        for( long i = 0L; i < nAryLen; i++ )
                            rIStm >> nTmp, pDXAry[ i ] = nTmp;
                    }
                    if ( nUnicodeCommentActionNumber == i )
                        ImplReadUnicodeComment( nUnicodeCommentStreamPos, rIStm, aStr );
                    rMtf.AddAction( new MetaTextArrayAction( aPt, aStr, pDXAry, (USHORT) nIndex, (USHORT) nLen ) );

                    if( pDXAry )
                        delete[] pDXAry;
                }
                break;

                case( GDI_STRETCHTEXT_ACTION ):
                {
                    ByteString  aByteStr;
                    INT32       nIndex, nLen, nWidth;

                    rIStm >> aPt >> nIndex >> nLen >> nTmp >> nWidth;
                    rIStm.Read( aByteStr.AllocBuffer( (USHORT)nTmp ), nTmp + 1 );
                    UniString aStr( aByteStr, eActualCharSet );
                    if ( nUnicodeCommentActionNumber == i )
                        ImplReadUnicodeComment( nUnicodeCommentStreamPos, rIStm, aStr );
                    rMtf.AddAction( new MetaStretchTextAction( aPt, nWidth, aStr, (USHORT) nIndex, (USHORT) nLen ) );
                }
                break;

                case( GDI_BITMAP_ACTION ):
                {
                    Bitmap aBmp;

                    rIStm >> aPt >> aBmp;
                    rMtf.AddAction( new MetaBmpAction( aPt, aBmp ) );
                }
                break;

                case( GDI_BITMAPSCALE_ACTION ):
                {
                    Bitmap aBmp;

                    rIStm >> aPt >> aSz >> aBmp;
                    rMtf.AddAction( new MetaBmpScaleAction( aPt, aSz, aBmp ) );
                }
                break;

                case( GDI_BITMAPSCALEPART_ACTION ):
                {
                    Bitmap  aBmp;
                    Size    aSz2;

                    rIStm >> aPt >> aSz >> aPt1 >> aSz2 >> aBmp;
                    rMtf.AddAction( new MetaBmpScalePartAction( aPt, aSz, aPt1, aSz2, aBmp ) );
                }
                break;

                case( GDI_PEN_ACTION ):
                {
                    INT32 nPenWidth;
                    INT16 nPenStyle;

                    ImplReadColor( rIStm, aColor );
                    rIStm >> nPenWidth >> nPenStyle;

                    aLineInfo.SetStyle( nPenStyle ? LINE_SOLID : LINE_NONE );
                    aLineInfo.SetWidth( nPenWidth );
                    bFatLine = nPenStyle && !aLineInfo.IsDefault();

                    rMtf.AddAction( new MetaLineColorAction( aColor, nPenStyle != 0 ) );
                }
                break;

                case( GDI_FILLBRUSH_ACTION ):
                {
                    INT16 nBrushStyle;

                    ImplReadColor( rIStm, aColor );
                    rIStm.SeekRel( 6L );
                    rIStm >> nBrushStyle;
                    rMtf.AddAction( new MetaFillColorAction( aColor, nBrushStyle != 0 ) );
                    rIStm.SeekRel( 2L );
                }
                break;

                case( GDI_MAPMODE_ACTION ):
                {
                    ImplReadMapMode( rIStm, aMapMode );
                    rMtf.AddAction( new MetaMapModeAction( aMapMode ) );
                }
                break;

                case( GDI_CLIPREGION_ACTION ):
                {
                    Region  aRegion;
                    INT16   nRegType;
                    INT16   bIntersect;
                    BOOL    bClip = FALSE;

                    rIStm >> nRegType >> bIntersect;
                    ImplReadRect( rIStm, aRect );

                    switch( nRegType )
                    {
                        case( 0 ):
                        break;

                        case( 1 ):
                        {
                            Rectangle aRegRect;

                            ImplReadRect( rIStm, aRegRect );
                            aRegion = Region( aRegRect );
                            bClip = TRUE;
                        }
                        break;

                        case( 2 ):
                        {
                            ImplReadPoly( rIStm, aPoly );
                            aRegion = Region( aPoly );
                            bClip = TRUE;
                        }
                        break;

                        case( 3 ):
                        {
                            PolyPolygon aPolyPoly;
                            INT32       nPolyCount;

                            rIStm >> nPolyCount;

                            for( USHORT i = 0; i < (USHORT) nPolyCount; i++ )
                            {
                                ImplReadPoly( rIStm, aPoly );
                                aPolyPoly.Insert( aPoly );
                            }

                            aRegion = Region( aPolyPoly );
                            bClip = TRUE;
                        }
                        break;
                    }

                    if( bIntersect )
                        aRegion.Intersect( aRect );

                    rMtf.AddAction( new MetaClipRegionAction( aRegion, bClip ) );
                }
                break;

                case( GDI_MOVECLIPREGION_ACTION ):
                {
                    rIStm >> nTmp >> nTmp1;
                    rMtf.AddAction( new MetaMoveClipRegionAction( nTmp, nTmp1 ) );
                }
                break;

                case( GDI_ISECTCLIPREGION_ACTION ):
                {
                    ImplReadRect( rIStm, aRect );
                    rMtf.AddAction( new MetaISectRectClipRegionAction( aRect ) );
                }
                break;

                case( GDI_RASTEROP_ACTION ):
                {
                    RasterOp    eRasterOp;
                    INT16       nRasterOp;

                    rIStm >> nRasterOp;

                    switch( nRasterOp )
                    {
                        case( 1 ):
                            eRasterOp = ROP_INVERT;
                        break;

                        case( 4 ):
                        case( 5 ):
                            eRasterOp = ROP_XOR;
                        break;

                        default:
                            eRasterOp = ROP_OVERPAINT;
                        break;
                    }

                    rMtf.AddAction( new MetaRasterOpAction( eRasterOp ) );
                }
                break;

                case( GDI_PUSH_ACTION ):
                {
                    aLIStack.Push( new LineInfo( aLineInfo ) );
                    rMtf.AddAction( new MetaPushAction( PUSH_ALL ) );
                }
                break;

                case( GDI_POP_ACTION ):
                {

                    LineInfo* pLineInfo = (LineInfo*) aLIStack.Pop();

                    // restore line info
                    if( pLineInfo )
                    {
                        aLineInfo = *pLineInfo;
                        delete pLineInfo;
                        bFatLine = ( LINE_NONE != aLineInfo.GetStyle() ) && !aLineInfo.IsDefault();
                    }

                    rMtf.AddAction( new MetaPopAction() );
                }
                break;

                case( GDI_GRADIENT_ACTION ):
                {
                    Color   aStartCol;
                    Color   aEndCol;
                    INT16   nStyle;
                    INT16   nAngle;
                    INT16   nBorder;
                    INT16   nOfsX;
                    INT16   nOfsY;
                    INT16   nIntensityStart;
                    INT16   nIntensityEnd;

                    ImplReadRect( rIStm, aRect );
                    rIStm >> nStyle;
                    ImplReadColor( rIStm, aStartCol );
                    ImplReadColor( rIStm, aEndCol );
                    rIStm >> nAngle >> nBorder >> nOfsX >> nOfsY >> nIntensityStart >> nIntensityEnd;

                    Gradient aGrad( (GradientStyle) nStyle, aStartCol, aEndCol );

                    aGrad.SetAngle( nAngle );
                    aGrad.SetBorder( nBorder );
                    aGrad.SetOfsX( nOfsX );
                    aGrad.SetOfsY( nOfsY );
                    aGrad.SetStartIntensity( nIntensityStart );
                    aGrad.SetEndIntensity( nIntensityEnd );
                    rMtf.AddAction( new MetaGradientAction( aRect, aGrad ) );
                }
                break;

                case( GDI_TRANSPARENT_COMMENT ):
                {
                    PolyPolygon aPolyPoly;
                    INT32       nFollowingActionCount;
                    INT16       nTrans;

                    rIStm >> aPolyPoly >> nTrans >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaTransparentAction( aPolyPoly, nTrans ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_FLOATTRANSPARENT_COMMENT ):
                {
                    GDIMetaFile aMtf;
                    Point       aPos;
                    Size        aSize;
                    Gradient    aGradient;
                    INT32       nFollowingActionCount;

                    rIStm >> aMtf >> aPos >> aSize >> aGradient >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaFloatTransparentAction( aMtf, aPos, aSize, aGradient ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_HATCH_COMMENT ):
                {
                    PolyPolygon aPolyPoly;
                    Hatch       aHatch;
                    INT32       nFollowingActionCount;

                    rIStm >> aPolyPoly >> aHatch >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaHatchAction( aPolyPoly, aHatch ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_REFPOINT_COMMENT ):
                {
                    Point   aRefPoint;
                    BOOL    bSet;
                    INT32   nFollowingActionCount;

                    rIStm >> aRefPoint >> bSet >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaRefPointAction( aRefPoint, bSet ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_TEXTLINECOLOR_COMMENT ):
                {
                    Color   aColor;
                    BOOL    bSet;
                    INT32   nFollowingActionCount;

                    rIStm >> aColor >> bSet >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaTextLineColorAction( aColor, bSet ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_TEXTLINE_COMMENT ):
                {
                    Point   aStartPt;
                    long    nWidth;
                    ULONG   nStrikeout;
                    ULONG   nUnderline;
                    INT32   nFollowingActionCount;

                    rIStm >> aStartPt >> nWidth >> nStrikeout >> nUnderline >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaTextLineAction( aStartPt, nWidth,
                                                            (FontStrikeout) nStrikeout,
                                                            (FontUnderline) nUnderline ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_GRADIENTEX_COMMENT ):
                {
                    PolyPolygon aPolyPoly;
                    Gradient    aGradient;
                    INT32       nFollowingActionCount;

                    rIStm >> aPolyPoly >> aGradient >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaGradientExAction( aPolyPoly, aGradient ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case( GDI_COMMENT_COMMENT ):
                {
                    ByteString  aComment;
                    long        nValue;
                    ULONG       nDataSize;
                    BYTE*       pData;
                    INT32       nFollowingActionCount;

                    rIStm >> aComment >> nValue >> nDataSize;

                    if( nDataSize )
                    {
                        pData = new BYTE[ nDataSize ];
                        rIStm.Read( pData, nDataSize );
                    }
                    else
                        pData = NULL;

                    rIStm >> nFollowingActionCount;
                    ImplSkipActions( rIStm, nFollowingActionCount );
                    rMtf.AddAction( new MetaCommentAction( aComment, nValue, pData, nDataSize ) );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    i += nFollowingActionCount;
#endif
                }
                break;

                case ( GDI_UNICODE_COMMENT ):
                {
                    nUnicodeCommentActionNumber = i + 1;
                    nUnicodeCommentStreamPos = rIStm.Tell() - 6;
                    rIStm.SeekRel( nActionSize - 4 );
                }
                break;

                default:
                    rIStm.SeekRel( nActionSize - 4L );
                break;
            }
        }

        // cleanup push-pop stack if neccessary
        for( void* pLineInfo = aLIStack.Pop(); pLineInfo; pLineInfo = aLIStack.Pop() )
            delete (LineInfo*) pLineInfo;
    }

    rIStm.SetNumberFormatInt( nOldFormat );
}

// ------------------------------------------------------------------------

void SVMConverter::ImplConvertToSVM1( SvStream& rOStm, GDIMetaFile& rMtf )
{
    ULONG               nPos;
    ULONG               nCountPos;
    Font                aSaveFont;
    const ULONG         nActionCount = rMtf.GetActionCount();
    const USHORT        nOldFormat = rOStm.GetNumberFormatInt();
    rtl_TextEncoding    eActualCharSet = gsl_getSystemTextEncoding();
    const Size          aPrefSize( rMtf.GetPrefSize() );
    BOOL                bRop_0_1 = FALSE;
    VirtualDevice       aSaveVDev;
    Color               aLineCol( COL_BLACK );
    Stack               aLineColStack;

    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    //MagicCode schreiben
    rOStm << "SVGDI";                                   // Kennung
    nPos = rOStm.Tell();
    rOStm << (INT16) 42;                                // HeaderSize
    rOStm << (INT16) 200;                               // VERSION
    rOStm << (INT32) aPrefSize.Width();
    rOStm << (INT32) aPrefSize.Height();
    ImplWriteMapMode( rOStm, rMtf.GetPrefMapMode() );

    // ActionCount wird spaeter geschrieben
    nCountPos = rOStm.Tell();
    rOStm.SeekRel( 4L );

    const INT32 nActCount = ImplWriteActions( rOStm, rMtf, aSaveVDev, bRop_0_1, aLineCol, aLineColStack, eActualCharSet );
    const ULONG nActPos = rOStm.Tell();

    rOStm.Seek( nCountPos );
    rOStm << nActCount;
    rOStm.Seek( nActPos );
    rOStm.SetNumberFormatInt( nOldFormat );

    // cleanup push-pop stack if neccessary
    for( void* pCol = aLineColStack.Pop(); pCol; pCol = aLineColStack.Pop() )
        delete (Color*) pCol;
}

// ------------------------------------------------------------------------

ULONG SVMConverter::ImplWriteActions( SvStream& rOStm, GDIMetaFile& rMtf,
                                      VirtualDevice& rSaveVDev, BOOL& rRop_0_1,
                                      Color& rLineCol, Stack& rLineColStack,
                                      rtl_TextEncoding& rActualCharSet )
{
    ULONG nCount = 0;
    for( ULONG i = 0, nActionCount = rMtf.GetActionCount(); i < nActionCount; i++ )
    {
        const MetaAction* pAction = rMtf.GetAction( i );

        switch( pAction->GetType() )
        {
            case( META_PIXEL_ACTION ):
            {
                MetaPixelAction* pAct = (MetaPixelAction*) pAction;

                rOStm << (INT16) GDI_PIXEL_ACTION;
                rOStm << (INT32) 18;
                rOStm << pAct->GetPoint();
                ImplWriteColor( rOStm, pAct->GetColor() );
                nCount++;
            }
            break;

            case( META_POINT_ACTION ):
            {
                MetaPointAction* pAct = (MetaPointAction*) pAction;

                rOStm << (INT16) GDI_POINT_ACTION;
                rOStm << (INT32) 12;
                rOStm << pAct->GetPoint();
                nCount++;
            }
            break;

            case( META_LINE_ACTION ):
            {
                MetaLineAction* pAct = (MetaLineAction*) pAction;
                const LineInfo& rInfo = pAct->GetLineInfo();
                const BOOL      bFatLine = ( !rInfo.IsDefault() && ( LINE_NONE != rInfo.GetStyle() ) );

                if( bFatLine )
                {
                    ImplWritePushAction( rOStm );
                    ImplWriteLineColor( rOStm, rLineCol, 1, rInfo.GetWidth() );
                }

                rOStm << (INT16) GDI_LINE_ACTION;
                rOStm << (INT32) 20;
                rOStm << pAct->GetStartPoint();
                rOStm << pAct->GetEndPoint();
                nCount++;

                if( bFatLine )
                {
                    ImplWritePopAction( rOStm );
                    nCount += 3;
                }
            }
            break;

            case( META_RECT_ACTION ):
            {
                MetaRectAction* pAct = (MetaRectAction*) pAction;

                rOStm << (INT16) GDI_RECT_ACTION;
                rOStm << (INT32) 28;
                ImplWriteRect( rOStm, pAct->GetRect() );
                rOStm << (INT32) 0;
                rOStm << (INT32) 0;
                nCount++;
            }
            break;

            case( META_ROUNDRECT_ACTION ):
            {
                MetaRoundRectAction* pAct = (MetaRoundRectAction*) pAction;

                rOStm << (INT16) GDI_RECT_ACTION;
                rOStm << (INT32) 28;
                ImplWriteRect( rOStm, pAct->GetRect() );
                rOStm << (INT32) pAct->GetHorzRound();
                rOStm << (INT32) pAct->GetVertRound();
                nCount++;
            }
            break;

            case( META_ELLIPSE_ACTION ):
            {
                MetaEllipseAction* pAct = (MetaEllipseAction*) pAction;

                rOStm << (INT16) GDI_ELLIPSE_ACTION;
                rOStm << (INT32) 20;
                ImplWriteRect( rOStm, pAct->GetRect() );
                nCount++;
            }
            break;

            case( META_ARC_ACTION ):
            {
                MetaArcAction* pAct = (MetaArcAction*) pAction;

                rOStm << (INT16) GDI_ARC_ACTION;
                rOStm << (INT32) 36;
                ImplWriteRect( rOStm, pAct->GetRect() );
                rOStm << pAct->GetStartPoint();
                rOStm << pAct->GetEndPoint();
                nCount++;
            }
            break;

            case( META_PIE_ACTION ):
            {
                MetaPieAction* pAct = (MetaPieAction*) pAction;

                rOStm << (INT16) GDI_PIE_ACTION;
                rOStm << (INT32) 36;
                ImplWriteRect( rOStm, pAct->GetRect() );
                rOStm << pAct->GetStartPoint();
                rOStm << pAct->GetEndPoint();
                nCount++;
            }
            break;

            case( META_CHORD_ACTION ):
            {
                MetaChordAction*    pAct = (MetaChordAction*) pAction;
                Polygon             aChordPoly( pAct->GetRect(), pAct->GetStartPoint(),
                                                pAct->GetEndPoint(), POLY_CHORD );
                const USHORT        nPoints = aChordPoly.GetSize();

                rOStm << (INT16) GDI_POLYGON_ACTION;
                rOStm << (INT32) ( 8 + ( nPoints << 3 ) );
                rOStm << (INT32) nPoints;

                for( USHORT n = 0; n < nPoints; n++ )
                    rOStm << aChordPoly[ n ];

                nCount++;
            }
            break;

            case( META_POLYLINE_ACTION ):
            {
                MetaPolyLineAction* pAct = (MetaPolyLineAction*) pAction;
                const Polygon&      rPoly = pAct->GetPolygon();
                const LineInfo&     rInfo = pAct->GetLineInfo();
                const USHORT        nPoints = rPoly.GetSize();
                const BOOL          bFatLine = ( !rInfo.IsDefault() && ( LINE_NONE != rInfo.GetStyle() ) );

                if( bFatLine )
                {
                    ImplWritePushAction( rOStm );
                    ImplWriteLineColor( rOStm, rLineCol, 1, rInfo.GetWidth() );
                }

                rOStm << (INT16) GDI_POLYLINE_ACTION;
                rOStm << (INT32) ( 8 + ( nPoints << 3 ) );
                rOStm << (INT32) nPoints;

                for( USHORT n = 0; n < nPoints; n++ )
                    rOStm << rPoly[ n ];

                nCount++;

                if( bFatLine )
                {
                    ImplWritePopAction( rOStm );
                    nCount += 3;
                }
            }
            break;

            case( META_POLYGON_ACTION ):
            {
                MetaPolygonAction*  pAct = (MetaPolygonAction*) pAction;
                const Polygon&      rPoly = pAct->GetPolygon();
                const USHORT        nPoints = rPoly.GetSize();

                rOStm << (INT16) GDI_POLYGON_ACTION;
                rOStm << (INT32) ( 8 + ( nPoints << 3 ) );
                rOStm << (INT32) nPoints;

                for( USHORT n = 0; n < nPoints; n++ )
                    rOStm << rPoly[ n ];

                nCount++;
            }
            break;

            case( META_POLYPOLYGON_ACTION ):
            {
                MetaPolyPolygonAction* pAct = (MetaPolyPolygonAction*) pAction;
                ImplWritePolyPolyAction( rOStm, pAct->GetPolyPolygon() );
                nCount++;
            }
            break;

            case( META_TEXT_ACTION ):
            {
                MetaTextAction* pAct = (MetaTextAction*) pAction;
                String          aUniText( pAct->GetText() );
                ByteString      aText( aUniText, rActualCharSet );
                const ULONG     nStrLen = aText.Len();

                if ( ImplWriteUnicodeComment( rOStm, aUniText ) )
                    nCount++;

                rOStm << (INT16) GDI_TEXT_ACTION;
                rOStm << (INT32) ( 24 + ( nStrLen + 1 ) );
                rOStm << pAct->GetPoint();
                rOStm << (INT32) pAct->GetIndex();
                rOStm << (INT32) pAct->GetLen();
                rOStm << (INT32) nStrLen;
                rOStm.Write( aText.GetBuffer(), nStrLen + 1 );
                nCount++;
            }
            break;

            case( META_TEXTARRAY_ACTION ):
            {
                MetaTextArrayAction*    pAct = (MetaTextArrayAction*)pAction;
                String                  aUniText( pAct->GetText() );
                ByteString              aText( aUniText, rActualCharSet );
                ULONG                   nAryLen;
                ULONG                   nLen = pAct->GetLen();
                const ULONG             nTextLen = aText.Len();
                long*                   pDXArray = pAct->GetDXArray();

                if ( ImplWriteUnicodeComment( rOStm, aUniText ) )
                    nCount++;

                if( ( nLen + pAct->GetIndex() ) > nTextLen )
                {
                    if( pAct->GetIndex() <= nTextLen )
                        nLen = nTextLen - pAct->GetIndex();
                    else
                        nLen = 0UL;
                }

                if( !pDXArray || !nLen )
                    nAryLen = 0;
                else
                    nAryLen = nLen - 1;

                rOStm << (INT16) GDI_TEXTARRAY_ACTION;
                rOStm << (INT32) ( 28 + ( nLen + 1 ) + ( nAryLen * 4 ) );
                rOStm << pAct->GetPoint();
                rOStm << (INT32) 0;
                rOStm << (INT32) nLen;
                rOStm << (INT32) nLen;
                rOStm << (INT32) nAryLen;
                rOStm.Write( aText.GetBuffer()+pAct->GetIndex(), nLen + 1 );

                for( ULONG n = 0UL ; n < nAryLen; n++ )
                    rOStm << (INT32) pDXArray[ n ];

                nCount++;
            }
            break;

            case( META_STRETCHTEXT_ACTION ):
            {
                MetaStretchTextAction*  pAct = (MetaStretchTextAction*) pAction;
                String                  aUniText( pAct->GetText() );
                ByteString              aText( aUniText, rActualCharSet );
                const ULONG             nStrLen = aText.Len();

                if ( ImplWriteUnicodeComment( rOStm, aUniText ) )
                    nCount++;

                rOStm << (INT16) GDI_STRETCHTEXT_ACTION;
                rOStm << (INT32) ( 28 + ( nStrLen + 1 ) );
                rOStm << pAct->GetPoint();
                rOStm << (INT32) pAct->GetIndex();
                rOStm << (INT32) pAct->GetLen();
                rOStm << (INT32) nStrLen;
                rOStm << (INT32) pAct->GetWidth();
                rOStm.Write( aText.GetBuffer(), nStrLen + 1 );
                nCount++;
            }
            break;

            case( META_BMP_ACTION ):
            {
                MetaBmpAction* pAct = (MetaBmpAction*) pAction;

                rOStm << (INT16) GDI_BITMAP_ACTION;
                rOStm << (INT32) 12;
                rOStm << pAct->GetPoint();
                rOStm << pAct->GetBitmap();
                nCount++;
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;

                rOStm << (INT16) GDI_BITMAPSCALE_ACTION;
                rOStm << (INT32) 20;
                rOStm << pAct->GetPoint();
                rOStm << pAct->GetSize();
                rOStm << pAct->GetBitmap();
                nCount++;
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;

                rOStm << (INT16) GDI_BITMAPSCALEPART_ACTION;
                rOStm << (INT32) 36;
                rOStm << pAct->GetDestPoint();
                rOStm << pAct->GetDestSize();
                rOStm << pAct->GetSrcPoint();
                rOStm << pAct->GetSrcSize();
                rOStm << pAct->GetBitmap();
                nCount++;
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                MetaBmpExAction*    pAct = (MetaBmpExAction*) pAction;
                const Bitmap        aBmp( Graphic( pAct->GetBitmapEx() ).GetBitmap() );

                rOStm << (INT16) GDI_BITMAP_ACTION;
                rOStm << (INT32) 12;
                rOStm << pAct->GetPoint();
                rOStm << aBmp;
                nCount++;
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                MetaBmpExScaleAction*   pAct = (MetaBmpExScaleAction*) pAction;
                const Bitmap            aBmp( Graphic( pAct->GetBitmapEx() ).GetBitmap() );

                rOStm << (INT16) GDI_BITMAPSCALE_ACTION;
                rOStm << (INT32) 20;
                rOStm << pAct->GetPoint();
                rOStm << pAct->GetSize();
                rOStm << aBmp;
                nCount++;
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                MetaBmpExScalePartAction* pAct = (MetaBmpExScalePartAction*) pAction;
                const Bitmap            aBmp( Graphic( pAct->GetBitmapEx() ).GetBitmap() );

                rOStm << (INT16) GDI_BITMAPSCALEPART_ACTION;
                rOStm << (INT32) 36;
                rOStm << pAct->GetDestPoint();
                rOStm << pAct->GetDestSize();
                rOStm << pAct->GetSrcPoint();
                rOStm << pAct->GetSrcSize();
                rOStm << aBmp;
                nCount++;
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                MetaGradientAction* pAct = (MetaGradientAction*) pAction;
                const Gradient&     rGrad = pAct->GetGradient();

                rOStm << (INT16) GDI_GRADIENT_ACTION;
                rOStm << (INT32) 46;
                ImplWriteRect( rOStm, pAct->GetRect() );
                rOStm << (INT16) rGrad.GetStyle();
                ImplWriteColor( rOStm, rGrad.GetStartColor() );
                ImplWriteColor( rOStm, rGrad.GetEndColor() );
                rOStm << (INT16) rGrad.GetAngle();
                rOStm << (INT16) rGrad.GetBorder();
                rOStm << (INT16) rGrad.GetOfsX();
                rOStm << (INT16) rGrad.GetOfsY();
                rOStm << (INT16) rGrad.GetStartIntensity();
                rOStm << (INT16) rGrad.GetEndIntensity();
                nCount++;
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                const MetaGradientExAction* pA = (MetaGradientExAction*) pAction;
                ULONG                       nOldPos, nNewPos;

                // write RefPoint comment
                rOStm << (INT16) GDI_GRADIENTEX_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write data
                rOStm << pA->GetPolyPolygon() << pA->GetGradient();
                rOStm << (INT32) 0; // number of actions that follow this comment

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos );
                rOStm.Seek( nNewPos );

                nCount++;
            }
            break;

            case( META_WALLPAPER_ACTION ):
            {
                MetaWallpaperAction*    pAct = (MetaWallpaperAction*) pAction;
                const Color&            rColor = pAct->GetWallpaper().GetColor();

                ImplWritePushAction( rOStm );
                ImplWriteLineColor( rOStm, rColor, 1 );
                ImplWriteFillColor( rOStm, rColor, 1 );

                rOStm << (INT16) GDI_RECT_ACTION;
                rOStm << (INT32) 28;
                ImplWriteRect( rOStm, pAct->GetRect() );
                rOStm << (INT32) 0;
                rOStm << (INT32) 0;

                ImplWritePopAction( rOStm );
                nCount += 5;
            }
            break;

            case( META_CLIPREGION_ACTION ):
            {
                MetaClipRegionAction*   pAct = (MetaClipRegionAction*) pAction;
                const Region&           rRegion = pAct->GetRegion();
                Rectangle               aClipRect;

                rOStm << (INT16) GDI_CLIPREGION_ACTION;
                rOStm << (INT32) 24;

                if( pAct->IsClipping() )
                {
                    aClipRect = rRegion.GetBoundRect();
                    rOStm << (INT16) 1;
                }
                else
                    rOStm << (INT16) 0;

                rOStm << (INT16) 0;
                ImplWriteRect( rOStm, aClipRect );

                if( pAct->IsClipping() )
                    ImplWriteRect( rOStm, aClipRect );

                nCount++;
            }
            break;

            case( META_ISECTRECTCLIPREGION_ACTION ):
            {
                MetaISectRectClipRegionAction* pAct = (MetaISectRectClipRegionAction*) pAction;

                rOStm << (INT16) GDI_ISECTCLIPREGION_ACTION;
                rOStm << (INT32) 20;
                rOStm << pAct->GetRect();
                nCount++;
            }
            break;

            case( META_MOVECLIPREGION_ACTION ):
            {
                MetaMoveClipRegionAction* pAct = (MetaMoveClipRegionAction*) pAction;

                rOStm << (INT16) GDI_MOVECLIPREGION_ACTION;
                rOStm << (INT32) 12;
                rOStm << (INT32) pAct->GetHorzMove();
                rOStm << (INT32) pAct->GetVertMove();
                nCount++;
            }
            break;

            case( META_LINECOLOR_ACTION ):
            {
                MetaLineColorAction* pAct = (MetaLineColorAction*) pAction;
                ImplWriteLineColor( rOStm, rLineCol = pAct->GetColor(), pAct->IsSetting() ? 1 : 0 );
                nCount++;
            }
            break;

            case( META_FILLCOLOR_ACTION ):
            {
                MetaFillColorAction* pAct = (MetaFillColorAction*) pAction;
                ImplWriteFillColor( rOStm, pAct->GetColor(), pAct->IsSetting() ? 1 : 0 );
                nCount++;
            }
            break;

            case( META_FONT_ACTION ):
            {
                rSaveVDev.SetFont( ( (MetaFontAction*) pAction )->GetFont() );
                ImplWriteFont( rOStm, rSaveVDev.GetFont(), rActualCharSet );
                nCount++;
            }
            break;

            case( META_TEXTCOLOR_ACTION ):
            {
                Font aSaveFont( rSaveVDev.GetFont() );

                aSaveFont.SetColor( ( (MetaTextColorAction*) pAction )->GetColor() );
                rSaveVDev.SetFont( aSaveFont );
                ImplWriteFont( rOStm, rSaveVDev.GetFont(), rActualCharSet );
                nCount++;
            }
            break;

            case( META_TEXTFILLCOLOR_ACTION ):
            {
                MetaTextFillColorAction*    pAct = (MetaTextFillColorAction*) pAction;
                Font                        aSaveFont( rSaveVDev.GetFont() );

                if( pAct->IsSetting() )
                    aSaveFont.SetFillColor( pAct->GetColor() );
                else
                    aSaveFont.SetFillColor( Color( COL_TRANSPARENT ) );

                rSaveVDev.SetFont( aSaveFont );
                ImplWriteFont( rOStm, rSaveVDev.GetFont(), rActualCharSet );
                nCount++;
            }
            break;

            case( META_TEXTALIGN_ACTION ):
            {
                Font aSaveFont( rSaveVDev.GetFont() );

                aSaveFont.SetAlign( ( (MetaTextAlignAction*) pAction )->GetTextAlign() );
                rSaveVDev.SetFont( aSaveFont );
                ImplWriteFont( rOStm, rSaveVDev.GetFont(), rActualCharSet );
                nCount++;
            }
            break;

            case( META_MAPMODE_ACTION ):
            {
                MetaMapModeAction* pAct = (MetaMapModeAction*) pAction;

                rOStm << (INT16) GDI_MAPMODE_ACTION;
                rOStm << (INT32) 30;
                ImplWriteMapMode( rOStm, pAct->GetMapMode() );
                nCount++;
            }
            break;

            case( META_PUSH_ACTION ):
            {
                ImplWritePushAction( rOStm );
                rLineColStack.Push( new Color( rLineCol ) );
                rSaveVDev.Push();
                nCount++;
            }
            break;

            case( META_POP_ACTION ):
            {
                Color* pCol = (Color*) rLineColStack.Pop();

                if( pCol )
                {
                    rLineCol = *pCol;
                    delete pCol;
                }

                ImplWritePopAction( rOStm );
                rSaveVDev.Pop();
                nCount++;
            }
            break;

            case( META_RASTEROP_ACTION ):
            {
                MetaRasterOpAction* pAct = (MetaRasterOpAction*) pAction;

                if( ( pAct->GetRasterOp() != ROP_0 ) && ( pAct->GetRasterOp() != ROP_1 ) )
                {
                    INT16 nRasterOp;

                    // Falls vorher ROP_0/1 gesetzt war, alten
                    // Zustand durch Pop erst wieder herstellen
                    if( rRop_0_1 )
                    {
                        ImplWritePopAction( rOStm );
                        rSaveVDev.Pop();
                        rRop_0_1 = FALSE;
                        nCount++;
                    }

                    switch( pAct->GetRasterOp() )
                    {
                        case( ROP_OVERPAINT ) : nRasterOp = 0; break;
                        case( ROP_XOR ) :       nRasterOp = 4; break;
                        case( ROP_INVERT ):     nRasterOp = 1; break;
                        default:                nRasterOp = 0; break;
                    }

                    ImplWriteRasterOpAction( rOStm, nRasterOp );
                    nCount++;
                }
                else
                {
                    ImplWritePushAction( rOStm );
                    rSaveVDev.Push();

                    if( pAct->GetRasterOp() == ROP_0 )
                    {
                        ImplWriteLineColor( rOStm, COL_BLACK, 1 );
                        ImplWriteFillColor( rOStm, COL_BLACK, 1 );
                    }
                    else
                    {
                        ImplWriteLineColor( rOStm, COL_WHITE, 1 );
                        ImplWriteFillColor( rOStm, COL_WHITE, 1 );
                    }

                    ImplWriteRasterOpAction( rOStm, 0 );
                    rRop_0_1 = TRUE;
                    nCount += 4;
                }
            }
            break;

            case( META_TRANSPARENT_ACTION ):
            {
                const PolyPolygon&  rPolyPoly = ( (MetaTransparentAction*) pAction )->GetPolyPolygon();
                const INT16         nTrans = ( (MetaTransparentAction*) pAction )->GetTransparence();
                const INT16         nBrushStyle = ( nTrans < 38 ) ? 8 : ( nTrans < 63 ) ? 9 : 10;
                ULONG               nOldPos, nNewPos;

                // write transparence comment
                rOStm << (INT16) GDI_TRANSPARENT_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write comment data
                rOStm << rPolyPoly;
                rOStm << nTrans;
                rOStm << (INT32) 15; // number of actions that follow this comment

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos );
                rOStm.Seek( nNewPos );

                {
                    // write actions for transparence
                    ImplWritePushAction( rOStm );
                    {
                        ImplWriteRasterOpAction( rOStm, 4 );
                        ImplWritePolyPolyAction( rOStm, rPolyPoly );

                        ImplWritePushAction( rOStm );
                        {
                            ImplWriteRasterOpAction( rOStm, 2 );
                            ImplWriteFillColor( rOStm, COL_BLACK, nBrushStyle );
                            ImplWritePolyPolyAction( rOStm, rPolyPoly );
                        }
                        ImplWritePopAction( rOStm );

                        ImplWriteRasterOpAction( rOStm, 4 );
                        ImplWritePolyPolyAction( rOStm, rPolyPoly );
                    }
                    ImplWritePopAction( rOStm );

                    ImplWritePushAction( rOStm );
                    {
                        ImplWriteFillColor( rOStm, Color(), 0 );
                        ImplWritePolyPolyAction( rOStm, rPolyPoly );
                    }
                    ImplWritePopAction( rOStm );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    nCount += 15;
#endif
                }

                nCount++;
            }
            break;

            case( META_FLOATTRANSPARENT_ACTION ):
            {
                const MetaFloatTransparentAction*   pA = (MetaFloatTransparentAction*) pAction;
                const GDIMetaFile&                  rTransMtf = pA->GetGDIMetaFile();
                const Point&                        rPos = pA->GetPoint();
                const Size&                         rSize = pA->GetSize();
                const Gradient&                     rGradient = pA->GetGradient();
                ULONG                               nOldPos, nNewPos;

                // write RefPoint comment
                rOStm << (INT16) GDI_FLOATTRANSPARENT_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write comment data
                rOStm << rTransMtf << rPos << rSize << rGradient;

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos + 4 );
                rOStm.Seek( ( nOldPos = nNewPos ) + 4 );

                {
                    // write actions for float transparence
                    ULONG           nAddCount;
                    GDIMetaFile     aMtf( rTransMtf );
                    const Size      aSrcSize( rTransMtf.GetPrefSize() );
                    Point           aSrcPt( rTransMtf.GetPrefMapMode().GetOrigin() );
                    const double    fScaleX = aSrcSize.Width() ? (double) rSize.Width() / aSrcSize.Width() : 1.0;
                    const double    fScaleY = aSrcSize.Height() ? (double) rSize.Height() / aSrcSize.Height() : 1.0;
                    long            nMoveX, nMoveY;

                    if( fScaleX != 1.0 || fScaleY != 1.0 )
                    {
                        aMtf.Scale( fScaleX, fScaleY );
                        aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                    }

                    nMoveX = rPos.X() - aSrcPt.X(), nMoveY = rPos.Y() - aSrcPt.Y();

                    if( nMoveX || nMoveY )
                        aMtf.Move( nMoveX, nMoveY );

                    nAddCount = ImplWriteActions( rOStm, aMtf, rSaveVDev, rRop_0_1, rLineCol, rLineColStack, rActualCharSet );
                    nNewPos = rOStm.Tell();
                    rOStm.Seek( nOldPos );
                    rOStm << (INT32) nAddCount;
                    rOStm.Seek( nNewPos );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    nCount += nAddCount;
#endif
                }

                nCount++;
            }
            break;

            case( META_HATCH_ACTION ):
            {
                const MetaHatchAction*  pA = (MetaHatchAction*) pAction;
                const PolyPolygon&      rPolyPoly = pA->GetPolyPolygon();
                const Hatch&            rHatch = pA->GetHatch();
                ULONG                   nOldPos, nNewPos, nAddCount;

                // write hatch comment
                rOStm << (INT16) GDI_HATCH_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write comment data
                rOStm << rPolyPoly;
                rOStm << rHatch;

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos + 4 );
                rOStm.Seek( ( nOldPos = nNewPos ) + 4 );

                {
                    // write actions for hatch
                    VirtualDevice   aVDev;
                    GDIMetaFile     aTmpMtf;

                    aVDev.AddHatchActions( rPolyPoly, rHatch, aTmpMtf );
                    nAddCount = ImplWriteActions( rOStm, aTmpMtf, rSaveVDev, rRop_0_1, rLineCol, rLineColStack, rActualCharSet );
                    nNewPos = rOStm.Tell();
                    rOStm.Seek( nOldPos );
                    rOStm << (INT32) nAddCount;
                    rOStm.Seek( nNewPos );

#ifdef CVTSVM_WRITE_SUBACTIONCOUNT
                    nCount += nAddCount;
#endif
                }

                nCount++;
            }
            break;

            case( META_REFPOINT_ACTION ):
            {
                const MetaRefPointAction*   pA = (MetaRefPointAction*) pAction;
                const Point&                rRefPoint = pA->GetRefPoint();
                const BOOL                  bSet = pA->IsSetting();
                ULONG                       nOldPos, nNewPos;

                // write RefPoint comment
                rOStm << (INT16) GDI_REFPOINT_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write data
                rOStm << rRefPoint << bSet;
                rOStm << (INT32) 0; // number of actions that follow this comment

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos );
                rOStm.Seek( nNewPos );

                nCount++;
            }
            break;

            case( META_TEXTLINECOLOR_ACTION ):
            {
                const MetaTextLineColorAction*  pA = (MetaTextLineColorAction*) pAction;
                const Color&                    rColor = pA->GetColor();
                const BOOL                      bSet = pA->IsSetting();
                ULONG                           nOldPos, nNewPos;

                // write RefPoint comment
                rOStm << (INT16) GDI_TEXTLINECOLOR_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write data
                rOStm << rColor << bSet;
                rOStm << (INT32) 0; // number of actions that follow this comment

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos );
                rOStm.Seek( nNewPos );

                nCount++;
            }
            break;

            case( META_TEXTLINE_ACTION ):
            {
                const MetaTextLineAction*   pA = (MetaTextLineAction*) pAction;
                const Point&                rStartPt = pA->GetStartPoint();
                const long                  nWidth = pA->GetWidth();
                const FontStrikeout         eStrikeout = pA->GetStrikeout();
                const FontUnderline         eUnderline = pA->GetUnderline();
                ULONG                       nOldPos, nNewPos;

                // write RefPoint comment
                rOStm << (INT16) GDI_TEXTLINE_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write data
                rOStm << rStartPt << nWidth << (ULONG) eStrikeout << (ULONG) eUnderline;
                rOStm << (INT32) 0; // number of actions that follow this comment

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos );
                rOStm.Seek( nNewPos );

                nCount++;
            }
            break;

            case( META_EPS_ACTION ):
            break;

            case( META_COMMENT_ACTION ):
            {
                const MetaCommentAction*    pA = (MetaCommentAction*) pAction;
                const ULONG                 nDataSize = pA->GetDataSize();
                ULONG                       nOldPos, nNewPos;

                // write RefPoint comment
                rOStm << (INT16) GDI_COMMENT_COMMENT;

                // we'll write the ActionSize later
                nOldPos = rOStm.Tell();
                rOStm.SeekRel( 4 );

                // write data
                rOStm << pA->GetComment() << pA->GetValue() << nDataSize;

                if( nDataSize )
                    rOStm.Write( pA->GetData(), nDataSize );

                rOStm << (INT32) 0; // number of actions that follow this comment

                // calculate and write ActionSize of comment
                nNewPos = rOStm.Tell();
                rOStm.Seek( nOldPos );
                rOStm << (INT32) ( nNewPos - nOldPos );
                rOStm.Seek( nNewPos );

                nCount++;
            }
            break;

#ifdef DBG_UTIL
            default:
            {
                ByteString aStr( "Missing implementation for Action#: " );
                aStr += ByteString::CreateFromInt32( pAction->GetType() );
                aStr += '!';
                DBG_ERROR( aStr.GetBuffer() );
            }
            break;
#endif

/*
            case( META_TEXTRECT_ACTION ):
            {
                MetaTextRectAction* pAct = (MetaTextRectAction*) pAction;

                rOStm << ;
                rOStm << ;

                nCount++;
            }
            break;
*/

/*
            case( META_MASK_ACTION ):
            {
                MetaMaskAction* pAct = (MetaMaskAction*) pAction;

                rOStm << ;
                rOStm << ;

                nCount++;
            }
            break;
*/

/*
            case( META_MASKSCALE_ACTION ):
            {
                MetaMaskScaleAction* pAct = (MetaMaskScaleAction*) pAction;

                rOStm << ;
                rOStm << ;

                nCount++;
            }
            break;
*/

/*
            case( META_MASKSCALEPART_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;

                rOStm << ;
                rOStm << ;

                nCount++;
            }
            break;
*/

/*
            case( META_ISECTREGIONCLIPREGION_ACTION ):
            {
                MetaISectRegionClipRegionAction* pAct = (MetaISectRegionClipRegionAction*) pAction;

                rOStm << ;
                rOStm << ;

                nCount++;
            }
            break;
*/
        }
    }

    return nCount;
}
