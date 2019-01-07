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
 * Page master used bye XFMasterPage.
 * It is the real object to define header and footer of pages.
 ************************************************************************/
#include <xfilter/xfpagemaster.hxx>
#include <xfilter/ixfstream.hxx>
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfborders.hxx>
#include <xfilter/xfshadow.hxx>
#include <xfilter/xfcolumns.hxx>
#include <xfilter/xfheaderstyle.hxx>
#include <xfilter/xffooterstyle.hxx>
#include <xfilter/xfbgimage.hxx>

XFPageMaster::XFPageMaster() : m_fPageWidth(0), m_fPageHeight(0), m_eUsage(enumXFPageUsageNone),
m_eTextDir(enumXFTextDirNone),
m_eSepAlign(enumXFAlignNone), m_fSepWidth(0), m_aSepColor(0), m_fSepSpaceAbove(0),
m_fSepSpaceBelow(0), m_nSepLengthPercent(0)
{
}

XFPageMaster::~XFPageMaster()
{
}

enumXFStyle XFPageMaster::GetStyleFamily()
{
    return enumXFStylePageMaster;
}

void    XFPageMaster::SetPageWidth(double width)
{
    m_fPageWidth = width;
}

void    XFPageMaster::SetPageHeight(double height)
{
    m_fPageHeight = height;
}

void    XFPageMaster::SetMargins(double left, double right,double top, double bottom)
{
    if( left != -1 )
        m_aMargin.SetLeft(left);
    if( right != -1 )
        m_aMargin.SetRight(right);
    if( top != -1 )
        m_aMargin.SetTop(top);
    if( bottom != -1 )
        m_aMargin.SetBottom(bottom);
}

void    XFPageMaster::SetBorders(std::unique_ptr<XFBorders> pBorders)
{
    m_pBorders = std::move( pBorders );
}

void    XFPageMaster::SetShadow(XFShadow *pShadow)
{
    if( pShadow == m_pShadow.get() )
        return;
    m_pShadow.reset( pShadow );
}

void    XFPageMaster::SetBackColor(XFColor color)
{
    m_aBackColor = color;
}

void    XFPageMaster::SetBackImage(std::unique_ptr<XFBGImage>& rImage)
{
    m_pBGImage = std::move(rImage);
}

void    XFPageMaster::SetColumns(XFColumns *pColumns)
{
    if( pColumns == m_pColumns.get() )
        return;
    m_pColumns.reset(pColumns);
}

 void   XFPageMaster::SetHeaderStyle(XFHeaderStyle *pHeaderStyle)
{
    if( pHeaderStyle == m_pHeaderStyle.get() )
        return;
    m_pHeaderStyle.reset( pHeaderStyle );
}

void    XFPageMaster::SetFooterStyle(XFFooterStyle *pFooterStyle)
{
    if( pFooterStyle == m_pFooterStyle.get() )
        return;
    m_pFooterStyle.reset( pFooterStyle );
}

void    XFPageMaster::SetFootNoteSeparator(
                             enumXFAlignType align,
                             double width,
                             sal_Int32 lengthPercent,
                             double spaceAbove,
                             double spaceBelow,
                             XFColor color
                             )
{
    m_eSepAlign = align;
    m_fSepWidth = width;
    m_nSepLengthPercent = lengthPercent;
    m_fSepSpaceAbove = spaceAbove;
    m_fSepSpaceBelow = spaceBelow;
    m_aSepColor = color;
}

