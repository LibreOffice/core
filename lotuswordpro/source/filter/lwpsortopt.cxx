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

#include "lwpsortopt.hxx"

void LwpSortKey::Read(LwpObjectStream *pStrm)
{
    pStrm->QuickRead(&m_nField, sizeof(m_nField));
    pStrm->QuickRead(&m_nFlag, sizeof(m_nFlag));
    pStrm->QuickRead(&m_nWord, sizeof(m_nWord));
    pStrm->SkipExtra();
}
LwpSortOption::LwpSortOption(LwpObjectStream* pStrm)
{
    Read(pStrm);
}
/**
 * @descr		Read sort option in VO_DOCUMNET from object stream
 **/
void LwpSortOption::Read(LwpObjectStream *pStrm)
{
    pStrm->QuickRead(&m_nCount, sizeof(m_nCount));
    pStrm->QuickRead(&m_nFlags, sizeof(m_nFlags));
    pStrm->QuickRead(&m_nText, sizeof(m_nText));
    for(sal_uInt8 i=0; i<3; i++)
    {
        m_Keys[i].Read(pStrm);
    }
    pStrm->SkipExtra();
}
/**
 * @descr		skip the sort option
 **/
void LwpSortOption::Skip(LwpObjectStream *pStrm)
{
    pStrm->SeekRel(DiskSize());
}
/**
 * @descr		return the disk size
 **/
sal_uInt16 LwpSortOption::DiskSize()
{
    sal_uInt16 keysize = 4 * sizeof(sal_uInt16);
    return (3* keysize + (2 * sizeof(sal_uInt16)) + sizeof(sal_uInt8) + sizeof(sal_uInt16));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
