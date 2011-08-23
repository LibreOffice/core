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

#include "lwpstory.hxx"
#include "lwpfilehdr.hxx"
#include "lwpholder.hxx"

LwpHeadContent::LwpHeadContent(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpContent(objHdr, pStrm)
{}


void LwpHeadContent::Read()
{
    LwpContent::Read();
    m_pObjStrm->SkipExtra();
}

LwpContent::LwpContent(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpDLNFVList(objHdr, pStrm)
{}
void LwpContent::Read()
{
    LwpDLNFVList::Read();

    LwpObjectStream* pStrm = m_pObjStrm;

    m_LayoutsWithMe.Read(pStrm);
    //sal_uInt16 nFlagsSkip;
    //pStrm->QuickRead(&nFlagsSkip, sizeof(nFlagsSkip));
    pStrm->QuickRead(&m_nFlags, sizeof(m_nFlags));
    m_nFlags &= ~(CF_CHANGED | CF_DISABLEVALUECHECKING);
    //LwpAtomHolder ClassName;
    //ClassName.Read(pStrm);
    m_ClassName.Read(pStrm);

    LwpObjectID SkipID;
    if(LwpFileHeader::m_nFileRevision >= 0x0006)
    {
        //SkipID.ReadIndexed(pStrm);
        //SkipID.ReadIndexed(pStrm);
        m_NextEnumerated.ReadIndexed(pStrm);
        m_PreviousEnumerated.ReadIndexed(pStrm);
    }

    if (LwpFileHeader::m_nFileRevision >= 0x0007)
    {
        if(LwpFileHeader::m_nFileRevision < 0x000B)
        {
            SkipID.ReadIndexed(pStrm);
            pStrm->SkipExtra();
        }
        else
        {
            sal_uInt8 HasNotify;
            pStrm->QuickRead(&HasNotify, sizeof(HasNotify));
            if(HasNotify)
            {
                SkipID.ReadIndexed(pStrm);
                pStrm->SkipExtra();
            }
        }
    }

    pStrm->SkipExtra();
}

LwpVirtualLayout* LwpContent::GetLayout(LwpVirtualLayout* pStartLayout)
{
    return m_LayoutsWithMe.GetLayout(pStartLayout);
}

sal_Bool LwpContent::HasNonEmbeddedLayouts()
{
    LwpVirtualLayout* pLayout = NULL;
    while( (pLayout = GetLayout(pLayout)) )
    {
        if(!pLayout->NoContentReference())
            return sal_True;
    }
    return sal_False;
}

sal_Bool LwpContent::IsStyleContent()
{
    LwpVirtualLayout* pLayout = NULL;
    while( (pLayout = GetLayout(pLayout)) )
    {
        if(pLayout->IsStyleLayout())
            return sal_True;
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
