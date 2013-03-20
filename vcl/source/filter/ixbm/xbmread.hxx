/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#if 1

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>

#ifdef _XBMPRIVATE

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
    sal_Bool                bStatus;

    void                InitTable();
    rtl::OString        FindTokenLine( SvStream* pInStm, const char* pTok1,
                                       const char* pTok2 = NULL, const char* pTok3 = NULL );
    long                ParseDefine( const sal_Char* pDefine );
    sal_Bool            ParseData( SvStream* pInStm, const rtl::OString& aLastLine, XBMFormat eFormat );


public:

                        XBMReader( SvStream& rStm );
    virtual             ~XBMReader();

    ReadState           ReadXBM( Graphic& rGraphic );
};

#endif // _XBMPRIVATE

sal_Bool ImportXBM( SvStream& rStream, Graphic& rGraphic );

#endif // _XBMREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
