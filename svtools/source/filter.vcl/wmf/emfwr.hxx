/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: emfwr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 13:21:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EMFWR_HXX
#define _EMFWR_HXX

#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include "fltcall.hxx"

// -------------
// - EMFWriter -
// -------------

class EMFWriter
{
private:

    VirtualDevice       maVDev;
    PFilterCallback     mpCallback;
    void*               mpCallerData;
    SvStream*           mpStm;
    BOOL*               mpHandlesUsed;
    ULONG               mnHandleCount;
    ULONG               mnLastPercent;
    ULONG               mnRecordCount;
    ULONG               mnRecordPos;
    BOOL                mbRecordOpen;
    BOOL                mbLineChanged;
    ULONG               mnLineHandle;
    BOOL                mbFillChanged;
    ULONG               mnFillHandle;
    BOOL                mbTextChanged;
    ULONG               mnTextHandle;

    void                ImplBeginRecord( ULONG nType );
    void                ImplEndRecord();

    ULONG               ImplAcquireHandle();
    void                ImplReleaseHandle( ULONG nHandle );

    BOOL                ImplPrepareHandleSelect( ULONG& rHandle, ULONG nSelectType );
    void                ImplCheckLineAttr();
    void                ImplCheckFillAttr();
    void                ImplCheckTextAttr();

    void                ImplWriteColor( const Color& rColor );
    void                ImplWriteRasterOp( RasterOp eRop );
    void                ImplWriteExtent( long nExtent );
    void                ImplWritePoint( const Point& rPoint );
    void                ImplWriteSize( const Size& rSize);
    void                ImplWriteRect( const Rectangle& rRect );
    void                ImplWritePath( const PolyPolygon& rPolyPoly, sal_Bool bClose );
    void                ImplWritePolygonRecord( const Polygon& rPoly, BOOL bClose );
    void                ImplWritePolyPolygonRecord( const PolyPolygon& rPolyPoly );
    void                ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt, const Size& rSz, UINT32 nROP );
    void                ImplWriteTextRecord( const Point& rPos, const String rText, const sal_Int32* pDXArray, sal_uInt32 nWidth );

    void                ImplWrite( const GDIMetaFile& rMtf );

public:

                        EMFWriter() {}

    BOOL                WriteEMF( const GDIMetaFile& rMtf, SvStream& rOStm,
                                  PFilterCallback pCallback, void* pCallerData );
};

#endif // _EMFWR_HXX
