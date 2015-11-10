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

#include <memory>

#include "clone.hxx"
#include "lwpoverride.hxx"
#include "lwpfilehdr.hxx"
#include "lwpatomholder.hxx"
#include "lwpborderstuff.hxx"
#include "lwpmargins.hxx"
#include "lwpbackgroundstuff.hxx"

/*class LwpOverride*/
LwpOverride::LwpOverride(LwpOverride const& rOther)
    : m_nValues(rOther.m_nValues)
    , m_nOverride(rOther.m_nOverride)
    , m_nApply(rOther.m_nApply)
{
}

void LwpOverride::ReadCommon(LwpObjectStream* pStrm)
{
    m_nValues = pStrm->QuickReaduInt16();
    m_nOverride = pStrm->QuickReaduInt16();
    m_nApply = pStrm->QuickReaduInt16();
    pStrm->SkipExtra();
}

void LwpOverride::Clear()
{
    m_nValues = 0;
    m_nOverride = 0;
    m_nApply = 0;
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
        else    /* State == STATE_OFF */
        {
            m_nValues &= ~nBits;
        }
    }
    m_nApply |= nBits;
}

/*class LwpTextLanguageOverride*/
LwpTextLanguageOverride::LwpTextLanguageOverride(LwpTextLanguageOverride const& rOther)
    : LwpOverride(rOther)
    , m_nLanguage(rOther.m_nLanguage)
{
}

LwpTextLanguageOverride* LwpTextLanguageOverride::clone() const
{
    return new LwpTextLanguageOverride(*this);
}

void LwpTextLanguageOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_nLanguage = pStrm->QuickReaduInt16();
    }

    pStrm->SkipExtra();

}

/*class LwpTextAttributeOverride*/
LwpTextAttributeOverride::LwpTextAttributeOverride(LwpTextAttributeOverride const& rOther)
    : LwpOverride(rOther)
    , m_nHideLevels(rOther.m_nHideLevels)
    , m_nBaseLineOffset(rOther.m_nBaseLineOffset)
{
}

LwpTextAttributeOverride* LwpTextAttributeOverride::clone() const
{
    return new LwpTextAttributeOverride(*this);
}

void LwpTextAttributeOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_nHideLevels = pStrm->QuickReaduInt16();

        if (LwpFileHeader::m_nFileRevision > 0x000A)
            m_nBaseLineOffset = pStrm->QuickReaduInt32();
    }

    pStrm->SkipExtra();
}

bool LwpTextAttributeOverride::IsHighlight()
{
    return (m_nValues & TAO_HIGHLIGHT) != 0;
}

/*class LwpKinsokuOptsOverride*/
LwpKinsokuOptsOverride::LwpKinsokuOptsOverride(LwpKinsokuOptsOverride const& rOther)
    : LwpOverride(rOther)
    , m_nLevels(rOther.m_nLevels)
{
}

LwpKinsokuOptsOverride* LwpKinsokuOptsOverride::clone() const
{
    return new LwpKinsokuOptsOverride(*this);
}

void LwpKinsokuOptsOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_nLevels = pStrm->QuickReaduInt16();
    }

    pStrm->SkipExtra();
}

/*class LwpBulletOverride*/
LwpBulletOverride::LwpBulletOverride(LwpBulletOverride const& rOther)
    : LwpOverride(rOther)
    , m_SilverBullet(rOther.m_SilverBullet)
    , m_bIsNull(rOther.m_bIsNull)
{
}

LwpBulletOverride* LwpBulletOverride::clone() const
{
    return new LwpBulletOverride(*this);
}

void LwpBulletOverride::Read(LwpObjectStream * pStrm)
{
    if (pStrm->QuickReadBool())
    {
        m_bIsNull= false;
        ReadCommon(pStrm);
        m_SilverBullet.ReadIndexed(pStrm);
    }
    else
        m_bIsNull = true;

    pStrm->SkipExtra();
}
void LwpBulletOverride::OverrideSkip(bool bOver)
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

void LwpBulletOverride::OverrideRightAligned(bool bOver)
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
LwpAlignmentOverride::LwpAlignmentOverride(LwpAlignmentOverride const& rOther)
    : LwpOverride(rOther)
    , m_nAlignType(rOther.m_nAlignType)
    , m_nPosition(rOther.m_nPosition)
    , m_nAlignChar(rOther.m_nAlignChar)
{
}

