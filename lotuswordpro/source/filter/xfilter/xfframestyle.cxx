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
/*************************************************************************
 * Change History
 * 2005-01-17 create this file.
 ************************************************************************/
#include	"xfframestyle.hxx"
#include	"xfcolumns.hxx"
#include	"xfborders.hxx"
#include	"xfshadow.hxx"
#include	"xfbgimage.hxx"

XFFrameStyle::XFFrameStyle()
{
    m_eWrap = enumXFWrapNone;
    m_nWrapLines = 0;	//not limited.
    m_pBorders = NULL;
    m_pColumns = NULL;
    m_pShadow = NULL;
    m_pBGImage = NULL;

    m_bProtectContent = sal_False;
    m_bProtectSize = sal_False;
    m_bProtectPos = sal_False;
    m_bEditable = sal_False;
    m_bPrintable = sal_True;
    m_bBackground = sal_False;
    m_nTransparency = 0;

    m_eTextDir = enumXFTextDirNone;
    m_eXPos = enumXFFrameXPosCenter;
    m_eXRel = enumXFFrameXRelParaContent;
    m_eYPos = enumXFFrameYPosTop;
    m_eYRel = enumXFFrameYRelPage;
}

XFFrameStyle::~XFFrameStyle()
{
    if( m_pBorders )
        delete m_pBorders;
    if( m_pColumns )
        delete m_pColumns;
    if( m_pShadow )
        delete m_pShadow;
    if( m_pBGImage )
        delete m_pBGImage;
}

void	XFFrameStyle::SetBorders(XFBorders *pBorders)
{
    if( m_pBorders )
        delete m_pBorders;
    m_pBorders = pBorders;
}

void	XFFrameStyle::SetColumns(XFColumns *pColumns)
{
    if( m_pColumns )
        delete m_pColumns;
    m_pColumns = pColumns;
}

void	XFFrameStyle::SetShadow(XFShadow *pShadow)
{
    if( m_pShadow )
        delete m_pShadow;
    m_pShadow = pShadow;
}

void	XFFrameStyle::SetBackImage(XFBGImage *image)
{
    if( m_pBGImage )
        delete m_pBGImage;
    m_pBGImage = image;
}

enumXFStyle	XFFrameStyle::GetStyleFamily()
{
    return enumXFStyleGraphics;
}

void	XFFrameStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    if( GetParentStyleName().getLength() > 0 )
        pAttrList->AddAttribute(A2OUSTR("style:parent-style-name"),GetParentStyleName());
    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("graphics") );
    //parent style name ignore now.
    pStrm->StartElement( A2OUSTR("style:style") );

    m_aMargins.ToXml(pStrm);

    pAttrList->Clear();

    /*if( m_eWrap == enumXFWrapBackground )
    {
        pAttrList->AddAttribute( A2OUSTR("style:run-through"), A2OUSTR("background") );
    }
    else
    {*/
    if( m_bBackground)
        pAttrList->AddAttribute( A2OUSTR("style:run-through"), A2OUSTR("background") );
    else
        pAttrList->AddAttribute( A2OUSTR("style:run-through"), A2OUSTR("foreground") );

    if( m_eWrap == enumXFWrapNone )
        pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("none") );
    else if( m_eWrap == enumXFWrapLeft )
        pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("left") );
    else if( m_eWrap == enumXFWrapRight )
        pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("right") );
    else if( m_eWrap == enumXFWrapParallel )
        pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("parallel") );
    else if( m_eWrap == enumXFWrapRunThrough )
        pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("run-through") );
    else if( m_eWrap == enumXFWrapBest )
        pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("dynamic") );
    //}
    //background
    if( m_aBackColor.IsValid() )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:background-color"), m_aBackColor.ToString() );
        pAttrList->AddAttribute( A2OUSTR("style:background-transparency"), Int16ToOUString(m_nTransparency) + A2OUSTR("%"));
    }

    //pad
    m_aPad.ToXml(pStrm);
    //margin:
    m_aMargins.ToXml(pStrm);
    //border
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);
    else
        pAttrList->AddAttribute( A2OUSTR("fo:border"), A2OUSTR("none") );
    //shadow
    if( m_pShadow )
        m_pShadow->ToXml(pStrm);
    //print
    if( m_bPrintable )
        pAttrList->AddAttribute( A2OUSTR("style:print-content"), A2OUSTR("true") );
    //text directory
    if( m_eTextDir != enumXFTextDirNone )
        pAttrList->AddAttribute( A2OUSTR("style:writing-mode"), GetTextDirName(m_eTextDir) );
    //protect:
    if( m_bProtectContent || m_bProtectSize || m_bProtectPos )
    {
        rtl::OUString protect;
        if( m_bProtectContent )
            protect += A2OUSTR("content");
        if( m_bProtectSize )
        {
            if( protect.getLength()>0 )
                protect += A2OUSTR(" ");
            protect += A2OUSTR("size");
        }
        if( m_bProtectPos )
        {
            if( protect.getLength()>0 )
                protect += A2OUSTR(" ");
            protect += A2OUSTR("position");
        }
        pAttrList->AddAttribute( A2OUSTR("style:protect"), protect );
    }
    //vertical pos and horizon pos:
    pAttrList->AddAttribute( A2OUSTR("style:vertical-pos"), GetFrameYPos(m_eYPos) );
    pAttrList->AddAttribute( A2OUSTR("style:vertical-rel"), GetFrameYRel(m_eYRel) );
    pAttrList->AddAttribute( A2OUSTR("style:horizontal-pos"), GetFrameXPos(m_eXPos) );
    pAttrList->AddAttribute( A2OUSTR("style:horizontal-rel"), GetFrameXRel(m_eXRel) );

    //
    pStrm->StartElement( A2OUSTR("style:properties") );
    if( m_pColumns )
        m_pColumns->ToXml(pStrm);
    if( m_pBGImage )
        m_pBGImage->ToXml(pStrm);
    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("style:style") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
