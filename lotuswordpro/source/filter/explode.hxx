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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_EXPLODE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_EXPLODE_HXX

#include <sal/types.h>
#include <memory>

class SvStream;

class HuffmanTreeNode
{
    std::unique_ptr<HuffmanTreeNode> left;
    std::unique_ptr<HuffmanTreeNode> right;
    sal_uInt32 value;
public:
    explicit HuffmanTreeNode(sal_uInt32 value = 0xffffffff) ;
    ~HuffmanTreeNode() ;
    HuffmanTreeNode * InsertNode(sal_uInt32 nValue, const sal_Char * pInCode);
    HuffmanTreeNode * QueryNode(const sal_Char *pCode);
    sal_uInt32 QueryValue(const sal_Char *pCode);
};

/**
 * @brief
 * define the function type for input read, output write
 */
#define CHUNK 16384
#define MAXWIN 4096     /* maximum window size */
class Decompression
{
public:
    Decompression(SvStream * pInStream, SvStream * pOutStream);
    /**
     * @brief
     * decompress from instream to outstream
     */
    sal_Int32 explode();
    sal_uInt32 ReadBits(sal_uInt16 iCount, sal_uInt32 & nBits) ;
    sal_uInt32 Decode(HuffmanTreeNode * pRoot);

    /**
     * @brief
     * compressed/decompressed stream
     */
    SvStream *m_pInStream;
    SvStream *m_pOutStream;

    sal_uInt32 m_nCurrent4Byte;   // bit buffer
    sal_uInt32 m_nBitsLeft;     // number of bits remained in bit buffer

    sal_uInt8 m_Buffer[CHUNK];  // input byte buffer
    sal_uInt8 *m_pBuffer;           // pointer to input buffer
    sal_uInt32 m_nBytesLeft;        // number of bytes remained in byte buffer

    sal_uInt8 m_Output[MAXWIN]; // output byte buffer
    sal_uInt32 m_nOutputBufferPos;  // pointer to output buffer

    sal_uInt32 m_iArrayOfM[16];

    std::unique_ptr<HuffmanTreeNode> m_Tree1, m_Tree2;

    void ConstructTree1();
    void ConstructTree2();
    void fillArray();
    static void ToString(sal_uInt32 nBits, sal_Char *pChar, sal_uInt32 nLen);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
