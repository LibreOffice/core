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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSVSTREAM_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSVSTREAM_HXX

#include "lwpheader.hxx"
#include "xfilter/xfglobal.hxx"

class SvStream;

/**
 * @brief   encapsulate XInputStream to provide SvStream like interfaces
*/
class LwpSvStream
{
public:
    LwpSvStream(SvStream* pStream, LwpSvStream * pCompressed = nullptr);
    sal_Int32 Read( void* bytes, sal_Int32 nBytesToRead );
    sal_Int64 SeekRel( sal_Int64 pos );
    sal_Int64 Tell();
    sal_Int64 Seek( sal_Int64 pos );

    LwpSvStream& ReadUInt8( sal_uInt8& rUInt8 );
    LwpSvStream& ReadUInt16( sal_uInt16& rUInt16 );
    LwpSvStream& ReadUInt32( sal_uInt32& rUInt32 );

    static const sal_uInt32 LWP_STREAM_BASE;

    LwpSvStream * GetCompressedStream()
    {
        return m_pCompressedStream;
    };
    SvStream * GetStream()
    {
        return m_pStream;
    }

private:
    // when the file opened is small file, m_pStream is the decompressed stream
    // when not, m_pStream contains the normal stream
    SvStream* m_pStream;

    // when the file opened is small file, this stream is for saving the
    // compressed stream which is to be used for Chart/OLE as BENTO container
    LwpSvStream * m_pCompressedStream;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
