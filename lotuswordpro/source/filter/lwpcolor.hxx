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

#ifndef _LWPCOLOR_HXX
#define _LWPCOLOR_HXX

#include "lwpobjstrm.hxx"
#include "lwpheader.hxx"
/**
 * @brief	lwpcolor class
 * 		(red, green, blue, extra)
*/
class LwpColor
{
public:
    LwpColor():m_nRed(0), m_nGreen(0), m_nBlue(0), m_nExtra(0){};
    ~LwpColor(){};
public:
    void Read(LwpObjectStream *pStrm);
    sal_uInt16 GetRed();
    sal_uInt16 GetGreen();
    sal_uInt16 GetBlue();
    BOOL IsValidColor();
    sal_uInt32 To24Color();
    //add by , 01/26/2005
    void operator = (const LwpColor& rOther);
    //end
    sal_Bool IsTransparent();
private:
    sal_uInt16 m_nRed;			// When extra is AGLRGB_INDEX, m_nRed holds the
    sal_uInt16 m_nGreen;		// hi 16 bits and m_nGreen holds the lo 16 bits of
    sal_uInt16 m_nBlue;		// the 32-bit LUT index.
    sal_uInt16 m_nExtra;
    enum ColorOverride {
          AGLRGB_RGB = 0,
          AGLRGB_BLACK = 1,
          AGLRGB_WHITE = 2,
          AGLRGB_GRAY = 3,
          AGLRGB_LTGRAY = 4,
          AGLRGB_DKGRAY = 5,
          AGLRGB_RED = 6,
          AGLRGB_GREEN = 7,
          AGLRGB_BLUE = 8,
          AGLRGB_INDEX = 98,
          AGLRGB_INVALID = 99,
          AGLRGB_TRANSPARENT = 100
    };
    void ResolveRGB();
};
inline sal_uInt16 LwpColor::GetRed()
{
    return m_nRed;
}
inline sal_uInt16 LwpColor::GetGreen()
{
    return m_nGreen;
}
inline sal_uInt16 LwpColor::GetBlue()
{
    return m_nBlue;
}
inline BOOL LwpColor::IsValidColor()
{
    return ((m_nExtra!=AGLRGB_INVALID) && (m_nExtra!=AGLRGB_TRANSPARENT));
}
inline sal_Bool LwpColor::IsTransparent()
{
    return (m_nExtra==AGLRGB_TRANSPARENT);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
