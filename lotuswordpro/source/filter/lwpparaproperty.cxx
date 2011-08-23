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
#include "lwpparaproperty.hxx"
#include "lwpobjtags.hxx"
#include "lwppara.hxx"

LwpParaProperty* LwpParaProperty::ReadPropertyList(LwpObjectStream* pFile,LwpObject* Whole)
{
    sal_uInt32 tag;
    LwpParaProperty* Prop= NULL;
    LwpParaProperty* NewProp= NULL;
    sal_uInt16 Len;

    for(;;)
    {
        pFile->QuickRead(&tag,sizeof(sal_uInt32));
        // Keep reading properties until we hit the end tag
        if (tag == TAG_ENDSUBOBJ)
            break;

        // Get the length of this property
        pFile->QuickRead(&Len,sizeof(sal_uInt16));

        // Create whatever kind of tag we just found
        switch (tag)
        {
            case TAG_PARA_ALIGN:
                NewProp = new LwpParaAlignProperty(pFile);
                break;

            case TAG_PARA_INDENT:
                NewProp = new LwpParaIndentProperty(pFile);
                break;

            case TAG_PARA_SPACING:
                NewProp = new LwpParaSpacingProperty(pFile);
                break;

            case TAG_PARA_BORDER:
                NewProp = new LwpParaBorderProperty(pFile);
                break;

            case TAG_PARA_BACKGROUND:
                NewProp = new LwpParaBackGroundProperty(pFile);
                break;

            case TAG_PARA_BREAKS:
                NewProp = new LwpParaBreaksProperty(pFile);
                break;

            case TAG_PARA_BULLET:
                NewProp = new LwpParaBulletProperty(pFile);
                static_cast<LwpPara*>(Whole)->SetBulletFlag(sal_True);
                break;

            case TAG_PARA_NUMBERING:
                NewProp = new LwpParaNumberingProperty(pFile);
                break;

            case TAG_PARA_TAB:
                NewProp = new LwpParaTabRackProperty(pFile);
                break;
/*
#ifdef KINSOKU
            case TAG_PARA_KINSOKU:
                NewProp = new CParaKinsokuProperty(pFile);
                break;
#endif

            case TAG_PARA_PROPLIST:
                NewProp = new CParaPropListProperty(pFile,Whole);
                break;

            case TAG_PARA_SHOW:
            case TAG_PARA_HIDE:
                // Ignore these properties
*/			default:
                pFile->SeekRel(Len);
                NewProp = NULL;
                break;
        }
        // Stick it at the beginning of the list
        if (NewProp)
        {
            NewProp->insert(Prop, NULL);
            Prop = NewProp;
        }
    }
    return Prop;
}

LwpParaAlignProperty::LwpParaAlignProperty(LwpObjectStream* pFile)
{
//    pFile->SeekRel(3);
     LwpObjectID align;
     align.ReadIndexed(pFile);
     m_pAlignment =static_cast<LwpAlignmentOverride*>
             (static_cast<LwpAlignmentPiece*>(align.obj(VO_ALIGNMENTPIECE))->GetOverride());
/*    if(pFile->GetFileRevision() < 0x000b)
    {
        CAlignmentPieceGuts guts;
        CAlignmentPiece::QuickReadGuts(pFile, &guts);
        cAlignment = GetPieceManager()->GetAlignmentPiece(&guts);
    }
    else
    {
        LwpFoundry* OldFoundry = pFile->GetFoundry();
        pFile->SetFoundry(GetPieceManager()->GetFoundry());
        cAlignment.QuickRead(pFile);
        pFile->SetFoundry(OldFoundry);
    } */
}

LwpParaAlignProperty::~LwpParaAlignProperty(void)
{
}

sal_uInt32	LwpParaAlignProperty::GetType(void)
{
    return PP_LOCAL_ALIGN;
}

