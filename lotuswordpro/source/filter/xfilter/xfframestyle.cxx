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
 * Frame style include position,size,rotation and so on.
 ************************************************************************/
#include <xfilter/xfframestyle.hxx>
#include <xfilter/xfcolumns.hxx>
#include <xfilter/xfborders.hxx>
#include <xfilter/xfshadow.hxx>
#include <xfilter/xfbgimage.hxx>

XFFrameStyle::XFFrameStyle()
    : m_eWrap(enumXFWrapNone)
    , m_bProtectContent(false)
    , m_bProtectSize(false)
    , m_bProtectPos(false)
    , m_nTransparency(0)
    , m_eTextDir(enumXFTextDirNone)
    , m_eXPos(enumXFFrameXPosCenter)
    , m_eXRel(enumXFFrameXRelParaContent)
    , m_eYPos(enumXFFrameYPosTop)
    , m_eYRel(enumXFFrameYRelPage)
{}

XFFrameStyle::~XFFrameStyle()
{
}

void    XFFrameStyle::SetBorders(std::unique_ptr<XFBorders> pBorders)
{
    m_pBorders = std::move(pBorders);
}

void    XFFrameStyle::SetColumns(XFColumns *pColumns)
{
    m_pColumns.reset(pColumns);
}

void    XFFrameStyle::SetShadow(XFShadow *pShadow)
{
    m_pShadow.reset(pShadow);
}

void    XFFrameStyle::SetBackImage(std::unique_ptr<XFBGImage>& rImage)
{
    m_pBGImage = std::move(rImage);
}

enumXFStyle XFFrameStyle::GetStyleFamily()
{
    return enumXFStyleGraphics;
}

void    XFFrameStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( "style:name", GetStyleName() );
    if( GetParentStyleName().getLength() > 0 )
        pAttrList->AddAttribute("style:parent-style-name",GetParentStyleName());
    pAttrList->AddAttribute( "style:family", "graphics" );
    //parent style name ignore now.
    pStrm->StartElement( "style:style" );

    m_aMargins.ToXml(pStrm);

    pAttrList->Clear();

    pAttrList->AddAttribute( "style:run-through", "foreground" );

    if( m_eWrap == enumXFWrapNone )
        pAttrList->AddAttribute( "style:wrap", "none" );
    else if( m_eWrap == enumXFWrapLeft )
        pAttrList->AddAttribute( "style:wrap", "left" );
    else if( m_eWrap == enumXFWrapRight )
        pAttrList->AddAttribute( "style:wrap", "right" );
    else if( m_eWrap == enumXFWrapParallel )
        pAttrList->AddAttribute( "style:wrap", "parallel" );
    else if( m_eWrap == enumXFWrapRunThrough )
        pAttrList->AddAttribute( "style:wrap", "run-through" );
    else if( m_eWrap == enumXFWrapBest )
        pAttrList->AddAttribute( "style:wrap", "dynamic" );
    //}
    //background
    if( m_aBackColor.IsValid() )
    {
        pAttrList->AddAttribute( "fo:background-color", m_aBackColor.ToString() );
        pAttrList->AddAttribute( "style:background-transparency", OUString::number(static_cast<sal_Int32>(m_nTransparency)) + "%");
    }

    //pad
    m_aPad.ToXml(pStrm);
    //margin:
    m_aMargins.ToXml(pStrm);
    //border
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);
    else
        pAttrList->AddAttribute( "fo:border", "none" );
    //shadow
    if( m_pShadow )
        m_pShadow->ToXml(pStrm);
    //print
    pAttrList->AddAttribute( "style:print-content", "true" );
    //text directory
    if( m_eTextDir != enumXFTextDirNone )
        pAttrList->AddAttribute( "style:writing-mode", GetTextDirName(m_eTextDir) );
    //protect:
    if( m_bProtectContent || m_bProtectSize || m_bProtectPos )
    {
        OUString protect;
        if( m_bProtectContent )
            protect += "content";
        if( m_bProtectSize )
        {
            if( protect.getLength()>0 )
                protect += " ";
            protect += "size";
        }
        if( m_bProtectPos )
        {
            if( protect.getLength()>0 )
                protect += " ";
            protect += "position";
        }
        pAttrList->AddAttribute( "style:protect", protect );
    }
    //vertical pos and horizon pos:
    pAttrList->AddAttribute( "style:vertical-pos", GetFrameYPos(m_eYPos) );
    pAttrList->AddAttribute( "style:vertical-rel", GetFrameYRel(m_eYRel) );
    pAttrList->AddAttribute( "style:horizontal-pos", GetFrameXPos(m_eXPos) );
    pAttrList->AddAttribute( "style:horizontal-rel", GetFrameXRel(m_eXRel) );

    pStrm->StartElement( "style:properties" );
    if( m_pColumns )
        m_pColumns->ToXml(pStrm);
    if( m_pBGImage )
        m_pBGImage->ToXml(pStrm);
    pStrm->EndElement( "style:properties" );

    pStrm->EndElement( "style:style" );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
