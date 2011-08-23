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

#ifndef _LWPOBJECTID_HXX
#define _LWPOBJECTID_HXX

#include "lwpheader.hxx"
#include "lwpobjstrm.hxx"
#include "lwpdefs.hxx"

class LwpObject;

/**
 * @brief	object id class
*/
class LwpObjectID
{
public:
    LwpObjectID(sal_uInt32 low=0, sal_uInt16 high=0);
private:
    sal_uInt32 m_nLow;
    sal_uInt16 m_nHigh;
    sal_uInt8 m_nIndex;
    sal_Bool m_bIsCompressed;
public:
    sal_uInt32  Read( LwpSvStream *pStrm );
    sal_uInt32 Read( LwpObjectStream *pStrm );
    sal_uInt32 ReadIndexed( LwpSvStream* pStrm );
    sal_uInt32 ReadIndexed( LwpObjectStream *pStrm );
    sal_uInt32 ReadCompressed( LwpSvStream* pStrm, LwpObjectID &prev );
    sal_uInt32 ReadCompressed( LwpObjectStream* pObj, LwpObjectID& prev );

    sal_uInt32 DiskSize() const;
    sal_uInt32 DiskSizeIndexed() const;
    BOOL IsNull() const;
    sal_Bool IsCompressed();

    sal_uInt32 GetLow() const ;
    sal_uInt16 GetHigh() const ;
    void SetLow(sal_uInt32 nl);
    void SetHigh(sal_uInt16 nh);

    BOOL operator == (const LwpObjectID &Other) const;
    BOOL operator != (const LwpObjectID &Other) const;
    sal_Char* GetBuffer(sal_Char* buf);
    LwpObject* obj(VO_TYPE tag=VO_INVALID) const;
    size_t HashCode() const;
};

inline BOOL LwpObjectID::IsNull() const
{
    return (m_nLow == 0) && (m_nHigh == 0);
}

inline BOOL LwpObjectID::operator == (const LwpObjectID &Other) const
{
    return (m_nHigh == Other.m_nHigh) && (m_nLow == Other.m_nLow);
}

inline BOOL LwpObjectID::operator != (const LwpObjectID &Other) const
{
    return (m_nHigh != Other.m_nHigh) || (m_nLow != Other.m_nLow);
}

inline sal_Bool LwpObjectID::IsCompressed()
{
    return m_bIsCompressed;
}
inline sal_uInt32 LwpObjectID::GetLow() const
{
    return m_nLow;
}
inline sal_uInt16 LwpObjectID::GetHigh() const
{
    return m_nHigh;
}
inline void LwpObjectID::SetLow(sal_uInt32 nl)
{
    m_nLow=nl;
}
inline void LwpObjectID::SetHigh(sal_uInt16 nh)
{
    m_nHigh = nh;
}
inline size_t LwpObjectID::HashCode() const
{
    rtl::OUString str;
    if(m_nIndex)
    {
        str = rtl::OUString(m_nIndex) + rtl::OUString(m_nHigh);
    }
    else
    {
        str = rtl::OUString(m_nLow) + rtl::OUString(m_nHigh);
    }
    return str.hashCode();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
