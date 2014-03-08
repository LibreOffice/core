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
 * Time style. The TIme format for time field.
 ************************************************************************/

#include "xftimestyle.hxx"

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
            pAttrList->AddAttribute( "number:style", "long" );
        pStrm->StartElement( "number:hours" );
        pStrm->EndElement( "number:hours" );
        break;
    case enumXFDateMinute:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( "number:style", "long" );
        if( m_nDecimalPos>0 )
            pAttrList->AddAttribute( "number:decimal-places", OUString::number(m_nDecimalPos) );

        pStrm->StartElement( "number:minutes" );
        pStrm->EndElement( "number:minutes" );
        break;
    case enumXFDateSecond:
        pAttrList->Clear();
        if( m_bLongFmt )
            pAttrList->AddAttribute( "number:style", "long" );
        pStrm->StartElement( "number:seconds" );
        pStrm->EndElement( "number:seconds" );
        break;
    case enumXFDateText:
        pAttrList->Clear();
        pStrm->StartElement( "number:text" );
        pStrm->Characters( m_strText );
        pStrm->EndElement( "number:text" );
        break;
    default:
        break;
    }
}

XFTimeStyle::XFTimeStyle()
{
    m_bFixed = sal_True;
    m_bAmPm = sal_False;
    m_bTruncate = sal_True;
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
    pAttrList->AddAttribute( "style:name", GetStyleName() );
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute("style:parent-style-name",GetParentStyleName());
    pAttrList->AddAttribute( "style:family", "data-style" );
    if (!m_bTruncate)
        pAttrList->AddAttribute( "number:truncate-on-overflow", "false" );
    pStrm->StartElement( "number:time-style" );

    std::vector<XFTimePart>::iterator it;
    for( it = m_aParts.begin(); it != m_aParts.end(); ++it )
    {
        (*it).ToXml(pStrm);
    }
    if( m_bAmPm )
    {
        pAttrList->Clear();
        pStrm->StartElement( "number:am-pm" );
        pStrm->EndElement( "number:am-pm" );
    }
    pStrm->EndElement( "number:time-style" );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
