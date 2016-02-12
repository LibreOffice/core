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

#ifndef INCLUDED_VCL_SOURCE_FILTER_IXBM_XBMREAD_HXX
#define INCLUDED_VCL_SOURCE_FILTER_IXBM_XBMREAD_HXX

#include <vcl/graphic.hxx>
#include <vcl/bitmapaccess.hxx>

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
    bool                bStatus;

    void            InitTable();
    OString         FindTokenLine( SvStream* pInStm, const char* pTok1,
                                       const char* pTok2 = nullptr, const char* pTok3 = nullptr );
    long            ParseDefine( const sal_Char* pDefine );
    bool            ParseData( SvStream* pInStm, const OString& aLastLine, XBMFormat eFormat );

public:

    explicit        XBMReader( SvStream& rStm );
    virtual         ~XBMReader();

    ReadState       ReadXBM( Graphic& rGraphic );
};

#endif // _XBMPRIVATE

VCL_DLLPUBLIC bool ImportXBM( SvStream& rStream, Graphic& rGraphic );

#endif // INCLUDED_VCL_SOURCE_FILTER_IXBM_XBMREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
