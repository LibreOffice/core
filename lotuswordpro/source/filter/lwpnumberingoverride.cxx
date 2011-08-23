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
* Numbering override of Wordpro.
************************************************************************/
/*************************************************************************
* Change History
* 2005-01-12 Create and implement.
************************************************************************/

#include	"lwpnumberingoverride.hxx"
#include	"lwpobjstrm.hxx"

LwpNumberingOverride::LwpNumberingOverride()
{
    m_nLevel = 0;
    m_nPosition = 0;
}

void	LwpNumberingOverride::Read(LwpObjectStream *pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);

        pStrm->QuickRead(&m_nLevel, sizeof(m_nLevel));
        pStrm->QuickRead(&m_nPosition, sizeof(m_nPosition));
    }

    pStrm->SkipExtra();
}
//add by , 02/03/2005
void LwpNumberingOverride::Override(LwpNumberingOverride* pOther)
{
    if (m_nApply & NO_LEVEL)
    {
        if (IsLevelOverridden())
        {
            pOther->OverrideLevel(m_nLevel);
        }
        else
        {
            pOther->RevertLevel();
        }
    }

    if (m_nApply & NO_POSITION)
    {
        if (IsPositionOverridden())
        {
            pOther->OverridePosition(m_nPosition);
        }
        else
        {
            pOther->RevertPosition();
        }
    }

    if (m_nApply & HEADING)
    {
        if (IsHeadingOverridden())
        {
            pOther->OverrideHeading(IsHeading());
        }
        else
        {
            pOther->RevertHeading();
        }
    }

    if (m_nApply & SMARTLEVEL)
    {
        if (IsSmartLevelOverridden())
        {
            pOther->OverrideSmartLevel(IsSmartLevel());
        }
        else
        {
            pOther->RevertSmartLevel();
        }
    }
}

void LwpNumberingOverride::operator=(const LwpOverride& rOther)
{
    LwpOverride::operator=(rOther);

    const LwpNumberingOverride* pOther =
        static_cast<LwpNumberingOverride*>((LwpOverride*)&rOther);

    m_nLevel = pOther->m_nLevel;
    m_nPosition = pOther->m_nPosition;
}

void LwpNumberingOverride::OverrideLevel(sal_uInt16 nNewLv)
{
    m_nLevel = nNewLv;
    LwpOverride::Override(NO_LEVEL, STATE_ON);
}
void LwpNumberingOverride::OverridePosition(sal_uInt16 nNewPos)
{
    m_nPosition = nNewPos;
    LwpOverride::Override(NO_POSITION, STATE_ON);
}
void LwpNumberingOverride::OverrideHeading(sal_Bool bVal)
{
    if (bVal)
    {
        LwpOverride::Override(HEADING, STATE_ON);
    }
    else
    {
        LwpOverride::Override(HEADING, STATE_OFF);
    }
}
void LwpNumberingOverride::OverrideSmartLevel(sal_Bool bVal)
{
    if (bVal)
    {
        LwpOverride::Override(SMARTLEVEL, STATE_ON);
    }
    else
    {
        LwpOverride::Override(SMARTLEVEL, STATE_OFF);
    }
}
//end add

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
