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
 * Time style. The Time format for time field.
 ************************************************************************/

#include <xfilter/xftimestyle.hxx>
#include <xfilter/ixfattrlist.hxx>

XFTimePart::XFTimePart()
{
    m_nDecimalPos = 0;
}

void    XFTimePart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    switch(m_ePart)
    {
    case enumXFDateHour:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( u"number:style"_ustr, u"long"_ustr );
        pStrm->StartElement( u"number:hours"_ustr );
        pStrm->EndElement( u"number:hours"_ustr );
        break;
    case enumXFDateMinute:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( u"number:style"_ustr, u"long"_ustr );
        if( m_nDecimalPos>0 )
            pAttrList->AddAttribute( u"number:decimal-places"_ustr, OUString::number(m_nDecimalPos) );

        pStrm->StartElement( u"number:minutes"_ustr );
        pStrm->EndElement( u"number:minutes"_ustr );
        break;
    case enumXFDateSecond:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( u"number:style"_ustr, u"long"_ustr );
        pStrm->StartElement( u"number:seconds"_ustr );
        pStrm->EndElement( u"number:seconds"_ustr );
        break;
    case enumXFDateText:
        pAttrList->Clear();
        pStrm->StartElement( u"number:text"_ustr );
        pStrm->Characters( m_strText );
        pStrm->EndElement( u"number:text"_ustr );
        break;
    default:
        break;
    }
}

XFTimeStyle::XFTimeStyle()
{
    m_bAmPm = false;
    m_bTruncate = true;
}

XFTimeStyle::~XFTimeStyle()
{
}

enumXFStyle XFTimeStyle::GetStyleFamily()
{
    return enumXFStyleTime;
}

void    XFTimeStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( u"style:name"_ustr, GetStyleName() );
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute(u"style:parent-style-name"_ustr,GetParentStyleName());
    pAttrList->AddAttribute( u"style:family"_ustr, u"data-style"_ustr );
    if (!m_bTruncate)
        pAttrList->AddAttribute( u"number:truncate-on-overflow"_ustr, u"false"_ustr );
    pStrm->StartElement( u"number:time-style"_ustr );

    for (auto & part : m_aParts)
    {
        part.ToXml(pStrm);
    }
    if( m_bAmPm )
    {
        pAttrList->Clear();
        pStrm->StartElement( u"number:am-pm"_ustr );
        pStrm->EndElement( u"number:am-pm"_ustr );
    }
    pStrm->EndElement( u"number:time-style"_ustr );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
