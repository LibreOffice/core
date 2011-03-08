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

#ifndef _ZCODEC_HXX
#define _ZCODEC_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

// -----------
// - Defines -
// -----------

#define DEFAULT_IN_BUFSIZE          (0x00008000UL)
#define DEFAULT_OUT_BUFSIZE         (0x00008000UL)

#define MAX_MEM_USAGE 8

//
// memory requirement using compress:
//  [ INBUFFER ] + [ OUTBUFFER ] + 128KB + 1 << (MEM_USAGE+9)
//
// memory requirement using decompress:
//  [ INBUFFER ] + [ OUTBUFFER ] + 32KB
//

#define ZCODEC_NO_COMPRESSION       (0x00000000UL)
#define ZCODEC_BEST_SPEED           (0x00000001UL)
#define ZCODEC_DEFAULT_COMPRESSION  (0x00000006UL)
#define ZCODEC_BEST_COMPRESSION     (0x00000009UL)

#define ZCODEC_DEFAULT_STRATEGY     (0x00000000UL)
#define ZCODEC_ZFILTERED            (0x00000100UL)
#define ZCODEC_ZHUFFMAN_ONLY        (0x00000200UL)

#define ZCODEC_UPDATE_CRC           (0x00010000UL)
#define ZCODEC_GZ_LIB               (0x00020000UL)

#define ZCODEC_PNG_DEFAULT ( ZCODEC_NO_COMPRESSION | ZCODEC_DEFAULT_STRATEGY | ZCODEC_UPDATE_CRC )
#define ZCODEC_DEFAULT  ( ZCODEC_DEFAULT_COMPRESSION | ZCODEC_DEFAULT_STRATEGY )

// ----------
// - ZCodec -
// ----------

class SvStream;

class TOOLS_DLLPUBLIC ZCodec
{
private:

    ULONG           mbInit;
    BOOL            mbStatus;
    BOOL            mbFinish;
    ULONG           mnMemUsage;
    SvStream*       mpIStm;
    BYTE*           mpInBuf;
    ULONG           mnInBufSize;
    ULONG           mnInToRead;
    SvStream*       mpOStm;
    BYTE*           mpOutBuf;
    ULONG           mnOutBufSize;

    ULONG           mnCRC;
    ULONG           mnCompressMethod;
    void*           mpsC_Stream;

    void            ImplInitBuf( BOOL nIOFlag );
    void            ImplWriteBack( void );

public:
                    ZCodec( ULONG nInBuf, ULONG nOutBuf, ULONG nMemUsage = MAX_MEM_USAGE );
                    ZCodec( void );
    virtual         ~ZCodec();

    virtual void    BeginCompression( ULONG nCompressMethod = ZCODEC_DEFAULT );
    virtual long    EndCompression();
    BOOL            IsFinished () const { return mbFinish; }

    long            Compress( SvStream& rIStm, SvStream& rOStm );
    long            Decompress( SvStream& rIStm, SvStream& rOStm );

    long            Write( SvStream& rOStm, const BYTE* pData, ULONG nSize );
    long            Read( SvStream& rIStm, BYTE* pData, ULONG nSize );
    long            ReadAsynchron( SvStream& rIStm, BYTE* pData, ULONG nSize );

    void            SetBreak( ULONG );
    ULONG           GetBreak( void );
    void            SetCRC( ULONG nCurrentCRC );
    ULONG           UpdateCRC( ULONG nLatestCRC, ULONG nSource );
    ULONG           UpdateCRC( ULONG nLatestCRC, BYTE* pSource, long nDatSize );
    ULONG           GetCRC();
};

class GZCodec : public ZCodec
{

public:
                    GZCodec(){};
                    ~GZCodec(){};
    virtual void    BeginCompression( ULONG nCompressMethod = ZCODEC_DEFAULT );
};

#endif // _ZCODEC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
