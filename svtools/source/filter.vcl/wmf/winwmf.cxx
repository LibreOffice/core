/*************************************************************************
 *
 *  $RCSfile: winwmf.cxx,v $
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

#include "winmtf.hxx"

//====================== MS-Windows-defines ===============================

#define W_META_SETBKCOLOR           0x0201
#define W_META_SETBKMODE            0x0102
#define W_META_SETMAPMODE           0x0103
#define W_META_SETROP2              0x0104
#define W_META_SETRELABS            0x0105
#define W_META_SETPOLYFILLMODE      0x0106
#define W_META_SETSTRETCHBLTMODE    0x0107
#define W_META_SETTEXTCHAREXTRA     0x0108
#define W_META_SETTEXTCOLOR         0x0209
#define W_META_SETTEXTJUSTIFICATION 0x020A
#define W_META_SETWINDOWORG         0x020B
#define W_META_SETWINDOWEXT         0x020C
#define W_META_SETVIEWPORTORG       0x020D
#define W_META_SETVIEWPORTEXT       0x020E
#define W_META_OFFSETWINDOWORG      0x020F
#define W_META_SCALEWINDOWEXT       0x0410
#define W_META_OFFSETVIEWPORTORG    0x0211
#define W_META_SCALEVIEWPORTEXT     0x0412
#define W_META_LINETO               0x0213
#define W_META_MOVETO               0x0214
#define W_META_EXCLUDECLIPRECT      0x0415
#define W_META_INTERSECTCLIPRECT    0x0416
#define W_META_ARC                  0x0817
#define W_META_ELLIPSE              0x0418
#define W_META_FLOODFILL            0x0419
#define W_META_PIE                  0x081A
#define W_META_RECTANGLE            0x041B
#define W_META_ROUNDRECT            0x061C
#define W_META_PATBLT               0x061D
#define W_META_SAVEDC               0x001E
#define W_META_SETPIXEL             0x041F
#define W_META_OFFSETCLIPRGN        0x0220
#define W_META_TEXTOUT              0x0521
#define W_META_BITBLT               0x0922
#define W_META_STRETCHBLT           0x0B23
#define W_META_POLYGON              0x0324
#define W_META_POLYLINE             0x0325
#define W_META_ESCAPE               0x0626
#define W_META_RESTOREDC            0x0127
#define W_META_FILLREGION           0x0228
#define W_META_FRAMEREGION          0x0429
#define W_META_INVERTREGION         0x012A
#define W_META_PAINTREGION          0x012B
#define W_META_SELECTCLIPREGION     0x012C
#define W_META_SELECTOBJECT         0x012D
#define W_META_SETTEXTALIGN         0x012E
#define W_META_DRAWTEXT             0x062F
#define W_META_CHORD                0x0830
#define W_META_SETMAPPERFLAGS       0x0231
#define W_META_EXTTEXTOUT           0x0a32
#define W_META_SETDIBTODEV          0x0d33
#define W_META_SELECTPALETTE        0x0234
#define W_META_REALIZEPALETTE       0x0035
#define W_META_ANIMATEPALETTE       0x0436
#define W_META_SETPALENTRIES        0x0037
#define W_META_POLYPOLYGON          0x0538
#define W_META_RESIZEPALETTE        0x0139
#define W_META_DIBBITBLT            0x0940
#define W_META_DIBSTRETCHBLT        0x0b41
#define W_META_DIBCREATEPATTERNBRUSH 0x0142
#define W_META_STRETCHDIB           0x0f43
#define W_META_EXTFLOODFILL         0x0548
#define W_META_RESETDC              0x014C
#define W_META_STARTDOC             0x014D
#define W_META_STARTPAGE            0x004F
#define W_META_ENDPAGE              0x0050
#define W_META_ABORTDOC             0x0052
#define W_META_ENDDOC               0x005E
#define W_META_DELETEOBJECT         0x01f0
#define W_META_CREATEPALETTE        0x00f7
#define W_META_CREATEBRUSH          0x00F8
#define W_META_CREATEPATTERNBRUSH   0x01F9
#define W_META_CREATEPENINDIRECT    0x02FA
#define W_META_CREATEFONTINDIRECT   0x02FB
#define W_META_CREATEBRUSHINDIRECT  0x02FC
#define W_META_CREATEBITMAPINDIRECT 0x02FD
#define W_META_CREATEBITMAP         0x06FE
#define W_META_CREATEREGION         0x06FF

//=================== Methoden von WMFReader ==============================

inline Point WMFReader::ReadPoint()
{
    short nX, nY;
    *pWMF >> nX >> nY;
    return Point( nX, nY );
}

// ------------------------------------------------------------------------

inline Point WMFReader::ReadYX()
{
    short nX, nY;
    *pWMF >> nY >> nX;
    return Point( nX, nY );
}

// ------------------------------------------------------------------------

Rectangle WMFReader::ReadRectangle()
{
    Point aBR, aTL;
    aBR = ReadYX();
    aTL = ReadYX();
    aBR.X()--;
    aBR.Y()--;
    return Rectangle( aTL, aBR );
}

// ------------------------------------------------------------------------

Size WMFReader::ReadYXExt()
{
    short nW, nH;
    *pWMF >> nH >> nW;
    return Size( nW, nH );
}

// ------------------------------------------------------------------------

void WMFReader::ReadRecordParams( USHORT nFunction )
{
    switch( nFunction )
    {
        case W_META_SETBKCOLOR:
        {
            pOut->SetBkColor( ReadColor() );
        }
        break;

        case W_META_SETBKMODE:
        {
            USHORT nDat;
            *pWMF >> nDat;
            pOut->SetBkMode( nDat );
        }
        break;

        // !!!
        case W_META_SETMAPMODE:
        {
            short nMapMode;
            *pWMF >> nMapMode;
#ifdef DBG_ASSERT
            if ( nMapMode != 8 )    // nur MM_ANISOTROPHIC == 8 wird unterstuetzt
            {
                DBG_ASSERT(0,"WMF-Import: MapMode ignored");
            }
#endif
        }
        break;

        case W_META_SETROP2:
        {
            UINT16 nROP2;
            *pWMF >> nROP2;
            pOut->SetRasterOp( nROP2 );
        }
        break;

        case W_META_SETTEXTCOLOR:
        {
            pOut->SetTextColor( ReadColor() );
        }
        break;

        case W_META_SETWINDOWORG:
        {
            pOut->SetWinOrg( ReadYX() );
        }
        break;

        case W_META_SETWINDOWEXT:
        {
            short nWidth, nHeight;
            *pWMF >> nHeight >> nWidth;
            ImplSetWMFSize( Size( nWidth, nHeight ) );
        }
        break;

        case W_META_OFFSETWINDOWORG:
        {
            short nXAdd, nYAdd;
            *pWMF >> nYAdd >> nXAdd;
            pOut->SetWinOrgOffset( nXAdd, nYAdd );
        }
        break;

        case W_META_SCALEWINDOWEXT:
        {
            short nXNum, nXDenom, nYNum, nYDenom;
            *pWMF >> nYDenom >> nYNum >> nXDenom >> nXNum;
            pOut->ScaleWinExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
        }
        break;

        case W_META_SETVIEWPORTORG:
        case W_META_SETVIEWPORTEXT:
        break;

        case W_META_OFFSETVIEWPORTORG:
        {
            short nXAdd, nYAdd;
            *pWMF >> nYAdd >> nXAdd;
            pOut->SetDevOrgOffset( nXAdd, nYAdd );
        }
        break;

        case W_META_SCALEVIEWPORTEXT:
        {
            short nXNum, nXDenom, nYNum, nYDenom;
            *pWMF >> nYDenom >> nYNum >> nXDenom >> nXNum;
            pOut->ScaleDevExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
        }
        break;

        case W_META_LINETO:
        {
            pOut->LineTo( ReadYX() );
        }
        break;

        case W_META_MOVETO:
        {
            pOut->MoveTo( ReadYX() );
        }
        break;

        case W_META_INTERSECTCLIPRECT:
        {
            pOut->IntersectClipRect( ReadRectangle() );
        }
        break;

        case W_META_RECTANGLE:
        {
            pOut->DrawRect( ReadRectangle() );
        }
        break;

        case W_META_ROUNDRECT:
        {
            Size aSize( ReadYXExt() );
            pOut->DrawRoundRect( ReadRectangle(), aSize );
        }
        break;

        case W_META_ELLIPSE:
        {
            pOut->DrawEllipse( ReadRectangle() );
        }
        break;

        case W_META_ARC:
        {
            Point aEnd( ReadYX() );
            Point aStart( ReadYX() );
            pOut->DrawArc( ReadRectangle(), aStart, aEnd );
        }
        break;

        case W_META_PIE:
        {
            Point aEnd( ReadYX() );
            Point aStart( ReadYX() );
            pOut->DrawPie( ReadRectangle(), aStart, aEnd );
        }
        break;

        case W_META_CHORD:
        {
            Point aEnd( ReadYX() );
            Point aStart( ReadYX() );
            pOut->DrawChord( ReadRectangle(), aStart, aEnd );
        }
        break;

        case W_META_POLYGON:
        {
            USHORT i,nPoints;
            *pWMF >> nPoints;
            Polygon aPoly( nPoints );
            for( i = 0; i < nPoints; i++ )
                aPoly[ i ] = ReadPoint();
            pOut->DrawPolygon( aPoly );
        }
        break;

        case W_META_POLYPOLYGON:
        {
            USHORT  i, nPoly, nPoints;
            USHORT* pnPoints;
            Point*  pPtAry;
            // Anzahl der Polygone:
            *pWMF >> nPoly;
            // Anzahl der Punkte eines jeden Polygons holen, Gesammtzahl der Punkte ermitteln:
            pnPoints = new USHORT[ nPoly ];
            nPoints = 0;
            for( i = 0; i < nPoly; i++ )
            {
                *pWMF >> pnPoints[i];
                nPoints += pnPoints[i];
            }
            // Polygonpunkte holen:
            pPtAry  = (Point*) new char[ nPoints * sizeof(Point) ];
            for ( i = 0; i < nPoints; i++ )
                pPtAry[ i ] = ReadPoint();
            // PolyPolygon Actions erzeugen
            PolyPolygon aPolyPoly( nPoly, pnPoints, pPtAry );
            pOut->DrawPolyPolygon( aPolyPoly );
            delete (char*) pPtAry;
            delete pnPoints;
        }
        break;

        case W_META_POLYLINE:
        {
            USHORT i,nPoints;
            *pWMF >> nPoints;
            Polygon aPoly( nPoints );
            for( i = 0; i < nPoints; i++ )
                aPoly[ i ] = ReadPoint();
            pOut->DrawPolyLine( aPoly );
        }
        break;

        case W_META_SAVEDC:
        {
            pOut->Push( bWinExtSet );
        }
        break;

        case W_META_RESTOREDC:
        {
            pOut->Pop();
        }
        break;

        case W_META_SETPIXEL:
        {
            const Color aColor = ReadColor();
            pOut->DrawPixel( ReadYX(), aColor );
        }
        break;

        case W_META_OFFSETCLIPRGN:
        {
            pOut->MoveClipRegion( ReadYXExt() );
        }
        break;

        case W_META_TEXTOUT:
        {
            USHORT nLength;
            *pWMF >> nLength;
            if ( nLength )
            {
                char*   pChar = new char[ ( nLength + 1 ) &~ 1 ];
                pWMF->Read( pChar, ( nLength + 1 ) &~ 1 );
                String aText( pChar, nLength, pOut->GetCharSet() );
                delete pChar;
                Point aPosition( ReadYX() );
                pOut->DrawText( aPosition, aText );
            }
        }
        break;

        case W_META_EXTTEXTOUT:
        {
            USHORT      i,nLen,nOptions,nData;
            long        nRecordSize,nRecSizeLeft, *pDXAry;
            Point       aPosition;
            Rectangle   aRect;

            pWMF->SeekRel(-6);
            *pWMF >> nRecordSize;
            pWMF->SeekRel(2);
            aPosition = ReadYX();
            *pWMF >> nLen >> nOptions;
            // Nur wenn der Text auch Zeichen enthaelt, macht die Ausgabe Sinn
            if( nLen )
            {
                if( nOptions )
                {
                    const Point aPt1( ReadPoint() );
                    const Point aPt2( ReadPoint() );
                    aRect = Rectangle( aPt1, aPt2 );
                }
                char* pChar = new char[ ( nLen + 1 ) &~ 1 ];
                pWMF->Read( pChar, ( nLen + 1 ) &~ 1 );
                String aText( pChar, nLen, pOut->GetCharSet() );
                delete pChar;

                nRecSizeLeft=(nRecordSize-7-(((long)nLen+1)>>1))<<1;

                if( nOptions )
                    nRecSizeLeft-=8;

                if( nRecSizeLeft >= (long) nLen )
                {
                    pDXAry = new long[ nLen ];

                    for( i = 0; i < nLen; i++ )
                    {
                        *pWMF >> nData;
                        pDXAry[ i ] = nData;
                    }
                    pOut->DrawText( aPosition, aText, pDXAry );
                    delete[] pDXAry;
                }
                else
                    pOut->DrawText( aPosition, aText );
            }
        }
        break;

        case W_META_SELECTOBJECT:
        {
            INT16   nObjIndex;
            *pWMF >> nObjIndex;
            pOut->SelectObject( nObjIndex );
        }
        break;

        case W_META_SETTEXTALIGN:
        {
            UINT16  nAlign;
            *pWMF >> nAlign;
            pOut->SetTextAlign( nAlign );
        }
        break;

        case W_META_BITBLT:
        case W_META_STRETCHBLT:
        case W_META_DIBBITBLT:
        case W_META_DIBSTRETCHBLT:
        case W_META_STRETCHDIB:
        {
            long        nWinROP;
            short       nSx, nSy, nSxe, nSye, nUsage;
            Bitmap      aBmp;

            *pWMF >> nWinROP;

            if( nFunction == W_META_STRETCHDIB )
                *pWMF >> nUsage;

            if( nFunction==W_META_STRETCHDIB || nFunction==W_META_STRETCHBLT || nFunction==W_META_DIBSTRETCHBLT )
                *pWMF >> nSye >> nSxe;

            *pWMF >> nSy >> nSx;

            if( nFunction == W_META_STRETCHDIB || nFunction == W_META_DIBBITBLT || nFunction == W_META_DIBSTRETCHBLT )
            {
                switch( nWinROP )
                {
                    case PATCOPY :
                    {
                        *pWMF >> nUsage;    // i don't know anything of this parameter, so its called nUsage
                        Size aDestSize( ReadYXExt() );
                        pOut->DrawRect( Rectangle( ReadYX(), aDestSize ), FALSE );
                    }
                    break;
                    default :
                    {
                        Size        aDestSize( ReadYXExt() );
                        Rectangle   aDestRect( ReadYX(), aDestSize );
                        aBmp.Read( *pWMF, FALSE );
                        aBmpSaveList.Insert( new BSaveStruct( aBmp, aDestRect, nWinROP ), LIST_APPEND );
                    }
                    break;
                }
            }
            else
            {
                if( aBmpSaveList.Count() )
                    pOut->ResolveBitmapActions( aBmpSaveList );
            }
        }
        break;

        case W_META_DIBCREATEPATTERNBRUSH:
        {
            Bitmap  aBmp;
            BitmapReadAccess* pBmp;
            UINT32  nRed = 0, nGreen = 0, nBlue = 0, nCount = 1;
            UINT16  nFunction;

            *pWMF >> nFunction >> nFunction;

            aBmp.Read( *pWMF, FALSE );
            pBmp = aBmp.AcquireReadAccess();
            if ( pBmp )
            {
                for ( INT32 y = 0; y < pBmp->Height(); y++ )
                {
                    for ( INT32 x = 0; x < pBmp->Width(); x++ )
                    {
                        const BitmapColor aColor( pBmp->GetColor( y, x ) );

                        nRed += aColor.GetRed();
                        nGreen += aColor.GetGreen();
                        nBlue += aColor.GetBlue();
                    }
                }
                nCount = pBmp->Height() * pBmp->Width();
                if ( !nCount )
                    nCount++;
                aBmp.ReleaseAccess( pBmp );
            }
            Color aColor( (BYTE)( nRed / nCount ), (BYTE)( nGreen / nCount ), (BYTE)( nBlue / nCount ) );
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( aColor, FALSE ) );
        }
        break;

        case W_META_DELETEOBJECT:
        {
            INT16 nIndex;
            *pWMF >> nIndex;
            pOut->DeleteObject( nIndex );
        }
        break;

        case W_META_CREATEPALETTE:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_CREATEBRUSH:
        {
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ), FALSE ) );
        }
        break;

        case W_META_CREATEPATTERNBRUSH:
        {
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ), FALSE ) );
        }
        break;

        case W_META_CREATEPENINDIRECT:
        {
            LineInfo    aLineInfo;
            USHORT      nStyle, nWidth, nHeight;

            *pWMF >> nStyle >> nWidth >> nHeight;

            if ( nWidth )
                aLineInfo.SetWidth( nWidth );

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
            pOut->CreateObject( GDI_PEN, new WinMtfLineStyle( ReadColor(), aLineInfo, bTransparent ) );
        }
        break;

        case W_META_CREATEBRUSHINDIRECT:
        {
            USHORT  nStyle;
            *pWMF >> nStyle;
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( ReadColor(), ( nStyle == BS_HOLLOW ) ? TRUE : FALSE ) );
        }
        break;

        case W_META_CREATEFONTINDIRECT:
        {
            Size    aFontSize;
            INT16   lfEscapement, lfOrientation, lfWeight;  // ( ehemals USHORT )

            LOGFONTW aLogFont;
            aFontSize = ReadYXExt();
            *pWMF >> lfEscapement >> lfOrientation >> lfWeight
                    >> aLogFont.lfItalic >> aLogFont.lfUnderline >> aLogFont.lfStrikeOut >> aLogFont.lfCharSet >> aLogFont.lfOutPrecision
                        >> aLogFont.lfClipPrecision >> aLogFont.lfQuality >> aLogFont.lfPitchAndFamily;
            pWMF->Read( aLogFont.lfFaceName, LF_FACESIZE );
            aLogFont.lfWidth = aFontSize.Width();
            aLogFont.lfHeight = aFontSize.Height();
            aLogFont.lfEscapement = lfEscapement;
            aLogFont.lfOrientation = lfOrientation;
            aLogFont.lfWeight = lfWeight;
            pOut->CreateObject( GDI_FONT, new WinMtfFontStyle( aLogFont ) );
        }
        break;

        case W_META_CREATEBITMAPINDIRECT:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_CREATEBITMAP:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_CREATEREGION:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_SETRELABS:
        case W_META_SETPOLYFILLMODE:
        case W_META_SETSTRETCHBLTMODE:
        case W_META_SETTEXTCHAREXTRA:
        case W_META_SETTEXTJUSTIFICATION:
        case W_META_EXCLUDECLIPRECT:
        case W_META_FLOODFILL:
        break;
        case W_META_PATBLT:
        {
            UINT32 nROP, nOldROP;
            *pWMF >> nROP;
            Size aSize = ReadYXExt();
            nOldROP = pOut->SetRasterOp( nROP );
            pOut->DrawRect( Rectangle( ReadYX(), aSize ), FALSE );
            pOut->SetRasterOp( nOldROP );
        }
        break;
        case W_META_ESCAPE:
        case W_META_FILLREGION:
        case W_META_FRAMEREGION:
        case W_META_INVERTREGION:
        case W_META_PAINTREGION:
        case W_META_SELECTCLIPREGION:
        case W_META_DRAWTEXT:
        case W_META_SETMAPPERFLAGS:
        case W_META_SETDIBTODEV:
        case W_META_SELECTPALETTE:
        case W_META_REALIZEPALETTE:
        case W_META_ANIMATEPALETTE:
        case W_META_SETPALENTRIES:
        case W_META_RESIZEPALETTE:
        case W_META_EXTFLOODFILL:
        case W_META_RESETDC:
        case W_META_STARTDOC:
        case W_META_STARTPAGE:
        case W_META_ENDPAGE:
        case W_META_ABORTDOC:
        case W_META_ENDDOC:
        break;
    }
}

// ------------------------------------------------------------------------

BOOL WMFReader::ReadHeader()
{
    Rectangle   aPlaceableBound;
    ULONG       nl;

    // Einlesen des METAFILEHEADER, falls vorhanden
    *pWMF >> nl;

    if ( nl == 0x9ac6cdd7L )
    {
        INT16 nVal;

        // hmf (Unused) ueberlesen wir
        pWMF->SeekRel(2);

        // BoundRect
        *pWMF >> nVal; aPlaceableBound.Left() = nVal;
        *pWMF >> nVal; aPlaceableBound.Top() = nVal;
        *pWMF >> nVal; aPlaceableBound.Right() = nVal;
        *pWMF >> nVal; aPlaceableBound.Bottom() = nVal;

        // inch
        *pWMF >> nUnitsPerInch;

        // reserved
        pWMF->SeekRel( 4 );

        // checksum  pruefen wir lieber nicht
        pWMF->SeekRel( 2 );
    }
    else
    {
        // Unit wird bei den alten MTF's als MM_TEXT angenommen
        nUnitsPerInch = 96;
        pWMF->SeekRel( -4 ); // zurueck zum Anfang
    }

    // Einlesen des METAHEADER
    *pWMF >> nl; // Typ und Headergroesse

    if( nl != 0x00090001 )
    {
        pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
        return FALSE;
    }

    pWMF->SeekRel( 2 ); // Version (von Windows)
    pWMF->SeekRel( 4 ); // Size (der Datei in Words)
    pWMF->SeekRel( 2 ); // NoObjects (Maximale Anzahl der gleichzeitigen Objekte)
    pWMF->SeekRel( 4 ); // MaxRecord (Groesse des groessten Records in Words)
    pWMF->SeekRel( 2 ); // NoParameters (Unused

    pOut->SetWinOrg( aPlaceableBound.TopLeft() );

    ImplSetWMFSize( aPlaceableBound.GetSize() );

    return TRUE;
}

// ------------------------------------------------------------------------

void WMFReader::ImplSetWMFSize( const Size& rSize )
{
    pOut->SetWinExt( rSize );

    // try to calculate size of WMF
    if( !bWinExtSet && rSize.Width() > 1 && rSize.Height() > 1 )
    {
        const Fraction  aFrac( 1, nUnitsPerInch );
        MapMode         aWMFMap( MAP_INCH, Point(), aFrac, aFrac );
        Size            aSize100( OutputDevice::LogicToLogic( rSize, aWMFMap, MAP_100TH_MM ) );
        pOut->SetDevExt( Size( labs( aSize100.Width() ), labs( aSize100.Height() ) ) );
        bWinExtSet = TRUE;
    }
}

// ------------------------------------------------------------------------

void WMFReader::ReadWMF() // SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, PFilterCallback pcallback, void * pcallerdata)
{
    USHORT  nFunction;
    ULONG   nRecSize;
    ULONG   nPos, nPercent, nLastPercent;

    pOut->SetWinOrg( Point() );
    pOut->SetWinExt( Size( 1, 1 ) );
    pOut->SetDevExt( Size( 10000, 10000 ) );
    bWinExtSet = FALSE;

    nEndPos=pWMF->Seek( STREAM_SEEK_TO_END );
    pWMF->Seek( nStartPos );
    Callback( (USHORT) ( nLastPercent = 0 ) );

    if ( ReadHeader() )
    {

        nPos = pWMF->Tell();

        if( nEndPos - nStartPos )
        {
            while( TRUE )
            {
                nPercent = ( nPos - nStartPos ) * 100 / ( nEndPos - nStartPos );

                if( nLastPercent + 4 <= nPercent )
                {
                    if( Callback( (USHORT) nPercent ) )
                        break;

                    nLastPercent = nPercent;
                }

                *pWMF >> nRecSize >> nFunction;

                if( pWMF->GetError() || ( nRecSize < 3 ) || ( nRecSize==3 && nFunction==0 ) || pWMF->IsEof() )
                {

                    if( pWMF->IsEof() )
                        pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );

                    break;
                }

                if( aBmpSaveList.Count() &&
                    ( nFunction != W_META_STRETCHDIB ) &&
                    ( nFunction != W_META_DIBBITBLT ) &&
                    ( nFunction != W_META_DIBSTRETCHBLT ) )
                {
                    pOut->ResolveBitmapActions( aBmpSaveList );
                }

                ReadRecordParams( nFunction );
                pWMF->Seek( nPos += nRecSize * 2 );
            }
        }
        else
            pWMF->SetError( SVSTREAM_GENERALERROR );

        if( !pWMF->GetError() && aBmpSaveList.Count() )
            pOut->ResolveBitmapActions( aBmpSaveList );
    }
    if ( pWMF->GetError() )
        pWMF->Seek( nStartPos );
}

