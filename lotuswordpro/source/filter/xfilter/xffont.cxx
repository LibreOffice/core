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
 * Font object to serial to xml filter.
 ************************************************************************/
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xffont.hxx>
#include <xfilter/xfutil.hxx>

XFFont::XFFont()
    : m_nFontSize(0)
    , m_nFontSizeAsia(0)
    , m_nFontSizeComplex(0)
    , m_bItalic(false)
    , m_bItalicAsia(false)
    , m_bItalicComplex(false)
    , m_bBold(false)
    , m_bBoldAsia(false)
    , m_bBoldComplex(false)
    , m_eUnderline(enumXFUnderlineNone)
    , m_eCrossout(enumXFCrossoutNone)
    , m_eTransform(enumXFTransformNone)
    , m_bWordByWord(false)
    , m_nPosition(33)
    , m_nScale(58)
    , m_nFlag(0)
    , m_bTransparent(false)
{
}
/*
    The Following variable are to  be compared:
        OUString   m_strFontName;
        OUString   m_strFontNameAsia;
        OUString   m_strFontNameComplex;
        sal_Int16       m_nFontSize;
        sal_Int16       m_nFontSizeAsia;
        sal_Int16       m_nFontSizeComplex;
        sal_Bool        m_bItalic;
        sal_Bool        m_bItalicAsia;
        sal_Bool        m_bItalicComplex;
        sal_Bool        m_bBold;
        sal_Bool        m_bBoldAsia;
        sal_Bool        m_bBoldComplex;
        sal_Int16       m_nUnderline;
        sal_uInt32      m_nUnderlineColor;

        sal_uInt32      m_nFlag;
*/
bool operator==(XFFont const & f1, XFFont const & f2)
{
    //The most possible entry be first:
    if( f1.m_nFlag != f2.m_nFlag )
        return false;

    if( f1.m_strFontName != f2.m_strFontName ||
        f1.m_strFontNameAsia != f2.m_strFontNameAsia ||
        f1.m_strFontNameComplex != f2.m_strFontNameComplex
        )
        return false;

    if( f1.m_nFontSize != f2.m_nFontSize ||
        f1.m_nFontSizeAsia != f2.m_nFontSizeAsia ||
        f1.m_nFontSizeComplex != f2.m_nFontSizeComplex
        )
        return false;

    if( f1.m_bItalic != f2.m_bItalic ||
        f1.m_bItalicAsia != f2.m_bItalicAsia ||
        f1.m_bItalicComplex != f2.m_bItalicComplex
        )
        return false;

    if( f1.m_bBold != f2.m_bBold ||
        f1.m_bBoldAsia != f2.m_bBoldAsia ||
        f1.m_bBoldComplex != f2.m_bBoldComplex
        )
        return false;

    if( f1.m_nFlag&XFFONT_FLAG_UNDERLINE )
    {
        if( f1.m_eUnderline != f2.m_eUnderline )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_UNDERLINECOLOR )
    {
        if( f1.m_aUnderlineColor != f2.m_aUnderlineColor )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_CROSSOUT )
    {
        if( f1.m_eCrossout != f2.m_eCrossout )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_TRANSFORM )
    {
        if( f1.m_eTransform != f2.m_eTransform )
            return false;
    }

    if( f1.m_bWordByWord != f2.m_bWordByWord )
        return false;

    if( f1.m_nFlag&XFFONT_FLAG_POSITION )
    {
        if( f1.m_nPosition != f2.m_nPosition )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_SCALE )
    {
        if( f1.m_nScale != f2.m_nScale )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_COLOR )
    {
        if( f1.m_aColor != f2.m_aColor )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_BGCOLOR )
    {
        if( f1.m_aBackColor != f2.m_aBackColor )
            return false;
    }

    return true;
}

bool operator!=(XFFont const & f1, XFFont const & f2)
{
    return !(f1==f2);
}

