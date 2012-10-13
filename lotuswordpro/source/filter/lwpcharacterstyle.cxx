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
 Jan 2005           Created
 ************************************************************************/

#include "lwpcharacterstyle.hxx"
#include "lwpfilehdr.hxx"
#include "lwptools.hxx"
#include "lwpoverride.hxx"
#include "lwpatomholder.hxx"
#include "lwpfont.hxx"
#include "lwpfoundry.hxx"
#include "lwpcharborderoverride.hxx"

/*class LwpTextStyle*/
LwpTextStyle::LwpTextStyle(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpDLNFPVList(objHdr, pStrm),
    m_nFontID(0), m_nFinalFontID(0), m_nCSFlags(0), m_nUseCount(0),
    m_pDescription(new LwpAtomHolder), m_pLangOverride(new LwpTextLanguageOverride),
    m_pTxtAttrOverride(new LwpTextAttributeOverride),
    m_pCharacterBorderOverride(new LwpCharacterBorderOverride),
    m_pAmikakeOverride(new LwpAmikakeOverride),
    m_nStyleDefinition(0), m_nKey(0)
{
}

void LwpTextStyle::Read()
{
    LwpDLNFPVList::Read();
    ReadCommon();
}

LwpTextStyle::~LwpTextStyle()
{
    if (m_pDescription)
    {
        delete m_pDescription;
    }
    if (m_pLangOverride)
    {
        delete m_pLangOverride;
    }
    if (m_pTxtAttrOverride)
    {
        delete m_pTxtAttrOverride;
    }
    if (m_pCharacterBorderOverride)
    {
        delete m_pCharacterBorderOverride;
    }
    if (m_pAmikakeOverride)
    {
        delete m_pAmikakeOverride;
    }
}

void LwpTextStyle::ReadCommon()
{
    m_nFontID = m_pObjStrm->QuickReaduInt32();
    m_nFinalFontID = m_pObjStrm->QuickReaduInt32();
    m_nCSFlags = m_pObjStrm->QuickReaduInt16();
    m_nUseCount = m_pObjStrm->QuickReaduInt32();

    m_pDescription->Read(m_pObjStrm);

    m_pLangOverride->Read(m_pObjStrm);
    m_pTxtAttrOverride->Read(m_pObjStrm);
    if (LwpFileHeader::m_nFileRevision < 0x000B)
    {
        m_pCharacterBorderOverride->Read(m_pObjStrm);
        m_pAmikakeOverride->Read(m_pObjStrm);
    }
    else
    {
        m_CharacterBorder.ReadIndexed(m_pObjStrm);
        m_Amikake.ReadIndexed(m_pObjStrm);
    }
    sal_uInt16 nCount = 6;
    if (LwpFileHeader::m_nFileRevision > 0x0005)
        nCount = m_pObjStrm->QuickReaduInt16();

    m_FaceStyle.ReadIndexed(m_pObjStrm);

    if (nCount > 1)
    {
        m_SizeStyle.ReadIndexed(m_pObjStrm);
        m_AttributeStyle.ReadIndexed(m_pObjStrm);
        m_FontStyle.ReadIndexed(m_pObjStrm);
        m_CharacterBorderStyle.ReadIndexed(m_pObjStrm);
        m_AmikakeStyle.ReadIndexed(m_pObjStrm);
    }

    if (m_pObjStrm->CheckExtra())
    {
        m_nStyleDefinition = m_pObjStrm->QuickReaduInt32();

        if (m_pObjStrm->CheckExtra())
        {
            m_nKey = m_pObjStrm->QuickReaduInt16();
            m_pObjStrm->SkipExtra();
        }
    }

}

#include "xfilter/xfparastyle.hxx"
#include "xfilter/xffont.hxx"

void LwpTextStyle::RegisterStyle()
{
    if (!m_pFoundry)
    {
        assert(false);
        return;
    }

    XFTextStyle* pStyle = new XFTextStyle();

    //Set name
    OUString styleName = GetName()->str();
    pStyle->SetStyleName(styleName);

    //Create font
    LwpFontManager* pFontMgr = m_pFoundry->GetFontManger();
    XFFont* pFont = pFontMgr->CreateFont(m_nFinalFontID);
    pStyle->SetFont(pFont);

    //Set other properties if needed

    //Add style
    LwpStyleManager* pStyleMgr = m_pFoundry->GetStyleManager();
    pStyleMgr->AddStyle(*GetObjectID(), pStyle);

}

/*class LwpCharacterStyle*/
LwpCharacterStyle::LwpCharacterStyle(LwpObjectHeader& objHdr, LwpSvStream* pStrm) :
LwpTextStyle(objHdr, pStrm)
{
}

void LwpCharacterStyle::Read()
{
    LwpTextStyle::Read();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
