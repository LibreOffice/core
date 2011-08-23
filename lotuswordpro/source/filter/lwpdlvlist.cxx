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
 *  list class implementation
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#include "lwpdlvlist.hxx"
#include "lwpfilehdr.hxx"
#include "lwpproplist.hxx"

LwpDLVList::LwpDLVList(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpObject(objHdr, pStrm)
{}
/**
 * @descr		Read LwpDLVList data from object stream
 **/
void LwpDLVList::Read()
{
    LwpObjectStream* pObjStrm = m_pObjStrm;
    m_ListNext.ReadIndexed(pObjStrm);
    if( LwpFileHeader::m_nFileRevision < 0x0006 )
        pObjStrm->SkipExtra();

    m_ListPrevious.ReadIndexed(pObjStrm);
    if( LwpFileHeader::m_nFileRevision < 0x0006 )
        pObjStrm->SkipExtra();

}
LwpDLNFVList::LwpDLNFVList(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
        : LwpDLVList(objHdr, pStrm)
{}
/**
 * @descr		Read LwpDLNFVList data from object stream
 **/
void LwpDLNFVList::Read()
{
    LwpDLVList::Read();

    LwpObjectStream* pObjStrm = m_pObjStrm;

    m_ChildHead.ReadIndexed(pObjStrm);
    if( LwpFileHeader::m_nFileRevision < 0x0006 || !m_ChildHead.IsNull() )
        m_ChildTail.ReadIndexed(pObjStrm);
    if( LwpFileHeader::m_nFileRevision < 0x0006 )
        pObjStrm->SkipExtra();

    m_Parent.ReadIndexed(pObjStrm);
    if( LwpFileHeader::m_nFileRevision < 0x0006 )
        pObjStrm->SkipExtra();

    ReadName(pObjStrm);
}
/**
 * @descr		Read name of LwpDLNFVList from object stream
 **/
void LwpDLNFVList::ReadName(LwpObjectStream* pObjStrm)
{
    m_Name.Read(pObjStrm);
    if( LwpFileHeader::m_nFileRevision < 0x0006 )
        pObjStrm->SkipExtra();
}
/**
 * @descr		ctor of LwpDLNFPVList from object stream
 *			Note that m_bHasProperties is initialized to true
 **/
LwpDLNFPVList::LwpDLNFPVList(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpDLNFVList(objHdr, pStrm),
    m_bHasProperties(sal_True),m_pPropList(NULL)
{}
/**
 * @descr		Read name of LwpDLNFVList from object stream
 **/
void LwpDLNFPVList::Read()
{
    LwpDLNFVList::Read();

    LwpObjectStream* pObjStrm = m_pObjStrm;
    ReadPropertyList(pObjStrm);
    pObjStrm->SkipExtra();
}
/**
 * @descr		Read property list
 **/
void LwpDLNFPVList::ReadPropertyList(LwpObjectStream* pObjStrm)
{
    if( LwpFileHeader::m_nFileRevision >= 0x0000B)
    {
        pObjStrm->QuickRead(&m_bHasProperties, sizeof(m_bHasProperties));
        if (m_bHasProperties)
        {
            m_pPropList = new LwpPropList;
            m_pPropList->Read(pObjStrm);
        }
    }
}
/**
 * @descr		release property list
 **/
LwpDLNFPVList::~LwpDLNFPVList()
{
    if(m_pPropList)
    {
        delete m_pPropList;
    }
}

/**
 * @descr		Read head id and tail id
 **/
void LwpDLVListHeadTail::Read(LwpObjectStream* pObjStrm)
{
    m_ListHead.ReadIndexed(pObjStrm);
    if( (LwpFileHeader::m_nFileRevision < 0x0006) || !m_ListHead.IsNull())
    {
        m_ListTail.ReadIndexed(pObjStrm);
    }
    if( LwpFileHeader::m_nFileRevision < 0x0006)
    {
        pObjStrm->SkipExtra();
    }
}
/**
 * @descr		Read head id
 **/
void LwpDLVListHead::Read(LwpObjectStream* pObjStrm)
{
    m_objHead.ReadIndexed(pObjStrm);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
