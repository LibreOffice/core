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
 * Hyperlink object for OOo xml filter.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-1-31 create this file.
 ************************************************************************/
#ifndef		_XFHYPERLINK_HXX
#define		_XFHYPERLINK_HXX

class XFHyperlink : public XFContent
{
public:
    XFHyperlink();

public:
    void	SetHRef(rtl::OUString href);

    void	SetText(rtl::OUString text);

    void	SetName(rtl::OUString name);

    void	SetTargetFrame(rtl::OUString frame=A2OUSTR("_self"));

    virtual void	ToXml(IXFStream *pStrm);

private:
    rtl::OUString	m_strHRef;
    rtl::OUString	m_strName;
    rtl::OUString	m_strFrame;
    rtl::OUString	m_strText;
};

inline XFHyperlink::XFHyperlink()
{
    m_strFrame = A2OUSTR("_self");
}

inline void	XFHyperlink::SetHRef(rtl::OUString href)
{
    m_strHRef = href;
}

inline void XFHyperlink::SetName(rtl::OUString name)
{
    m_strName = name;
}

inline void XFHyperlink::SetTargetFrame(rtl::OUString frame)
{
    m_strFrame = frame;
}

inline void XFHyperlink::SetText(rtl::OUString text)
{
    m_strText = text;
}

inline void XFHyperlink::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("xlink:type"), A2OUSTR("simple") );
    pAttrList->AddAttribute( A2OUSTR("xlink:href"), m_strHRef );
    if( m_strName.getLength() > 0 )
        pAttrList->AddAttribute( A2OUSTR("office:name"), m_strName );
    pAttrList->AddAttribute( A2OUSTR("office:target-frame-name"), m_strFrame );
    pAttrList->AddAttribute( A2OUSTR("xlink:show"), A2OUSTR("replace") );
    pStrm->StartElement( A2OUSTR("text:a") );

    pAttrList->Clear();
    if( GetStyleName().getLength()>0 )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    pStrm->StartElement( A2OUSTR("text:span") );
    if( m_strText.getLength() )
        pStrm->Characters(m_strText);
    else
        pStrm->Characters(m_strHRef);
    pStrm->EndElement( A2OUSTR("text:span") );

    pStrm->EndElement( A2OUSTR("text:a") );
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
