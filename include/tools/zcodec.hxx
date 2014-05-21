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

#ifndef INCLUDED_TOOLS_ZCODEC_HXX
#define INCLUDED_TOOLS_ZCODEC_HXX

#include <tools/toolsdllapi.h>

#define ZCODEC_NO_COMPRESSION       (0x00000000UL)
#define ZCODEC_BEST_SPEED           (0x00000001UL)
#define ZCODEC_DEFAULT_COMPRESSION  (0x00000006UL)
#define ZCODEC_BEST_COMPRESSION     (0x00000009UL)

#define ZCODEC_UPDATE_CRC           (0x00010000UL)
#define ZCODEC_GZ_LIB               (0x00020000UL)

#define ZCODEC_PNG_DEFAULT ( ZCODEC_NO_COMPRESSION | ZCODEC_UPDATE_CRC )
#define ZCODEC_DEFAULT  ( ZCODEC_DEFAULT_COMPRESSION )

class SvStream;

class TOOLS_DLLPUBLIC ZCodec
{
private:
    sal_uIntPtr     mbInit;
    bool            mbStatus;
    bool            mbFinish;
    SvStream*       mpIStm;
    sal_uInt8*      mpInBuf;
    sal_uIntPtr     mnInBufSize;
    sal_uIntPtr     mnInToRead;
    SvStream*       mpOStm;
    sal_uInt8*      mpOutBuf;
    sal_uIntPtr     mnOutBufSize;

    sal_uIntPtr     mnCRC;
    sal_uIntPtr     mnCompressMethod;
    void*           mpsC_Stream;

    void            ImplInitBuf( bool nIOFlag );
    void            ImplWriteBack();

public:
                    ZCodec( sal_uIntPtr nInBuf = 0x8000UL, sal_uIntPtr nOutBuf = 0x8000UL );
    virtual         ~ZCodec();

    virtual void    BeginCompression( sal_uIntPtr nCompressMethod = ZCODEC_DEFAULT );
    virtual long    EndCompression();
    bool            IsFinished () const { return mbFinish; }

    long            Compress( SvStream& rIStm, SvStream& rOStm );
    long            Decompress( SvStream& rIStm, SvStream& rOStm );

    long            Write( SvStream& rOStm, const sal_uInt8* pData, sal_uIntPtr nSize );
    long            Read( SvStream& rIStm, sal_uInt8* pData, sal_uIntPtr nSize );
    long            ReadAsynchron( SvStream& rIStm, sal_uInt8* pData, sal_uIntPtr nSize );

    void            SetBreak( sal_uIntPtr );
    sal_uIntPtr     GetBreak();
    void            SetCRC( sal_uIntPtr nCurrentCRC );
    sal_uIntPtr     UpdateCRC( sal_uIntPtr nLatestCRC, sal_uInt8* pSource, long nDatSize );
    sal_uIntPtr     GetCRC();
};

class GZCodec : public ZCodec
{
public:
                    GZCodec(){};
                    virtual ~GZCodec(){};
    virtual void    BeginCompression( sal_uIntPtr nCompressMethod = ZCODEC_DEFAULT ) SAL_OVERRIDE
    {
        ZCodec::BeginCompression( nCompressMethod | ZCODEC_GZ_LIB );
    };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
