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

#ifndef _EMFWR_HXX
#define _EMFWR_HXX

#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <bf_svtools/fltcall.hxx>

// -------------
// - EMFWriter -
// -------------

namespace binfilter
{

class EMFWriter
{
private:

    VirtualDevice		maVDev;
    FilterConfigItem*	mpFilterConfigItem;
    SvStream*			mpStm;
    BOOL*				mpHandlesUsed;
    ULONG				mnHandleCount;
    ULONG				mnLastPercent;
    ULONG				mnRecordCount;
    ULONG				mnRecordPos;
    BOOL				mbRecordOpen;
    BOOL				mbLineChanged;
    sal_uInt32			mnLineHandle;
    BOOL				mbFillChanged;
    sal_uInt32			mnFillHandle;
    BOOL				mbTextChanged;
    sal_uInt32			mnTextHandle;

    void				ImplBeginRecord( sal_uInt32 nType );
    void				ImplEndRecord();
                        
    ULONG				ImplAcquireHandle();
    void				ImplReleaseHandle( ULONG nHandle );
                        
    BOOL				ImplPrepareHandleSelect( sal_uInt32& rHandle, ULONG nSelectType );
    void				ImplCheckLineAttr();
    void				ImplCheckFillAttr();
    void				ImplCheckTextAttr();
                        
    void				ImplWriteColor( const Color& rColor );
    void				ImplWriteRasterOp( RasterOp eRop );
    void				ImplWriteExtent( long nExtent );
    void				ImplWritePoint( const Point& rPoint );
    void				ImplWriteSize( const Size& rSize);
    void				ImplWriteRect( const Rectangle& rRect );
    void				ImplWritePath( const PolyPolygon& rPolyPoly, sal_Bool bClose );
    void				ImplWritePolygonRecord( const Polygon& rPoly, BOOL bClose );
    void				ImplWritePolyPolygonRecord( const PolyPolygon& rPolyPoly );
    void				ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt, const Size& rSz, UINT32 nROP );
    void				ImplWriteTextRecord( const Point& rPos, const String rText, const sal_Int32* pDXArray, sal_uInt32 nWidth );

    void				ImplWrite( const GDIMetaFile& rMtf );

public:

                        EMFWriter() {}

    BOOL				WriteEMF( const GDIMetaFile& rMtf, SvStream& rOStm, FilterConfigItem* pConfigItem = NULL );
};

}

#endif // _EMFWR_HXX
