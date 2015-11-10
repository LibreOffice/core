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
* Border override of Wordpro.
************************************************************************/
#include <memory>

#include "clone.hxx"
#include "lwpparaborderoverride.hxx"
#include "lwpborderstuff.hxx"
#include "lwpshadow.hxx"
#include "lwpmargins.hxx"

LwpParaBorderOverride::LwpParaBorderOverride()
{
    m_pBorderStuff = new LwpBorderStuff();
    m_pBetweenStuff = new LwpBorderStuff();
    m_pShadow = new LwpShadow();
    m_pMargins = new LwpMargins();

    m_eAboveType = PB_NONE;
    m_eBelowType = PB_NONE;
    m_eRightType = PB_NONE;
    m_eBetweenType = PB_NONE;

    m_nAboveWidth = 0;
    m_nBelowWidth = 0;
    m_nBetweenWidth = 0;
    m_nRightWidth = 0;

    m_nBetweenMargin = 0;
}

LwpParaBorderOverride::LwpParaBorderOverride(LwpParaBorderOverride const& rOther)
    : LwpOverride(rOther)
    , m_pBorderStuff(nullptr)
    , m_pBetweenStuff(nullptr)
    , m_pShadow(nullptr)
    , m_pMargins(nullptr)
    , m_eAboveType(rOther.m_eAboveType)
    , m_eBelowType(rOther.m_eBelowType)
    , m_eRightType(rOther.m_eRightType)
    , m_eBetweenType(rOther.m_eBetweenType)
    , m_nAboveWidth(rOther.m_nAboveWidth)
    , m_nBelowWidth(rOther.m_nBelowWidth)
    , m_nBetweenWidth(rOther.m_nBetweenWidth)
    , m_nRightWidth(rOther.m_nRightWidth)
    , m_nBetweenMargin(rOther.m_nBetweenMargin)
{
    std::unique_ptr<LwpBorderStuff> pBorderStuff(::clone(rOther.m_pBorderStuff));
    std::unique_ptr<LwpBorderStuff> pBetweenStuff(::clone(rOther.m_pBetweenStuff));
    std::unique_ptr<LwpShadow> pShadow(::clone(rOther.m_pShadow));
    std::unique_ptr<LwpMargins> pMargins(::clone(rOther.m_pMargins));
    m_pBorderStuff = pBorderStuff.release();
    m_pBetweenStuff = pBetweenStuff.release();
    m_pShadow = pShadow.release();
    m_pMargins = pMargins.release();
}

LwpParaBorderOverride::~LwpParaBorderOverride()
{
    delete m_pBorderStuff;
    delete m_pBetweenStuff;
    delete m_pShadow;
    delete m_pMargins;
}

LwpParaBorderOverride* LwpParaBorderOverride::clone() const
{
    return new LwpParaBorderOverride(*this);
}

void LwpParaBorderOverride::Read(LwpObjectStream *pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);
        //start here:
        m_pBorderStuff->Read(pStrm);
        m_pShadow->Read(pStrm);
        m_pMargins->Read(pStrm);

        m_eAboveType = static_cast<BorderWidthType>(pStrm->QuickReaduInt16());
        m_eBelowType = static_cast<BorderWidthType>(pStrm->QuickReaduInt16());
        m_eRightType = static_cast<BorderWidthType>(pStrm->QuickReaduInt16());

        if( pStrm->CheckExtra() )
        {
            m_pBetweenStuff->Read(pStrm);

            m_eBetweenType = static_cast<BorderWidthType>(pStrm->QuickReaduInt16());
            m_nBetweenWidth = pStrm->QuickReaduInt32();
            m_nBetweenMargin = pStrm->QuickReaduInt32();

            if( pStrm->CheckExtra() )
            {
                m_eRightType = static_cast<BorderWidthType>(pStrm->QuickReaduInt16());
                m_nRightWidth = pStrm->QuickReaduInt32();
            }
        }
    }

    pStrm->SkipExtra();
}

