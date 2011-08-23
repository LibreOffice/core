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

#ifndef _XBMREAD_HXX
#define _XBMREAD_HXX

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>

namespace binfilter
{

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
    SvStream&			rIStm;
    Bitmap				aBmp1;
    BitmapWriteAccess*	pAcc1;
    short*				pHexTable;
    BitmapColor			aWhite;
    BitmapColor			aBlack;
    long				nLastPos;
    long				nWidth;
    long				nHeight;
    BOOL				bStatus;

    void				InitTable();
    ByteString			FindTokenLine( SvStream* pInStm, const char* pTok1,
                                       const char* pTok2 = NULL, const char* pTok3 = NULL );
    long				ParseDefine( const sal_Char* pDefine );
    BOOL				ParseData( SvStream* pInStm, const ByteString& aLastLine, XBMFormat eFormat );


public:

                        XBMReader( SvStream& rStm );
    virtual				~XBMReader();

    ReadState			ReadXBM( Graphic& rGraphic );
};

#endif // _XBMPRIVATE

// -------------
// - ImportXBM -
// -------------

BOOL ImportXBM( SvStream& rStream, Graphic& rGraphic );

}

#endif // _XBMREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
