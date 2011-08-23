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

#include "lwpoverride.hxx"
#include "lwpfilehdr.hxx"
#include "lwpatomholder.hxx"
#include "lwpborderstuff.hxx"
#include "lwpmargins.hxx"
#include "lwpbackgroundstuff.hxx"

/*class LwpOverride*/
void LwpOverride::ReadCommon(LwpObjectStream* pStrm)
{
    pStrm->QuickRead(&m_nValues, 2);
    pStrm->QuickRead(&m_nOverride, 2);
    pStrm->QuickRead(&m_nApply, 2);
    pStrm->SkipExtra();
}

void LwpOverride::Clear()
{
    m_nValues = 0;
    m_nOverride = 0;
    m_nApply = 0;
}

void LwpOverride::operator=(const LwpOverride& rOther)
{
    m_nValues = rOther.m_nValues;
    m_nOverride = rOther.m_nOverride;
    m_nApply = rOther.m_nApply;
}

void LwpOverride::Override(sal_uInt16 nBits, STATE eState)
{
    if (eState == STATE_STYLE)
    {
        m_nValues &= ~nBits;
        m_nOverride &= ~nBits;
    }
    else
    {
        m_nOverride |= nBits;
        if (eState == STATE_ON)
        {
            m_nValues |= nBits;
        }
        else	/* State == STATE_OFF */
        {
            m_nValues &= ~nBits;
        }
    }
    m_nApply |= nBits;
}

/*class LwpTextLanguageOverride*/
void LwpTextLanguageOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        pStrm->QuickRead(&m_nLanguage, 2);
    }

    pStrm->SkipExtra();

}

/*class LwpTextAttributeOverride*/
void LwpTextAttributeOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        pStrm->QuickRead(&m_nHideLevels, 2);

        if (LwpFileHeader::m_nFileRevision > 0x000A)
        {
            pStrm->QuickRead(&m_nBaseLineOffset, 4);
        }
    }

    pStrm->SkipExtra();
}

sal_Bool LwpTextAttributeOverride::IsHighLight()
{
    return (m_nValues & TAO_HIGHLIGHT);
}

/*class LwpKinsokuOptsOverride*/
void LwpKinsokuOptsOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        pStrm->QuickRead(&m_nLevels, 2);
    }

    pStrm->SkipExtra();
}

/*class LwpBulletOverride*/
void LwpBulletOverride::Read(LwpObjectStream * pStrm)
{
    if (pStrm->QuickReadBool())
    {
        m_bIsNull= sal_False;
        ReadCommon(pStrm);
        m_SilverBullet.ReadIndexed(pStrm);
    }
    else
        m_bIsNull = sal_True;

    pStrm->SkipExtra();
}
void LwpBulletOverride::OverrideSkip(sal_Bool bOver)
{
    if (bOver)
    {
        LwpOverride::Override(BO_SKIP, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_SKIP, STATE_OFF);
    }
}

void LwpBulletOverride::OverrideRightAligned(sal_Bool bOver)
{
    if(bOver)
    {
        LwpOverride::Override(BO_RIGHTALIGN,STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_RIGHTALIGN,STATE_OFF);
    }
}

void LwpBulletOverride::operator=(const LwpOverride& rOther)
{
    LwpOverride::operator=(rOther);
    const LwpBulletOverride* pBullet = static_cast<LwpBulletOverride*>((LwpOverride*)&rOther);

    m_SilverBullet = pBullet->m_SilverBullet;

}

void LwpBulletOverride::OverrideSilverBullet(LwpObjectID aID)
{
    if (!aID.IsNull())
    {
        m_SilverBullet = aID;
    }

    LwpOverride::Override(BO_SILVERBULLET,STATE_ON);
}

void LwpBulletOverride::Override(LwpBulletOverride* pOther)
{
    if (m_nApply & BO_SILVERBULLET)
    {
        if (IsSilverBulletOverridden())
        {
            pOther->OverrideSilverBullet(m_SilverBullet);
        }
        else
        {
            pOther->RevertSilverBullet();
        }
    }

    if (m_nApply & BO_SKIP)
    {
        if (IsSkipOverridden())
        {
            pOther->OverrideSkip(IsSkip());
        }
        else
        {
            pOther->RevertSkip();
        }
    }

    if (m_nApply & BO_RIGHTALIGN)
    {
        if (IsRightAlignedOverridden())
        {
            pOther->OverrideRightAligned(IsRightAligned());
        }
        else
        {
            pOther->RevertRightAligned();
        }
    }

}

