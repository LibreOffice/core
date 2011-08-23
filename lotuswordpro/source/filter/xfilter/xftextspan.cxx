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
 * An span of text with style.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-12-23 create this file.
 ************************************************************************/
#include	"xftextspan.hxx"
#include	"ixfstream.hxx"
#include	"ixfattrlist.hxx"
#include	"xftextcontent.hxx"

XFTextSpan::XFTextSpan()
{

}

XFTextSpan::XFTextSpan(rtl::OUString& text,
                       rtl::OUString& style
                       )
{
    Add(text);
    SetStyleName(style);
}

XFTextSpan::~XFTextSpan()
{
    std::vector<IXFContent*>::iterator it;
    for( it = m_aContents.begin(); it != m_aContents.end(); it++ )
    {
        IXFContent	*pContent = *it;
        if( pContent )
        {
            delete pContent;
        }
    }
    m_aContents.clear();
}

enumXFContent XFTextSpan::GetContentType()
{
    return enumXFContentSpan;
}

void	XFTextSpan::SetText(rtl::OUString& text)
{
    m_aContents.clear();
    Add(text);
}

rtl::OUString	XFTextSpan::GetText()
{
    rtl::OUString	text;
    std::vector<IXFContent*>::iterator it;

    it = m_aContents.begin();
    if( it != m_aContents.end() )
    {
        IXFContent	*pContent = *it;
        if( pContent->GetContentType() != enumXFContentText )
        {
            assert(false);
        }
        XFTextContent *pText = (XFTextContent*)pContent;
        if( pText )
            text = pText->GetText();
    }

    return text;
}

void	XFTextSpan::Add(IXFContent *pContent)
{
    m_aContents.push_back(pContent);
}

void	XFTextSpan::Add(rtl::OUString& text)
{
    IXFContent *pText = new XFTextContent(text);
    Add(pText);
}

void	XFTextSpan::ToXml(IXFStream *pStrm)
{
    rtl::OUString style = GetStyleName();
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    assert(pAttrList);

    pAttrList->Clear();
    if( style.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    pStrm->StartElement( A2OUSTR("text:span") );

    std::vector<IXFContent*>::iterator it;
    for( it= m_aContents.begin(); it!= m_aContents.end(); it++ )
    {
        IXFContent *pContent = *it;
        if( pContent )
            pContent->ToXml(pStrm);
    }

    pStrm->EndElement( A2OUSTR("text:span") );
}

void	XFTextSpanStart::ToXml(IXFStream *pStrm)
{
    rtl::OUString style = GetStyleName();
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    assert(pAttrList);

    pAttrList->Clear();
    if( style.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    pStrm->StartElement( A2OUSTR("text:span") );

    std::vector<IXFContent*>::iterator it;
    for( it= m_aContents.begin(); it!= m_aContents.end(); it++ )
    {
        IXFContent *pContent = *it;
        if( pContent )
            pContent->ToXml(pStrm);
    }
}
void	XFTextSpanEnd::ToXml(IXFStream *pStrm)
{
    std::vector<IXFContent*>::iterator it;
    for( it= m_aContents.begin(); it!= m_aContents.end(); it++ )
    {
        IXFContent *pContent = *it;
        if( pContent )
            pContent->ToXml(pStrm);
    }
    pStrm->EndElement( A2OUSTR("text:span") );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
