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
 *  For LWP filter architecture prototype - table cell numerics format
 */
/*************************************************************************
 * Change History
 Mar 2005		 	Created
 ************************************************************************/

#include	"lwpoverride.hxx"
#include	"lwpobjid.hxx"
#include 	"lwptblcell.hxx"
#include "lwppara.hxx"

//////////////////////////////////////////////////////////////////
 LwpCellList::LwpCellList(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpDLVList(objHdr, pStrm)
{}

LwpCellList::~LwpCellList()
{}

void LwpCellList::Read()
{
    // CDLFVList read construction
    LwpDLVList::Read();

    LwpDLVListHeadTail cChild;
    cChild.Read(m_pObjStrm);

    cParent.ReadIndexed(m_pObjStrm);

    if (LwpFileHeader::m_nFileRevision < 0x0006)
        m_pObjStrm->SkipExtra();

    cValue.ReadIndexed(m_pObjStrm);

    LwpObjectID cDependent;
    cDependent.ReadIndexed(m_pObjStrm);

    cColumn = (sal_uInt8) m_pObjStrm->QuickReaduInt16();		// written as a sal_uInt16
//	sal_uInt8 cCellFlags = (sal_uInt8) m_pObjStrm->QuickReaduInt16();	// written as a sal_uInt16
    m_pObjStrm->SeekRel(2);//CellFlags
    m_pObjStrm->SkipExtra();

}

void  LwpCellList::Parse(IXFStream* /*pOutputStream*/)
{
}

void LwpCellList::Convert(XFCell * pCell, LwpTableLayout* /*pCellsMap*/)
{
    LwpObjectID aValueID = GetValueID();
    LwpNumericValue* pValue = (LwpNumericValue*)aValueID.obj();
    if (pValue)
    {
        pCell->SetValue( pValue->GetValue() );
    }
}

//////////////////////////////////////////////////////////////////
 LwpNumericValue::LwpNumericValue(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpObject(objHdr, pStrm)
{}

LwpNumericValue::~LwpNumericValue()
{}

void LwpNumericValue::Read()
{
    m_pObjStrm->QuickRead(&cNumber, sizeof(cNumber));
    m_pObjStrm->SkipExtra();

}
 void  LwpNumericValue::Parse(IXFStream* /*pOutputStream*/)
{
}
//////////////////////////////////////////////////////////////////
 LwpRowList::LwpRowList(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpDLVList(objHdr, pStrm)
{}

LwpRowList::~LwpRowList()
{}

void LwpRowList::Read()
{
    // CDLFVList read construction
    LwpDLVList::Read();

    cChild.Read(m_pObjStrm);
    cParent.ReadIndexed(m_pObjStrm);

    if (LwpFileHeader::m_nFileRevision < 0x0006)
        m_pObjStrm->SkipExtra();

    cRowID = m_pObjStrm->QuickReaduInt16();

    m_pObjStrm->SkipExtra();
}
 void  LwpRowList::Parse(IXFStream* /*pOutputStream*/)
{
}

//////////////////////////////////////////////////////////////////
LwpTableRange::LwpTableRange(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    :LwpDLVList(objHdr, pStrm)
{}

LwpTableRange::~LwpTableRange()
{}

void LwpTableRange::Read()
{
    LwpDLVList::Read();

    cqTable.ReadIndexed(m_pObjStrm);
    cpCellRange.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

void  LwpTableRange::Parse(IXFStream* /*pOutputStream*/)
{
}

//////////////////////////////////////////////////////////////////
 LwpCellRange::LwpCellRange(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpObject(objHdr, pStrm)
{}

LwpCellRange::~LwpCellRange()
{}

void LwpCellRange::Read()
{
    cpFolder.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

void  LwpCellRange::Parse(IXFStream* /*pOutputStream*/)
{
}

//////////////////////////////////////////////////////////////////

 LwpFolder::LwpFolder(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpDLVList(objHdr, pStrm)
{}

LwpFolder::~LwpFolder()
{}

void LwpFolder::Read()
{
    // CDLFVList read construction
    LwpDLVList::Read();

    cChild.Read(m_pObjStrm);
    cParent.ReadIndexed(m_pObjStrm);

    if (LwpFileHeader::m_nFileRevision < 0x0006)
        m_pObjStrm->SkipExtra();

    cqTable.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

void  LwpFolder::Parse(IXFStream* /*pOutputStream*/)
{
}

//////////////////////////////////////////////////////////////////

LwpDependent::LwpDependent(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpDLVList(objHdr, pStrm)
{}

LwpDependent::~LwpDependent()
{}

void LwpDependent::Read()
{
    LwpDLVList::Read();

    cFormulaInfo.ReadIndexed(m_pObjStrm);
    cReferenceOffset = m_pObjStrm->QuickReaduInt16();
    cFlags = (sal_uInt8)m_pObjStrm->QuickReaduInt16();	// Written as lushort.

    m_pObjStrm->SkipExtra();
}

void  LwpDependent::Parse(IXFStream* /*pOutputStream*/)
{
}
//////////////////////////////////////////////////////////////////

void LwpRowSpecifier::QuickRead(LwpObjectStream *pStrm)
{
    cRow = pStrm->QuickReaduInt16();
    cQualifier.QuickRead(pStrm);
}
void LwpColumnSpecifier::QuickRead(LwpObjectStream *pStrm)
{
    cColumn = (sal_uInt8)pStrm->QuickReaduInt16();
    cQualifier.QuickRead(pStrm);
}

void LwpRowColumnQualifier::QuickRead(LwpObjectStream *pStrm)
{
    // written as lushort for future flags
    cFlags = (sal_uInt8)pStrm->QuickReaduInt16();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