/*class LwpAlignmentOverride*/
void LwpAlignmentOverride::Read(LwpObjectStream * pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        pStrm->QuickRead(&m_nAlignType, 1);
        pStrm->QuickRead(&m_nPosition, 4);
        pStrm->QuickRead(&m_nAlignChar, 2);
    }

    pStrm->SkipExtra();
}

/*class LwpSpacingCommonOverride*/
void LwpSpacingCommonOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        pStrm->QuickRead(&m_nSpacingType, 2);
        pStrm->QuickRead(&m_nAmount, 4);
        pStrm->QuickRead(&m_nMultiple, 4);
    }

    pStrm->SkipExtra();
}

/*class LwpSpacingOverride*/
LwpSpacingOverride::LwpSpacingOverride() :
m_pSpacing(new LwpSpacingCommonOverride),
m_pAboveLineSpacing(new LwpSpacingCommonOverride),
m_pParaSpacingAbove(new LwpSpacingCommonOverride),
m_pParaSpacingBelow(new LwpSpacingCommonOverride)
{
}

LwpSpacingOverride& LwpSpacingOverride::operator=(LwpSpacingOverride& other)
{
    LwpOverride::operator=(other);
    *m_pSpacing = *other.m_pSpacing;
    *m_pAboveLineSpacing = *other.m_pAboveLineSpacing;
    *m_pParaSpacingAbove = *other.m_pParaSpacingAbove;
    *m_pParaSpacingAbove = *other.m_pParaSpacingAbove;
    return *this;
}

LwpSpacingOverride::~LwpSpacingOverride()
{
    if (m_pSpacing)
    {
        delete m_pSpacing;
    }
    if (m_pAboveLineSpacing)
    {
        delete m_pAboveLineSpacing;
    }
    if (m_pParaSpacingAbove)
    {
        delete m_pParaSpacingAbove;
    }
    if (m_pParaSpacingBelow)
    {
        delete m_pParaSpacingBelow;
    }
}

void LwpSpacingOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_pSpacing->Read(pStrm);
        if (LwpFileHeader::m_nFileRevision >= 0x000d)
        {
            m_pAboveLineSpacing->Read(pStrm);
        }
        m_pParaSpacingAbove->Read(pStrm);
        m_pParaSpacingBelow->Read(pStrm);
    }

    pStrm->SkipExtra();
}

/*class LwpIndentOverride*/
void LwpIndentOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);

        pStrm->QuickRead(&m_nAll, 4);
        pStrm->QuickRead(&m_nFirst, 4);
        pStrm->QuickRead(&m_nRest, 4);
        pStrm->QuickRead(&m_nRight, 4);
    }

    pStrm->SkipExtra();
}


/*class LwpAmikakeOverride*/
LwpAmikakeOverride::LwpAmikakeOverride() :
m_pBackgroundStuff(new LwpBackgroundStuff), m_nType(AMIKAKE_NONE)
{
}

LwpAmikakeOverride::~LwpAmikakeOverride()
{
    if (m_pBackgroundStuff)
    {
        delete m_pBackgroundStuff;
    }
}

void LwpAmikakeOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_pBackgroundStuff->Read(pStrm);
    }
    else
    {
        Clear();
    }

    if (pStrm->CheckExtra())
    {
        pStrm->QuickRead(&m_nType, 2);
        pStrm->SkipExtra();
    }
    else
    {
        m_nType = AMIKAKE_NONE;
    }
}

void LwpAlignmentOverride::Override(LwpAlignmentOverride* other)//add by  1-24
{
    if (m_nOverride & AO_TYPE)
    {
        other->OverrideAlignment(m_nAlignType);
    }
/*	if (m_nOverride & AO_POSITION)
    {
        Other->OverridePosition(GetPosition());
    }
    if (m_nOverride & AO_CHAR)
    {
        Other->OverrideAlignChar(GetAlignChar());
    }
*/
}

void LwpAlignmentOverride::OverrideAlignment(AlignType val)//add by  1-24
{
    m_nAlignType = val;
    m_nOverride |= AO_TYPE;
}

