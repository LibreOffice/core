/*************************************************************************
 *
 *  $RCSfile: enhwmf.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-22 14:40:59 $
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

#include "winmtf.hxx"

//=========================== GDI-Array ===================================

#define EMR_HEADER                      1
#define EMR_POLYBEZIER                  2
#define EMR_POLYGON                     3
#define EMR_POLYLINE                    4
#define EMR_POLYBEZIERTO                5
#define EMR_POLYLINETO                  6
#define EMR_POLYPOLYLINE                7
#define EMR_POLYPOLYGON                 8
#define EMR_SETWINDOWEXTEX              9
#define EMR_SETWINDOWORGEX              10
#define EMR_SETVIEWPORTEXTEX            11
#define EMR_SETVIEWPORTORGEX            12
#define EMR_SETBRUSHORGEX               13
#define EMR_EOF                         14
#define EMR_SETPIXELV                   15
#define EMR_SETMAPPERFLAGS              16
#define EMR_SETMAPMODE                  17
#define EMR_SETBKMODE                   18
#define EMR_SETPOLYFILLMODE             19
#define EMR_SETROP2                     20
#define EMR_SETSTRETCHBLTMODE           21
#define EMR_SETTEXTALIGN                22
#define EMR_SETCOLORADJUSTMENT          23
#define EMR_SETTEXTCOLOR                24
#define EMR_SETBKCOLOR                  25
#define EMR_OFFSETCLIPRGN               26
#define EMR_MOVETOEX                    27
#define EMR_SETMETARGN                  28
#define EMR_EXCLUDECLIPRECT             29
#define EMR_INTERSECTCLIPRECT           30
#define EMR_SCALEVIEWPORTEXTEX          31
#define EMR_SCALEWINDOWEXTEX            32
#define EMR_SAVEDC                      33
#define EMR_RESTOREDC                   34
#define EMR_SETWORLDTRANSFORM           35
#define EMR_MODIFYWORLDTRANSFORM        36
#define EMR_SELECTOBJECT                37
#define EMR_CREATEPEN                   38
#define EMR_CREATEBRUSHINDIRECT         39
#define EMR_DELETEOBJECT                40
#define EMR_ANGLEARC                    41
#define EMR_ELLIPSE                     42
#define EMR_RECTANGLE                   43
#define EMR_ROUNDRECT                   44
#define EMR_ARC                         45
#define EMR_CHORD                       46
#define EMR_PIE                         47
#define EMR_SELECTPALETTE               48
#define EMR_CREATEPALETTE               49
#define EMR_SETPALETTEENTRIES           50
#define EMR_RESIZEPALETTE               51
#define EMR_REALIZEPALETTE              52
#define EMR_EXTFLOODFILL                53
#define EMR_LINETO                      54
#define EMR_ARCTO                       55
#define EMR_POLYDRAW                    56
#define EMR_SETARCDIRECTION             57
#define EMR_SETMITERLIMIT               58
#define EMR_BEGINPATH                   59
#define EMR_ENDPATH                     60
#define EMR_CLOSEFIGURE                 61
#define EMR_FILLPATH                    62
#define EMR_STROKEANDFILLPATH           63
#define EMR_STROKEPATH                  64
#define EMR_FLATTENPATH                 65
#define EMR_WIDENPATH                   66
#define EMR_SELECTCLIPPATH              67
#define EMR_ABORTPATH                   68

#define EMR_GDICOMMENT                  70
#define EMR_FILLRGN                     71
#define EMR_FRAMERGN                    72
#define EMR_INVERTRGN                   73
#define EMR_PAINTRGN                    74
#define EMR_EXTSELECTCLIPRGN            75
#define EMR_BITBLT                      76
#define EMR_STRETCHBLT                  77
#define EMR_MASKBLT                     78
#define EMR_PLGBLT                      79
#define EMR_SETDIBITSTODEVICE           80
#define EMR_STRETCHDIBITS               81
#define EMR_EXTCREATEFONTINDIRECTW      82
#define EMR_EXTTEXTOUTA                 83
#define EMR_EXTTEXTOUTW                 84
#define EMR_POLYBEZIER16                85
#define EMR_POLYGON16                   86
#define EMR_POLYLINE16                  87
#define EMR_POLYBEZIERTO16              88
#define EMR_POLYLINETO16                89
#define EMR_POLYPOLYLINE16              90
#define EMR_POLYPOLYGON16               91
#define EMR_POLYDRAW16                  92
#define EMR_CREATEMONOBRUSH             93
#define EMR_CREATEDIBPATTERNBRUSHPT     94
#define EMR_EXTCREATEPEN                95
#define EMR_POLYTEXTOUTA                96
#define EMR_POLYTEXTOUTW                97
#define EMR_SETICMMODE                  98
#define EMR_CREATECOLORSPACE            99
#define EMR_SETCOLORSPACE              100
#define EMR_DELETECOLORSPACE           101
#define EMR_GLSRECORD                  102
#define EMR_GLSBOUNDEDRECORD           103
#define EMR_PIXELFORMAT                104

//-----------------------------------------------------------------------------------

static float GetSwapFloat( SvStream& rSt )
{
    float   fTmp;
    sal_Int8* pPtr = (sal_Int8*)&fTmp + 3;
    rSt >> *pPtr-- >> *pPtr-- >> *pPtr-- >> *pPtr;  // Little Endian <-> Big Endian switch
    return fTmp;
}

SvStream& operator>>( SvStream& rIn, XForm& rXForm )
{
    if ( sizeof( float ) != 4 )
    {
        DBG_ERROR( "EnhWMFReader::sizeof( float ) != 4" );
        rXForm = XForm();
    }
    else
    {
#ifdef __BIGENDIAN
    rXForm.eM11 = GetSwapFloat( rIn );
    rXForm.eM12 = GetSwapFloat( rIn );
    rXForm.eM21 = GetSwapFloat( rIn );
    rXForm.eM22 = GetSwapFloat( rIn );
    rXForm.eDx = GetSwapFloat( rIn );
    rXForm.eDy = GetSwapFloat( rIn );
#else
    rIn >> rXForm.eM11 >> rXForm.eM12 >> rXForm.eM21 >> rXForm.eM22
            >> rXForm.eDx >> rXForm.eDy;
#endif
    }
    return rIn;
}

BOOL EnhWMFReader::ReadEnhWMF() // SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, PFilterCallback pcallback, void * pcallerdata)
{
    UINT32 nStretchBltMode = 0;
    UINT32  nRecType, nRecSize, nNextPos;
    UINT32  nWidth, nHeight, nPoints, nColor, nIndex;
    UINT32  nDat32, nNom1, nDen1, nNom2, nDen2;
    INT32   nX32, nY32, nx32, ny32;
    INT16   nX16, nY16;

    sal_Bool    bFlag, bStatus = ReadHeader();

    while( bStatus && nRecordCount-- )
    {
        *pWMF >> nRecType >> nRecSize;

        if ( ( nRecSize < 8 ) || ( nRecSize & 3 ) )     // Parameter sind immer durch 4 teilbar
        {
            bStatus = FALSE;
            break;
        }
        nNextPos = pWMF->Tell() + ( nRecSize - 8 );

        if( aBmpSaveList.Count() && ( nRecType != EMR_STRETCHBLT ) && ( nRecType != EMR_STRETCHDIBITS ) )
                pOut->ResolveBitmapActions( aBmpSaveList );

        bFlag = sal_False;

        switch( nRecType )
        {
            case EMR_HEADER :                           // wir haben schon laengst einen header eingelesen
            break;

            case EMR_POLYBEZIERTO :
                bFlag = sal_True;
            case EMR_POLYBEZIER :
            {
                pWMF->SeekRel( 16 );
                *pWMF >> nPoints;
                sal_uInt16 i = 0;
                if ( bFlag )
                {
                    i++;
                    nPoints++;
                }
                Polygon aPoly( (sal_uInt16)nPoints );
                for( ; i < (sal_uInt16)nPoints; i++ )
                {
                    *pWMF >> nX32 >> nY32;
                    aPoly[ i ] = Point( nX32, nY32 );
                }
                pOut->DrawPolyBezier( aPoly, bFlag, bRecordPath );
            }
            break;

            case EMR_POLYGON :
            {
                pWMF->SeekRel( 16 );
                *pWMF >> nPoints;
                Polygon aPoly( (UINT16)nPoints );
                for( UINT16 k = 0; k < (UINT16)nPoints; k++ )
                {
                    *pWMF >> nX32 >> nY32;
                    aPoly[ k ] = Point( nX32, nY32 );
                }
                pOut->DrawPolygon( aPoly, bRecordPath );
            }
            break;

            case EMR_POLYLINETO :
                bFlag = sal_True;
            case EMR_POLYLINE :
            {
                pWMF->SeekRel( 0x10 );
                *pWMF >> nPoints;
                UINT16 i = 0;
                if ( bFlag )
                {
                    i++;
                    nPoints++;
                }
                Polygon aPolygon( (UINT16)nPoints );
                for ( ; i < (UINT16)nPoints; i++ )
                {
                    *pWMF >> nX32 >> nY32;
                    aPolygon[ i ] = Point( nX32, nY32 );
                }
                pOut->DrawPolyLine( aPolygon, bFlag, bRecordPath );
            }
            break;

            case EMR_POLYPOLYLINE :
            {
                UINT16* pnPoints;

                INT32   i, nPoly;
                pWMF->SeekRel( 0x10 );

                // Anzahl der Polygone:
                *pWMF >> nPoly >> i;

                // Anzahl der Punkte eines jeden Polygons holen, Gesammtzahl der Punkte ermitteln:
                pnPoints = new UINT16[ nPoly ];

                for ( i = 0; i < nPoly; i++ )
                {
                    *pWMF >> nPoints;
                    pnPoints[ i ] = (UINT16)nPoints;
                }

                // Polygonpunkte holen:

                for ( i = 0; i < nPoly; i++ )
                {
                    Polygon aPoly( pnPoints[ i ] );
                    for( UINT16 k = 0; k < pnPoints[ i ]; k++ )
                    {
                        *pWMF >> nX32 >> nY32;
                        aPoly[ k ] = Point( nX32, nY32 );
                    }
                    pOut->DrawPolyLine( aPoly, sal_False, bRecordPath );
                }
                delete pnPoints;
            }
            break;

            case EMR_POLYPOLYGON :
            {
                UINT16* pnPoints;
                Point*  pPtAry;

                INT32   i, nPoly, nGesPoints;
                pWMF->SeekRel( 0x10 );

                // Anzahl der Polygone:
                *pWMF >> nPoly >> nGesPoints;

                // Anzahl der Punkte eines jeden Polygons holen, Gesammtzahl der Punkte ermitteln:
                pnPoints = new UINT16[ nPoly ];

                for ( i = 0; i < nPoly; i++ )
                {
                    *pWMF >> nPoints;
                    pnPoints[ i ] = (UINT16)nPoints;
                }
                // Polygonpunkte holen:
                pPtAry  = (Point*) new char[ nGesPoints * sizeof(Point) ];

                for ( i = 0; i < nGesPoints; i++ )
                {
                    *pWMF >> nX32 >> nY32;
                    pPtAry[ i ] = Point( nX32, nY32 );
                }
                // PolyPolygon Actions erzeugen
                PolyPolygon aPolyPoly( (UINT16)nPoly, pnPoints, pPtAry );
                pOut->DrawPolyPolygon( aPolyPoly, bRecordPath );
                delete (char*) pPtAry;
                delete pnPoints;
            }
            break;

            case EMR_SETWINDOWEXTEX :
            {                                                       // #75383#
                *pWMF >> nWidth >> nHeight;
                if( nWidth > 1 && nHeight > 1 )
                    pOut->SetWinExt( Size( nWidth, nHeight ) );
            }
            break;

            case EMR_SETWINDOWORGEX :
            {
                *pWMF >> nX32 >> nY32;
                pOut->SetWinOrg( Point( nX32, nY32 ) );
            }
            break;

            case EMR_SETVIEWPORTEXTEX :
            case EMR_SETVIEWPORTORGEX :
            break;

            case EMR_SCALEVIEWPORTEXTEX :
            {
                *pWMF >> nNom1 >> nDen1 >> nNom2 >> nDen2;
                pOut->ScaleDevExt( (double)nNom1 / nDen1, (double)nNom2 / nDen2 );
            }
            break;

            case EMR_SCALEWINDOWEXTEX :
            {
                *pWMF >> nNom1 >> nDen1 >> nNom2 >> nDen2;
                pOut->ScaleWinExt( (double)nNom1 / nDen1, (double)nNom2 / nDen2 );
            }
            break;

            case EMR_SETBRUSHORGEX :
            break;
            case EMR_EOF :
                nRecordCount = 0;           // #76846#
            break;

            case EMR_SETPIXELV :
            {
                *pWMF >> nX32 >> nY32;
                pOut->DrawPixel( Point( nX32, nY32 ), ReadColor() );
            }
            break;

            case EMR_SETMAPPERFLAGS :
            break;

            case EMR_SETMAPMODE :
            {
                UINT32 nMapMode;
                *pWMF >> nMapMode;
                if ( nMapMode )
                    break;
            }
            break;

            case EMR_SETBKMODE :
            {
                *pWMF >> nDat32;
                pOut->SetBkMode( nDat32 );
            }
            break;

            case EMR_SETPOLYFILLMODE :
            break;

            case EMR_SETROP2 :
            {
                *pWMF >> nDat32;
                pOut->SetRasterOp( nDat32 );
            }
            break;

            case EMR_SETSTRETCHBLTMODE :
            {
                *pWMF >> nStretchBltMode;
            }
            break;

            case EMR_SETTEXTALIGN :
            {
                *pWMF >> nDat32;
                pOut->SetTextAlign( nDat32 );
            }
            break;

            case EMR_SETCOLORADJUSTMENT :
            break;

            case EMR_SETTEXTCOLOR :
            {
                pOut->SetTextColor( ReadColor() );
            }
            break;

            case EMR_SETBKCOLOR :
            {
                pOut->SetBkColor( ReadColor() );
            }
            break;

            case EMR_OFFSETCLIPRGN :
            {
                *pWMF >> nX32 >> nY32;
                pOut->MoveClipRegion( Size( nX32, nY32 ) );
            }
            break;

            case EMR_MOVETOEX :
            {
                *pWMF >> nX32 >> nY32;
                pOut->MoveTo( Point( nX32, nY32 ) );
            }
            break;

            case EMR_SETMETARGN :
            case EMR_EXCLUDECLIPRECT :
            break;

            case EMR_INTERSECTCLIPRECT :
            {
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32;
                pOut->IntersectClipRect( ReadRectangle( nX32, nY32, nx32, ny32 ) );
            }
            break;

            case EMR_SAVEDC :
            {
                pOut->Push();
            }
            break;

            case EMR_RESTOREDC :
            {
                pOut->Pop();
            }
            break;

            case EMR_SETWORLDTRANSFORM :
            {
                XForm aTempXForm;
                *pWMF >> aTempXForm;
                pOut->SetWorldTransform( aTempXForm );
            }
            break;

            case EMR_MODIFYWORLDTRANSFORM :
            {
                UINT32  nMode;
                XForm   aTempXForm;
                *pWMF >> aTempXForm >> nMode;
                pOut->ModifyWorldTransform( aTempXForm, nMode );
            }
            break;

            case EMR_SELECTOBJECT :
            {
                *pWMF >> nIndex;
                pOut->SelectObject( nIndex );
            }
            break;

            case EMR_CREATEPEN :
            {
                *pWMF >> nIndex;
                if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                {

                    LineInfo    aLineInfo;
                    UINT32      nStyle;
                    Size        aSize;

                    *pWMF >> nStyle >> aSize.Width() >> aSize.Height();

                    if ( aSize.Width() )
                        aLineInfo.SetWidth( aSize.Width() );

                    BOOL bTransparent = FALSE;
                    UINT16 nDashCount = 0;
                    UINT16 nDotCount = 0;
                    switch( nStyle )
                    {
                        case PS_DASHDOTDOT :
                            nDotCount++;
                        case PS_DASHDOT :
                            nDashCount++;
                        case PS_DOT :
                            nDotCount++;
                        break;
                        case PS_DASH :
                            nDashCount++;
                        break;
                        case PS_NULL :
                            bTransparent = TRUE;
                            aLineInfo.SetStyle( LINE_NONE );
                        break;
                        default :
                        case PS_INSIDEFRAME :
                        case PS_SOLID :
                            aLineInfo.SetStyle( LINE_SOLID );
                    }
                    if ( nDashCount | nDotCount )
                    {
                        aLineInfo.SetStyle( LINE_DASH );
                        aLineInfo.SetDashCount( nDashCount );
                        aLineInfo.SetDotCount( nDotCount );
                    }
                    pOut->CreateObject( nIndex, GDI_PEN, new WinMtfLineStyle( ReadColor(), aLineInfo, bTransparent ) );
                }
            }
            break;
            case EMR_CREATEBRUSHINDIRECT :
            {
                UINT32  nStyle;
                *pWMF >> nIndex;
                if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                {
                    *pWMF >> nStyle;
                    pOut->CreateObject( nIndex, GDI_BRUSH, new WinMtfFillStyle( ReadColor(), ( nStyle == BS_HOLLOW ) ? TRUE : FALSE ) );
                }
            }
            break;
            case EMR_DELETEOBJECT :
            {
                *pWMF >> nIndex;
                if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                    pOut->DeleteObject( nIndex );
            }
            break;

            case EMR_ANGLEARC :
            break;

            case EMR_ELLIPSE :
            {
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32;
                pOut->DrawEllipse( ReadRectangle( nX32, nY32, nx32, ny32 ) );
            }
            break;

            case EMR_RECTANGLE :
            {
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32;
                pOut->DrawRect( ReadRectangle( nX32, nY32, nx32, ny32 ) );
            }
            break;

            case EMR_ROUNDRECT :
            {
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nWidth >> nHeight;
                Size aSize( Size( nWidth, nHeight ) );
                pOut->DrawRoundRect( ReadRectangle( nX32, nY32, nx32, ny32 ), aSize );
            }
            break;

            case EMR_ARC :
            {
                UINT32 nStartX, nStartY, nEndX, nEndY;
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                pOut->DrawArc( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
            }
            break;

            case EMR_CHORD :
            {
                UINT32 nStartX, nStartY, nEndX, nEndY;
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                pOut->DrawChord( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
            }
            break;

            case EMR_PIE :
            {
                UINT32 nStartX, nStartY, nEndX, nEndY;
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                pOut->DrawPie( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
            }
            break;

            case EMR_SELECTPALETTE :
            case EMR_CREATEPALETTE :
            case EMR_SETPALETTEENTRIES :
            case EMR_RESIZEPALETTE :
            case EMR_REALIZEPALETTE :
            case EMR_EXTFLOODFILL :
            break;

            case EMR_LINETO :
            {
                *pWMF >> nX32 >> nY32;
                pOut->LineTo( Point( nX32, nY32 ), bRecordPath );
            }
            break;

            case EMR_ARCTO :
            {
                UINT32 nStartX, nStartY, nEndX, nEndY;
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                pOut->DrawArc( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ), TRUE );
            }
            break;
            case EMR_POLYDRAW :
            case EMR_SETARCDIRECTION :
            case EMR_SETMITERLIMIT :
            break;
            case EMR_BEGINPATH :
            {
                pOut->ClearPath();
                bRecordPath = sal_True;
            }
            break;
            case EMR_ABORTPATH :
                pOut->ClearPath();
            case EMR_ENDPATH :
                bRecordPath = sal_False;
            break;
            case EMR_CLOSEFIGURE :
                pOut->ClosePath();
            break;
            case EMR_FILLPATH :
                pOut->StrokeAndFillPath( sal_False, sal_True );
            break;
            case EMR_STROKEANDFILLPATH :
                pOut->StrokeAndFillPath( sal_True, sal_True );
            break;
            case EMR_STROKEPATH :
                pOut->StrokeAndFillPath( sal_True, sal_False );
            break;
            case EMR_FLATTENPATH :
            case EMR_WIDENPATH :
            break;
            case EMR_SELECTCLIPPATH :
            {
                sal_Int32 nClippingMode;
                *pWMF >> nClippingMode;
                pOut->SetClipPath( pOut->GetPathObj(), nClippingMode );
            }
            break;

            case EMR_GDICOMMENT :
            case EMR_FILLRGN :
            case EMR_FRAMERGN :
            case EMR_INVERTRGN :
            case EMR_PAINTRGN :
            case EMR_EXTSELECTCLIPRGN :
            break;
            case EMR_BITBLT :
            {
                UINT32      nRasterOp;
                Size        aDestExt;
                Point       aDestOrg;
                Bitmap      aBmp;

                pWMF->SeekRel( 0x10 );
                *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nRasterOp;
                aDestOrg = Point( nX32, nY32 );
                aDestExt = Size( nx32, ny32 );
                *pWMF >> nX32 >> nY32 >> nColor;

                UINT32 nNewRop = R2_BLACK;
                switch( nRasterOp )
                {
                    case DSTINVERT :
                        nNewRop = R2_NOT;
                    break;
                    case 0x00990066 :
                    case SRCINVERT:
                        nNewRop = R2_XORPEN;
                    break;
                    case BLACKNESS :
                        nColor = 0;
                    break;
                    case WHITENESS :
                        nColor = 0xffffff;
                    break;
                }
                pOut->Push();
                UINT32 nOldRop = pOut->SetRasterOp( nNewRop );
                pOut->DrawRect( Rectangle( aDestOrg, aDestExt ) );
                pOut->SetRasterOp( nOldRop );
                pOut->Pop();
            }
            break;

            case EMR_STRETCHBLT :
            {
                INT32   xDest, yDest, cxDest, cyDest, xSrc, ySrc, cxSrc, cySrc;
                UINT32  dwRop, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc;
                XForm   xformSrc;

                UINT32  nStartPos = pWMF->Tell() - 8;

                pWMF->SeekRel( 0x10 );
                *pWMF >> xDest >> yDest >> cxDest >> cyDest >> dwRop >> xSrc >> ySrc
                        >> xformSrc >> nColor >> iUsageSrc >> offBmiSrc >> cbBmiSrc
                            >> offBitsSrc >> cbBitsSrc >> cxSrc >> cySrc;

                Bitmap      aBitmap;
                Rectangle   aRect( Point( xDest, yDest ), Size( cxDest+1, cyDest+1 ) );

                if ( offBmiSrc )
                {
                    UINT32  nSize = cbBmiSrc + cbBitsSrc + 14;
                    char*   pBuf = new char[ nSize ];

                    SvMemoryStream aTmp( pBuf, nSize, STREAM_READ | STREAM_WRITE );
                    aTmp.ObjectOwnsMemory( TRUE );
                    aTmp << (BYTE)'B'
                         << (BYTE)'M'
                         << (UINT32)cbBitsSrc
                         << (UINT16)0
                         << (UINT16)0
                         << (UINT32)cbBmiSrc + 14;
                    pWMF->Seek( nStartPos + offBmiSrc );
                    pWMF->Read( pBuf + 14, cbBmiSrc );
                    pWMF->Seek( nStartPos + offBitsSrc );
                    pWMF->Read( pBuf + 14 + cbBmiSrc, cbBitsSrc );
                    aTmp.Seek( 0 );
                    aBitmap.Read( aTmp, TRUE );
                }
                aBmpSaveList.Insert( new BSaveStruct( aBitmap, aRect, dwRop ), LIST_APPEND );
            }
            break;

            case EMR_STRETCHDIBITS :
            {
                INT32   xDest, yDest, xSrc, ySrc, cxSrc, cySrc, cxDest, cyDest;
                UINT32  offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, iUsageSrc, dwRop;
                UINT32  nStartPos = pWMF->Tell() - 8;

                pWMF->SeekRel( 0x10 );
                *pWMF >> xDest >> yDest >> xSrc >> ySrc >> cxSrc >> cySrc >> offBmiSrc >> cbBmiSrc >> offBitsSrc
                    >> cbBitsSrc >> iUsageSrc >> dwRop >> cxDest >> cyDest;

                Bitmap      aBitmap;
                Rectangle   aRect( Point( xDest, yDest ), Size( cxDest+1, cyDest+1 ) );

                UINT32 nSize = cbBmiSrc + cbBitsSrc + 14;
                char* pBuf = new char[ nSize ];
                SvMemoryStream aTmp( pBuf, nSize, STREAM_READ | STREAM_WRITE );
                aTmp.ObjectOwnsMemory( TRUE );
                aTmp << (BYTE)'B'
                     << (BYTE)'M'
                     << (UINT32)cbBitsSrc
                     << (UINT16)0
                     << (UINT16)0
                     << (UINT32)cbBmiSrc + 14;
                pWMF->Seek( nStartPos + offBmiSrc );
                pWMF->Read( pBuf + 14, cbBmiSrc );
                pWMF->Seek( nStartPos + offBitsSrc );
                pWMF->Read( pBuf + 14 + cbBmiSrc, cbBitsSrc );
                aTmp.Seek( 0 );
                aBitmap.Read( aTmp, TRUE );
                aBmpSaveList.Insert( new BSaveStruct( aBitmap, aRect, dwRop ), LIST_APPEND );
            }
            break;

            case EMR_MASKBLT :
            case EMR_PLGBLT :
            case EMR_SETDIBITSTODEVICE :
            break;

            case EMR_EXTCREATEFONTINDIRECTW :
            {
                *pWMF >> nIndex;
                if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                {
                    LOGFONTW aLogFont;
                    *pWMF >> aLogFont.lfHeight >> aLogFont.lfWidth >> aLogFont.lfEscapement >> aLogFont.lfOrientation >> aLogFont.lfWeight >> aLogFont.lfItalic
                                >> aLogFont.lfUnderline >> aLogFont.lfStrikeOut >> aLogFont.lfCharSet >> aLogFont.lfOutPrecision >> aLogFont.lfClipPrecision
                                    >> aLogFont.lfQuality >> aLogFont.lfPitchAndFamily;

                    for ( int i = 0; i < LF_FACESIZE; i++ )
                    {
                        UINT16 nChar;
                        *pWMF >> nChar;
                        aLogFont.lfFaceName[ i ] = (BYTE)nChar;
                    }
                    pOut->CreateObject( nIndex, GDI_FONT, new WinMtfFontStyle( aLogFont ) );
                }
            }
            break;
            case EMR_EXTTEXTOUTA :
                bFlag = sal_True;
            case EMR_EXTTEXTOUTW :
            {
                INT32           nLeft, nTop, nRight, nBottom, ptlReferenceX, ptlReferenceY, nGfxMode, nXScale, nYScale;
                UINT32          nCurPos, nLen, nOffString, nOptions, offDx;

                nCurPos = pWMF->Tell() - 8;

                *pWMF >> nLeft >> nTop >> nRight >> nBottom >> nGfxMode >> nXScale >> nYScale
                    >> ptlReferenceX >> ptlReferenceY >> nLen >> nOffString >> nOptions;

                pWMF->SeekRel( 0x10 );
                *pWMF >> offDx;

                Point aPos;

                if ( nGfxMode != GM_COMPATIBLE )
                    aPos = Point( nLeft, nTop );
                else
                    aPos = Point( ptlReferenceX, ptlReferenceY );

                if ( nLen )
                {
                    pWMF->Seek( nCurPos + nOffString );
                    if ( bFlag )
                    {
                        sal_Char* pBuf = new sal_Char[ nLen ];
                        pWMF->Read( pBuf, nLen );
                        String aText( pBuf, (sal_uInt16)nLen, pOut->GetCharSet() );
                        pOut->DrawText( aPos, aText, NULL, bRecordPath );
                        delete pBuf;
                    }
                    else
                    {
                        sal_Unicode* pBuf = new sal_Unicode[ nLen ];
                        pWMF->Read( pBuf, nLen << 1 );
#ifdef __BIGENDIAN
                        sal_Char nTmp, *pTmp = (sal_Char*)( pBuf + nLen );
                        while ( pTmp-- != (sal_Char*)pBuf )
                        {
                            nTmp = *pTmp--;
                            pTmp[ 1 ] = *pTmp;
                            *pTmp = nTmp;
                        }
#endif
                        String aText( pBuf, (xub_StrLen)nLen );
                        pOut->DrawText( aPos, aText, NULL, bRecordPath );
                        delete pBuf;
                    }
                }
            }
            break;

            case EMR_POLYBEZIERTO16 :
                bFlag = sal_True;
            case EMR_POLYBEZIER16 :
            {
                pWMF->SeekRel( 16 );
                *pWMF >> nPoints;
                UINT16 i = 0;
                if ( bFlag )
                {
                    i++;
                    nPoints++;
                }
                Polygon aPoly( (UINT16)nPoints );
                for( ; i < (UINT16)nPoints; i++ )
                {
                    *pWMF >> nX16 >> nY16;
                    aPoly[ i ] = Point( nX16, nY16 );
                }
                pOut->DrawPolyBezier( aPoly, bFlag, bRecordPath );  // Line( aPoly, bFlag );
            }
            break;
            case EMR_POLYGON16 :
            {
                pWMF->SeekRel( 16 );
                *pWMF >> nPoints;
                Polygon aPoly( (UINT16)nPoints );
                for( UINT16 k = 0; k < (UINT16)nPoints; k++ )
                {
                    *pWMF >> nX16 >> nY16;
                    aPoly[ k ] = Point( nX16, nY16 );
                }
                pOut->DrawPolygon( aPoly, bRecordPath );
            }
            break;
            case EMR_POLYLINETO16 :
                bFlag = sal_True;
            case EMR_POLYLINE16 :
            {
                pWMF->SeekRel( 16 );
                *pWMF >> nPoints;
                UINT16 i = 0;
                if ( bFlag )
                {
                    i++;
                    nPoints++;
                }

                Polygon aPoly( (UINT16)nPoints );
                for( ; i < (UINT16)nPoints; i++ )
                {
                    *pWMF >> nX16 >> nY16;
                    aPoly[ i ] = Point( nX16, nY16 );
                }
                pOut->DrawPolyLine( aPoly, bFlag, bRecordPath );
            }
            break;

            case EMR_POLYPOLYLINE16 :
            {
                UINT16* pnPoints;

                INT32   i, nPoly, nGesPoints;
                pWMF->SeekRel( 0x10 );
                // Anzahl der Polygone:
                *pWMF >> nPoly >> nGesPoints;
                // Anzahl der Punkte eines jeden Polygons holen, Gesammtzahl der Punkte ermitteln:
                pnPoints = new UINT16[ nPoly ];
                for ( i = 0; i < nPoly; i++ )
                {
                    *pWMF >> nPoints;
                    pnPoints[ i ] = (UINT16)nPoints;
                }
                // Polygonpunkte holen:
                for ( i = 0; i < nPoly; i++ )
                {
                    Polygon aPolygon( pnPoints[ i ] );
                    for ( UINT16 k = 0; k < pnPoints[ i ]; k++ )
                    {
                        *pWMF >> nX16 >> nY16;
                        aPolygon[ k ] = Point( nX16, nY16 );
                    }
                    pOut->DrawPolyLine( aPolygon, sal_False, bRecordPath );
                }
                delete pnPoints;
            }
            break;

            case EMR_POLYPOLYGON16 :
            {
                UINT16* pnPoints;
                Point*  pPtAry;

                INT32   i, nPoly, nGesPoints;
                pWMF->SeekRel( 0x10 );
                // Anzahl der Polygone:
                *pWMF >> nPoly >> nGesPoints;
                // Anzahl der Punkte eines jeden Polygons holen, Gesammtzahl der Punkte ermitteln:
                pnPoints = new UINT16[ nPoly ];
                for ( i = 0; i < nPoly; i++ )
                {
                    *pWMF >> nPoints;
                    pnPoints[ i ] = (UINT16)nPoints;
                }
                // Polygonpunkte holen:
                pPtAry  = (Point*) new char[ nGesPoints * sizeof(Point) ];
                for ( i = 0; i < nGesPoints; i++ )
                {
                    *pWMF >> nX16 >> nY16;
                    pPtAry[ i ] = Point( nX16, nY16 );
                }

                // PolyPolygon Actions erzeugen
                PolyPolygon aPolyPoly( (UINT16)nPoly, pnPoints, pPtAry );
                pOut->DrawPolyPolygon( aPolyPoly, bRecordPath );
                delete (char*) pPtAry;
                delete pnPoints;
            };
            break;
            case EMR_POLYDRAW16 :
            break;
            case EMR_CREATEMONOBRUSH :
            case EMR_CREATEDIBPATTERNBRUSHPT :
            case EMR_EXTCREATEPEN :
            case EMR_POLYTEXTOUTA :
            case EMR_POLYTEXTOUTW :
            case EMR_SETICMMODE :
            break;
            case EMR_CREATECOLORSPACE :
            case EMR_SETCOLORSPACE :
            case EMR_DELETECOLORSPACE :
            break;
            case EMR_GLSRECORD :
            case EMR_GLSBOUNDEDRECORD :
            case EMR_PIXELFORMAT :
            break;

            default :
//              bStatus = FALSE;
            break;
        }
        pWMF->Seek( nNextPos );
    }
    if( aBmpSaveList.Count() )
        pOut->ResolveBitmapActions( aBmpSaveList );

    if ( bStatus )
        pWMF->Seek(nEndPos);

    return bStatus;
};

//-----------------------------------------------------------------------------------

BOOL EnhWMFReader::ReadHeader()
{
    Rectangle   aPlaceableBound;
    UINT32      nUINT32, nHeaderSize, nPalEntries;
    INT32       nLeft, nTop, nRight, nBottom;
    INT32       nPixX, nPixY, nMillX, nMillY;

    // METAFILEHEADER SPARE ICH MIR HIER
    // Einlesen des METAHEADER
    *pWMF >> nUINT32 >> nHeaderSize;
    if ( nUINT32 != 1 )         // Typ
        return FALSE;

    // bound size
    pWMF->SeekRel( 16 );

    // picture frame size
    *pWMF >> nLeft >> nTop >> nRight >> nBottom;
    aPlaceableBound.Left() = nLeft;
    aPlaceableBound.Top() = nTop;
    aPlaceableBound.Right() = nRight;
    aPlaceableBound.Bottom() = nBottom;

    *pWMF >> nUINT32;                                   // signature

    if ( nUINT32 != 0x464d4520 )
        return FALSE;

    *pWMF >> nUINT32;                                   // nVersion
    *pWMF >> nEndPos;                                   // size of metafile
    nEndPos += nStartPos;
    *pWMF >> nRecordCount;

    if ( !nRecordCount )
        return FALSE;

    pWMF->SeekRel( 0xc );
    *pWMF >> nPalEntries >> nPixX >> nPixY >> nMillX >> nMillY;

    pOut->SetDevExt( aPlaceableBound.GetSize() );
    pOut->SetWinOrg( Point( FRound( ( (double) aPlaceableBound.Left() * nPixX ) / ( nMillX * 100.0 ) ),
                                FRound( ( (double) aPlaceableBound.Top() * nPixY ) / ( nMillY * 100.0 ) ) ) );
    pOut->SetWinExt( Size( FRound( ( (double) aPlaceableBound.GetWidth() * nPixX ) / ( nMillX * 100.0 ) ),
                                FRound( ( (double) aPlaceableBound.GetHeight() * nPixY ) / ( nMillY * 100.0 ) ) ) );
    pWMF->Seek( nStartPos + nHeaderSize );
    return TRUE;
}

//-----------------------------------------------------------------------------------

Rectangle EnhWMFReader::ReadRectangle( INT32 x1, INT32 y1, INT32 x2, INT32 y2 )
{
    Point aTL ( Point( x1, y1 ) );
    Point aBR( Point( --x2, --y2 ) );
    return Rectangle( aTL, aBR );
}

EnhWMFReader::~EnhWMFReader()
{

};

