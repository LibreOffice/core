/*************************************************************************
 *
 *  $RCSfile: ccidecom.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CCIDECOM_HXX
#define _CCIDECOM_HXX

#include <tools/stream.hxx>


#define CCI_OPTION_2D               1       // 2D-Komprimierung (statt 1D)
#define CCI_OPTION_EOL              2       // EOL-Codes am Ende jeder Zeile vorhanden
#define CCI_OPTION_BYTEALIGNEOL     4       // Fuellbits vor jedem EOL-Code, so dass
                                            // Ende von EOL auf Bytes aligend
#define CCI_OPTION_BYTEALIGNROW     8       // Rows beginnen immer auf Byte-Grenze
#define CCI_OPTION_INVERSEBITORDER  16

// Eintrag in eine Huffman-Tabelle:
struct CCIHuffmanTableEntry {
    USHORT nValue;    // Der Daten-Wert.
    USHORT nCode;     // Der Code durch den der Daten-Wert repraesentiert wird.
    USHORT nCodeBits; // Laenge des Codes in Bits.
};


// Eintrag in eine Hash-Tabelle zur schnellen Dekodierung
struct CCILookUpTableEntry {
    USHORT nValue;
    USHORT nCodeBits;
};


class CCIDecompressor {

public:

    CCIDecompressor( ULONG nOptions, UINT32 nImageWidth );
    ~CCIDecompressor();

    void StartDecompression( SvStream & rIStream );

    BOOL DecompressScanline(BYTE * pTarget, ULONG nTargetBits );

private:

    void MakeLookUp(const CCIHuffmanTableEntry * pHufTab,
                    const CCIHuffmanTableEntry * pHufTabSave,
                    CCILookUpTableEntry * pLookUp,
                    USHORT nHuffmanTableSize,
                    USHORT nMaxCodeBits);

    BOOL ReadEOL( UINT32 nMaxFillBits );

    BOOL Read2DTag();

    BYTE ReadBlackOrWhite();

    USHORT ReadCodeAndDecode(const CCILookUpTableEntry * pLookUp,
                             USHORT nMaxCodeBits);

    void FillBits(BYTE * pTarget, USHORT nTargetBits,
                  USHORT nBitPos, USHORT nNumBits,
                  BYTE nBlackOrWhite);

    USHORT CountBits(const BYTE * pData, USHORT nDataSizeBits,
                     USHORT nBitPos, BYTE nBlackOrWhite);

    void Read1DScanlineData(BYTE * pTarget, USHORT nTargetBits);

    void Read2DScanlineData(BYTE * pTarget, USHORT nTargetBits);

    BOOL bTableBad;

    BOOL bStatus;

    BYTE* pByteSwap;

    SvStream * pIStream;

    UINT32 nEOLCount;

    UINT32 nWidth;

    ULONG nOptions;

    BOOL bFirstEOL;

    CCILookUpTableEntry * pWhiteLookUp;
    CCILookUpTableEntry * pBlackLookUp;
    CCILookUpTableEntry * p2DModeLookUp;
    CCILookUpTableEntry * pUncompLookUp;

    ULONG nInputBitsBuf;
    USHORT nInputBitsBufSize;

    BYTE * pLastLine;
    ULONG nLastLineSize;
};


#endif

