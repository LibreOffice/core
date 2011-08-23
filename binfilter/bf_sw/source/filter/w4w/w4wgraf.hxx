/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/gen.hxx>
class Graphic; 
class SvStream; 
class Bitmap; 
class OutputDevice; 
namespace binfilter {


class RGBQuad;
class BmpFileHd;
class BmpInfoHd;
struct WmfFileHd;
class BmpFileHd;


class SwW4WGraf
{
    Point      aVectPos;          // W4W-Vector: aktuelle Position MoveTo/LineTo
    SvStream&  rInp;            // aus diesem Stream lesen
    Graphic*   pGraph;
    Bitmap*    pBmp;
    RGBQuad*   pPal;
    BmpFileHd* pBmpHead;
    BmpInfoHd* pBmpInfo;
    USHORT     nPalColors;
    USHORT 	   nVectMinX;
    USHORT 	   nVectMaxX;
    USHORT 	   nVectMinY;
    USHORT 	   nVectMaxY;
//	Size       aVectSiz;          // Maximale Ausdehnung der W4W-Vectorgrafik (wird von ReadHexPoint() gesezt)

    short      nStat;             // 1 = File-Grafik, 2 = Mem-Grafik
    BYTE       nError;

    void   ReadHexPoint(Point& aPnt);
    int    GetNextRecord();
    int    GetNextVectRec(OutputDevice& rOut);
    short  GetHexByte();
    USHORT GetHexUShort();
    ULONG  GetHexULong();
    long   GetVarSize();
    short  ReadPalette( long nVarSize );
    short  DefaultPalette( USHORT nColors );
    short  ReadBitmap( long nVarSize );
    void   SkipBytes( register long nBytes );

    ULONG ReadWmfHeader( WmfFileHd& rHd );
    short ReadW4WGrafBMap( long nTyp, long nWidth, long nHeight );
    short ReadW4WGrafVect( long nTyp, long nWidth, long nHeight );
    short ReadGrafFile( long nTyp, long nWidth, long nHeight );

    BOOL  CheckW4WVector();       // enth„lt die W4W-Grafik Vektordaten ? (Joe)

    sal_Char ReadChar() 	{ sal_Char c = 0; rInp.Read( &c, 1 ); return c; }

public:
    SwW4WGraf( SvStream& rIstream );
    ~SwW4WGraf();
    short Read( long nGrafPDSType, long nGrWidthTw, long nGrHeightTw );
    Graphic* GetGraphic() const { return pGraph; };
};

} //namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
