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

#include "lwpcolor.hxx"
/**
 * @descr		read color and then resolve the RGB values
*/
void LwpColor::Read(LwpObjectStream *pStrm)
{
    pStrm->QuickRead(&m_nRed, sizeof(m_nRed));
    pStrm->QuickRead(&m_nGreen, sizeof(m_nGreen));
    pStrm->QuickRead(&m_nBlue, sizeof(m_nBlue));
    pStrm->QuickRead(&m_nExtra, sizeof(m_nExtra));
    ResolveRGB();
}

/**
 * @descr		return the BGR format
*/
sal_uInt32 LwpColor::To24Color()
{
    return ((m_nRed >> 8) |
                (m_nGreen & 0xFF00) |
                (static_cast<sal_uInt32>((m_nBlue & 0xFF00) << 8)));
}
/**
 * @descr		resolver RGB values per the extra bytes
*/
void LwpColor::ResolveRGB()
{
    switch(m_nExtra)
    {
        case AGLRGB_RGB:
            break;
        case AGLRGB_RED:
            m_nRed = 0xFFFF;
            m_nGreen = 0x0000;
            m_nBlue = 0x0000;
            break;
        case AGLRGB_GREEN:
            m_nRed = 0x0000;
            m_nGreen = 0xFFFF;
            m_nBlue = 0x0000;
            break;
        case AGLRGB_BLUE:
            m_nRed = 0x0000;
            m_nGreen = 0x0000;
            m_nBlue = 0xFFFF;
            break;
        case AGLRGB_BLACK:
            m_nRed = 0x0000;
            m_nGreen = 0x0000;
            m_nBlue = 0x0000;
            break;
        case AGLRGB_WHITE:
            m_nRed = 0xFFFF;
            m_nGreen = 0xFFFF;
            m_nBlue = 0xFFFF;
            break;
        case AGLRGB_GRAY:
            m_nRed = 0x8080;
            m_nGreen = 0x8080;
            m_nBlue = 0x8080;
            break;
        case AGLRGB_LTGRAY:
            m_nRed = 0xC0C0;
            m_nGreen = 0xC0C0;
            m_nBlue = 0xC0C0;
            break;
        case AGLRGB_DKGRAY:
            m_nRed = 0x4040;
            m_nGreen = 0x4040;
            m_nBlue = 0x4040;
            break;
        case AGLRGB_INVALID:
        case AGLRGB_TRANSPARENT:
        default:
            m_nRed = 0;
            m_nGreen = 0;
            m_nBlue = 0;
        }
}
void LwpColor::operator = (const LwpColor& rOther)
{
    m_nRed = rOther.m_nRed;
    m_nGreen = rOther.m_nGreen;
    m_nBlue = rOther.m_nBlue;
    m_nExtra = rOther.m_nExtra;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
