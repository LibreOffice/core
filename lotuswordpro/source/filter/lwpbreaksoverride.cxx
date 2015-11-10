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
* Breaks override of Wordpro.
************************************************************************/
#include <memory>

#include "clone.hxx"
#include "lwpbreaksoverride.hxx"
#include "lwpobjstrm.hxx"
#include "lwpatomholder.hxx"

LwpBreaksOverride::LwpBreaksOverride()
{
    m_pNextStyle = new LwpAtomHolder();
}

LwpBreaksOverride::LwpBreaksOverride(LwpBreaksOverride const& rOther)
    : LwpOverride(rOther)
    , m_pNextStyle(nullptr)
{
    std::unique_ptr<LwpAtomHolder> pNextStyle(::clone(rOther.m_pNextStyle));
    m_pNextStyle = pNextStyle.release();
}

LwpBreaksOverride::~LwpBreaksOverride()
{
    delete m_pNextStyle;
}

LwpBreaksOverride* LwpBreaksOverride::clone() const
{
    return new LwpBreaksOverride(*this);
}

void    LwpBreaksOverride::Read(LwpObjectStream *pStrm)
{
    if (pStrm->QuickReadBool())
    {
        ReadCommon(pStrm);

        m_pNextStyle->Read(pStrm);
    }

    pStrm->SkipExtra();
}

void LwpBreaksOverride::Override(LwpBreaksOverride* pOther)
{
    if (m_nApply & BO_PAGEBEFORE)
    {
        if (IsPageBreakBeforeOverridden())
        {
            pOther->OverridePageBreakBefore(IsPageBreakBefore());
        }
        else
        {
            pOther->RevertPageBreakBefore();
        }
    }
    if (m_nApply & BO_PAGEAFTER)
    {
        if (IsPageBreakAfterOverridden())
        {
            pOther->OverridePageBreakAfter(IsPageBreakAfter());
        }
        else
        {
            pOther->RevertPageBreakAfter();
        }
    }
    if (m_nApply & BO_KEEPTOGETHER)
    {
        if (IsPageBreakWithinOverridden())
        {
            pOther->OverridePageBreakWithin(IsPageBreakWithin());
        }
        else
        {
            pOther->RevertPageBreakWithin();
        }
    }
    if (m_nApply & BO_COLBEFORE)
    {
        if (IsColumnBreakBeforeOverridden())
        {
            pOther->OverrideColumnBreakBefore(IsColumnBreakBefore());
        }
        else
        {
            pOther->RevertColumnBreakBefore();
        }
    }
    if (m_nApply & BO_COLAFTER)
    {
        if (IsColumnBreakAfterOverridden())
        {
            pOther->OverrideColumnBreakAfter(IsColumnBreakAfter());
        }
        else
        {
            pOther->RevertColumnBreakAfter();
        }
    }
    if (m_nApply & BO_KEEPPREV)
    {
        if (IsKeepWithPreviousOverridden())
        {
            pOther->OverrideKeepWithPrevious(IsKeepWithPrevious());
        }
        else
        {
            pOther->RevertKeepWithPrevious();
        }
    }
    if (m_nApply & BO_KEEPNEXT)
    {
        if (IsKeepWithNextOverridden())
        {
            pOther->OverrideKeepWithNext(IsKeepWithNext());
        }
        else
        {
            pOther->RevertKeepWithNext();
        }
    }
    if (m_nApply & BO_USENEXTSTYLE)
    {
        if (IsUseNextStyleOverridden())
        {
            pOther->OverrideUseNextStyle(IsUseNextStyle());
        }
        else
        {
            pOther->RevertUseNextStyle();
        }
    }
}

void LwpBreaksOverride::OverridePageBreakBefore(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_PAGEBEFORE, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_PAGEBEFORE, STATE_OFF);
    }
}
void LwpBreaksOverride::OverridePageBreakAfter(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_PAGEAFTER, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_PAGEAFTER, STATE_OFF);
    }
}
void LwpBreaksOverride::OverridePageBreakWithin(bool bVal)
{
    // Note the flipped logic
    if(!bVal)
    {
        LwpOverride::Override(BO_KEEPTOGETHER, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_KEEPTOGETHER, STATE_OFF);
    }
}
void LwpBreaksOverride::OverrideColumnBreakBefore(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_COLBEFORE, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_COLBEFORE, STATE_OFF);
    }
}
void LwpBreaksOverride::OverrideColumnBreakAfter(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_COLAFTER, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_COLAFTER, STATE_OFF);
    }
}
void LwpBreaksOverride::OverrideKeepWithNext(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_KEEPNEXT, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_KEEPNEXT, STATE_OFF);
    }
}
void LwpBreaksOverride::OverrideKeepWithPrevious(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_KEEPPREV, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_KEEPPREV, STATE_OFF);
    }
}
void LwpBreaksOverride::OverrideUseNextStyle(bool bVal)
{
    if(bVal)
    {
        LwpOverride::Override(BO_USENEXTSTYLE, STATE_ON);
    }
    else
    {
        LwpOverride::Override(BO_USENEXTSTYLE, STATE_OFF);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
