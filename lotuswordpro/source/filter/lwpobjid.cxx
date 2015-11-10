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

#include "lwpobjid.hxx"
#include "lwpfilehdr.hxx"
#include "lwpobjfactory.hxx"
#include "lwpglobalmgr.hxx"

LwpObjectID::LwpObjectID(sal_uInt32 low, sal_uInt16 high)
    :m_nLow(low), m_nHigh(high), m_nIndex(0), m_bIsCompressed(false)
{
}
/**
 * @descr       Read object id with format: low(4bytes)+high(2bytes) from stream
 *          for LWP7 record
*/
sal_uInt32 LwpObjectID::Read(LwpSvStream *pStrm)
{
    pStrm->ReadUInt32( m_nLow );
    pStrm->ReadUInt16( m_nHigh );
    return DiskSize();
}
/**
 * @descr       Read object id with format: low(4bytes)+high(2bytes) from object stream
*/
sal_uInt32 LwpObjectID::Read(LwpObjectStream *pObj)
{
    m_nLow = pObj->QuickReaduInt32();
    m_nHigh = pObj->QuickReaduInt16();
    return DiskSize();
}
/**
 * @descr       Read object id with indexed format from stream
 *          if index>0, lowid is get from time table per the index
*           else    index+lowid+highid
*/
sal_uInt32 LwpObjectID::ReadIndexed(LwpSvStream *pStrm)
{
//note the m_nLow store the index instead of time from the timetable as in LWP
    m_bIsCompressed = false;
    if( LwpFileHeader::m_nFileRevision < 0x000B)
    {
        return Read(pStrm);
    }

    pStrm->ReadUInt8( m_nIndex );

    if (m_nIndex)
    {
        m_bIsCompressed = true;
        //m_nLow = index;       //note the m_nLow stores the index instead of the actual time id
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpObjectFactory* pFactory = pGlobal->GetLwpObjFactory();
        LwpIndexManager& rIdxMgr = pFactory->GetIndexManager();
        m_nLow = rIdxMgr.GetObjTime( (sal_uInt16)m_nIndex);
    }
    else
    {
        pStrm->ReadUInt32( m_nLow );
    }
    pStrm->ReadUInt16( m_nHigh );
    return DiskSizeIndexed();
}

/**
 * @descr       Read object id with indexed format from object stream
 *          if index>0, lowid is get from time table per the index
*           else    index+lowid+highid
*/
sal_uInt32 LwpObjectID::ReadIndexed(LwpObjectStream *pStrm)
{
    m_bIsCompressed = false;
    if(LwpFileHeader::m_nFileRevision < 0x000B)
    {
        return Read(pStrm);
    }

    m_nIndex = pStrm->QuickReaduInt8();
    if (m_nIndex)
    {
        m_bIsCompressed = true;
        //m_nLow = index;       //note the m_nLow stores the index instead of the actual time id
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpObjectFactory* pFactory = pGlobal->GetLwpObjFactory();
        LwpIndexManager& rIdxMgr = pFactory->GetIndexManager();
        m_nLow = rIdxMgr.GetObjTime( (sal_uInt16)m_nIndex);
    }
    else
        m_nLow = pStrm->QuickReaduInt32();
    m_nHigh = pStrm->QuickReaduInt16();
    return DiskSizeIndexed();
}
/**
 * @descr       Read object id with compressed format from object stream
 *          if diff == 255: 255+lowid+highid
 *          else    lowid equals to the lowid of previous low id
 *              and high id = the high id of previous id + diff +1
*/
sal_uInt32 LwpObjectID::ReadCompressed( LwpObjectStream* pObj, LwpObjectID &prev )
{
    sal_uInt8 diff = pObj->QuickReaduInt8();
    sal_uInt32 len=1;

    if (diff == 255)
    {
        len += Read(pObj);
    }
    else
    {
        m_nLow = prev.GetLow();
        m_nHigh = prev.GetHigh() + diff +1;
    }
    return len;
}
/**
 * @descr       return the size of indexed object id
*/
sal_uInt32 LwpObjectID::DiskSizeIndexed() const
{
    return sizeof(sal_uInt8)
        + ((m_nIndex != 0) ? 0 : sizeof(m_nLow))
        + sizeof(m_nHigh);
}
/**
 * @descr       get object from object factory per the object id
*/
rtl::Reference<LwpObject> LwpObjectID::obj(VO_TYPE tag) const
{
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpObjectFactory* pObjMgr = pGlobal->GetLwpObjFactory();
    if(IsNull())
    {
        return nullptr;
    }
    rtl::Reference<LwpObject> pObj = pObjMgr->QueryObject(*this);
    if( tag!=VO_INVALID &&  (pObj.is()) )
    {
        if(static_cast<sal_uInt32>(tag) != pObj->GetTag())
        {
            pObj.clear();
        }
    }
    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
