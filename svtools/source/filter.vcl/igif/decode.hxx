/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: decode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:41:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DECODE_HXX
#define _DECODE_HXX

#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

struct GIFLZWTableEntry;

class GIFLZWDecompressor
{
    GIFLZWTableEntry*   pTable;
    HPBYTE              pOutBuf;
    HPBYTE              pOutBufData;
    HPBYTE              pBlockBuf;
    ULONG               nInputBitsBuf;
    USHORT              nTableSize;
    USHORT              nClearCode;
    USHORT              nEOICode;
    USHORT              nCodeSize;
    USHORT              nOldCode;
    USHORT              nOutBufDataLen;
    USHORT              nInputBitsBufSize;
    BOOL                bEOIFound;
    BYTE                nDataSize;
    BYTE                nBlockBufSize;
    BYTE                nBlockBufPos;

    void                AddToTable(USHORT nPrevCode, USHORT nCodeFirstData);
    BOOL                ProcessOneCode();


public:

                        GIFLZWDecompressor( BYTE cDataSize );
                        ~GIFLZWDecompressor();

    HPBYTE              DecompressBlock( HPBYTE pSrc, BYTE cBufSize, ULONG& rCount, BOOL& rEOI );
};

#endif
