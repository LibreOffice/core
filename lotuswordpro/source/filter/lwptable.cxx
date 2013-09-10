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
/**
 * @file
 *  For LWP filter architecture prototype - table object
 */
/*************************************************************************
 * Change History
 Mar 2005           Created
 ************************************************************************/

 #include "lwptable.hxx"

 LwpSuperTable::LwpSuperTable(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpContent(objHdr, pStrm)
{}

LwpSuperTable::~LwpSuperTable()
{}

void LwpSuperTable::Read()
{
    LwpContent::Read();
    m_pObjStrm->SkipExtra();

}

void  LwpSuperTable::Parse(IXFStream* /*pOutputStream*/)
{
}

//Added by  for XFConvert refactor, 03/31/2005
void LwpSuperTable::XFConvert(XFContentContainer* /*pCont*/)
{
}

//End of Add
/*****************************************************************************/
 LwpTable::LwpTable(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpContent(objHdr, pStrm)
{}

LwpTable::~LwpTable()
{}

void LwpTable::Read()
{
    LwpContent::Read();
    //m_RowCache.Read(m_pObjStrm);
    //m_ColumnCache.Read(m_pObjStrm);

    //m_CPNotifyList.Read(m_pObjStrm);
    //m_CPTempVersionedNotifyList.Read(m_pObjStrm);

    m_nRow = m_pObjStrm->QuickReaduInt16();
    m_nColumn = m_pObjStrm->QuickReaduInt16();

    m_nHeight = m_pObjStrm->QuickReadInt32();
    m_nWidth = m_pObjStrm->QuickReadInt32();

    m_nDefaultAutoGrowRowHeight = m_pObjStrm->QuickReadInt32();

    m_nAttributes = m_pObjStrm->QuickReaduInt16();

    m_Layout.ReadIndexed(m_pObjStrm);

    m_DefaultCellStyle.ReadIndexed(m_pObjStrm);
    if (LwpFileHeader::m_nFileRevision >= 0x0007)
        m_CPNotifyList.Read(m_pObjStrm);

    m_pObjStrm->SkipExtra();
}

sal_Bool LwpTable::IsNumberDown()
{
    if (m_nAttributes & NUMBER_DOWN)
        return sal_True;
    return sal_False;
}

void  LwpTable::Parse(IXFStream* /*pOutputStream*/)
{
}

 LwpSuperTableLayout* LwpTable::GetSuperTableLayout()
{
    LwpTableLayout* pLayout = dynamic_cast<LwpTableLayout*>(m_Layout.obj());
    if(pLayout)
        return dynamic_cast<LwpSuperTableLayout*>(pLayout->GetParent()->obj());

    return NULL;
}

 /*****************************************************************************/
 LwpTableHeading::LwpTableHeading(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpTable(objHdr, pStrm)
{}

LwpTableHeading::~LwpTableHeading()
{}

void LwpTableHeading::Read()
{
    m_pObjStrm->SkipExtra();
}

void  LwpTableHeading::Parse(IXFStream* /*pOutputStream*/)
{
}

 /*****************************************************************************/
LwpParallelColumns::LwpParallelColumns(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpTable(objHdr, pStrm)
{
}

LwpParallelColumns::~LwpParallelColumns()
{
}

void LwpParallelColumns::Read()
{
    LwpTable::Read();
    cDefaultLeftColumnStyle.ReadIndexed(m_pObjStrm);
    cDefaultRightColumnStyle.ReadIndexed(m_pObjStrm);

    m_pObjStrm->SkipExtra();
}
 /*****************************************************************************/
LwpGlossary::LwpGlossary(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpParallelColumns(objHdr, pStrm)
{
}

LwpGlossary::~LwpGlossary()
{
}
sal_uInt16 LwpGlossary::GetNumIndexRows(void)
{
    if (GetRow() > 0 && GetRow() <= MAX_NUM_ROWS)
        return GetRow() - 1;    // Minus one row for repeated heading.
    return 0;
}

void LwpGlossary::Read()
{
    LwpParallelColumns::Read();

    sal_uInt16 FiledEntries = m_pObjStrm->QuickReaduInt16();
    sal_uInt16 NumIndexRows = GetNumIndexRows();

    if (FiledEntries < NumIndexRows)
    {
        /* We'll have to do sequential (slow) searches. */
        m_pObjStrm->SeekRel( FiledEntries * sizeof(sal_uInt16));
    }
    else
    {
        if (NumIndexRows)
        {
            sal_uInt16 EntriesRead = (FiledEntries > NumIndexRows)? NumIndexRows:FiledEntries;

            for (sal_uInt16 EntryCount = 1; EntryCount <= EntriesRead; EntryCount++)
                m_pObjStrm->QuickReaduInt16();

            if (FiledEntries > EntriesRead)
                m_pObjStrm->SeekRel((FiledEntries - EntriesRead)* sizeof(sal_uInt16));
        }
        else
            m_pObjStrm->SeekRel(FiledEntries * sizeof(sal_uInt16));
    }

    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