LwpIndentOverride& LwpIndentOverride::operator=(LwpIndentOverride& other)
{
    LwpOverride::operator=(other);
    m_nAll   = other.m_nAll;
    m_nFirst = other.m_nFirst;
    m_nRest  = other.m_nRest;
    m_nRight = other.m_nRight;
    return *this;
}

void LwpIndentOverride::Override(LwpIndentOverride* other)
{
    if(m_nOverride & IO_ALL)
        other->OverrideIndentAll(m_nAll);
    if(m_nOverride & IO_FIRST)
        other->OverrideIndentFirst(m_nFirst);
    if(m_nOverride & IO_RIGHT)
        other->OverrideIndentRight(m_nRight);
    if(m_nOverride & IO_REST)
        other->OverrideIndentRest(m_nRest);
    if(m_nOverride & IO_USE_RELATIVE)
        other->OverrideUseRelative(IsUseRelative());
    if (m_nOverride & IO_REL_FLAGS)
        other->OverrideRelative(GetRelative());
}

sal_uInt16 LwpIndentOverride::GetRelative()
{
    if ((m_nOverride & IO_REL_FLAGS) == IO_REL_FIRST)
        return RELATIVE_FIRST;
    else if ((m_nOverride & IO_REL_FLAGS) == IO_REL_ALL)
        return RELATIVE_ALL;
    return RELATIVE_REST;
}

sal_Bool LwpIndentOverride::IsUseRelative()
{
    return (m_nValues & IO_USE_RELATIVE) != 0;
}

void LwpIndentOverride::OverrideIndentAll(sal_Int32 val)
{
    m_nAll = val;
    m_nOverride |= IO_ALL;
}

void LwpIndentOverride::OverrideIndentFirst(sal_Int32 val)
{
    m_nFirst = val;
    m_nOverride |= IO_FIRST;
}

void LwpIndentOverride::OverrideIndentRight(sal_Int32 val)
{
    m_nRight = val;
    m_nOverride |= IO_RIGHT;
}

void LwpIndentOverride::OverrideIndentRest(sal_Int32 val)
{
    m_nRest = val;
//    m_nAll = val;
//    m_nFirst = 0-val;
    m_nOverride |= IO_REST;
}

void LwpIndentOverride::OverrideUseRelative(sal_Bool use)
{
    if (use)
    {
        m_nOverride |= IO_USE_RELATIVE;
        m_nValues |= IO_USE_RELATIVE;
    }
    else
    {
        m_nOverride &= ~IO_USE_RELATIVE;
        m_nValues &= ~IO_USE_RELATIVE;
    }
}

void LwpIndentOverride::OverrideRelative(sal_uInt16 relative)
{
    sal_uInt16 Flag;

    if (relative == RELATIVE_FIRST)
        Flag = IO_REL_FIRST;
    else if (relative == RELATIVE_ALL)
        Flag = IO_REL_ALL;
    else
        Flag = IO_REL_REST;

    m_nOverride &= ~IO_REL_FLAGS;
    m_nOverride |= Flag;
}

void LwpSpacingOverride::Override(LwpSpacingOverride* other)
{
//    if (IsAlwaysOverridden())
//		other->SetAlways(IsAlways());//To be done
    if (other)
    {
        m_pSpacing->Override(other->GetSpacing());
        m_pAboveLineSpacing->Override(other->GetAboveLineSpacing());
        m_pParaSpacingAbove->Override(other->GetAboveSpacing());
        m_pParaSpacingBelow->Override(other->GetBelowSpacing());
    }
}

void LwpSpacingCommonOverride::Override(LwpSpacingCommonOverride* other)
{
    if (m_nOverride & SPO_TYPE)
        other->OverrideType(m_nSpacingType);
    if (m_nOverride & SPO_AMOUNT)
        other->OverrideAmount(m_nAmount);
    if (m_nOverride & SPO_MULTIPLE)
        other->OverrideMultiple(m_nMultiple);
}

void LwpSpacingCommonOverride::OverrideType(SpacingType val)
{
    m_nSpacingType = val;
    m_nOverride |= SPO_TYPE;
}

void LwpSpacingCommonOverride::OverrideAmount(sal_Int32 val)
{
    m_nAmount = val;
    m_nOverride |= SPO_AMOUNT;
}

void LwpSpacingCommonOverride::OverrideMultiple(sal_Int32 val)
{
    m_nMultiple = val;
    m_nOverride |= SPO_MULTIPLE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
