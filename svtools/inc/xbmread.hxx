/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xbmread.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:39:04 $
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

#ifndef _XBMREAD_HXX
#define _XBMREAD_HXX

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

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

                        XBMReader( SvStream& rStm );
    virtual             ~XBMReader();

    ReadState           ReadXBM( Graphic& rGraphic );
};

#endif // _XBMPRIVATE

// -------------
// - ImportXBM -
// -------------

BOOL ImportXBM( SvStream& rStream, Graphic& rGraphic );

#endif // _XBMREAD_HXX
