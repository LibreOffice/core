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

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfmargins.hxx>

XFMargins::XFMargins()
{
    Reset();
}

void    XFMargins::Reset()
{
    m_fLeft = 0;
    m_fRight = 0;
    m_fTop = 0;
    m_fBottom = 0;
    m_nFlag = 0;
}

void    XFMargins::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    //margin left:
    if( m_nFlag&XFMARGINS_FLAG_LEFT )
    {
        pAttrList->AddAttribute( u"fo:margin-left"_ustr, OUString::number(m_fLeft) + "cm" );
    }
    //margin right:
    if( m_nFlag&XFMARGINS_FLAG_RIGHT )
    {
        pAttrList->AddAttribute(u"fo:margin-right"_ustr, OUString::number(m_fRight) + "cm" );
    }
    //margin top:
    if( m_nFlag&XFMARGINS_FLAG_TOP )
    {
        pAttrList->AddAttribute(u"fo:margin-top"_ustr, OUString::number(m_fTop) + "cm" );
    }
    //margin bottom:
    if( m_nFlag&XFMARGINS_FLAG_BOTTOM )
    {
        pAttrList->AddAttribute(u"fo:margin-bottom"_ustr, OUString::number(m_fBottom) + "cm" );
    }
}

bool operator==(XFMargins const & indent1, XFMargins const & indent2)
{
    return (
        (indent1.m_nFlag == indent2.m_nFlag) &&
        (indent1.m_fLeft == indent2.m_fLeft) &&
        (indent1.m_fRight == indent2.m_fRight) &&
        (indent1.m_fTop == indent2.m_fTop) &&
        (indent1.m_fBottom == indent2.m_fBottom)
        );
}
bool operator!=(XFMargins const & indent1, XFMargins const & indent2)
{
    return !(indent1==indent2);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
