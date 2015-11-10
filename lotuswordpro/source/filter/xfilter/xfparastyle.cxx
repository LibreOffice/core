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
 * Styles for paragraph.
 * Styles for paragraph may include many style,include font,indent,margin,
 * shadow,line height,and so on.
 ************************************************************************/
#include "xfparastyle.hxx"
#include "xffont.hxx"
#include "xfborders.hxx"
#include "xftabstyle.hxx"
#include "xfbgimage.hxx"

XFParaStyle::XFParaStyle()
    : m_eAlignType(enumXFAlignNone)
    , m_eLastLineAlign(enumXFAlignNone)
    , m_bJustSingleWord(false)
    , m_bKeepWithNext(false)
    , m_fTextIndent(0)
    , m_pBorders(nullptr)
    , m_pBGImage(nullptr)
    , m_nPageNumber(0)
    , m_bNumberLines(true)
    , m_nLineNumberRestart(0)
    , m_nFlag(0)
    , m_bNumberRight(false)
{
}


XFParaStyle::XFParaStyle(const XFParaStyle& other)
    : XFStyle(other)
    , m_eAlignType(other.m_eAlignType)
    , m_eLastLineAlign(other.m_eLastLineAlign)
    , m_bJustSingleWord(other.m_bJustSingleWord)
    , m_bKeepWithNext(other.m_bKeepWithNext)
    , m_fTextIndent(other.m_fTextIndent)
    , m_aBackColor(other.m_aBackColor)
    , m_aMargin(other.m_aMargin)
    , m_aPadding(other.m_aPadding)
    , m_pFont(other.m_pFont)
    , m_aShadow(other.m_aShadow)
    , m_aDropcap(other.m_aDropcap)
    , m_aLineHeight(other.m_aLineHeight)
    , m_aBreaks(other.m_aBreaks)
    , m_nPageNumber(other.m_nPageNumber)
    , m_bNumberLines(other.m_bNumberLines)
    , m_nLineNumberRestart(other.m_nLineNumberRestart)
    , m_nFlag(other.m_nFlag)
    , m_bNumberRight(other.m_bNumberRight)
{
    m_strParentStyleName = other.m_strParentStyleName;

    if( other.m_pBorders )
        m_pBorders = new XFBorders(*other.m_pBorders);
    else
        m_pBorders = nullptr;

    if( other.m_pBGImage )
        m_pBGImage = new XFBGImage(*other.m_pBGImage);
    else
        m_pBGImage = nullptr;

    for (size_t i = 0; i < other.m_aTabs.GetCount(); ++i)
    {
        const IXFStyle* pStyle = other.m_aTabs.Item(i);
        if( pStyle )
        {
            const XFTabStyle* pTabStyle = dynamic_cast<const XFTabStyle*>(pStyle);
            if( pTabStyle )
            {
                XFTabStyle *pCopyStyle = new XFTabStyle(*pTabStyle);
                m_aTabs.AddStyle(pCopyStyle);
            }
        }
    }
}

XFParaStyle& XFParaStyle::operator=(const XFParaStyle& other)
{
    // Check for self-assignment
    if (this != &other)
    {
        // first , clean member
        delete m_pBGImage;
        m_aTabs.Reset();

        m_strParentStyleName = other.m_strParentStyleName;
        m_nFlag = other.m_nFlag;
        m_eAlignType = other.m_eAlignType;
        m_eLastLineAlign = other.m_eLastLineAlign;
        m_bJustSingleWord = other.m_bJustSingleWord;
        m_bKeepWithNext = other.m_bKeepWithNext;
        m_fTextIndent = other.m_fTextIndent;
        m_bNumberLines = other.m_bNumberLines;
        m_nLineNumberRestart = other.m_nLineNumberRestart;
        m_bNumberRight = other.m_bNumberRight;

        m_pFont = other.m_pFont;

        if( other.m_pBorders )
            m_pBorders = new XFBorders(*other.m_pBorders);
        else
            m_pBorders = nullptr;
        m_aBackColor = other.m_aBackColor;
        if( other.m_pBGImage )
            m_pBGImage = new XFBGImage(*other.m_pBGImage);
        else
            m_pBGImage = nullptr;

        m_aShadow = other.m_aShadow;
        m_aMargin = other.m_aMargin;
        m_aDropcap = other.m_aDropcap;
        m_aLineHeight = other.m_aLineHeight;
        m_aPadding = other.m_aPadding;
        m_aBreaks = other.m_aBreaks;

        for (size_t i=0; i<other.m_aTabs.GetCount(); ++i)
        {
            const IXFStyle *pStyle = other.m_aTabs.Item(i);
            if( pStyle )
            {
                const XFTabStyle *pTabStyle = dynamic_cast<const XFTabStyle*>(pStyle);
                if( pTabStyle )
                {
                    XFTabStyle *pCopyStyle = new XFTabStyle(*pTabStyle);
                    m_aTabs.AddStyle(pCopyStyle);
                }
            }
        }
    }
    return *this;
}

