/*************************************************************************
 *
 *  $RCSfile: xbmread.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
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

#ifndef _XBMREAD_HXX
#define _XBMREAD_HXX

#ifdef VCL

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#else // VCL

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _FLTDEFS_HXX
#include "fltdefs.hxx"
#endif

#endif // VCL

#ifdef _XBMPRIVATE

// ---------
// - Enums -
// ---------

enum XBMFormat
{
    XBM10,
    XBM11
};

enum ReadState
{
    XBMREAD_OK,
    XBMREAD_ERROR,
    XBMREAD_NEED_MORE
};

// -------------
// - XBMReader -
// -------------

#ifdef VCL

class XBMReader : public GraphicReader
{
    SvStream&           rIStm;
    Bitmap              aBmp1;
    BitmapWriteAccess*  pAcc1;
    short*              pHexTable;
    BitmapColor         aWhite;
    BitmapColor         aBlack;
    long                nLastPos;
    long                nWidth;
    long                nHeight;
    BOOL                bStatus;

    void                InitTable();
    ByteString          FindTokenLine( SvStream* pInStm, const char* pTok1,
                                       const char* pTok2 = NULL, const char* pTok3 = NULL );
    long                ParseDefine( const sal_Char* pDefine );
    BOOL                ParseData( SvStream* pInStm, const ByteString& aLastLine, XBMFormat eFormat );


public:

                        XBMReader( SvStream& rStm, void* pCallData );
    virtual             ~XBMReader();

    ReadState           ReadXBM( Graphic& rGraphic );
};

#else // VCL

class XBMReader : public GraphicReader
{
    SvStream&   rIStm;
    PDIBBYTE    pMonoDIB;
    PDIBBYTE    pMonoDIBBytes;
    PDIBBYTE    pRow1;
    PDIBBYTE    pMonoFile;
    short       pHexTable[ 256 ];
    long        nLastPos;
    long        nWidth;
    long        nHeight;
    long        nWidthAl1;
    long        nMonoTotal;
    BOOL        bStatus;

    void        InitTable();
    void        CreateMonoDIB( long nWidth, long nHeight );
    void        CreateGraphic( Graphic& rGraphic );
    String      FindTokenLine( SvStream* pInStm, const char* pTok1,
                               const char* pTok2 = NULL, const char* pTok3 = NULL );
    long        ParseDefine( const char* pDefine );
    BOOL        ParseData( SvStream* pInStm, const String& aLastLine, XBMFormat eFormat );


public:

                XBMReader( SvStream& rStm, void* pCallData );
    virtual     ~XBMReader();

    ReadState   ReadXBM( Graphic& rGraphic );
};

#endif // VCL
#endif // _XBMPRIVATE

// -------------
// - ImportXBM -
// -------------

BOOL ImportXBM( SvStream& rStream, Graphic& rGraphic, void* pCallerData );

#endif // _XBMREAD_HXX
