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
 * 2005-01-13	Created
 ************************************************************************/
#ifndef		_LWPSHADOW_HXX
#define		_LWPSHADOW_HXX

#include	"lwpcolor.hxx"
#include	"lwptools.hxx"

class LwpShadow
{
public:
    LwpShadow():m_nDirX(0),m_nDirY(0){}
public:
    void Read(LwpObjectStream *pStrm)
    {
        m_aColor.Read(pStrm);
        pStrm->QuickRead(&m_nDirX, sizeof(m_nDirX));
        pStrm->QuickRead(&m_nDirY, sizeof(m_nDirY));
        pStrm->SkipExtra();
    }

    double	GetOffsetX();

    double	GetOffsetY();

    LwpColor GetColor();

    //add by , 01/26/2005
    inline void operator = (const LwpShadow& rOther);
    //end add

private:
    LwpColor		m_aColor;
    sal_Int32		m_nDirX;
    sal_Int32		m_nDirY;
};

inline double	LwpShadow::GetOffsetX()
{
    //	return 0.102;
    return LwpTools::ConvertFromUnitsToMetric(m_nDirX);
}

inline double	LwpShadow::GetOffsetY()
{
    return LwpTools::ConvertFromUnitsToMetric(m_nDirY);
}

inline LwpColor LwpShadow::GetColor()
{
    return m_aColor;
}

inline void LwpShadow::operator = (const LwpShadow& rOther)
{
    m_aColor = rOther.m_aColor;
    m_nDirX = rOther.m_nDirX;
    m_nDirY = rOther.m_nDirY;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
