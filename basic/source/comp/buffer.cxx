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

#include <buffer.hxx>
#include <parser.hxx>

#include <basic/sberrors.hxx>

const sal_uInt32 UP_LIMIT=0xFFFFFF00;

// The SbiBuffer will be expanded in increments of at least 16 Bytes.
// This is necessary, because many classes emanate from a buffer length
// of x*16 Bytes.

SbiBuffer::SbiBuffer( SbiParser* p, short n )
    : pParser(p)
    , nInc(std::max(16, ((n + 15) / 16) * 16))
{
}

// Reach out the buffer
// This lead to the deletion of the buffer!

char* SbiBuffer::GetBuffer()
{
    pCur = nullptr;
    return pBuf.release();
}

// Test, if the buffer can contain n Bytes.
// In case of doubt it will be enlarged

bool SbiBuffer::Check( sal_Int32 n )
{
    if( !n )
    {
        return true;
    }
    if( nOff + n  >  nSize )
    {
        if( nInc == 0 )
        {
            return false;
        }

        sal_Int32 nn = 0;
        while( nn < n )
        {
            nn = nn + nInc;
        }
        if( ( nSize + nn ) > UP_LIMIT )
        {
            pParser->Error( ERRCODE_BASIC_PROG_TOO_LARGE );
            nInc = 0;
            pBuf.reset();
            return false;
        }
        auto p(std::make_unique<char[]>(nSize + nn));
        if (nSize)
            memcpy(p.get(), pBuf.get(), nSize);
        pBuf = std::move(p);
        pCur = pBuf.get() + nOff;
        nSize += nn;
    }
    return true;
}

// Patch of a Location

void SbiBuffer::Patch( sal_uInt32 off, sal_uInt32 val )
{
    if( ( off + sizeof( sal_uInt32 ) ) < nOff )
        write(pBuf.get() + off, val);
}

// Forward References upon label and procedures
// establish a linkage. The beginning of the linkage is at the passed parameter,
// the end of the linkage is 0.

void SbiBuffer::Chain( sal_uInt32 off )
{
    if (!pBuf)
        return;

    for (sal_uInt32 i = off; i;)
    {
        if( i >= nOff )
        {
            pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, "BACKCHAIN" );
            break;
        }
        char* ip = pBuf.get() + i;
        sal_uInt8* pTmp = reinterpret_cast<sal_uInt8*>(ip);
        i = pTmp[0] | (pTmp[1] << 8) | (pTmp[2] << 16) | (pTmp[3] << 24);
        write(ip, nOff);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
