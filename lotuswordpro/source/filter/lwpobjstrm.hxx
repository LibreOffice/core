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
/*************************************************************************
 * @file
 *  object stream header file
 *  stream for one LwpObject body data
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPOBJSTRM_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPOBJSTRM_HXX

#include "lwpheader.hxx"
#include "lwpsvstream.hxx"
/**
 * @brief   stream class for LwpObject body data
 *          provide stream like interface to read object data
*/
class LwpObjectStream
{
public:
    LwpObjectStream(LwpSvStream *pStrm, bool isCompressed, sal_uInt16 size);
    ~LwpObjectStream();
private:
    sal_uInt8* m_pContentBuf;           //The content buffer of the object
    sal_uInt8 m_SmallBuffer[100];       //To avoid frequent new
    enum
    {
        IO_BUFFERSIZE = 0xFF00      //Refer to LWP, not sure if it is enough
    };
    sal_uInt16 m_nBufSize;              //The total size of m_pContentBuf
    sal_uInt16 m_nReadPos;          //The position of the quick read
    LwpSvStream* m_pStrm;
    bool m_bCompressed;
public:
    sal_uInt16 QuickRead(void* buf, sal_uInt16 len);
    sal_uInt16 GetPos() { return m_nReadPos; }
    void SeekRel(sal_uInt16 pos);
    bool Seek( sal_uInt16 pos);
    void SkipExtra();
    sal_uInt16 CheckExtra();

    bool QuickReadBool(bool *pFailure=nullptr);
    sal_uInt32 QuickReaduInt32(bool *pFailure=nullptr);
    sal_uInt16 QuickReaduInt16(bool *pFailure=nullptr);
    sal_uInt8 QuickReaduInt8(bool *pFailure=nullptr);
    sal_Int32 QuickReadInt32(bool *pFailure=nullptr);
    sal_Int16 QuickReadInt16(bool *pFailure=nullptr);
    double QuickReadDouble(bool *pFailure=nullptr);

    OUString QuickReadStringPtr();

    void ReadComplete();
    LwpSvStream *GetStream();
private:
    void ReadStream();
    void Read2Buffer();
    sal_uInt8* AllocBuffer(sal_uInt16 size);
    static sal_uInt16 DecompressBuffer(sal_uInt8* pDst, sal_uInt8* pSrc, sal_uInt16 Size);
    void ReleaseBuffer();
};

inline LwpSvStream *LwpObjectStream::GetStream()
{
    return m_pStrm;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
