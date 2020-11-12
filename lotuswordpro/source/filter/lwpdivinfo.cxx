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

#include "lwpdivinfo.hxx"
#include "lwpdoc.hxx"
#include <lwpobjstrm.hxx>
#include <lwpfilehdr.hxx>
#include "lwpholder.hxx"
#include "lwppagehint.hxx"
#include <sal/log.hxx>

LwpDivInfo::LwpDivInfo(LwpObjectHeader const& objHdr, LwpSvStream* pStrm)
    : LwpObject(objHdr, pStrm)
    , m_nFlags(0)
    , m_nPageNoStyle(0)
{
}

LwpDivInfo::~LwpDivInfo() {}

void LwpDivInfo::Read()
{
    SkipFront();
    m_ParentID.ReadIndexed(m_pObjStrm.get());
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }
    m_Name.Read(m_pObjStrm.get());
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }

    m_LayoutID.ReadIndexed(m_pObjStrm.get());
    m_nFlags = m_pObjStrm->QuickReaduInt16();
    if (LwpFileHeader::m_nFileRevision < 0x0010) // In 98, graphic links count too
    {
        if ((m_nFlags & DI_ANYOLEDDELINKS) == 0)
            m_nFlags &= ~DI_KNOWIFANYOLEDDELINKS;
    }

    m_ExternalName.Read(m_pObjStrm.get());
    m_ExternalType.Read(m_pObjStrm.get());
    m_ClassName.Read(m_pObjStrm.get());
    m_InitialLayoutID.ReadIndexed(m_pObjStrm.get());

    m_nPageNoStyle = m_pObjStrm->QuickReaduInt16();
    m_TabColor.Read(m_pObjStrm.get());

    // read filler page stuff
    m_FillerPageTextID.ReadIndexed(m_pObjStrm.get());

    // read external file object stuff
    sal_uInt16 type = m_pObjStrm->QuickReaduInt16();
    //cpExternalFile = LNULL;

    SAL_WARN_IF(type != 0, "lwp", "should be 0");
    m_pObjStrm->SkipExtra();
}

void LwpDivInfo::SkipFront()
{
    LwpObjectID toSkip;

    toSkip.ReadIndexed(m_pObjStrm.get()); // skip ListNext;
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }
    toSkip.ReadIndexed(m_pObjStrm.get()); // skip ListPrevious;
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        m_pObjStrm->SkipExtra();
    }
    toSkip.ReadIndexed(m_pObjStrm.get()); // skip Head;
    if (LwpFileHeader::m_nFileRevision < 0x0006)
    {
        toSkip.ReadIndexed(m_pObjStrm.get()); //skip tail
        m_pObjStrm->SkipExtra();
    }
}

void LwpDivInfo::GetNumberOfPages(sal_uInt16& nPageno)
{
    if (IsGotoable())
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

sal_uInt16 LwpDivInfo::GetMaxNumberOfPages() const
{
    LwpDocument* pDiv = dynamic_cast<LwpDocument*>(m_ParentID.obj().get());
    if (!pDiv)
        return 0;
    LwpDLVListHeadTailHolder* pHeadTail
        = dynamic_cast<LwpDLVListHeadTailHolder*>(pDiv->GetPageHintsID().obj().get());
    if (pHeadTail)
    {
        LwpPageHint* pPageHint = dynamic_cast<LwpPageHint*>(pHeadTail->GetTail().obj().get());
        if (pPageHint && !pPageHint->GetPageLayoutID().IsNull())
        {
            return pPageHint->GetPageNumber();
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
