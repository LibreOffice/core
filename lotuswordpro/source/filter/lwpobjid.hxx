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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPOBJID_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPOBJID_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>

#include "lwpheader.hxx"
#include "lwpobjstrm.hxx"
#include "lwpdefs.hxx"

class LwpObject;

/**
 * @brief   object id class
*/
class LwpObjectID
{
public:
    LwpObjectID();
private:
    sal_uInt32 m_nLow;
    sal_uInt16 m_nHigh;
    sal_uInt8 m_nIndex;
    bool m_bIsCompressed;
public:
    void Read( LwpSvStream *pStrm );
    sal_uInt32 Read( LwpObjectStream *pStrm );
    void ReadIndexed( LwpSvStream* pStrm );
    sal_uInt32 ReadIndexed( LwpObjectStream *pStrm );
    void ReadCompressed( LwpObjectStream* pObj, LwpObjectID& prev );

    /**
     * @descr       return the size of object id with format: low(4bytes)+high(2bytes)
    */
    static sal_uInt32 DiskSize() { return sizeof(sal_uInt32) + sizeof(sal_uInt16); } // sizeof(m_nLow) + sizeof(m_nHigh)
    sal_uInt32 DiskSizeIndexed() const;
    bool IsNull() const;

    sal_uInt32 GetLow() const ;
    sal_uInt16 GetHigh() const ;

    bool operator == (const LwpObjectID &Other) const;
    bool operator != (const LwpObjectID &Other) const;
    rtl::Reference<LwpObject> obj(VO_TYPE tag=VO_INVALID) const;
    size_t HashCode() const;
};

inline bool LwpObjectID::IsNull() const
{
    return (m_nLow == 0) && (m_nHigh == 0);
}

inline bool LwpObjectID::operator == (const LwpObjectID &Other) const
{
    return (m_nHigh == Other.m_nHigh) && (m_nLow == Other.m_nLow);
}

inline bool LwpObjectID::operator != (const LwpObjectID &Other) const
{
    return (m_nHigh != Other.m_nHigh) || (m_nLow != Other.m_nLow);
}

inline sal_uInt32 LwpObjectID::GetLow() const
{
    return m_nLow;
}

inline sal_uInt16 LwpObjectID::GetHigh() const
{
    return m_nHigh;
}

inline size_t LwpObjectID::HashCode() const
{
    return static_cast<size_t>(
        (m_nIndex == 0 ? 23 * m_nLow : 27 * m_nIndex) + 29 * m_nHigh);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
