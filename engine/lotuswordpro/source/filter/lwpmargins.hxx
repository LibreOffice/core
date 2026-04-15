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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPMARGINS_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPMARGINS_HXX

#include <lwpobjstrm.hxx>
#include <lwptools.hxx>

const sal_uInt8 MARGIN_LEFT = 0;
const sal_uInt8 MARGIN_RIGHT = 1;
const sal_uInt8 MARGIN_TOP = 2;
const sal_uInt8 MARGIN_BOTTOM = 3;

class LwpMargins
{
public:
    LwpMargins():m_nLeft(0),m_nTop(0),m_nRight(0),m_nBottom(0){}
public:
    void    Read(LwpObjectStream *pStrm)
    {
        m_nLeft = pStrm->QuickReadInt32();
        m_nTop = pStrm->QuickReadInt32();
        m_nRight = pStrm->QuickReadInt32();
        m_nBottom = pStrm->QuickReadInt32();
        pStrm->SkipExtra();
    }
    inline double GetMarginsValue(sal_uInt8 nWhichSide);
private:
    sal_Int32       m_nLeft;
    sal_Int32       m_nTop;
    sal_Int32       m_nRight;
    sal_Int32       m_nBottom;
};

inline double LwpMargins::GetMarginsValue(sal_uInt8 nWhichSide)
{
    switch (nWhichSide)
    {
    case MARGIN_LEFT://left
        return LwpTools::ConvertFromUnits(m_nLeft);
    case MARGIN_RIGHT://right
        return LwpTools::ConvertFromUnits(m_nRight);
    case MARGIN_TOP://top
        return LwpTools::ConvertFromUnits(m_nTop);
    case MARGIN_BOTTOM://bottom
        return LwpTools::ConvertFromUnits(m_nBottom);
    }
    // FIXME: this is needed to avoid warning: control reaches end of non-void function
    //        a better solution would be to enum value for the parameter side
    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
