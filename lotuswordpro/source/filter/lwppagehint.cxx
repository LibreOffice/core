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
 Feb 2005			Created
 ************************************************************************/

#include "lwppagehint.hxx"


void LwpSLVListHead::Read(LwpObjectStream* pObjStrm)
{
    m_ListHead.ReadIndexed(pObjStrm);
    pObjStrm->SkipExtra();
}

void LwpContentHintHead::Read(LwpObjectStream* pObjStrm)
{
    m_ListHead.Read(pObjStrm);
    pObjStrm->SkipExtra();
}

void LwpFootnoteSeenHead::Read(LwpObjectStream* pObjStrm)
{
    m_ListHead.Read(pObjStrm);
    pObjStrm->SkipExtra();
}

LwpPageHint::LwpPageHint(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpDLVList(objHdr,pStrm)
{
}

void LwpPageHint::Read()
{
    LwpDLVList::Read();

    m_PageLayout.ReadIndexed(m_pObjStrm);
    //m_ContentHints.ReadIndexed(m_pObjStrm);
    m_ContentHints.Read(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nLastFootnoteSeen,sizeof(m_nLastFootnoteSeen));
    m_pObjStrm->QuickRead(&m_nLastFootnoteProcessed,sizeof(m_nLastFootnoteProcessed));
    m_pObjStrm->QuickRead(&m_nFlags,sizeof(m_nFlags));
    m_pObjStrm->QuickRead(&m_nPageNumber,sizeof(m_nPageNumber));
    m_pObjStrm->QuickRead(&m_nNumberStyle,sizeof(m_nNumberStyle));
    m_BeforeText.Read(m_pObjStrm);
    m_AfterText.Read(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nNumberFlags,sizeof(m_nNumberFlags));
    m_pObjStrm->QuickRead(&m_nRenderedPageNumber,sizeof(m_nRenderedPageNumber));
    m_CurrentSection.ReadIndexed(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nCurrentSectionPage,sizeof(m_nCurrentSectionPage));
    //m_FootnoteSeen.ReadIndexed(m_pObjStrm);
    m_FootnoteSeen.Read(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nLayoutPageNumber,sizeof(m_nLayoutPageNumber));

    m_pObjStrm->SkipExtra();
}

void LwpPageHint::Parse(IXFStream * /*pOutputStream*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
