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
 * Tabrack for LwpTabPiece object.
 ************************************************************************/
#include "lwptabrack.hxx"
#include "lwpobjstrm.hxx"
#include "lwpslvlist.hxx"

LwpTab::LwpTab()
{
    m_nX = 0;
    m_nAlignChar = 0;       //be careful,not quite sure it's 8-bit,perhaps 16-bit.
    m_nType = 0;
    m_nLeader = 0;
    m_nRelativeType = 0;
}

void    LwpTab::Read(LwpObjectStream *pStrm)
{
    m_nX = pStrm->QuickReaduInt32();
    m_nType = pStrm->QuickReaduInt8();
    m_nLeader = pStrm->QuickReaduInt8();
    m_nRelativeType = pStrm->QuickReaduInt8();
    m_nAlignChar = pStrm->QuickReaduInt16();
}

LwpTabRack::LwpTabRack(LwpObjectHeader objHdr, LwpSvStream* pStrm):LwpObject(objHdr,pStrm)
{
    m_nNumTabs = 0;
}

void LwpTabRack::Read()
{
//  LwpObjectID     m_NextID;
    m_NextID.ReadIndexed(m_pObjStrm);

    m_nNumTabs = m_pObjStrm->QuickReaduInt16();
    for( int i=0; i<m_nNumTabs; i++ )
    {
        m_aTabs[i].Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
    m_pObjStrm->SkipExtra();
}

LwpTab* LwpTabRack::Lookup(sal_uInt16 nIndex)
{
    /* Is the tab in this tabrack? */
    if(nIndex<m_nNumTabs)
    {
        return &m_aTabs[nIndex];
    }
    //return NULL;
    /* It's not in this tabrack, so get it out of our next. */
    if (!GetNext())
        return nullptr;        /* ouch */

    return GetNext()->Lookup(nIndex - m_nNumTabs);
}

LwpTabRack* LwpTabRack::GetNext()
{
    LwpTabRack* pTabRack = dynamic_cast<LwpTabRack*>(m_NextID.obj().get());
    return pTabRack;
}

sal_uInt16 LwpTabRack::GetNumTabs()
{
    sal_uInt16 nNum=0;
    if(GetNext())
    {
        nNum = GetNext()->GetNumTabs();
    }
    return m_nNumTabs+nNum;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
