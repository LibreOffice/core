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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFHYPERLINK_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFHYPERLINK_HXX

class XFHyperlink : public XFContent
{
public:
    XFHyperlink();

public:
    void    SetHRef(const OUString& href);

    void    SetText(const OUString& text);

    virtual void    ToXml(IXFStream *pStrm) override;

private:
    OUString   m_strHRef;
    OUString   m_strName;
    OUString   m_strFrame;
    OUString   m_strText;
};

inline XFHyperlink::XFHyperlink()
{
    m_strFrame = "_self";
}

inline void XFHyperlink::SetHRef(const OUString& href)
{
    m_strHRef = href;
}

inline void XFHyperlink::SetText(const OUString& text)
{
    m_strText = text;
}

inline void XFHyperlink::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( "xlink:type", "simple" );
    pAttrList->AddAttribute( "xlink:href", m_strHRef );
    if( !m_strName.isEmpty() )
        pAttrList->AddAttribute( "office:name", m_strName );
    pAttrList->AddAttribute( "office:target-frame-name", m_strFrame );
    pAttrList->AddAttribute( "xlink:show", "replace" );
    pStrm->StartElement( "text:a" );

    pAttrList->Clear();
    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( "text:style-name", GetStyleName() );
    pStrm->StartElement( "text:span" );
    if( !m_strText.isEmpty() )
        pStrm->Characters(m_strText);
    else
        pStrm->Characters(m_strHRef);
    pStrm->EndElement( "text:span" );

    pStrm->EndElement( "text:a" );
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
