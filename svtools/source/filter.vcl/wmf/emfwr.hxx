/*************************************************************************
 *
 *  $RCSfile: emfwr.hxx,v $
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
    void                ImplWritePolygonRecord( const Polygon& rPoly, BOOL bClose );
    void                ImplWritePolyPolygonRecord( const PolyPolygon& rPolyPoly );
    void                ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt, const Size& rSz, UINT32 nROP );
    void                ImplWriteTextRecord( const Point& rPos, const String rText, const long* pDXArray, UINT32 nWidth );

    void                ImplWrite( const GDIMetaFile& rMtf );

public:

                        EMFWriter() {}

    BOOL                WriteEMF( const GDIMetaFile& rMtf, SvStream& rOStm,
                                  PFilterCallback pCallback, void* pCallerData );
};

#endif // _EMFWR_HXX