/**
 *
    <style:page-master style:name="pm1">
        <style:properties fo:page-width="20.999cm" fo:page-height="29.699cm" style:num-format="1"
            style:print-orientation="portrait" fo:margin-top="1.249cm" fo:margin-bottom="1.249cm"
            fo:margin-left="3.175cm" fo:margin-right="3.175cm" style:writing-mode="lr-tb"
            style:layout-grid-color="#c0c0c0" style:layout-grid-lines="42"
            style:layout-grid-base-height="0.494cm" style:layout-grid-ruby-height="0.141cm"
            style:layout-grid-mode="none" style:layout-grid-ruby-below="false" style:layout-grid-print="false"
            style:layout-grid-display="false" style:footnote-max-height="0cm">
                <style:footnote-sep style:width="0.018cm" style:distance-before-sep="0.101cm"
                    style:distance-after-sep="0.101cm" style:adjustment="left" style:rel-width="25%" style:color="#000000"/>
        </style:properties>
        <style:header-style>
            <style:properties fo:min-height="1.291cm" fo:margin-bottom="0.792cm" style:dynamic-spacing="true"/>
        </style:header-style>
        <style:footer-style>
            <style:properties fo:min-height="1.291cm" fo:margin-top="0.792cm" style:dynamic-spacing="true"/>
        </style:footer-style>
    </style:page-master>

 */
void    XFPageMaster::ToXml(IXFStream *pStream)
{
    IXFAttrList *pAttrList = pStream->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute("style:name",GetStyleName());

    if( m_eUsage != enumXFPageUsageNone )
        pAttrList->AddAttribute("style:page-usage", GetPageUsageName(m_eUsage));

    pStream->StartElement( "style:page-master" );

    //style:properties
    pAttrList->Clear();
    if( m_fPageWidth != 0 )
        pAttrList->AddAttribute( "fo:page-width", OUString::number(m_fPageWidth) + "cm" );
    if( m_fPageHeight != 0 )
        pAttrList->AddAttribute( "fo:page-height", OUString::number(m_fPageHeight) + "cm" );

    m_aMargin.ToXml(pStream);

    pAttrList->AddAttribute( "style:print-orientation", "portrait" );

    if( m_pBorders )
        m_pBorders->ToXml(pStream);
    if( m_pShadow )
        pAttrList->AddAttribute( "style:shadow", m_pShadow->ToString() );

    if( m_aBackColor.IsValid() )
        pAttrList->AddAttribute( "fo:background-color", m_aBackColor.ToString() );

    //text directory
    if( m_eTextDir != enumXFTextDirNone )
        pAttrList->AddAttribute( "style:writing-mode", GetTextDirName(m_eTextDir) );

    pStream->StartElement( "style:properties" );
    if( m_pColumns )
        m_pColumns->ToXml(pStream);

    if( m_pBGImage )
        m_pBGImage->ToXml(pStream);

    if( m_eSepAlign || m_nSepLengthPercent>0 || m_fSepSpaceAbove>0 || m_fSepSpaceBelow>0 )
    {
        pAttrList->Clear();
        pAttrList->AddAttribute( "style:width", OUString::number(m_fSepWidth) + "cm" );
        pAttrList->AddAttribute( "style:distance-before-sep", OUString::number(m_fSepSpaceAbove) + "cm" );
        pAttrList->AddAttribute( "style:distance-after-sep", OUString::number(m_fSepSpaceBelow) + "cm" );
        pAttrList->AddAttribute( "style:color", m_aSepColor.ToString() );
        if( m_eSepAlign == enumXFAlignStart )
            pAttrList->AddAttribute( "style:adjustment", "left" );
        else if( m_eSepAlign == enumXFAlignCenter )
            pAttrList->AddAttribute( "style:adjustment", "center" );
        else if( m_eSepAlign == enumXFAlignEnd )
            pAttrList->AddAttribute( "style:adjustment", "right" );
        pAttrList->AddAttribute( "style:rel-width", OUString::number(m_nSepLengthPercent) + "%" );
        pStream->StartElement( "style:footnote-sep" );
        pStream->EndElement( "style:footnote-sep" );
    }

    pStream->EndElement( "style:properties" );

    //header style:
    if( m_pHeaderStyle )
        m_pHeaderStyle->ToXml(pStream);
    //footer style:
    if( m_pFooterStyle )
        m_pFooterStyle->ToXml(pStream);

    pStream->EndElement( "style:page-master" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
