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

#ifndef INCLUDED_TOOLS_XZCODEC_HXX
#define INCLUDED_TOOLS_XZCODEC_HXX

#include <tools/toolsdllapi.h>

class SvStream;

class TOOLS_DLLPUBLIC XZCodec
{
    enum State { STATE_INIT, STATE_DECOMPRESS, STATE_COMPRESS };
    State           meState;
    bool            mbStatus;
    sal_uInt8*      mpInBuf;
    sal_uIntPtr     mnInBufSize;
    sal_uInt8*      mpOutBuf;
    sal_uIntPtr     mnOutBufSize;

    void*           mpLzma_Stream;

    void            InitDecompress(SvStream & inStream);

public:
                    XZCodec( sal_uIntPtr nInBuf = 0x8000UL, sal_uIntPtr nOutBuf = 0x8000UL );
                    ~XZCodec();

    void            BeginCompression( void );
    long            EndCompression( void );

    long            Compress( SvStream& rIStm, SvStream& rOStm );
    long            Decompress( SvStream& rIStm, SvStream& rOStm );
    bool            AttemptDecompression( SvStream& rIStm, SvStream& rOStm );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
