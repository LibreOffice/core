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
#include "lwpsvstream.hxx"
#include <tools/stream.hxx>

const sal_uInt32 LwpSvStream::LWP_STREAM_BASE = 0x0010;

LwpSvStream::LwpSvStream(SvStream* pStream, LwpSvStream * pCompressed)
    : m_pStream(pStream), m_pCompressedStream(pCompressed)
{
}

/**
 * @descr       read nBytesToRead bytes to buf
*/
sal_Int32 LwpSvStream::Read(void* buf, sal_Int32 nBytesToRead)
{
    return m_pStream->Read(buf,nBytesToRead);
}

LwpSvStream& LwpSvStream::ReadUInt8( sal_uInt8& rUInt8 )
{
    m_pStream->ReadUChar( rUInt8 );
    return *this;
}

LwpSvStream& LwpSvStream::ReadUInt16( sal_uInt16& rUInt16 )
{
    m_pStream->ReadUInt16( rUInt16 );
    return *this;
}

LwpSvStream& LwpSvStream::ReadUInt32( sal_uInt32& rUInt32 )
{
    m_pStream->ReadUInt32( rUInt32 );
    return *this;
}

/**
 * @descr       SeekRel in stream
*/
sal_Int64 LwpSvStream::SeekRel(sal_Int64 pos)
{
    return m_pStream->SeekRel(pos);
}
/**
 * @descr       Get the current position in stream
*/
sal_Int64 LwpSvStream::Tell()
{
        return m_pStream->Tell();
}
/**
 * @descr       Seek to pos
*/
sal_Int64 LwpSvStream::Seek(sal_Int64 pos)
{
        return m_pStream->Seek(pos);
}
/**
 * @descr       Return the stream data length
*/
//sal_Int64 LwpSvStream::GetLength()
//{
//  return m_pStream->get
//}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