LwpAlignmentOverride* LwpAlignmentOverride::clone() const
{
    return new LwpAlignmentOverride(*this);
}

void LwpAlignmentOverride::Read(LwpObjectStream * pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_nAlignType = static_cast<AlignType>(pStrm->QuickReaduInt8());
        m_nPosition = pStrm->QuickReaduInt32();
        m_nAlignChar = pStrm->QuickReaduInt16();
    }

    pStrm->SkipExtra();
}

/*class LwpSpacingCommonOverride*/
LwpSpacingCommonOverride::LwpSpacingCommonOverride(LwpSpacingCommonOverride const& rOther)
    : LwpOverride(rOther)
    , m_nSpacingType(rOther.m_nSpacingType)
    , m_nAmount(rOther.m_nAmount)
    , m_nMultiple(rOther.m_nMultiple)
{
}

LwpSpacingCommonOverride* LwpSpacingCommonOverride::clone() const
{
    return new LwpSpacingCommonOverride(*this);
}

void LwpSpacingCommonOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        m_nSpacingType = static_cast<SpacingType>(pStrm->QuickReaduInt16());
        m_nAmount = pStrm->QuickReadInt32();
        m_nMultiple = pStrm->QuickReadInt32();
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

LwpSpacingOverride::LwpSpacingOverride(LwpSpacingOverride const& rOther)
    : LwpOverride(rOther)
    , m_pSpacing(nullptr)
    , m_pAboveLineSpacing(nullptr)
    , m_pParaSpacingAbove(nullptr)
    , m_pParaSpacingBelow(nullptr)
{
    std::unique_ptr<LwpSpacingCommonOverride> pSpacing(::clone(rOther.m_pSpacing));
    std::unique_ptr<LwpSpacingCommonOverride> pAboveLineSpacing(::clone(rOther.m_pAboveLineSpacing));
    std::unique_ptr<LwpSpacingCommonOverride> pParaSpacingAbove(::clone(rOther.m_pParaSpacingAbove));
    std::unique_ptr<LwpSpacingCommonOverride> pParaSpacingBelow(::clone(rOther.m_pParaSpacingBelow));
    m_pSpacing = pSpacing.release();
    m_pAboveLineSpacing = pAboveLineSpacing.release();
    m_pParaSpacingAbove = pParaSpacingAbove.release();
    m_pParaSpacingBelow = pParaSpacingBelow.release();
}

LwpSpacingOverride* LwpSpacingOverride::clone() const
{
    return new LwpSpacingOverride(*this);
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
LwpIndentOverride::LwpIndentOverride(LwpIndentOverride const& rOther)
    : LwpOverride(rOther)
    , m_nAll(rOther.m_nAll)
    , m_nFirst(rOther.m_nFirst)
    , m_nRest(rOther.m_nRest)
    , m_nRight(rOther.m_nRight)
{
}

LwpIndentOverride* LwpIndentOverride::clone() const
{
    return new LwpIndentOverride(*this);
}

void LwpIndentOverride::Read(LwpObjectStream* pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);

        m_nAll = pStrm->QuickReadInt32();
        m_nFirst = pStrm->QuickReadInt32();
        m_nRest = pStrm->QuickReadInt32();
        m_nRight = pStrm->QuickReadInt32();
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

LwpAmikakeOverride::LwpAmikakeOverride(LwpAmikakeOverride const& rOther)
    : LwpOverride(rOther)
    , m_pBackgroundStuff(nullptr)
    , m_nType(rOther.m_nType)
{
    std::unique_ptr<LwpBackgroundStuff> pBackgroundStuff(::clone(rOther.m_pBackgroundStuff));
    m_pBackgroundStuff = pBackgroundStuff.release();
}

LwpAmikakeOverride* LwpAmikakeOverride::clone() const
{
    return new LwpAmikakeOverride(*this);
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
        m_nType = pStrm->QuickReaduInt16();
        pStrm->SkipExtra();
    }
    else
    {
        m_nType = AMIKAKE_NONE;
    }
}

void LwpAlignmentOverride::Override(LwpAlignmentOverride* other)
{
    if (m_nOverride & AO_TYPE)
    {
        other->OverrideAlignment(m_nAlignType);
    }
}

void LwpAlignmentOverride::OverrideAlignment(AlignType val)
{
    m_nAlignType = val;
    m_nOverride |= AO_TYPE;
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

bool LwpIndentOverride::IsUseRelative()
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

void LwpIndentOverride::OverrideUseRelative(bool use)
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