void LwpParaBorderOverride::Override(LwpParaBorderOverride* pOther)
{
    if (m_nApply & PBO_STUFF)
    {
        if (IsBorderStuffOverridden())
        {
            pOther->OverrideBorderStuff(m_pBorderStuff);
        }
        else
        {
            pOther->RevertBorderStuff();
        }
    }

    if (m_nApply & PBO_BETWEENSTUFF)
    {
        if (IsBetweenStuffOverridden())
        {
            pOther->OverrideBetweenStuff(m_pBetweenStuff);
        }
        else
        {
            pOther->RevertBetweenStuff();
        }
    }

    if (m_nApply & PBO_SHADOW)
    {
        if (IsShadowOverridden())
        {
            pOther->OverrideShadow(m_pShadow);
        }
        else
        {
            pOther->RevertShadow();
        }
    }

    if (m_nApply & PBO_MARGINS)
    {
        if (IsMarginsOverridden())
        {
            pOther->OverrideMargins(m_pMargins);
        }
        else
        {
            pOther->RevertMargins();
        }
    }

    if (m_nApply & PBO_ABOVETYPE)
    {
        if (IsAboveTypeOverridden())
        {
            pOther->OverrideAboveType(m_eAboveType);
        }
        else
        {
            pOther->RevertAboveType();
        }
    }

    if (m_nApply & PBO_BELOWTYPE)
    {
        if (IsBelowTypeOverridden())
        {
            pOther->OverrideBelowType(m_eBelowType);
        }
        else
        {
            pOther->RevertBelowType();
        }
    }

    if (m_nApply & PBO_RIGHTTYPE)
    {
        if (IsRightTypeOverridden())
        {
            pOther->OverrideRightType(m_eRightType);
        }
        else
        {
            pOther->RevertRightType();
        }
    }

    if (m_nApply & PBO_BETWEENTYPE)
    {
        if (IsBetweenTypeOverridden())
        {
            pOther->OverrideBetweenType(m_eBetweenType);
        }
        else
        {
            pOther->RevertBetweenType();
        }
    }

    if (m_nApply & PBO_ABOVE)
    {
        if (IsAboveWidthOverridden())
        {
            pOther->OverrideAboveWidth(m_nAboveWidth);
        }
        else
        {
            pOther->RevertAboveWidth();
        }
    }

    if (m_nApply & PBO_BELOW)
    {
        if (IsBelowWidthOverridden())
        {
            pOther->OverrideBelowWidth(m_nBelowWidth);
        }
        else
        {
            pOther->RevertBelowWidth();
        }
    }

    if (m_nApply & PBO_BETWEEN)
    {
        if (IsBetweenWidthOverridden())
        {
            pOther->OverrideBetweenWidth(m_nBetweenWidth);
        }
        else
        {
            pOther->RevertBetweenWidth();
        }
    }

    if (m_nApply & PBO_BETWEENMARGIN)
    {
        if (IsBetweenMarginOverridden())
        {
            pOther->OverrideBetweenMargin(m_nBetweenMargin);
        }
        else
        {
            pOther->RevertBetweenMargin();
        }
    }

    if (m_nApply & PBO_RIGHT)
    {
        if (IsRightWidthOverridden())
        {
            pOther->OverrideRightWidth(m_nRightWidth);
        }
        else
        {
            pOther->RevertRightWidth();
        }
    }
}

void LwpParaBorderOverride::OverrideBorderStuff(LwpBorderStuff* pBorderStuff)
{
    *m_pBorderStuff = *pBorderStuff;
    LwpOverride::Override(PBO_STUFF, STATE_ON);
}
void LwpParaBorderOverride::OverrideBetweenStuff(LwpBorderStuff* pBorderStuff)
{
    *m_pBetweenStuff = *pBorderStuff;
    LwpOverride::Override(PBO_BETWEENSTUFF, STATE_ON);
}
void LwpParaBorderOverride::OverrideShadow(LwpShadow* pShadow)
{
    *m_pShadow = *pShadow;
    LwpOverride::Override(PBO_SHADOW, STATE_ON);
}
void LwpParaBorderOverride::OverrideMargins(LwpMargins* pMargins)
{
    *m_pMargins = *pMargins;
    LwpOverride::Override(PBO_MARGINS, STATE_ON);
}
void LwpParaBorderOverride::OverrideAboveType(BorderWidthType eNewType)
{
    m_eAboveType = eNewType;
    LwpOverride::Override(PBO_ABOVETYPE, STATE_ON);
}
void LwpParaBorderOverride::OverrideBelowType(BorderWidthType eNewType)
{
    m_eBelowType = eNewType;
    LwpOverride::Override(PBO_BELOWTYPE, STATE_ON);
}
void LwpParaBorderOverride::OverrideRightType(BorderWidthType eNewType)
{
    m_eRightType = eNewType;
    LwpOverride::Override(PBO_RIGHTTYPE, STATE_ON);
}
void LwpParaBorderOverride::OverrideBetweenType(BorderWidthType eNewType)
{
    m_eBetweenType = eNewType;
    LwpOverride::Override(PBO_BETWEENTYPE, STATE_ON);
}
void LwpParaBorderOverride::OverrideAboveWidth(sal_uInt32 nNewWidth)
{
    m_nAboveWidth = nNewWidth;
    LwpOverride::Override(PBO_ABOVE, STATE_ON);
}
void LwpParaBorderOverride::OverrideBelowWidth(sal_uInt32 nNewWidth)
{
    m_nBelowWidth = nNewWidth;
    LwpOverride::Override(PBO_BELOW, STATE_ON);
}
void LwpParaBorderOverride::OverrideBetweenWidth(sal_uInt32 nNewWidth)
{
    m_nBetweenWidth = nNewWidth;
    LwpOverride::Override(PBO_BETWEEN, STATE_ON);
}
void LwpParaBorderOverride::OverrideRightWidth(sal_uInt32 nNewWidth)
{
    m_nRightWidth = nNewWidth;
    LwpOverride::Override(PBO_RIGHT, STATE_ON);
}
void LwpParaBorderOverride::OverrideBetweenMargin(sal_uInt32 nNewMargin)
{
    m_nBetweenMargin = nNewMargin;
    LwpOverride::Override(PBO_BETWEENMARGIN, STATE_ON);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
