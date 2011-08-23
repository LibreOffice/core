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

#ifndef _DECODE_HXX
#define _DECODE_HXX

#include <vcl/bmpacc.hxx>

namespace binfilter
{

struct GIFLZWTableEntry;

class GIFLZWDecompressor
{
    GIFLZWTableEntry*	pTable;
    HPBYTE				pOutBuf;
    HPBYTE				pOutBufData;
    HPBYTE				pBlockBuf;
    ULONG				nInputBitsBuf;
    USHORT				nTableSize;
    USHORT				nClearCode;
    USHORT				nEOICode;
    USHORT				nCodeSize;
    USHORT				nOldCode;
    USHORT				nOutBufDataLen;
    USHORT				nInputBitsBufSize;
    BOOL				bEOIFound;
    BYTE				nDataSize;
    BYTE				nBlockBufSize;
    BYTE				nBlockBufPos;

    void				AddToTable(USHORT nPrevCode, USHORT nCodeFirstData);
    BOOL				ProcessOneCode();


public:

                        GIFLZWDecompressor( BYTE cDataSize );
                        ~GIFLZWDecompressor();

    HPBYTE				DecompressBlock( HPBYTE pSrc, BYTE cBufSize, ULONG& rCount, BOOL& rEOI );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
