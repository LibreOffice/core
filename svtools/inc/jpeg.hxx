/*************************************************************************
 *
 *  $RCSfile: jpeg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-07 19:53:37 $
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

#ifndef _JPEG_HXX
#define _JPEG_HXX

#ifdef VCL

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _FLTCALL_HXX
#include "fltcall.hxx"
#endif

#else // VCL

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _FLTCALL_HXX
#include "fltcall.hxx"
#endif
#ifndef _FLTDEFS_HXX
#include "fltdefs.hxx"
#endif

#endif // VCL

#ifdef _JPEGPRIVATE

// --------
// - Enum -
// --------

enum ReadState
{
    JPEGREAD_OK,
    JPEGREAD_ERROR,
    JPEGREAD_NEED_MORE
};

// --------------
// - JPEGReader -
// --------------

#ifdef VCL

class JPEGReader : public GraphicReader
{
    SvStream&           rIStm;
    Bitmap              aBmp;
    Bitmap              aBmp1;
    BitmapWriteAccess*  pAcc;
    BitmapWriteAccess*  pAcc1;
    void*               pBuffer;
    long                nLastPos;
    long                nFormerPos;
    long                nLastLines;

    Graphic             CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines );
    void                FillBitmap();

public:

    void*               CreateBitmap( const Size& rSize, BOOL bGray, long& rAlignedWidth, long& rTopDown );

public:



                        JPEGReader( SvStream& rStm, void* pCallData );
    virtual             ~JPEGReader();


    ReadState           Read( Graphic& rGraphic );
};

// --------------
// - JPEGWriter -
// --------------

class JPEGWriter
{
    SvStream&           rOStm;
    Bitmap              aBmp;
    BitmapReadAccess*   pAcc;
    BYTE*               pBuffer;
    PFilterCallback     pCallback;
    void*               pCallerData;
    BOOL                bNative;

public:

    void*               GetScanline( long nY );

public:

                        JPEGWriter( SvStream& rOStm, PFilterCallback pCallback, void* pCallData );
                        ~JPEGWriter() {};

    BOOL                Write( const Graphic& rGraphic, sal_Bool bIgnoreOptions );
};

#else // VCL

class JPEGReader : public GraphicReader
{
    Graphic     aImGraphic;
    SvStream&   rIStm;
    PDIBBYTE    pMonoDIB;
    PDIBBYTE    pRow1;
    PDIBBYTE    pMonoFile;
    long        nLastPos;
    long        nFormerPos;
    long        nLastLines;
    long        nWidthAl1;
    long        nTotal;
    long        nMonoTotal;

    void        CreateMonoDIB( const Size& rSize );
    void        CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines );


public:

                JPEGReader( SvStream& rStm, void* pCallData );
    virtual     ~JPEGReader();


    ReadState   ReadJpeg( Graphic& rGraphic );
};

#endif // VCL
#endif // _JPEGPRIVATE

// ---------------------
// - Import/ExportJPEG -
// ---------------------

BOOL ImportJPEG( SvStream& rStream, Graphic& rGraphic, void* pCallerData );

BOOL ExportJPEG( SvStream& rStream, const Graphic& rGraphic,
                 PFilterCallback pCallback, void* pCallerData, sal_Bool bIgnoreOptions );

#endif // _JPEG_HXX
