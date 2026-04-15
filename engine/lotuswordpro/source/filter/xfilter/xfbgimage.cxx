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
 * Background image.
 ************************************************************************/
#include <xfilter/xfbgimage.hxx>
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfutil.hxx>
#include "xfbase64.hxx"

XFBGImage::XFBGImage()
    : m_bUserFileLink(false)
    , m_bRepeate(false)
    , m_bStretch(false)
    , m_bPosition(true)
    , m_eHoriAlign(enumXFAlignCenter)
    , m_eVertAlign(enumXFAlignCenter)
{}

void XFBGImage::SetImageData(sal_uInt8 const *buf, int len)
{
    m_strData = XFBase64::Encode(buf,len);
    m_bUserFileLink = false;
}

bool    XFBGImage::Equal(IXFStyle * /* pStyle */)
{
    return false;
}

void    XFBGImage::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    if( m_bUserFileLink )
    {
        pAttrList->AddAttribute( u"xlink:href"_ustr, m_strFileName);
    }

    pAttrList->AddAttribute( u"xlink:type"_ustr, u"simple"_ustr );
    pAttrList->AddAttribute( u"xlink:actuate"_ustr, u"onLoad"_ustr);

    if( m_bPosition )
    {
        OUString str = GetAlignName(m_eVertAlign) + " ";
        if( m_eHoriAlign == enumXFAlignStart )
            str += "left";
        else if( m_eHoriAlign == enumXFAlignCenter )
            str += "center";
        else if( m_eHoriAlign == enumXFAlignEnd )
            str += "right";

        pAttrList->AddAttribute( u"style:position"_ustr, str );
        pAttrList->AddAttribute( u"style:repeat"_ustr, u"no-repeat"_ustr );
    }
    else if( m_bRepeate )
        pAttrList->AddAttribute( u"style:repeat"_ustr, u"repeat"_ustr );
    else if( m_bStretch )
        pAttrList->AddAttribute( u"style:repeat"_ustr, u"stretch"_ustr );

    pStrm->StartElement( u"style:background-image"_ustr );

    if( !m_bUserFileLink )
    {
        pAttrList->Clear();
        pStrm->StartElement( u"office:binary-data"_ustr );
        pStrm->Characters(m_strData);
        pStrm->EndElement( u"office:binary-data"_ustr );
    }

    pStrm->EndElement( u"style:background-image"_ustr );
}

bool operator==(XFBGImage const & img1, XFBGImage const & img2)
{
    if( img1.m_bUserFileLink != img2.m_bUserFileLink )
        return false;
    if( img1.m_bUserFileLink )
    {
        if( img1.m_strFileName != img2.m_strFileName )
            return false;
    }
    else
    {
        //I'll not compare the content of the two buffer,it's time consuming.
        return false;
    }
    if( img1.m_bPosition != img2.m_bPosition )
        return false;
    if( img1.m_bRepeate != img2.m_bRepeate )
        return false;
    if( img1.m_bStretch != img2.m_bStretch )
        return false;
    if( img1.m_bPosition )
    {
        if( (img1.m_eHoriAlign != img2.m_eHoriAlign) || (img1.m_eVertAlign != img2.m_eVertAlign) )
            return false;
    }
    return true;
}

bool operator!=(XFBGImage const & img1, XFBGImage const & img2)
{
    return !(img1==img2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
