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

#include "lwpcolor.hxx"
#include "lwpatomholder.hxx"
#include "lwplayout.hxx"
#include "lwpdlvlist.hxx"
#include "lwpobj.hxx"
#include "lwpsection.hxx"
#include "xfilter/xfsection.hxx"

LwpOrderedObject::LwpOrderedObject(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpDLNFVList(objHdr, pStrm)
{}

/**
 * @descr: read object information
 *
 */
void LwpOrderedObject::Read()
{
    LwpDLNFVList::Read();

    m_ListList.ReadIndexed(m_pObjStrm);
    m_Para.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

LwpSection::LwpSection(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpOrderedObject(objHdr, pStrm)
{}

LwpSection::~LwpSection()
{

}

/**
 * @descr: read object information
 *
 */
void LwpSection::Read()
{
    LwpOrderedObject::Read();
    m_pObjStrm->QuickRead(&m_Flags, sizeof(m_Flags));
    m_PageLayout.ReadIndexed(m_pObjStrm);
    m_Color.Read(m_pObjStrm);
    m_AtomHolder.Read(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

/**
 * @descr: do nothing
 *
 */
void LwpSection::Parse(IXFStream * /*pOutputStream*/)
{

}

LwpIndexSection::LwpIndexSection(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpSection(objHdr, pStrm)
{}

LwpIndexSection::~LwpIndexSection()
{
}

/**
 * @descr: read object information
 *
 */
void LwpIndexSection::Read()
{
    LwpSection::Read();
    m_TextMarker.Read(m_pObjStrm);
    m_ParentName.Read(m_pObjStrm);
    m_DivisionName.Read(m_pObjStrm);
    m_SectionName.Read(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nForm, sizeof(m_nForm));
    m_pObjStrm->QuickRead(&m_nFlags, sizeof(m_nFlags));
    m_pObjStrm->SkipExtra();
}

sal_Bool LwpIndexSection::IsFormatRunin()
{
    return (m_nFlags & RUN_IN);

}

sal_Bool LwpIndexSection::IsFormatSeparator()
{
    return (m_nFlags & SEPARATORS);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
