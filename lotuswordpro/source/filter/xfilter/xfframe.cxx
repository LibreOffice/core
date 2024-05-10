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
 * Frame whose anchor is not page.
 ************************************************************************/
#include <xfilter/xfglobal.hxx>
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfframe.hxx>

#define     ZINDEX_MIN 0

XFFrame::XFFrame()
    : XFFrame::XFFrame(false)
{}

XFFrame::XFFrame(bool isTextBox)
    : m_eAnchor(enumXFAnchorPara)
    , m_nAnchorPage(0)
    , m_strName(XFGlobal::GenFrameName())  // give it a default name
    , m_nZIndex(ZINDEX_MIN)
    , m_fMinHeight(0)
    , m_eType(enumXFFrameTextbox)
    , m_nFrameFlag(0)
    , m_isTextBox(isTextBox)
{}

XFFrame::~XFFrame()
{
}

void    XFFrame::Add(XFContent *pContent)
{
    if (!pContent)
        return;
    XFContentContainer::Add(pContent);
    if( pContent->GetContentType() == enumXFContentFrame )
    {
        XFFrame *pFrame = static_cast<XFFrame*>(pContent);
        if( pFrame )
        {
            pFrame->SetZIndex(pFrame->m_nZIndex + 1);
        }
    }

}

void    XFFrame::SetZIndex(sal_uInt32 zIndex)
{
    if( zIndex == m_nZIndex )
        return;
    m_nZIndex = zIndex;
    AdjustZIndex();
}

enumXFFrameType XFFrame::GetFrameType() const
{
    return m_eType;
}

enumXFContent   XFFrame::GetContentType()
{
    return enumXFContentFrame;
}

void    XFFrame::ToXml(IXFStream *pStrm)
{
    StartFrame(pStrm);

    XFContentContainer::ToXml(pStrm);

    EndFrame(pStrm);
}

void    XFFrame::StartFrame(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( u"draw:style-name"_ustr, GetStyleName() );

    assert(!m_strName.isEmpty() && "name should not be null.");
    if( !m_strName.isEmpty() && !m_isTextBox)
        pAttrList->AddAttribute( u"draw:name"_ustr, m_strName );
    //anchor type:
    switch( m_eAnchor )
    {
    case enumXFAnchorPara:
        pAttrList->AddAttribute( u"text:anchor-type"_ustr, u"paragraph"_ustr );
        break;
    case enumXFAnchorPage:
        pAttrList->AddAttribute( u"text:anchor-type"_ustr, u"page"_ustr );
        pAttrList->AddAttribute( u"text:anchor-page-number"_ustr, OUString::number(m_nAnchorPage) );
        break;
    case enumXFAnchorChar:
        pAttrList->AddAttribute( u"text:anchor-type"_ustr, u"char"_ustr );
        break;
    case enumXFAnchorAsChar:
        pAttrList->AddAttribute( u"text:anchor-type"_ustr, u"as-char"_ustr );
        break;
    case enumXFAnchorFrame:
        pAttrList->AddAttribute( u"text:anchor-type"_ustr, u"frame"_ustr );
        break;
    default:
        break;
    }
    pAttrList->AddAttribute( u"svg:x"_ustr, OUString::number(m_aRect.GetX()) + "cm" );
    pAttrList->AddAttribute( u"svg:y"_ustr, OUString::number(m_aRect.GetY()) + "cm" );
    pAttrList->AddAttribute( u"svg:width"_ustr, OUString::number(m_aRect.GetWidth()) + "cm" );
    if( m_nFrameFlag& XFFRAME_FLAG_MINHEIGHT )
    {
        pAttrList->AddAttribute( u"fo:min-height"_ustr, OUString::number(m_fMinHeight) + "cm" );
        if( m_nFrameFlag&XFFRAME_FLAG_MAXHEIGHT )
            pAttrList->AddAttribute( u"fo:max-height"_ustr, OUString::number(0) + "cm" );
    }
    else
        pAttrList->AddAttribute( u"svg:height"_ustr, OUString::number(m_aRect.GetHeight()) + "cm" );

    pAttrList->AddAttribute( u"draw:z-index"_ustr, OUString::number(m_nZIndex) );

    if( !m_strNextLink.isEmpty() )
        pAttrList->AddAttribute( u"draw:chain-next-name"_ustr, m_strNextLink );

    pStrm->StartElement( u"draw:text-box"_ustr );
}

void    XFFrame::EndFrame(IXFStream *pStrm)
{
    pStrm->EndElement( u"draw:text-box"_ustr );
}

void    XFFrame::AdjustZIndex()
{
    for( int i=0; i<GetCount(); i++ )
    {
        rtl::Reference<XFContent> pContent = GetContent(i);
        if( pContent.is() )
        {
            if( pContent->GetContentType() == enumXFContentFrame )
            {
                XFFrame *pFrame = static_cast<XFFrame*>(pContent.get());
                pFrame->m_nZIndex = m_nZIndex + 1;
                pFrame->AdjustZIndex();
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
