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
#include "lwpobjid.hxx"
#endif

#include "lwpfilehdr.hxx"
#include "lwpobjfactory.hxx"
#include "lwpglobalmgr.hxx"

LwpObjectID::LwpObjectID(sal_uInt32 low, sal_uInt16 high)
    :m_nLow(low), m_nHigh(high), m_nIndex(0), m_bIsCompressed(sal_False)
{
}
/**
 * @descr		Read object id with format: low(4bytes)+high(2bytes) from stream
 *			for LWP7 record
*/
sal_uInt32 LwpObjectID::Read(LwpSvStream *pStrm)
{
    pStrm->Read(&m_nLow, sizeof(m_nLow));
    pStrm->Read(&m_nHigh, sizeof(m_nHigh));
    return DiskSize();
}
/**
 * @descr		Read object id with format: low(4bytes)+high(2bytes) from object stream
*/
sal_uInt32 LwpObjectID::Read(LwpObjectStream *pObj)
{
    pObj->QuickRead(&m_nLow, sizeof(m_nLow));
    pObj->QuickRead(&m_nHigh, sizeof(m_nHigh));
    return DiskSize();
}
/**
 * @descr		Read object id with indexed format from stream
 *			if index>0, lowid is get from time table per the index
*			else	index+lowid+highid
*/
sal_uInt32 LwpObjectID::ReadIndexed(LwpSvStream *pStrm)
{
//note the m_nLow store the index instead of time from the timetable as in LWP
    m_bIsCompressed = sal_False;
    if( LwpFileHeader::m_nFileRevision < 0x000B)
    {
        return Read(pStrm);
    }

    pStrm->Read(&m_nIndex, sizeof(m_nIndex));

    if (m_nIndex)
    {
        m_bIsCompressed = sal_True;
        //m_nLow = index;		//note the m_nLow stores the index instead of the actual time id
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpObjectFactory* pFactory = pGlobal->GetLwpObjFactory();
        LwpIndexManager* pIdxMgr = pFactory->GetIndexManager();
        m_nLow =  pIdxMgr->GetObjTime( (sal_uInt16)m_nIndex);
    }
    else
    {
        pStrm->Read(&m_nLow, sizeof(m_nLow));
    }
    pStrm->Read(&m_nHigh, sizeof(m_nHigh));
    return DiskSizeIndexed();
}

/**
 * @descr		Read object id with indexed format from object stream
 *			if index>0, lowid is get from time table per the index
*			else	index+lowid+highid
*/
sal_uInt32 LwpObjectID::ReadIndexed(LwpObjectStream *pStrm)
{
    m_bIsCompressed = sal_False;
    if(LwpFileHeader::m_nFileRevision < 0x000B)
    {
        return Read(pStrm);
    }

    pStrm->QuickRead(&m_nIndex, sizeof(m_nIndex));
    if (m_nIndex)
    {
        m_bIsCompressed = sal_True;
        //m_nLow = index;		//note the m_nLow stores the index instead of the actual time id
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpObjectFactory* pFactory = pGlobal->GetLwpObjFactory();
        LwpIndexManager* pIdxMgr = pFactory->GetIndexManager();
        m_nLow = pIdxMgr->GetObjTime( (sal_uInt16)m_nIndex);
    }
    else
    {
        pStrm->QuickRead(&m_nLow, sizeof(m_nLow));
    }
    pStrm->QuickRead(&m_nHigh, sizeof(m_nHigh));
    return DiskSizeIndexed();
}
/**
 * @descr		Read object id with compressed format from stream
 *			if diff == 255: 255+lowid+highid
 *			else	lowid equals to the lowid of previous low id
 * 				and high id = the high id of previous id + diff +1
*/
sal_uInt32 LwpObjectID::ReadCompressed( LwpSvStream* pStrm, LwpObjectID &prev )
{
    sal_uInt32 len=0;
    sal_uInt8 diff;

    len = pStrm->Read( &diff, sizeof(diff));
    if (diff == 255)
    {
        len += Read(pStrm);
    }
    else
    {
        m_nLow = prev.GetLow();
        m_nHigh = prev.GetHigh() + diff +1;
    }
    return len;
}
/**
 * @descr		Read object id with compressed format from object stream
 *			if diff == 255: 255+lowid+highid
 *			else	lowid equals to the lowid of previous low id
 * 				and high id = the high id of previous id + diff +1
*/
sal_uInt32 LwpObjectID::ReadCompressed( LwpObjectStream* pObj, LwpObjectID &prev )
{

    sal_uInt32 len=0;
    sal_uInt8 diff;

    len += pObj->QuickRead( &diff, sizeof(diff));

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
 * @descr		return the size of indexed object id
*/
sal_uInt32 LwpObjectID::DiskSizeIndexed() const
{
    return sizeof(sal_uInt8)
        + (((0 < m_nIndex) && (m_nIndex <= 255)) ? 0 : sizeof(m_nLow))
        + sizeof(m_nHigh);
}
/**
 * @descr		return the size of object id with format: low(4bytes)+high(2bytes)
*/
sal_uInt32 LwpObjectID::DiskSize() const
{
    return sizeof(m_nLow) + sizeof(m_nHigh);
}
/**
 * @descr		get object from object factory per the object id
*/
LwpObject* LwpObjectID::obj(VO_TYPE tag) const
{
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpObjectFactory* pObjMgr = pGlobal->GetLwpObjFactory();
    if(IsNull())
    {
        return NULL;
    }
    LwpObject* pObj = pObjMgr->QueryObject(*this);
    if( tag!=VO_INVALID &&  (pObj) )
    {
        if(tag!=pObj->GetTag())
        {
            pObj=NULL;
        }
    }
    return(pObj);
}
/**
 * @descr		returns a buffer that contains the highid + lowid
 */
sal_Char* LwpObjectID::GetBuffer(sal_Char *buf)
{
    buf[0] = m_nHigh && 0xFF00;
    buf[1] = m_nHigh && 0x00FF;
    buf[2] = m_nLow && 0xFF000000;
    buf[3] = m_nLow && 0x00FF0000;
    buf[4] = m_nLow && 0x0000FF00;
    buf[5] = m_nLow && 0x000000FF;
    return buf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
