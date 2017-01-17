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

#include <string.h>
#include <stdlib.h>
#include <rtl/strbuf.hxx>
#include <tools/stream.hxx>
#include "dxfgrprd.hxx"

// we use an own ReadLine function, because Stream::ReadLine stops if
// a 0-sign occurs; this function converts 0-signs to blanks and reads
// a complete line until a cr/lf is found

OString DXFReadLine(SvStream& rIStm)
{
    char  buf[256 + 1];
    bool  bEnd = false;
    sal_uLong nOldFilePos = rIStm.Tell();
    char  c = 0;

    OStringBuffer aBuf;

    while( !bEnd && !rIStm.GetError() )   // !!! do not check for EOF
                                          // !!! because we read blockwise
    {
        sal_uInt16 nLen = (sal_uInt16)rIStm.Read( buf, sizeof(buf)-1 );
        if( !nLen )
        {
            if( aBuf.isEmpty() )
                return OString();
            else
                break;
        }

        for( sal_uInt16 n = 0; n < nLen ; n++ )
        {
            c = buf[n];
            if( c != '\n' && c != '\r' )
            {
                if( !c )
                    c = ' ';
                aBuf.append(c);
            }
            else
            {
                bEnd = true;
                break;
            }
        }
    }

    if( !bEnd && !rIStm.GetError() && !aBuf.isEmpty() )
        bEnd = true;

    nOldFilePos += aBuf.getLength();
    if( rIStm.Tell() > nOldFilePos )
        nOldFilePos++;
    rIStm.Seek( nOldFilePos );  // seek because of BlockRead above!

    if( bEnd && (c=='\r' || c=='\n'))  // special treatment of DOS files
    {
        char cTemp(0);
        rIStm.Read(&cTemp, 1);
        if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
            rIStm.Seek( nOldFilePos );
    }

    return aBuf.makeStringAndClear();
}

void DXFSkipLine(SvStream& rIStm)
{
    while (rIStm.good())
    {
        char  buf[256 + 1];
        sal_uInt16 nLen = (sal_uInt16)rIStm.Read(buf, sizeof(buf) - 1);
        for (sal_uInt16 n = 0; n < nLen; n++)
        {
            char c = buf[n];
            if ((c == '\n') || (c == '\r'))
            {
                rIStm.SeekRel(n-nLen+1); // return stream to next to current position
                char c1 = 0;
                rIStm.Read(&c1, 1);
                if (c1 == c || (c1 != '\n' && c1!= '\r'))
                    rIStm.SeekRel(-1);
                return;
            }
        }
    }
}

DXFGroupReader::DXFGroupReader(SvStream & rIStream)
  : rIS(rIStream)
  , bStatus(true)
  , nLastG(0)
  , nGCount(0)
  , S()
  , I(0)
{
    rIS.Seek(STREAM_SEEK_TO_END);
    nFileSize=rIS.Tell();
    rIS.Seek(0);
}

sal_uInt16 DXFGroupReader::Read()
{
    sal_uInt16 nG = 0;
    if ( bStatus )
    {
        nGCount++;
        nG = (sal_uInt16)ReadI();
        if ( bStatus )
        {
            if      (nG<  10) ReadS();
            else if (nG<  60) F = ReadF();
            else if (nG<  80) I = ReadI();
            else if (nG<  90) DXFSkipLine(rIS);
            else if (nG<  99) I = ReadI();
            else if (nG==100) ReadS();
            else if (nG==102) ReadS();
            else if (nG==105) DXFSkipLine(rIS);
            else if (nG< 140) DXFSkipLine(rIS);
            else if (nG< 148) F = ReadF();
            else if (nG< 170) DXFSkipLine(rIS);
            else if (nG< 176) I = ReadI();
            else if (nG< 180) DXFSkipLine(rIS); // ReadI();
            else if (nG< 210) DXFSkipLine(rIS);
            else if (nG< 240) F = ReadF();
            else if (nG<=369) DXFSkipLine(rIS);
            else if (nG< 999) DXFSkipLine(rIS);
            else if (nG<1010) ReadS();
            else if (nG<1060) F = ReadF();
            else if (nG<1072) I = ReadI();
            else bStatus = false;
        }
    }
    if ( !bStatus )
    {
        nG = 0;
        SetS();
        if ( nGCount != 0xffffffff )
        {
            // InfoBox(NULL,String("Error in group # ")+String(nGCount)).Execute();
            nGCount=0xffffffff;
        }
    }
    nLastG = nG;
    return nG;
}

void DXFGroupReader::SetS()
{
    S = "EOF";
}

long DXFGroupReader::ReadI()
{
    OString s = DXFReadLine(rIS);
    char *p=s.pData->buffer;
    const char *end = s.pData->buffer + s.pData->length;

    while((p != end) && (*p==0x20)) p++;

    if ((p == end) || ((*p<'0' || *p>'9') && *p!='-')) {
        bStatus=false;
        return 0;
    }

    long res = 0, nv = 1;
    if (*p == '-') {
        nv=-1;
        p++;
    }

    while ((p != end) && *p >= '0' && *p <= '9') {
        res=res*10+static_cast<long>(*p-'0');
        p++;
    }

    while ((p != end) && (*p==0x20)) p++;
    if (p != end) {
        bStatus=false;
        return 0;
    }

    return res*nv;
}

double DXFGroupReader::ReadF()
{
    OString s = DXFReadLine(rIS);
    char *p = s.pData->buffer;
    const char *end = s.pData->buffer + s.pData->length;

    while((p != end) && (*p==0x20)) p++;
    if ((p == end) || ((*p<'0' || *p>'9') && *p!='.' && *p!='-')) {
        bStatus=false;
        return 0.0;
    }
    return atof(p);
}

void DXFGroupReader::ReadS()
{
    S = DXFReadLine(rIS);
}

sal_uInt64 DXFGroupReader::remainingSize() const
{
    return rIS.remainingSize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
