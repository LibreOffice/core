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
 * Tab style for paragraph.include tab offset and tab char properties.
 ************************************************************************/
#include "xftabstyle.hxx"
#include <xfilter/ixfattrlist.hxx>

XFTabStyle::XFTabStyle()
    : m_eType(enumXFTabNone)
    , m_fLength(0)
{}

void XFTabStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( u"style:position"_ustr, OUString::number(m_fLength) + "cm" );
    switch(m_eType)
    {
    case enumXFTabLeft:
        pAttrList->AddAttribute( u"style:type"_ustr, u"left"_ustr );
        break;
    case enumXFTabCenter:
        pAttrList->AddAttribute( u"style:type"_ustr, u"center"_ustr );
        break;
    case enumXFTabRight:
        pAttrList->AddAttribute( u"style:type"_ustr, u"right"_ustr );
        break;
    case enumXFTabChar:
        pAttrList->AddAttribute( u"style:type"_ustr, u"char"_ustr );
        break;
    default:
        break;
    }
    //delimiter:
    if( m_eType == enumXFTabChar )
        pAttrList->AddAttribute( u"style:char"_ustr, m_strDelimiter );
    //leader char:
    if( !m_strLeader.isEmpty() )
        pAttrList->AddAttribute( u"style:leader-char"_ustr, m_strLeader );

    pStrm->StartElement( u"style:tab-stop"_ustr );
    pStrm->EndElement( u"style:tab-stop"_ustr );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
