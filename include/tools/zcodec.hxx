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

#define ZCODEC_NO_COMPRESSION       0
#define ZCODEC_DEFAULT_COMPRESSION  6

class SvStream;

// The overall client call protocol is one of:
// * BeginCompression, Compress, EndCompression
// * BeginCompression, Decompress, EndCompression
// * BeginCompression, Write*, EndCompression
// * BeginCompression, Read*, EndCompression
// * BeginCompression, ReadAsynchron*, EndCompression
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC ZCodec
{
    enum State { STATE_INIT, STATE_DECOMPRESS, STATE_COMPRESS };
    State           meState;
    bool            mbStatus;
    bool            mbFinish;
    sal_uInt8*      mpInBuf;
    size_t          mnInBufSize;
    size_t          mnInToRead;
    SvStream*       mpOStm;
    sal_uInt8*      mpOutBuf;
    size_t          mnOutBufSize;

    sal_uInt32      mnCRC;
    int             mnCompressLevel;
    bool            mbUpdateCrc;
    bool            mbGzLib;
    void*           mpsC_Stream;

    void            InitCompress();
    void            InitDecompress(SvStream & inStream);
    void            ImplWriteBack();
    void            UpdateCRC( sal_uInt8 const * pSource, long nDatSize );

public:
                    ZCodec( size_t nInBufSize = 32768, size_t nOutBufSize = 32768 );
                    ~ZCodec();

    void            BeginCompression( int nCompressLevel = ZCODEC_DEFAULT_COMPRESSION, bool updateCrc = false, bool gzLib = false );
    long            EndCompression();

    void            Compress( SvStream& rIStm, SvStream& rOStm );
    long            Decompress( SvStream& rIStm, SvStream& rOStm );
    bool            AttemptDecompression( SvStream& rIStm, SvStream& rOStm );

    void            Write( SvStream& rOStm, const sal_uInt8* pData, sal_uInt32 nSize );
    long            Read( SvStream& rIStm, sal_uInt8* pData, sal_uInt32 nSize );
    long            ReadAsynchron( SvStream& rIStm, sal_uInt8* pData, sal_uInt32 nSize );

    void            SetBreak( size_t );
    size_t          GetBreak() const;
    void            SetCRC( sal_uInt32 nCurrentCRC );
    sal_uInt32      GetCRC() const { return mnCRC;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