LwpParaIndentProperty::LwpParaIndentProperty(LwpObjectStream* pFile)
{
//    LwpObjectID indent;
    m_aIndentID.ReadIndexed(pFile);
    m_pIndent = static_cast<LwpIndentOverride*>
            (static_cast<LwpIndentPiece*>(m_aIndentID.obj(VO_INDENTPIECE))->GetOverride());
}

LwpParaIndentProperty::~LwpParaIndentProperty(void)
{
}

sal_uInt32 LwpParaIndentProperty::GetType(void)
{
    return PP_LOCAL_INDENT;
}

LwpParaSpacingProperty::LwpParaSpacingProperty(LwpObjectStream* pFile)
{
    LwpObjectID spacing;
    spacing.ReadIndexed(pFile);
    m_pSpacing = static_cast<LwpSpacingOverride*>
            (static_cast<LwpSpacingPiece*>(spacing.obj(VO_SPACINGPIECE))->GetOverride());
}

LwpParaSpacingProperty::~LwpParaSpacingProperty(void)
{
}

sal_uInt32 LwpParaSpacingProperty::GetType(void)
{
    return PP_LOCAL_SPACING;
}

// 01/25/2004////////////////////////////////////////////////////////
LwpParaBorderProperty::LwpParaBorderProperty(LwpObjectStream* pStrm) :
m_pParaBorderOverride(NULL)
{
    LwpObjectID aParaBorder;
    aParaBorder.ReadIndexed(pStrm);

    if (!aParaBorder.IsNull())
    {
        m_pParaBorderOverride = static_cast<LwpParaBorderOverride*>
            (static_cast<LwpParaBorderPiece*>(aParaBorder.obj())->GetOverride());
    }
}

LwpParaBreaksProperty::LwpParaBreaksProperty(LwpObjectStream* pStrm) :
m_pBreaks(NULL)
{
    LwpObjectID aBreaks;
    aBreaks.ReadIndexed(pStrm);

    if (!aBreaks.IsNull())
    {
        m_pBreaks = static_cast<LwpBreaksOverride*>
            (static_cast<LwpBreaksPiece*>(aBreaks.obj())->GetOverride());
    }
}

LwpParaBulletProperty::LwpParaBulletProperty(LwpObjectStream* pStrm) :
m_pBullet(new LwpBulletOverride)
{
    m_pBullet->Read(pStrm);
}

LwpParaBulletProperty::~LwpParaBulletProperty()
{
    if (m_pBullet)
    {
        delete m_pBullet;
    }
}

LwpParaNumberingProperty::LwpParaNumberingProperty(LwpObjectStream * pStrm)
{
    LwpObjectID aNumberingPiece;
    aNumberingPiece.ReadIndexed(pStrm);
    if (aNumberingPiece.IsNull())
    {
        return;
    }

    m_pNumberingOverride = static_cast<LwpNumberingOverride*>
        (static_cast<LwpNumberingPiece*>(aNumberingPiece.obj(VO_NUMBERINGPIECE))->GetOverride());

}
//end//////////////////////////////////////////////////////////////////////

LwpParaTabRackProperty::LwpParaTabRackProperty(LwpObjectStream* pFile)
{
    LwpObjectID aTabRack;
    aTabRack.ReadIndexed(pFile);
    m_pTabOverride =static_cast<LwpTabOverride*>(static_cast<LwpTabPiece*>(aTabRack.obj())->GetOverride());
}

LwpParaTabRackProperty::~LwpParaTabRackProperty()
{
}

LwpParaBackGroundProperty::LwpParaBackGroundProperty(LwpObjectStream* pFile)
{
    LwpObjectID background;
    background.ReadIndexed(pFile);
    m_pBackground = static_cast<LwpBackgroundOverride*>(
        static_cast<LwpBackgroundPiece*>(background.obj())->GetOverride());
}

LwpParaBackGroundProperty::~LwpParaBackGroundProperty()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