XFParaStyle::~XFParaStyle()
{
    delete m_pBorders;
    delete m_pBGImage;
}

enumXFStyle XFParaStyle::GetStyleFamily()
{
    return enumXFStylePara;
}

void    XFParaStyle::SetFont(rtl::Reference<XFFont> const & pFont)
{
    m_pFont = pFont;
}

void    XFParaStyle::SetIndent(double indent )
{
    m_fTextIndent = indent;
}

void    XFParaStyle::SetMargins(double left, double right, double top, double bottom)
{
    if( left != -1 )
        m_aMargin.SetLeft(left);
    if( right != -1 )
        m_aMargin.SetRight(right);
    if( top != -1 )
        m_aMargin.SetTop(top);
    if( bottom != -1 )
        m_aMargin.SetBottom( bottom );
}

void    XFParaStyle::SetShadow(enumXFShadowPos pos, double offset, XFColor& color)
{
    m_aShadow.SetPosition(pos);
    m_aShadow.SetOffset(offset);
    m_aShadow.SetColor(color);
}

void    XFParaStyle::SetBackColor(XFColor& color)
{
    m_aBackColor = color;
    m_nFlag |= XFPARA_FLAG_BACKCOLOR;
}

void    XFParaStyle::SetBackImage(XFBGImage *image)
{
    delete m_pBGImage;
    m_pBGImage = image;
}

void    XFParaStyle::SetBorders(XFBorders *pBorders)
{
    delete m_pBorders;
    m_pBorders = pBorders;
}

void    XFParaStyle::SetDropCap(sal_Int16 nLength,
                       sal_Int16 nLines,
                       double fDistance
                       )
{
    assert(nLength>=1);
    assert(nLines>=2);
    assert(fDistance>=0);

    m_aDropcap.SetCharCount(nLength);
    m_aDropcap.SetLines(nLines);
    m_aDropcap.SetDistance(fDistance);
}

void    XFParaStyle::SetLineHeight(enumLHType type, double value)
{
    if( type == enumLHNone )
    {
        return;
    }
    switch(type)
    {
    case enumLHHeight:
        m_aLineHeight.SetHeight(value)  ;
        break;
    case enumLHLeast:
        m_aLineHeight.SetLeastHeight(value);
        break;
    case enumLHPercent: //perhaps i should redesign the interface here,ohm..
        m_aLineHeight.SetPercent((sal_Int32)value);
        break;
    case enumLHSpace:
        m_aLineHeight.SetSpace(value*0.5666);   //don't known why,just suspect.
        break;
    default:
        break;
    }
}

void    XFParaStyle::AddTabStyle(enumXFTab type, double len, sal_Unicode leader, sal_Unicode delimiter)
{
    XFTabStyle  *tab = new XFTabStyle();
    tab->SetTabType(type);
    tab->SetLength(len);
    tab->SetLeaderChar(leader);
    tab->SetDelimiter(delimiter);
    m_aTabs.AddStyle(tab);
}

/**
 *Affirm whether two XFParaStyle objects are equal.
 */
bool    XFParaStyle::Equal(IXFStyle *pStyle)
{
    if( this == pStyle )
        return true;
    if( !pStyle || pStyle->GetStyleFamily() != enumXFStylePara )
        return false;

    XFParaStyle *pOther = static_cast<XFParaStyle*>(pStyle);

    if( m_nFlag != pOther->m_nFlag )
        return false;
    if( m_strParentStyleName != pOther->m_strParentStyleName )
        return false;
    if( m_strMasterPage != pOther->m_strMasterPage )
        return false;
    if( m_fTextIndent != pOther->m_fTextIndent )
        return false;
    if( m_bJustSingleWord != pOther->m_bJustSingleWord )
        return false;
    if( m_bKeepWithNext != pOther->m_bKeepWithNext )
        return false;
    //line number:
    if( m_bNumberLines != pOther->m_bNumberLines )
        return false;
    if( m_nLineNumberRestart != pOther->m_nLineNumberRestart )
        return false;
    //align:
    if( m_eAlignType != pOther->m_eAlignType )
        return false;
    //last line align:
    if( m_eLastLineAlign != pOther->m_eLastLineAlign )
        return false;

    //shadow:
    if( m_aShadow != pOther->m_aShadow )
        return false;
    //margin:
    if( m_aMargin != pOther->m_aMargin )
        return false;

    if( m_aPadding != pOther->m_aPadding )
        return false;

    //dropcap:
    if( m_aDropcap != pOther->m_aDropcap )
        return false;
    //line height:
    if( m_aLineHeight != pOther->m_aLineHeight )
        return false;
    //breaks:
    if( m_aBreaks != pOther->m_aBreaks )
        return false;
    if( m_nPageNumber != pOther->m_nPageNumber )
        return false;
    if( m_aTabs != pOther->m_aTabs )
        return false;

    //font:
    if( m_pFont.is() )
    {
        if( !pOther->m_pFont.is() )
            return false;
        if(*m_pFont != *pOther->m_pFont )
            return false;
    }
    else if( pOther->m_pFont.is() )
        return false;

    //border:
    if( m_pBorders )
    {
        if( !pOther->m_pBorders )
            return false;
        if( *m_pBorders != *pOther->m_pBorders )
            return false;
    }
    else if( pOther->m_pBorders )
        return false;

    if( m_pBGImage )
    {
        if( !pOther->m_pBGImage )
            return false;
        if( *m_pBGImage != *pOther->m_pBGImage )
            return false;
    }
    else if( pOther->m_pBGImage )
        return false;//add end

    //number right
    if (    m_bNumberRight  != pOther->m_bNumberRight)
        return false;

    return true;
}