void XFFont::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( m_nFlag & XFFONT_FLAG_NAME )
    {
        pAttrList->AddAttribute(u"style:font-name"_ustr,m_strFontName );
    }

    if( m_nFlag & XFFONT_FLAG_NAME_ASIA )
    {
        pAttrList->AddAttribute(u"style:font-name-asian"_ustr,m_strFontNameAsia );
    }

    if( m_nFlag & XFFONT_FLAG_NAME_COMPLEX )
    {
        pAttrList->AddAttribute(u"style:font-name-complex"_ustr, m_strFontNameComplex);
    }

    //font size:
    if( (m_nFlag & XFFONT_FLAG_SIZE) && m_nFontSize != 0 )
    {
        OUString strSize = OUString::number(m_nFontSize) + "pt";
        pAttrList->AddAttribute(u"fo:font-size"_ustr,strSize);
    }
    if( (m_nFlag & XFFONT_FLAG_SIZE_ASIA) && m_nFontSizeAsia )
    {
        OUString strSize = OUString::number(m_nFontSizeAsia) + "pt";
        pAttrList->AddAttribute(u"style:font-size-asian"_ustr,strSize);
    }
    if( (m_nFlag & XFFONT_FLAG_SIZE_COMPLEX) && m_nFontSizeComplex )
    {
        OUString strSize = OUString::number(m_nFontSizeComplex) + "pt";
        pAttrList->AddAttribute(u"style:font-size-complex"_ustr,strSize);
    }

    //italic flag:
    if( ( m_nFlag & XFFONT_FLAG_ITALIC) && m_bItalic )
    {
        pAttrList->AddAttribute(u"fo:font-style"_ustr, u"italic"_ustr);
    }
    if( (m_nFlag & XFFONT_FLAG_ITALIC_ASIA) && m_bItalicAsia )
    {
        pAttrList->AddAttribute(u"style:font-style-asian"_ustr, u"italic"_ustr);
    }
    if( (m_nFlag & XFFONT_FLAG_ITALIC_COMPLEX) && m_bItalicComplex )
    {
        pAttrList->AddAttribute(u"style:font-style-complex"_ustr, u"italic"_ustr);
    }

    //Bold flag:
    if( (m_nFlag & XFFONT_FLAG_BOLD) && m_bBold )
    {
        pAttrList->AddAttribute(u"fo:font-weight"_ustr, u"bold"_ustr);
    }
    if( (m_nFlag & XFFONT_FLAG_BOLD_ASIA) && m_bBoldAsia )
    {
        pAttrList->AddAttribute(u"style:font-weight-asian"_ustr, u"bold"_ustr);
    }
    if( (m_nFlag & XFFONT_FLAG_BOLD_ASIA) && m_bBoldComplex )
    {
        pAttrList->AddAttribute(u"style:font-weight-complex"_ustr, u"bold"_ustr);
    }
    //underline:
    if( (m_nFlag & XFFONT_FLAG_UNDERLINE) && m_eUnderline )
    {
        pAttrList->AddAttribute(u"style:text-underline"_ustr, GetUnderlineName(m_eUnderline) );
        if( m_nFlag & XFFONT_FLAG_UNDERLINECOLOR )
        {
            pAttrList->AddAttribute( u"style:text-underline-color"_ustr, m_aUnderlineColor.ToString() );
        }
        else
            pAttrList->AddAttribute( u"style:text-underline-color"_ustr, u"font-color"_ustr );
    }

    //enumCrossoutType  m_eCrossout;
    if( (m_nFlag & XFFONT_FLAG_CROSSOUT) && m_eCrossout )
    {
        pAttrList->AddAttribute(u"style:text-crossing-out"_ustr, GetCrossoutName(m_eCrossout) );
    }

    if( m_nFlag & XFFONT_FLAG_UNDERLINE || m_nFlag & XFFONT_FLAG_CROSSOUT )
    {
        if( m_bWordByWord )
            pAttrList->AddAttribute(u"fo:score-spaces"_ustr, u"false"_ustr );
        else
            pAttrList->AddAttribute(u"fo:score-spaces"_ustr, u"true"_ustr );
    }

    if( (m_nFlag & XFFONT_FLAG_TRANSFORM) && m_eTransform )
    {
        //enumTransformSmallCap is different:
        if( m_eTransform == enumXFTransformSmallCaps )
            pAttrList->AddAttribute(u"fo:font-variant"_ustr, GetTransformName(m_eTransform) );
        else
            pAttrList->AddAttribute(u"fo:text-transform"_ustr, GetTransformName(m_eTransform) );
    }

    //position & scale:
    if( ((m_nFlag & XFFONT_FLAG_SCALE) && m_nScale>0 ) ||
        ((m_nFlag & XFFONT_FLAG_POSITION) && m_nPosition != 0)
        )
    {
        OUString tmp = OUString::number(m_nPosition) + "% "
                        + OUString::number(m_nScale) + "%";
        pAttrList->AddAttribute(u"style:text-position"_ustr, tmp );
    }

    //Color:
    if( m_nFlag & XFFONT_FLAG_COLOR )
    {
        pAttrList->AddAttribute( u"fo:color"_ustr, m_aColor.ToString() );
    }

    if( m_nFlag & XFFONT_FLAG_BGCOLOR )
    {
        if (m_bTransparent)
            pAttrList->AddAttribute( u"style:text-background-color"_ustr, u"transparent"_ustr);
        else
            pAttrList->AddAttribute( u"style:text-background-color"_ustr, m_aBackColor.ToString() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
