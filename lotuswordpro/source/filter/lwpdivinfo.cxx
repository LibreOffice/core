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

#include "lwpdivinfo.hxx"
#include "lwpobjstrm.hxx"
#include "lwpfilehdr.hxx"
#include "lwpholder.hxx"
#include "lwppagehint.hxx"

LwpDivInfo::LwpDivInfo(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpObject(objHdr, pStrm)
{}

LwpDivInfo::~LwpDivInfo(){}

void LwpDivInfo::Read()
{

    SkipFront();
    m_ParentID.ReadIndexed(m_pObjStrm);
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }
    m_Name.Read(m_pObjStrm);
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }

    m_LayoutID.ReadIndexed(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nFlags, sizeof(m_nFlags));
    if (LwpFileHeader::m_nFileRevision < 0x0010)  // In 98, graphic links count too
    {
        if ((m_nFlags & DI_ANYOLEDDELINKS) == 0)
            m_nFlags &= ~DI_KNOWIFANYOLEDDELINKS;
    }

    m_ExternalName.Read(m_pObjStrm);
    m_ExternalType.Read(m_pObjStrm);
    m_ClassName.Read(m_pObjStrm);
    m_InitialLayoutID.ReadIndexed(m_pObjStrm);

    m_pObjStrm->QuickRead(&m_nPageNoStyle, sizeof(m_nPageNoStyle));
    m_TabColor.Read(m_pObjStrm);

    // read filler page stuff
    m_FillerPageTextID.ReadIndexed(m_pObjStrm);

    // read external file object stuff
    sal_uInt16 type;
    m_pObjStrm->QuickRead(&type, sizeof(type));
    //cpExternalFile = LNULL;

    assert(type==0);
    m_pObjStrm->SkipExtra();

}

void LwpDivInfo::SkipFront()
{
    LwpObjectID toSkip;

    toSkip.ReadIndexed(m_pObjStrm); // skip ListNext;
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }
    toSkip.ReadIndexed(m_pObjStrm);	// skip ListPrevious;
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }
    toSkip.ReadIndexed(m_pObjStrm);	// skip Head;
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        toSkip.ReadIndexed(m_pObjStrm);	//skip tail
        m_pObjStrm->SkipExtra();
    }
}

void LwpDivInfo::GetNumberOfPages(sal_uInt16 & nPageno)
{
    if(IsGotoable())
    {
        if (IsOleDivision())
        {
            //not support now
            return;
        }
        else
        {
            nPageno += GetMaxNumberOfPages();
        }
    }
}

sal_uInt16 LwpDivInfo::GetMaxNumberOfPages()
{
    LwpDocument* pDiv = GetDivision();
    if(!pDiv)
        return 0;
    LwpDLVListHeadTailHolder* pHeadTail = static_cast<LwpDLVListHeadTailHolder*>(pDiv->GetPageHintsID()->obj());
    if(pHeadTail)
    {
        LwpPageHint* pPageHint =static_cast<LwpPageHint*>(pHeadTail->GetTail()->obj());
        if(pPageHint && !pPageHint->GetPageLayoutID()->IsNull())
        {
            return pPageHint->GetPageNumber();
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
