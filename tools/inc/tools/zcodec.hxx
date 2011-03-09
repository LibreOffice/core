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

    sal_uIntPtr         mbInit;
    sal_Bool            mbStatus;
    sal_Bool            mbFinish;
    sal_uIntPtr         mnMemUsage;
    SvStream*       mpIStm;
    sal_uInt8*          mpInBuf;
    sal_uIntPtr         mnInBufSize;
    sal_uIntPtr         mnInToRead;
    SvStream*       mpOStm;
    sal_uInt8*          mpOutBuf;
    sal_uIntPtr         mnOutBufSize;

    sal_uIntPtr         mnCRC;
    sal_uIntPtr         mnCompressMethod;
    void*           mpsC_Stream;

    void            ImplInitBuf( sal_Bool nIOFlag );
    void            ImplWriteBack( void );

public:
                    ZCodec( sal_uIntPtr nInBuf, sal_uIntPtr nOutBuf, sal_uIntPtr nMemUsage = MAX_MEM_USAGE );
                    ZCodec( void );
    virtual         ~ZCodec();

    virtual void    BeginCompression( sal_uIntPtr nCompressMethod = ZCODEC_DEFAULT );
    virtual long    EndCompression();
    sal_Bool            IsFinished () const { return mbFinish; }

    long            Compress( SvStream& rIStm, SvStream& rOStm );
    long            Decompress( SvStream& rIStm, SvStream& rOStm );

    long            Write( SvStream& rOStm, const sal_uInt8* pData, sal_uIntPtr nSize );
    long            Read( SvStream& rIStm, sal_uInt8* pData, sal_uIntPtr nSize );
    long            ReadAsynchron( SvStream& rIStm, sal_uInt8* pData, sal_uIntPtr nSize );

    void            SetBreak( sal_uIntPtr );
    sal_uIntPtr         GetBreak( void );
    void            SetCRC( sal_uIntPtr nCurrentCRC );
    sal_uIntPtr         UpdateCRC( sal_uIntPtr nLatestCRC, sal_uIntPtr nSource );
    sal_uIntPtr         UpdateCRC( sal_uIntPtr nLatestCRC, sal_uInt8* pSource, long nDatSize );
    sal_uIntPtr         GetCRC();
};

class GZCodec : public ZCodec
{

public:
                    GZCodec(){};
                    ~GZCodec(){};
    virtual void    BeginCompression( sal_uIntPtr nCompressMethod = ZCODEC_DEFAULT );
};

#endif // _ZCODEC_HXX