void    XFParaStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    OUString style = GetStyleName();

    pAttrList->Clear();
    if( !style.isEmpty() )
        pAttrList->AddAttribute("style:name",GetStyleName());
    pAttrList->AddAttribute("style:family", "paragraph");
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute("style:parent-style-name",GetParentStyleName());

    if( !m_strMasterPage.isEmpty() )
        pAttrList->AddAttribute("style:master-page-name",m_strMasterPage);
    pStrm->StartElement("style:style");

    //Paragraph properties:
    pAttrList->Clear();

    //text indent:
    if( m_fTextIndent )
    {
        pAttrList->AddAttribute("fo:text-indent", OUString::number(m_fTextIndent) + "cm" );
    }
    //padding:
    m_aPadding.ToXml(pStrm);
    //margin:
    m_aMargin.ToXml(pStrm);

    //text align:
    if( m_eAlignType != enumXFAlignNone )
    {
        pAttrList->AddAttribute("fo:text-align", GetAlignName(m_eAlignType) );
    }
    //last line align:
    if( m_eLastLineAlign != enumXFAlignNone )
    {
        pAttrList->AddAttribute("fo:fo:text-align-last", GetAlignName(m_eLastLineAlign) );
        if( m_bJustSingleWord )
            pAttrList->AddAttribute("style:justify-single-word", "true" );
    }
    //line number:
    if( m_bNumberLines )
    {
        pAttrList->AddAttribute( "text:number-lines", "true" );
        pAttrList->AddAttribute( "text:line-number", OUString::number(m_nLineNumberRestart) );
    }
    else
    {
        pAttrList->AddAttribute( "text:number-lines", "false" );
        assert(m_nLineNumberRestart>0);
        pAttrList->AddAttribute( "text:line-number", "0" );
    }

    //shadow:
    m_aShadow.ToXml(pStrm);
    //borders:
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);
    //line height:
    m_aLineHeight.ToXml(pStrm);

    //background color:
    if( m_nFlag&XFPARA_FLAG_BACKCOLOR && m_aBackColor.IsValid() )
    {
        pAttrList->AddAttribute("fo:background-color", m_aBackColor.ToString() );
    }
    //Font properties:
    if( m_pFont.is() )
        m_pFont->ToXml(pStrm);

    //page number:
    if( m_nPageNumber )
        pAttrList->AddAttribute("fo:page-number", OUString::number(m_nPageNumber) );
    //page breaks:
    m_aBreaks.ToXml(pStrm);

    if( m_bKeepWithNext )
        pAttrList->AddAttribute("fo:fo:keep-with-next", "true" );

    pStrm->StartElement("style:properties");

    //dropcap:
    m_aDropcap.ToXml(pStrm);

    //tabs:
    if( m_aTabs.GetCount() > 0 )
    {
        pAttrList->Clear();
        pStrm->StartElement( "style:tab-stops" );
        m_aTabs.ToXml(pStrm);
        pStrm->EndElement( "style:tab-stops" );
    }

    //background color:
    if( m_pBGImage )
        m_pBGImage->ToXml(pStrm);

    pStrm->EndElement("style:properties");

    pStrm->EndElement("style:style");
}

XFDefaultParaStyle::XFDefaultParaStyle()
{
    m_fTabDistance = 1.28;
}

enumXFStyle XFDefaultParaStyle::GetStyleFamily()
{
    return enumXFStyleDefaultPara;
}

void XFDefaultParaStyle::ToXml(IXFStream * pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pAttrList->AddAttribute("style:family", "paragraph");
    pStrm->StartElement("style:default-style");

    //Paragraph properties:
    pAttrList->Clear();

    pAttrList->AddAttribute("style:tab-stop-distance", OUString::number(m_fTabDistance) + "cm" );

    pStrm->StartElement("style:properties");
    pStrm->EndElement("style:properties");
    pStrm->EndElement("style:default-style");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
