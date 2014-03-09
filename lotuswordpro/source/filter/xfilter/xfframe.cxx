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
#include "xfframe.hxx"
#include "xfparagraph.hxx"
#define     ZINDEX_MIN 0
XFFrame::XFFrame()
{
    m_isTextBox = sal_False;
    m_eAnchor = enumXFAnchorPara;
    m_nAnchorPage = 0;
    m_nZIndex = ZINDEX_MIN;
    //give it a default name:
    m_strName = XFGlobal::GenFrameName();
    m_fMinHeight = 0;
    m_fMaxHeight = 0;
    m_nFlag = 0;
    m_eType = enumXFFrameTextbox;
}
XFFrame::XFFrame(sal_Bool isTextBox):m_isTextBox(isTextBox)
{
    m_eAnchor = enumXFAnchorPara;
    m_nAnchorPage = 0;
    m_nZIndex = ZINDEX_MIN;
    //give it a default name:
    m_strName = XFGlobal::GenFrameName();
    m_fMinHeight = 0;
    m_fMaxHeight = 0;
    m_nFlag = 0;
    m_eType = enumXFFrameTextbox;
}

XFFrame::~XFFrame()
{
}

void    XFFrame::Add(IXFContent *pContent)
{
    if (!pContent)
        return;
    XFContentContainer::Add(pContent);
    if( pContent->GetContentType() == enumXFContentFrame )
    {
        XFFrame *pFrame = (XFFrame*)pContent;
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

enumXFFrameType XFFrame::GetFrameType()
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
        pAttrList->AddAttribute( "draw:style-name", GetStyleName() );

    assert(!m_strName.isEmpty());    //name should not be null.
    if( !m_strName.isEmpty() && m_isTextBox == sal_False)
        pAttrList->AddAttribute( "draw:name", m_strName );
    //anchor type:
    switch( m_eAnchor )
    {
    case enumXFAnchorPara:
        pAttrList->AddAttribute( "text:anchor-type", "paragraph" );
        break;
    case enumXFAnchorPage:
        pAttrList->AddAttribute( "text:anchor-type", "page" );
        pAttrList->AddAttribute( "text:anchor-page-number", OUString::number(m_nAnchorPage) );
        break;
    case enumXFAnchorChar:
        pAttrList->AddAttribute( "text:anchor-type", "char" );
        break;
    case enumXFAnchorAsChar:
        pAttrList->AddAttribute( "text:anchor-type", "as-char" );
        break;
    case enumXFAnchorFrame:
        pAttrList->AddAttribute( "text:anchor-type", "frame" );
        break;
    default:
        break;
    }
    pAttrList->AddAttribute( "svg:x", OUString::number(m_aRect.GetX()) + "cm" );
    pAttrList->AddAttribute( "svg:y", OUString::number(m_aRect.GetY()) + "cm" );
    pAttrList->AddAttribute( "svg:width", OUString::number(m_aRect.GetWidth()) + "cm" );
    if( m_nFlag& XFFRAME_FLAG_MINHEIGHT )
    {
        pAttrList->AddAttribute( "fo:min-height", OUString::number(m_fMinHeight) + "cm" );
        if( m_nFlag&XFFRAME_FLAG_MAXHEIGHT )
            pAttrList->AddAttribute( "fo:max-height", OUString::number(m_fMaxHeight) + "cm" );
    }
    else
        pAttrList->AddAttribute( "svg:height", OUString::number(m_aRect.GetHeight()) + "cm" );

    pAttrList->AddAttribute( "draw:z-index", OUString::number(m_nZIndex) );

    if( !m_strNextLink.isEmpty() )
        pAttrList->AddAttribute( "draw:chain-next-name", m_strNextLink );

    pStrm->StartElement( "draw:text-box" );
}

void    XFFrame::EndFrame(IXFStream *pStrm)
{
    pStrm->EndElement( "draw:text-box" );
}

void    XFFrame::AdjustZIndex()
{
    for( int i=0; i<GetCount(); i++ )
    {
        IXFContent *pContent = GetContent(i);
        if( pContent )
        {
            if( pContent->GetContentType() == enumXFContentFrame )
            {
                XFFrame *pFrame = (XFFrame*)pContent;
                pFrame->m_nZIndex = m_nZIndex + 1;
                pFrame->AdjustZIndex();
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
