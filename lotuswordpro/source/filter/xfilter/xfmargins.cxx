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
 * Margins object, include left margin,right margin,top margin and bottom margin.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-19 create this file.
 ************************************************************************/


#include	"xfmargins.hxx"

XFMargins::XFMargins()
{
    Reset();
}

void	XFMargins::Reset()
{
    m_fLeft = 0;
    m_fRight = 0;
    m_fTop = 0;
    m_fBottom = 0;
    m_nFlag = 0;
}

void	XFMargins::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    //margin left:
    if( m_nFlag&XFMARGINS_FLAG_LEFT )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:margin-left"), DoubleToOUString(m_fLeft) + A2OUSTR("cm") );
    }
    //margin right:
    if( m_nFlag&XFMARGINS_FLAG_RIGHT )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:margin-right"), DoubleToOUString(m_fRight) + A2OUSTR("cm") );
    }
    //margin top:
    if( m_nFlag&XFMARGINS_FLAG_TOP )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:margin-top"), DoubleToOUString(m_fTop) + A2OUSTR("cm") );
    }
    //margin bottom:
    if( m_nFlag&XFMARGINS_FLAG_BOTTOM )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:margin-bottom"), DoubleToOUString(m_fBottom) + A2OUSTR("cm") );
    }
}

bool operator==(XFMargins& indent1, XFMargins& indent2)
{
    return (
        (indent1.m_nFlag == indent2.m_nFlag) &&
        (indent1.m_fLeft == indent2.m_fLeft) &&
        (indent1.m_fRight == indent2.m_fRight) &&
        (indent1.m_fTop == indent2.m_fTop) &&
        (indent1.m_fBottom == indent2.m_fBottom)
        );
}
bool operator!=(XFMargins& indent1, XFMargins& indent2)
{
    return !(indent1==indent2);
}

XFMargins& XFMargins::operator=(XFMargins& other)
{
    m_nFlag = other.m_nFlag;
    m_fLeft = other.m_fLeft;
    m_fRight = other.m_fRight;
    m_fTop = other.m_fTop;
    m_fBottom = other.m_fBottom;
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
