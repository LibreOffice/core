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
 * Style for <text:p> or <text:h> or <text:span>
 * Now we have only font attributes.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-12-26 create this file.
 ************************************************************************/
#include	"xftextstyle.hxx"
#include	"ixfattrlist.hxx"
#include	"ixfstream.hxx"
#include	"xffont.hxx"
#include	"xfutil.hxx"

XFTextStyle::XFTextStyle()
{
    m_pFont = NULL;
}

XFTextStyle::~XFTextStyle()
{
}

void	XFTextStyle::SetFont(XFFont *font)
{
    m_pFont = font;
}

sal_Bool	XFTextStyle::Equal(IXFStyle *pStyle)
{
    if( !pStyle || pStyle->GetStyleFamily() != enumXFStyleText )
    {
        return sal_False;
    }

    XFTextStyle	*pOther = (XFTextStyle*)pStyle;
    if( !pOther )
    {
        return sal_False;
    }

    if( m_pFont )
    {
        if( !pOther->m_pFont )
            return sal_False;
        if( *m_pFont != *pOther->m_pFont )
            return sal_False;
    }
    else if( pOther->m_pFont )
        return sal_False;

    return sal_True;
}

enumXFStyle	XFTextStyle::GetStyleFamily()
{
    return enumXFStyleText;
}

void	XFTextStyle::ToXml(IXFStream *strm)
{
    IXFAttrList	*pAttrList = strm->GetAttrList();
    rtl::OUString style = GetStyleName();

    pAttrList->Clear();
    if( style.getLength() )
        pAttrList->AddAttribute(rtl::OUString::createFromAscii("style:name"),GetStyleName());
    if( GetParentStyleName().getLength() > 0 )
        pAttrList->AddAttribute(A2OUSTR("style:parent-style-name"),GetParentStyleName());

    pAttrList->AddAttribute(rtl::OUString::createFromAscii("style:family"),A2OUSTR("text") );
    strm->StartElement(rtl::OUString::createFromAscii("style:style"));

    //Font properties:
    pAttrList->Clear();
    //font name:
    if( m_pFont )
        m_pFont->ToXml(strm);

    strm->StartElement(rtl::OUString::createFromAscii("style:properties"));
    strm->EndElement(rtl::OUString::createFromAscii("style:properties"));

    strm->EndElement(rtl::OUString::createFromAscii("style:style"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
