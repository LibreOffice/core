/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "explode.hxx"
#include <tools/stream.hxx>

#include <algorithm>
#include <assert.h>
#include <math.h>

    const char Tree1String[][32] = {
        "101",
        "11",
           "100",
        "011",
        "0101",
        "0100",
        "0011",
        "00101",
        "00100",
        "00011",
        "00010",
        "000011",
        "000010",
        "000001",
        "0000001",
        "0000000",
    };

    const char Tree2String[][32] = {
        "11"    ,
        "1011"  ,
           "1010"  ,
        "10011"  ,
        "10010"  ,
        "10001"  ,
        "10000"  ,
        "011111"  ,
        "011110"  ,
        "011101"  ,
        "011100"  ,
        "011011"  ,
        "011010"  ,
        "011001"  ,
        "011000"  ,
        "010111"  ,
        "010110" ,
        "010101" ,
        "010100" ,
        "010011" ,
        "010010" ,
        "010001" ,
        "0100001" ,
        "0100000" ,
        "0011111" ,
        "0011110" ,
        "0011101" ,
        "0011100" ,
        "0011011" ,
        "0011010" ,
        "0011001" ,
        "0011000" ,
        "0010111" ,
        "0010110" ,
        "0010101" ,
        "0010100" ,
        "0010011" ,
        "0010010" ,
        "0010001" ,
        "0010000" ,
        "0001111" ,
        "0001110" ,
        "0001101" ,
        "0001100" ,
        "0001011" ,
        "0001010" ,
        "0001001" ,
        "0001000" ,
        "00001111",
        "00001110",
        "00001101",
        "00001100",
        "00001011",
        "00001010",
        "00001001",
        "00001000",
        "00000111",
        "00000110",
        "00000101",
        "00000100",
        "00000011",
        "00000010",
        "00000001",
        "00000000",
    };

Decompression::Decompression(SvStream * pInStream, SvStream * pOutStream)
    : m_pInStream(pInStream)
    , m_pOutStream(pOutStream)
    , m_nCurrent4Byte(0)
    , m_nBitsLeft(0)
    , m_pBuffer(m_Buffer)
    , m_nBytesLeft(0)
    , m_nOutputBufferPos(0)
{
    if (!m_pInStream || !m_pOutStream )
    {
        assert(false);
    }
    ConstructTree1();
    ConstructTree2();
    fillArray();
}
/**
 * @descr   read specified bits from input stream
 * @argument iCount - number of bits to be read, less than 31
 * @argument nBits - bits read
 * @return  0 - read OK, otherwise error
 */
