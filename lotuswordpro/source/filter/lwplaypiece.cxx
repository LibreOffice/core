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
 Jan 28 2005			Created
 ************************************************************************/

#include "lwplaypiece.hxx"

#include "lwpfilehdr.hxx"
LwpRotor::LwpRotor()
{}

LwpRotor::~LwpRotor()
{}

void LwpRotor:: Read(LwpObjectStream *pStrm)
{
    m_nRotation = pStrm->QuickReadInt16();
}

LwpLayoutGeometry::LwpLayoutGeometry(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutGeometry::~LwpLayoutGeometry()
{}

void LwpLayoutGeometry::Read()
{
    LwpVirtualPiece::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_nWidth = m_pObjStrm->QuickReadInt32();
        m_nHeight = m_pObjStrm->QuickReadInt32();
        m_Origin.Read(m_pObjStrm);
        m_AbsoluteOrigin.Read(m_pObjStrm);
        m_ContainerRotor.Read(m_pObjStrm);
        m_ContentOrientation = m_pObjStrm->QuickReaduInt8();
        m_pObjStrm->SkipExtra();
    }
}
void LwpLayoutGeometry::Parse(IXFStream* /*pOutputStream*/)
{}

LwpLayoutScale::LwpLayoutScale(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutScale::~LwpLayoutScale()
{}

void LwpLayoutScale::Read()
{
    LwpVirtualPiece::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_nScaleMode = m_pObjStrm->QuickReaduInt16();
        m_nScalePercentage = m_pObjStrm->QuickReaduInt32();
        m_nScaleWidth = m_pObjStrm->QuickReadInt32();
        m_nScaleHeight = m_pObjStrm->QuickReadInt32();
        m_nContentRotation = m_pObjStrm->QuickReaduInt16();
        m_Offset.Read(m_pObjStrm);

        m_nPlacement = m_pObjStrm->QuickReaduInt16();
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutScale::Parse(IXFStream* /*pOutputStream*/)
{}

LwpLayoutMargins::LwpLayoutMargins(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutMargins::~LwpLayoutMargins()
{}

void LwpLayoutMargins::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_Margins.Read(m_pObjStrm);
        m_ExtMargins.Read(m_pObjStrm);
        m_ExtraMargins.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutMargins::Parse(IXFStream* /*pOutputStream*/)
{}

LwpLayoutBorder::LwpLayoutBorder(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutBorder::~LwpLayoutBorder()
{}

void LwpLayoutBorder::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_BorderStuff.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutBorder::Parse(IXFStream* /*pOutputStream*/)
{}

LwpLayoutBackground::LwpLayoutBackground(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutBackground::~LwpLayoutBackground()
{}

void LwpLayoutBackground::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_BackgroundStuff.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutBackground::Parse(IXFStream* /*pOutputStream*/)
{}

LwpExternalBorder::LwpExternalBorder()
{}

LwpExternalBorder::~LwpExternalBorder()
{}

void LwpExternalBorder:: Read(LwpObjectStream *pStrm)
{
    if( LwpFileHeader::m_nFileRevision >= 0x000F )
    {
        //enum {BORDER,JOIN};
        m_LeftName.Read(pStrm);
        m_TopName.Read(pStrm);
        m_RightName.Read(pStrm);
        m_BottomName.Read(pStrm);
        // TODO: Do not know what it is for
        /*cLeftName = CStyleMgr::GetUniqueMetaFileName(cLeftName,BORDER);
        cRightName = CStyleMgr::GetUniqueMetaFileName(cRightName,BORDER);
        cTopName = CStyleMgr::GetUniqueMetaFileName(cTopName,BORDER);
        cBottomName = CStyleMgr::GetUniqueMetaFileName(cBottomName,BORDER);*/
        pStrm->SkipExtra();
    }
}

LwpLayoutExternalBorder::LwpLayoutExternalBorder(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutExternalBorder::~LwpLayoutExternalBorder()
{}

void LwpLayoutExternalBorder::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_ExtranalBorder.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutExternalBorder::Parse(IXFStream* /*pOutputStream*/)
{}

LwpColumnInfo::LwpColumnInfo()
{}

LwpColumnInfo::~LwpColumnInfo()
{}

void LwpColumnInfo:: Read(LwpObjectStream *pStrm)
{
    m_nWidth = pStrm->QuickReadInt32();
    m_nGap = pStrm->QuickReadInt32();
}

LwpLayoutColumns::LwpLayoutColumns(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm),m_pColumns(NULL)
{}

LwpLayoutColumns::~LwpLayoutColumns()
{
    if(m_pColumns)
    {
        delete[] m_pColumns;
        m_pColumns = NULL;
    }

}

void LwpLayoutColumns::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_nNumCols = m_pObjStrm->QuickReaduInt16();
        m_pColumns = new LwpColumnInfo[m_nNumCols];
        for(int i=0; i<m_nNumCols; i++)
        {
            m_pColumns[i].Read(m_pObjStrm);
        }
        m_pObjStrm->SkipExtra();
    }
}

double LwpLayoutColumns::GetColWidth(sal_uInt16 nIndex)
{
    if(nIndex >= m_nNumCols)
    {
        return 0;
    }
    return m_pColumns[nIndex].GetWidth();
}

double LwpLayoutColumns::GetColGap(sal_uInt16 nIndex)
{
    if(nIndex >= m_nNumCols)
    {
        return 0;
    }
    return m_pColumns[nIndex].GetGap();
}

void LwpLayoutColumns::Parse(IXFStream* /*pOutputStream*/)
{}

LwpLayoutGutters::LwpLayoutGutters(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutGutters::~LwpLayoutGutters()
{}

void LwpLayoutGutters::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_BorderBuffer.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}


void LwpLayoutGutters::Parse(IXFStream* /*pOutputStream*/)
{}

LwpJoinStuff::LwpJoinStuff()
{}

LwpJoinStuff::~LwpJoinStuff()
{}

#include "lwpstyledef.hxx"
void LwpJoinStuff:: Read(LwpObjectStream *pStrm)
{
    m_nWidth = pStrm->QuickReadInt32();
    m_nHeight = pStrm->QuickReadInt32();
    m_nPercentage = pStrm->QuickReaduInt16();
    m_nID = pStrm->QuickReaduInt16();
    m_nCorners = pStrm->QuickReaduInt16();
    m_nScaling = pStrm->QuickReaduInt16();
    m_Color.Read(pStrm);
    pStrm->SkipExtra();

    // Bug fix: if reading in from something older than Release 9
    // then check for the external ID and change it to solid.
    if (LwpFileHeader::m_nFileRevision < 0x0010)
    {
        if (m_nID & EXTERNAL_ID)
            m_nID = MITRE;
    }
}

LwpLayoutJoins::LwpLayoutJoins(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutJoins::~LwpLayoutJoins()
{}

void LwpLayoutJoins::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_JoinStuff.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutJoins::Parse(IXFStream* /*pOutputStream*/)
{}

LwpLayoutShadow::LwpLayoutShadow(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{}

LwpLayoutShadow::~LwpLayoutShadow()
{}

void LwpLayoutShadow::Read()
{
    LwpVirtualPiece::Read();

    if( LwpFileHeader::m_nFileRevision >= 0x000B )
    {
        m_Shadow.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutShadow::Parse(IXFStream* /*pOutputStream*/)
{}
/**************************************************************************
 * @descr: Constructor
 * @param:
 * @param:
 * @return:
**************************************************************************/
LwpLayoutRelativityGuts::LwpLayoutRelativityGuts()
{
    m_nRelType = LAY_PARENT_RELATIVE;
    m_nRelFromWhere = LAY_UPPERLEFT;
    m_RelDistance.SetX(0);
    m_RelDistance.SetY(0);
    m_nTether = LAY_UPPERLEFT;
    m_nTetherWhere = LAY_BORDER;
    m_nFlags = 0;
}
/**************************************************************************
 * @descr: Read LayoutRelativityGuts' infomation.
 * @param:
 * @param:
 * @return:
**************************************************************************/
void LwpLayoutRelativityGuts::Read(LwpObjectStream *pStrm)
{
    m_nRelType = pStrm->QuickReaduInt8();
    m_nRelFromWhere = pStrm->QuickReaduInt8();
    m_RelDistance.Read(pStrm);
    m_nTether = pStrm->QuickReaduInt8();
    m_nTetherWhere = pStrm->QuickReaduInt8();
    if(LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_nFlags = pStrm->QuickReaduInt8();
    }
    else
    {
        m_nFlags = 0;
    }
}
/**************************************************************************
 * @descr: Constructor
 * @param:
 * @param:
 * @return:
**************************************************************************/
LwpLayoutRelativity::LwpLayoutRelativity(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpVirtualPiece(objHdr, pStrm)
{
}
/**************************************************************************
 * @descr: destructor
 * @param:
 * @param:
 * @return:
**************************************************************************/
LwpLayoutRelativity::~LwpLayoutRelativity()
{
}

void LwpLayoutRelativity::Read()
{
    LwpVirtualPiece::Read();
    if(LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_RelGuts.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }
}

void LwpLayoutRelativity::Parse(IXFStream * /*pOutputStream*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
