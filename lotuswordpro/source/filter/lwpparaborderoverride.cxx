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
/*************************************************************************
* Change History
* 2005-01-11 create and implement.
************************************************************************/
#include	"lwpparaborderoverride.hxx"
#include	"lwpborderstuff.hxx"
#include	"lwpshadow.hxx"
#include	"lwpmargins.hxx"

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

LwpParaBorderOverride::~LwpParaBorderOverride()
{
    if( m_pBorderStuff )
        delete m_pBorderStuff;
    if( m_pBetweenStuff )
        delete m_pBetweenStuff;
    if( m_pShadow )
        delete m_pShadow;
    if( m_pMargins )
        delete m_pMargins;
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

        pStrm->QuickRead(&m_eAboveType,2);
        pStrm->QuickRead(&m_eBelowType,2);
        pStrm->QuickRead(&m_eRightType,2);

        if( pStrm->CheckExtra() )
        {
            m_pBetweenStuff->Read(pStrm);

            pStrm->QuickRead(&m_eBetweenType,2);
            pStrm->QuickRead(&m_nBetweenWidth,4 );
            pStrm->QuickRead(&m_nBetweenMargin,4 );

            if( pStrm->CheckExtra() )
            {
                pStrm->QuickRead( &m_eRightType, 2);
                pStrm->QuickRead( &m_nRightWidth, 4);
            }
        }
    }

    pStrm->SkipExtra();
}

// 01/26/2005
void LwpParaBorderOverride::operator=(const LwpOverride& rOther)
{
    LwpOverride::operator=(rOther);
    const LwpParaBorderOverride* pParaBorder = static_cast<LwpParaBorderOverride*>((LwpOverride*)&rOther);

    if (pParaBorder->m_pBorderStuff)
    {
        *m_pBorderStuff = *(pParaBorder->m_pBorderStuff);
    }
    if (pParaBorder->m_pBetweenStuff)
    {
        *m_pBetweenStuff = *(pParaBorder->m_pBetweenStuff);
    }
    if (pParaBorder->m_pShadow)
    {
        *m_pShadow = *(pParaBorder->m_pShadow);
    }
    if (pParaBorder->m_pMargins)
    {
        *m_pMargins = *(pParaBorder->m_pMargins);
    }

    m_eAboveType = pParaBorder->m_eAboveType;
    m_eBelowType = pParaBorder->m_eBelowType;
    m_eRightType = pParaBorder->m_eRightType;
    m_eBetweenType = pParaBorder->m_eBelowType;

    m_nAboveWidth = pParaBorder->m_nAboveWidth;
    m_nBelowWidth = pParaBorder->m_nBelowWidth;
    m_nBetweenWidth = pParaBorder->m_nBetweenWidth;
    m_nRightWidth = pParaBorder->m_nRightWidth;

    m_nBetweenMargin = pParaBorder->m_nBetweenMargin;
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

//end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
