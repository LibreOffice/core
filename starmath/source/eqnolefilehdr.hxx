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

#include <sal/types.h>
#include <tools/stream.hxx>

class SotStorage;

#define EQNOLEFILEHDR_SIZE 28

class EQNOLEFILEHDR
{
public:
    EQNOLEFILEHDR() : nCBHdr(0),nVersion(0),
        nCf(0),nCBObject(0),nReserved1(0),nReserved2(0),
        nReserved3(0), nReserved4(0) {}
    explicit EQNOLEFILEHDR(sal_uInt32 nLenMTEF) : nCBHdr(0x1c),nVersion(0x20000),
        nCf(0xc1c6),nCBObject(nLenMTEF),nReserved1(0),nReserved2(0x0014F690),
        nReserved3(0x0014EBB4), nReserved4(0) {}

    sal_uInt16   nCBHdr;     // length of header, sizeof(EQNOLEFILEHDR) = 28
    sal_uInt32   nVersion;   // hiword = 2, loword = 0
    sal_uInt16   nCf;        // clipboard format ("MathType EF")
    sal_uInt32   nCBObject;  // length of MTEF data following this header
    sal_uInt32   nReserved1; // not used
    sal_uInt32   nReserved2; // not used
    sal_uInt32   nReserved3; // not used
    sal_uInt32   nReserved4; // not used

    void Read(SvStream* pS)
    {
        pS->ReadUInt16( nCBHdr );
        pS->ReadUInt32( nVersion );
        pS->ReadUInt16( nCf );
        pS->ReadUInt32( nCBObject );
        pS->ReadUInt32( nReserved1 );
        pS->ReadUInt32( nReserved2 );
        pS->ReadUInt32( nReserved3 );
        pS->ReadUInt32( nReserved4 );
    }
    void Write(SvStream* pS)
    {
        pS->WriteUInt16( nCBHdr );
        pS->WriteUInt32( nVersion );
        pS->WriteUInt16( nCf );
        pS->WriteUInt32( nCBObject );
        pS->WriteUInt32( nReserved1 );
        pS->WriteUInt32( nReserved2 );
        pS->WriteUInt32( nReserved3 );
        pS->WriteUInt32( nReserved4 );
    }
};

bool GetMathTypeVersion( SotStorage* pStor, sal_uInt8 &nVersion );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
