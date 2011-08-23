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
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#include "lwpobjhdr.hxx"
#include "lwpdefs.hxx"
#include "lwpfilehdr.hxx"

LwpObjectHeader::LwpObjectHeader()
    : m_nTag(0), m_ID(), m_nSize(0), m_bCompressed(sal_False)
    // m_nHeaderSize(0), m_nVersionID(0), m_nRefCount(0), m_nFlagBits(0),
    //m_nNextVersionOffset(0), m_nNextVersionID(0),
{};
/**
 * @descr  read header from stream
  */
void LwpObjectHeader::Read(LwpSvStream &rStrm)
{

    sal_uInt8 nFlagBits = 0;
    sal_uInt32 nVersionID = 0;
    sal_uInt32 nRefCount = 0;
    sal_uInt32 nNextVersionOffset = 0;
    sal_uInt32 nNextVersionID = 0;
    sal_uInt32 nHeaderSize = 0;

    if ( LwpFileHeader::m_nFileRevision < 0x000B)
    {
        rStrm.Read(&m_nTag, sizeof(m_nTag));
        m_ID.Read(&rStrm);
        rStrm.Read(&nVersionID, sizeof(nVersionID));
        rStrm.Read(&nRefCount, sizeof(nRefCount));
        rStrm.Read(&nNextVersionOffset, sizeof(nNextVersionOffset));

        nHeaderSize = sizeof(m_nTag) + m_ID.DiskSize()
            + sizeof(nVersionID)
            + sizeof(nRefCount)
            + sizeof(nNextVersionOffset)
            + sizeof(m_nSize);

        if ((m_nTag == TAG_AMI) || ( LwpFileHeader::m_nFileRevision < 0x0006))
        {
            rStrm.Read(&nNextVersionID, sizeof(nNextVersionID));
            nHeaderSize += sizeof(nNextVersionID);
        }
        rStrm.Read(&m_nSize, sizeof(m_nSize));
    }
    else
    {
        sal_uInt16 VOType = 0;
        rStrm.Read(&VOType, sizeof(VOType));
        rStrm.Read(&nFlagBits, sizeof(nFlagBits));

        m_nTag = static_cast<sal_uInt32>(VOType);
        m_ID.ReadIndexed(&rStrm);
        nHeaderSize = sizeof(VOType) + sizeof(nFlagBits) + m_ID.DiskSizeIndexed();

        sal_uInt8 tmpByte;
        sal_uInt16 tmpShort;
        switch (nFlagBits & VERSION_BITS)
        {
            case ONE_BYTE_VERSION:
                rStrm.Read(&tmpByte, sizeof(tmpByte));
                nVersionID = static_cast<sal_uInt32>( tmpByte );
                nHeaderSize++;
                break;

            case TWO_BYTE_VERSION:
                rStrm.Read(&tmpShort, sizeof(tmpShort));
                nVersionID = static_cast<sal_uInt32>( tmpShort );
                nHeaderSize += 2;
                break;

            case FOUR_BYTE_VERSION:
                rStrm.Read(&nVersionID, sizeof(nVersionID));
                nHeaderSize += 4;
                break;
            case DEFAULT_VERSION:	//fall through
            default:
                nVersionID = 2;
                break;		// nothing
        }

        switch (nFlagBits & REFCOUNT_BITS)
        {
            case ONE_BYTE_REFCOUNT:
                rStrm.Read(&tmpByte, sizeof(tmpByte));
                nRefCount = static_cast<sal_uInt32>( tmpByte );
                nHeaderSize++;
                break;

            case TWO_BYTE_REFCOUNT:
                rStrm.Read(&tmpShort, sizeof(tmpShort));
                nRefCount = static_cast<sal_uInt32>( tmpShort );
                nHeaderSize += 2;
                break;

            case FOUR_BYTE_REFCOUNT:	//through
            default:
                rStrm.Read(&nRefCount, sizeof(nRefCount));
                nHeaderSize += 4;
                break;
        }

        if (nFlagBits & HAS_PREVOFFSET)
        {
            rStrm.Read(&nNextVersionOffset, sizeof(nNextVersionOffset));
            nHeaderSize += 4;
        }
        else
            nNextVersionOffset = BAD_OFFSET;

        switch (nFlagBits & SIZE_BITS)
        {
            case ONE_BYTE_SIZE:
                rStrm.Read(&tmpByte, sizeof(tmpByte));
                m_nSize = static_cast<sal_uInt32>( tmpByte );
                nHeaderSize++;
                break;

            case TWO_BYTE_SIZE:
                rStrm.Read(&tmpShort, sizeof(tmpShort));
                m_nSize = static_cast<sal_uInt32>(tmpShort);
                nHeaderSize += 2;
                break;

            case FOUR_BYTE_SIZE:	//go through
            default:
                rStrm.Read(&m_nSize, sizeof(m_nSize));
                nHeaderSize += 4;
                break;
        }

        if (nFlagBits & DATA_COMPRESSED)
        {
            m_bCompressed = sal_True;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
