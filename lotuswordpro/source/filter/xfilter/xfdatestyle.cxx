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
 * Date style. The date format for date field.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-20 create this file.
 ************************************************************************/
#include	"xfdatestyle.hxx"

XFDatePart::XFDatePart()
{
    m_bTexture = sal_False;
}

void	XFDatePart::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    switch(m_ePart)
    {
    case enumXFDateUnknown:
        break;
    case enumXFDateYear:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:year") );
        pStrm->EndElement( A2OUSTR("number:year") );
        break;
    case enumXFDateMonth:
        pAttrList->Clear();
        if( m_bTexture )
            pAttrList->AddAttribute( A2OUSTR("number:textual"), A2OUSTR("true") );
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:month") );
        pStrm->EndElement( A2OUSTR("number:month") );
        break;
    case enumXFDateMonthDay:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:day") );
        pStrm->EndElement( A2OUSTR("number:day") );
        break;
    case enumXFDateWeekDay:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:day-of-week") );
        pStrm->EndElement( A2OUSTR("number:day-of-week") );
        break;
    case enumXFDateYearWeek:
        pAttrList->Clear();
        pStrm->StartElement( A2OUSTR("number:week-of-year") );
        pStrm->EndElement( A2OUSTR("number:week-of-year") );
        break;
    case enumXFDateEra:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:era") );
        pStrm->EndElement( A2OUSTR("number:era") );
        break;
    case enumXFDateQuarter:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:quarter") );
        pStrm->EndElement( A2OUSTR("number:quarter") );
        break;
    case enumXFDateHour:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:hours") );
        pStrm->EndElement( A2OUSTR("number:hours") );
        break;
    case enumXFDateMinute:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        pStrm->StartElement( A2OUSTR("number:minutes") );
        pStrm->EndElement( A2OUSTR("number:minutes") );
        break;
    case enumXFDateSecond:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( A2OUSTR("number:style"), A2OUSTR("long") );
        if( m_nDecimalPos )
            pAttrList->AddAttribute( A2OUSTR("number:decimal-places"), Int32ToOUString(m_nDecimalPos) );
        pStrm->StartElement( A2OUSTR("number:seconds") );
        pStrm->EndElement( A2OUSTR("number:seconds") );
        break;
    case enumXFDateAmPm:
        pStrm->StartElement( A2OUSTR("number:am-pm") );
        pStrm->EndElement( A2OUSTR("number:am-pm") );
        break;
    case enumXFDateText:
        pAttrList->Clear();
        pStrm->StartElement( A2OUSTR("number:text") );
        pStrm->Characters( m_strText );
        pStrm->EndElement( A2OUSTR("number:text") );
        break;
    }
}

XFDateStyle::XFDateStyle()
{
    m_bFixed = sal_False;
    m_bAutoOrder = sal_False;
}

XFDateStyle::~XFDateStyle()
{
}

enumXFStyle XFDateStyle::GetStyleFamily()
{
    return enumXFStyleDate;
}

void	XFDateStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("data-style") );

    pStrm->StartElement( A2OUSTR("number:date-style") );

    m_aParts.ToXml(pStrm);

    pStrm->EndElement( A2OUSTR("number:date-style") );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