sal_uInt32 Decompression::ReadBits(sal_uInt16 iCount, sal_uInt32 & nBits)
{
    if ( (iCount == 0) || (iCount > 31 ) )
    {
        return 1;
    }

    /* load at least need bits into val */
    sal_uInt32 val = m_nCurrent4Byte; /* bit accumulator */
    while (m_nBitsLeft < iCount)
    {
        if (m_nBytesLeft == 0)
        {
            m_nBytesLeft = m_pInStream->ReadBytes(m_Buffer, CHUNK);
            m_pBuffer = m_Buffer;
            if (m_nBytesLeft == 0)  return 1;
        }
        val |= static_cast<sal_uInt32>(*m_pBuffer++) << m_nBitsLeft;       /* load eight bits */
        m_nBytesLeft --;
        m_nBitsLeft += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    m_nCurrent4Byte = val >> iCount;
    m_nBitsLeft -= iCount;

    /* return need bits, zeroing the bits above that */
    nBits = val & ((1U << iCount) - 1);

    return 0;
}
/**
 * @descr   decompress input and write output
 * @return  0 - read OK, otherwise error
 */
sal_Int32 Decompression::explode()
{
    /* The first 2 bytes are parameters */
    sal_uInt32 P1;
    if (0 != ReadBits(8, P1))/* 0 or 1 */
        return -1;

    /* I think this means 0=binary and 1=ascii file, but in RESOURCEs I saw always 0 */
    if (P1 >= 1) // changed per 's review comments
        return -1;

    sal_uInt32 P2;
    if (0 != ReadBits(8, P2))
        return -1;

    /* must be 4,5 or 6 and it is a parameter for the decompression algorithm */
    if (P2 < 4 || P2 > 6)
        return -2;

    m_nOutputBufferPos = 0;
    /* Now, a bit stream follows, which is decoded as described below: */
    /*  The algorithm terminates as soon as it runs out of bits. */
    while(true)
    {
        // read 1 bit (take bits from the lowest value (LSB) to the MSB i.e. bit 0, bit 1 etc ...)
        sal_uInt32 iBit;
        if (0 != ReadBits(1, iBit))
            break;
        if ( 0 == (iBit & 0x01) )
        {
            //if the bit is 0 read 8 bits and write it to the output as it is.
            sal_uInt32 symbol;
            if (0 != ReadBits(8, symbol))
                break;
            m_Output[m_nOutputBufferPos++] = static_cast<sal_uInt8>(symbol);
            if (m_nOutputBufferPos == MAXWIN)
            {
                m_pOutStream->WriteBytes(m_Output, m_nOutputBufferPos);
                m_nOutputBufferPos = 0;
            }
            continue;
        }
        // if the bit is 1 we have here a length/distance pair:
        // -decode a number with Hufmman Tree #1; variable bit length, result is 0x00 .. 0x0F -> L1
        sal_uInt32 L1 = Decode(m_Tree1.get());
        sal_uInt32 Length;
        if (L1 <= 7)
        {
            //if L1 <= 7:
            //           LENGTH = L1 + 2
            Length = L1 + 2;
        }
        else
        {
            // if L1 > 7
            //            read more (L1-7) bits -> L2
            //             LENGTH = L2 + M[L1-7] + 2
            sal_uInt32 L2;
            if (0 != ReadBits(static_cast<sal_uInt16>(L1 - 7), L2))
                break;
            Length = L2 + 2 + m_iArrayOfM[L1 -7];
        }
        if (Length == 519)
        {
            // end of compressed data
            break;
        }

        // - decode another number with Hufmann Tree #2 giving result 0x00..0x3F -> D1
        sal_uInt32 D1 = Decode(m_Tree2.get());
        sal_uInt32 D2;
        if (Length == 2)
        {
            //       if LENGTH == 2
            //              D1 = D1 << 2
            //              read 2 bits -> D2
            D1 = D1 << 2;
            if (0 != ReadBits(2, D2))
                break;
        }
        else
        {
            //        else
            //               D1 = D1 << P2  // the parameter 2
            //               read P2 bits -> D2
            D1 = D1 << P2;
            if (0 != ReadBits(static_cast<sal_uInt16>(P2), D2))
                break;
        }
        // DISTANCE = (D1 | D2) + 1
        sal_uInt32 distance = (D1 | D2) + 1;

            // - now copy LENGTH bytes from (output_ptr-DISTANCE) to output_ptr
            // write current buffer to output
        m_pOutStream->WriteBytes(m_Output, m_nOutputBufferPos);
        m_nOutputBufferPos = 0;

        // remember current position
        sal_uInt32 nOutputPos = m_pOutStream->Tell();
        if (distance > nOutputPos)
            return -3; // format error

        m_pOutStream->Flush();
        // point back to copy position and read bytes
        m_pOutStream->SeekRel(-static_cast<tools::Long>(distance));
        sal_uInt8 sTemp[MAXWIN];
        sal_uInt32 nRead = std::min(distance, Length);
        m_pOutStream->ReadBytes(sTemp, nRead);
        if (nRead != Length)
        {
            // fill the buffer with read content repeatedly until full
            for (sal_uInt32 i=nRead; i<Length; i++)
            {
                sTemp[i] = sTemp[i-nRead];
            }
        }

        // restore output stream position
        m_pOutStream->Seek(nOutputPos);

           // write current buffer to output
        m_pOutStream->WriteBytes(sTemp, Length);
    }
    return 0;
}
/**
 * @descr   bits to string
 * @return
 */
void Decompression::ToString(sal_uInt32 nBits, char *pChar, sal_uInt32 nLen)
{
    sal_uInt32 nBit;
    for (sal_uInt32 i=nLen; i > 0; i--)
    {
        nBit = (nBits >> (i -1) ) & 0x01;
        pChar[nLen - i]  = nBit ? '1':'0';
    }
    pChar[nLen] = '\0';
}

/**
 * @descr   decode tree 1 for length
 * @return  the decoded value
 */
sal_uInt32 Decompression::Decode(HuffmanTreeNode * pRoot)
{
    sal_uInt32 nRet(0);
    sal_uInt32 nRead, nReadAlready;

    if( 0 != ReadBits(1, nReadAlready))
        return 0; // something wrong

    for (sal_uInt16 i=2; i <= 8; i++)
    {
        if ( 0 != ReadBits(1, nRead))
            return 0; // something wrong

        nReadAlready  = (nReadAlready << 1) | (nRead & 0x01);

        char sCode[16];
        ToString(nReadAlready, sCode, i);
        nRet = pRoot->QueryValue(sCode);
        if (nRet != 0xffffffff)
        {
            break;
        }
    }
    return nRet;
}
/**
 * @descr   construct tree 1 for length
 * @return
 */
void Decompression::ConstructTree1()
{   // Huffman Tree #1
    // The first huffman tree (the Section called Decompression algorithm HUFFMAN) contains the length values. It is described by the following table:
    // value (hex)  code (binary)
    // 0    101
    // 1    11
    // 2    100
    // 3    011
    // 4    0101
    // 5    0100
    // 6    0011
    // 7    0010 1
    // 8    0010 0
    // 9    0001 1
    // a    0001 0
    // b    0000 11
    // c    0000 10
    // d    0000 01
    // e    0000 001
    // f    0000 000
    m_Tree1.reset( new HuffmanTreeNode());
    for (sal_uInt32 i=0; i< 16; i++)
    {
        m_Tree1->InsertNode(i, Tree1String[i]);
    }
    /*
    m_Tree1->InsertNode(0,   "101");
    m_Tree1->InsertNode(1,   "11");
    m_Tree1->InsertNode(2,   "100");
    m_Tree1->InsertNode(3,   "011");
    m_Tree1->InsertNode(4,   "0101");
    m_Tree1->InsertNode(5,   "0100");
    m_Tree1->InsertNode(6,   "0011");
    m_Tree1->InsertNode(7,   "00101");
    m_Tree1->InsertNode(8,   "00100");
    m_Tree1->InsertNode(9,   "00011");
    m_Tree1->InsertNode(10, "00010");
    m_Tree1->InsertNode(11, "000011");
    m_Tree1->InsertNode(12, "000010");
    m_Tree1->InsertNode(13, "000001");
    m_Tree1->InsertNode(14, "0000001");
    m_Tree1->InsertNode(15, "0000000");
    */
}
/**
 * @descr   construct tree 2 for distance
 * @return
 */
void Decompression::ConstructTree2()
{

    m_Tree2.reset(new HuffmanTreeNode());
    for (sal_uInt32 i=0; i< 64; i++)
    {
        m_Tree2->InsertNode(i, Tree2String[i]);
    }
    //where bits should be read from the left to the right.
}
/**
 * @descr
 * @return
 */
void Decompression::fillArray()
{
    m_iArrayOfM[0] = 7;
    for (int i=1; i < 16; i++)
    {
        m_iArrayOfM[i]  = m_iArrayOfM[i - 1]+ static_cast<sal_uInt32>(pow(2.0, i-1));//2
    }
}

HuffmanTreeNode::HuffmanTreeNode(sal_uInt32 nValue):value(nValue)
{
}
HuffmanTreeNode::~HuffmanTreeNode()
{
}

HuffmanTreeNode * HuffmanTreeNode::InsertNode(sal_uInt32 nValue, const char * pInCode)
{
    HuffmanTreeNode *pNew = new HuffmanTreeNode(nValue);
    std::string aCode(pInCode);

    // query its parents
    const char cLast = aCode.back();
    aCode.pop_back();
    HuffmanTreeNode * pParent = QueryNode(aCode.c_str());
    if (!pParent)
    {
        pParent = InsertNode(0xffffffff, aCode.c_str());
    }
    if (cLast == '0')
        pParent->left.reset(pNew);
    else // (cChar == '1')
        pParent->right.reset(pNew);

    return pNew;
}

HuffmanTreeNode * HuffmanTreeNode::QueryNode(const char * pCode)
{
    sal_uInt32 nLen = strlen(pCode);

    HuffmanTreeNode * pNode = this; // this is the root
    for(sal_uInt32 i=0; i<nLen && pNode; i++)
    {
        char cChar= pCode[i];
        if (cChar == '0')
        {
            pNode = pNode->left.get();
        }
        else // (cChar == '1')
        {
            pNode = pNode->right.get();
        }
    }
    return pNode;
}

sal_uInt32 HuffmanTreeNode::QueryValue(const char * pCode)
{
    HuffmanTreeNode * pNode =QueryNode(pCode);
    if (pNode)
        return pNode->value;

    return 0xffffffff;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
