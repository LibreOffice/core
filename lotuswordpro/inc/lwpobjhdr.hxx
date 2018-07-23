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

#ifndef INCLUDED_LOTUSWORDPRO_INC_LWPOBJHDR_HXX
#define INCLUDED_LOTUSWORDPRO_INC_LWPOBJHDR_HXX

#include "lwpobjid.hxx"
#include "lwpobjtags.hxx"
/**
 * @brief   Base class of all Lwp VO objects
*/
class LwpObjectHeader
{
public:
    LwpObjectHeader();
private:
    sal_uInt32 m_nTag;
    LwpObjectID m_ID;
    sal_uInt32 m_nSize;     //Data size
    bool m_bCompressed;

    //localize the following members to save memory
    //sal_uInt8 m_nFlagBits;
    //sal_uInt32 m_nVersionID;
    //sal_uInt32 m_nRefCount;
    //sal_uInt32 m_nNextVersionOffset;
    //sal_uInt32 m_nNextVersionID;
    //sal_uInt32 m_nHeaderSize;

    enum
    {
        VERSION_BITS            = 0x03,
        DEFAULT_VERSION         = 0x00,
        ONE_BYTE_VERSION        = 0x01,
        TWO_BYTE_VERSION        = 0x02,
        FOUR_BYTE_VERSION       = 0x03,

        REFCOUNT_BITS           = 0x0C,
        ONE_BYTE_REFCOUNT       = 0x04,
        TWO_BYTE_REFCOUNT       = 0x08,
        FOUR_BYTE_REFCOUNT      = 0x0C,

        SIZE_BITS               = 0x30,
        ONE_BYTE_SIZE           = 0x10,
        TWO_BYTE_SIZE           = 0x20,
        FOUR_BYTE_SIZE          = 0x30,

        HAS_PREVOFFSET          = 0x40,
        DATA_COMPRESSED     = 0x80
    };

public:
    bool Read(LwpSvStream &pStrm);
    inline sal_uInt32 GetTag();
    inline sal_uInt32 GetSize();
    inline LwpObjectID& GetID();
    inline bool IsCompressed();
};

inline sal_uInt32 LwpObjectHeader::GetTag()
{
    return m_nTag;
}
inline sal_uInt32 LwpObjectHeader::GetSize()
{
    return m_nSize;
}
inline LwpObjectID& LwpObjectHeader::GetID()
{
    return m_ID;
}
inline bool LwpObjectHeader::IsCompressed()
{
    return m_bCompressed;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
