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

#include "lwpparaproperty.hxx"
#include "lwpobjtags.hxx"
#include "lwppara.hxx"

LwpParaProperty* LwpParaProperty::ReadPropertyList(LwpObjectStream* pFile,rtl::Reference<LwpObject> const & Whole)
{
    LwpParaProperty* Prop= nullptr;
    LwpParaProperty* NewProp= nullptr;

    for(;;)
    {
        bool bFailure;

        sal_uInt32 tag = pFile->QuickReaduInt32(&bFailure);
        // Keep reading properties until we hit the end tag or
        // the stream ends
        if (bFailure || tag == TAG_ENDSUBOBJ)
            break;

        // Get the length of this property
        sal_uInt16 Len = pFile->QuickReaduInt16(&bFailure);

        if (bFailure)
            break;

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
                static_cast<LwpPara*>(Whole.get())->SetBulletFlag(true);
                break;

            case TAG_PARA_NUMBERING:
                NewProp = new LwpParaNumberingProperty(pFile);
                break;

            case TAG_PARA_TAB:
                NewProp = new LwpParaTabRackProperty(pFile);
                break;

            default:
                pFile->SeekRel(Len);
                NewProp = nullptr;
                break;
        }
        // Stick it at the beginning of the list
        if (NewProp)
        {
            NewProp->insert(Prop, nullptr);
            Prop = NewProp;
        }
    }
    return Prop;
}

LwpParaAlignProperty::LwpParaAlignProperty(LwpObjectStream* pFile)
{
    LwpObjectID align;
    align.ReadIndexed(pFile);

    LwpAlignmentPiece *pAlignmentPiece = dynamic_cast<LwpAlignmentPiece*>(align.obj(VO_ALIGNMENTPIECE).get());
    m_pAlignment = pAlignmentPiece ? dynamic_cast<LwpAlignmentOverride*>(pAlignmentPiece->GetOverride()) : nullptr;

}

LwpParaAlignProperty::~LwpParaAlignProperty()
{
}

sal_uInt32  LwpParaAlignProperty::GetType()
{
    return PP_LOCAL_ALIGN;
}

LwpParaIndentProperty::LwpParaIndentProperty(LwpObjectStream* pFile)
{
    m_aIndentID.ReadIndexed(pFile);

    LwpIndentPiece *pIndentPiece = dynamic_cast<LwpIndentPiece*>(m_aIndentID.obj(VO_INDENTPIECE).get());
    m_pIndent = pIndentPiece ? dynamic_cast<LwpIndentOverride*>(pIndentPiece->GetOverride()) : nullptr;
}

LwpParaIndentProperty::~LwpParaIndentProperty()
{
}

sal_uInt32 LwpParaIndentProperty::GetType()
{
    return PP_LOCAL_INDENT;
}

LwpParaSpacingProperty::LwpParaSpacingProperty(LwpObjectStream* pFile)
{
    LwpObjectID spacing;
    spacing.ReadIndexed(pFile);

    LwpSpacingPiece *pSpacingPiece = dynamic_cast<LwpSpacingPiece*>(spacing.obj(VO_SPACINGPIECE).get());
    m_pSpacing = pSpacingPiece ? dynamic_cast<LwpSpacingOverride*>(pSpacingPiece->GetOverride()) : nullptr;
}

LwpParaSpacingProperty::~LwpParaSpacingProperty()
{
}

sal_uInt32 LwpParaSpacingProperty::GetType()
{
    return PP_LOCAL_SPACING;
}

// 01/25/2004
LwpParaBorderProperty::LwpParaBorderProperty(LwpObjectStream* pStrm) :
m_pParaBorderOverride(nullptr)
{
    LwpObjectID aParaBorder;
    aParaBorder.ReadIndexed(pStrm);

    if (!aParaBorder.IsNull())
    {
        LwpParaBorderPiece *pParaBorderPiece = dynamic_cast<LwpParaBorderPiece*>(aParaBorder.obj().get());
        m_pParaBorderOverride = pParaBorderPiece ? dynamic_cast<LwpParaBorderOverride*>(pParaBorderPiece->GetOverride()) : nullptr;
    }
}

LwpParaBreaksProperty::LwpParaBreaksProperty(LwpObjectStream* pStrm) :
m_pBreaks(nullptr)
{
    LwpObjectID aBreaks;
    aBreaks.ReadIndexed(pStrm);

    if (!aBreaks.IsNull())
    {
        LwpBreaksPiece *pBreaksPiece = dynamic_cast<LwpBreaksPiece*>(aBreaks.obj().get());
        m_pBreaks = pBreaksPiece ? dynamic_cast<LwpBreaksOverride*>(pBreaksPiece->GetOverride()) : nullptr;
    }
}

LwpParaBulletProperty::LwpParaBulletProperty(LwpObjectStream* pStrm) :
m_pBullet(new LwpBulletOverride)
{
    m_pBullet->Read(pStrm);
}

LwpParaBulletProperty::~LwpParaBulletProperty()
{
    delete m_pBullet;
}

LwpParaNumberingProperty::LwpParaNumberingProperty(LwpObjectStream * pStrm)
    : m_pNumberingOverride(nullptr)
{
    LwpObjectID aNumberingPiece;
    aNumberingPiece.ReadIndexed(pStrm);
    if (aNumberingPiece.IsNull())
    {
        return;
    }

    LwpNumberingPiece *pNumberingPiece = dynamic_cast<LwpNumberingPiece*>(aNumberingPiece.obj(VO_NUMBERINGPIECE).get());
    m_pNumberingOverride = pNumberingPiece ? dynamic_cast<LwpNumberingOverride*>(pNumberingPiece->GetOverride()) : nullptr;
}

LwpParaTabRackProperty::LwpParaTabRackProperty(LwpObjectStream* pFile)
{
    LwpObjectID aTabRack;
    aTabRack.ReadIndexed(pFile);

    LwpTabPiece *pTabPiece = dynamic_cast<LwpTabPiece*>(aTabRack.obj().get());
    m_pTabOverride = pTabPiece ? dynamic_cast<LwpTabOverride*>(pTabPiece->GetOverride()) : nullptr;
}

LwpParaTabRackProperty::~LwpParaTabRackProperty()
{
}

LwpParaBackGroundProperty::LwpParaBackGroundProperty(LwpObjectStream* pFile)
{
    LwpObjectID background;
    background.ReadIndexed(pFile);

    LwpBackgroundPiece *pBackgroundPiece = dynamic_cast<LwpBackgroundPiece*>(background.obj().get());
    m_pBackground = pBackgroundPiece ? dynamic_cast<LwpBackgroundOverride*>(pBackgroundPiece->GetOverride()) : nullptr;
}

LwpParaBackGroundProperty::~LwpParaBackGroundProperty()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
