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
/*************************************************************************
 * Change History
 * 2004-12-23 create this file.
 * 2005-01-14 add scale,position,emphasize support.
 * 2005-01-24 move some tool function to xfutil.hxx
 ************************************************************************/
#include	"xffont.hxx"

XFFont::XFFont()
{
    m_nFontSize = 0;
    m_nFontSizeAsia = 0;
    m_nFontSizeComplex = 0;
    m_bItalic = sal_False;
    m_bItalicAsia = sal_False;
    m_bItalicComplex = sal_False;
    m_bBold = sal_False;
    m_bBoldAsia = sal_False;
    m_bBoldComplex = sal_False;
    m_eUnderline = enumXFUnderlineNone;
    m_eCrossout = enumXFCrossoutNone;
    m_eRelief = enumXFReliefNone;
    m_eTransform = enumXFTransformNone;
    m_eEmphasize = enumXFEmphasizeNone;
    m_bEmphasizeTop = sal_True;
    m_bOutline = sal_False;
    m_bShadow = sal_False;
    m_bBlink = sal_False;
    m_nPosition = 33;
    m_nScale = 58;
    m_fCharSpace = 0;
    m_nWidthScale = 100;
    m_bWordByWord = sal_False;
    m_nFlag = 0;
}
/*
    The Following variable are to  be compared:
        rtl::OUString	m_strFontName;
        rtl::OUString	m_strFontNameAsia;
        rtl::OUString	m_strFontNameComplex;
        sal_Int16		m_nFontSize;
        sal_Int16		m_nFontSizeAsia;
        sal_Int16		m_nFontSizeComplex;
        sal_Bool		m_bItalic;
        sal_Bool		m_bItalicAsia;
        sal_Bool		m_bItalicComplex;
        sal_Bool		m_bBold;
        sal_Bool		m_bBoldAsia;
        sal_Bool		m_bBoldComplex;
        sal_Int16		m_nUnderline;
        sal_uInt32		m_nUnderlineColor;

        sal_uInt32		m_nFlag;
*/
bool operator==(XFFont& f1, XFFont& f2)
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

    if( f1.m_nFlag&XFFONT_FLAG_RELIEF )
    {
        if( f1.m_eRelief != f2.m_eRelief )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_TRANSFORM )
    {
        if( f1.m_eTransform != f2.m_eTransform )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_EMPHASIZE )
    {
        if( f1.m_eEmphasize != f2.m_eEmphasize )
            return false;
        if( f1.m_bEmphasizeTop != f2.m_bEmphasizeTop )
            return false;
    }

    if( f1.m_bWordByWord != f2.m_bWordByWord )
        return false;

    if( f1.m_nFlag&XFFONT_FLAG_OUTLINE )
    {
        if( f1.m_bOutline != f2.m_bOutline )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_SHADOW )
    {
        if( f1.m_bShadow != f2.m_bShadow )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_BLINK )
    {
        if( f1.m_bBlink != f2.m_bBlink )
            return false;
    }

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

    if( f1.m_nFlag&XFFONT_FLAG_CHARSPACE )
    {
        if( f1.m_fCharSpace != f2.m_fCharSpace )
            return false;
    }

    if( f1.m_nFlag&XFFONT_FLAG_WIDTHSCALE )
    {
        if( f1.m_nWidthScale != f2.m_nWidthScale )
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

bool operator!=(XFFont& f1, XFFont& f2)
{
    return !(f1==f2);
}


void XFFont::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( m_nFlag & XFFONT_FLAG_NAME )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-name"),m_strFontName );
    }

    if( m_nFlag & XFFONT_FLAG_NAME_ASIA )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-name-asian"),m_strFontNameAsia );
    }

    if( m_nFlag & XFFONT_FLAG_NAME_COMPLEX )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-name-complex"), m_strFontNameComplex);
    }

    //font size:
    if( (m_nFlag & XFFONT_FLAG_SIZE) && m_nFontSize != 0 )
    {
        rtl::OUString strSize = Int32ToOUString(m_nFontSize);
        strSize += A2OUSTR("pt");
        pAttrList->AddAttribute(A2OUSTR("fo:font-size"),strSize);
    }
    if( (m_nFlag & XFFONT_FLAG_SIZE_ASIA) && m_nFontSizeAsia )
    {
        rtl::OUString strSize = Int32ToOUString(m_nFontSizeAsia);
        strSize += A2OUSTR("pt");
        pAttrList->AddAttribute(A2OUSTR("style:font-size-asian"),strSize);
    }
    if( (m_nFlag & XFFONT_FLAG_SIZE_COMPLEX) && m_nFontSizeComplex )
    {
        rtl::OUString strSize = Int32ToOUString(m_nFontSizeComplex);
        strSize += A2OUSTR("pt");
        pAttrList->AddAttribute(A2OUSTR("style:font-size-complex"),strSize);
    }

    //italic flag:
    if( ( m_nFlag & XFFONT_FLAG_ITALIC) && m_bItalic )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:font-style"), A2OUSTR("italic"));
    }
    if( (m_nFlag & XFFONT_FLAG_ITALIC_ASIA) && m_bItalicAsia )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-style-asian"), A2OUSTR("italic"));
    }
    if( (m_nFlag & XFFONT_FLAG_ITALIC_COMPLEX) && m_bItalicComplex )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-style-complex"), A2OUSTR("italic"));
    }

    //Bold flag:
    if( (m_nFlag & XFFONT_FLAG_BOLD) && m_bBold )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:font-weight"), A2OUSTR("bold"));
    }
    if( (m_nFlag & XFFONT_FLAG_BOLD_ASIA) && m_bBoldAsia )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-weight-asian"), A2OUSTR("bold"));
    }
    if( (m_nFlag & XFFONT_FLAG_BOLD_ASIA) && m_bBoldComplex )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-weight-complex"), A2OUSTR("bold"));
    }
    //underline:
    if( (m_nFlag & XFFONT_FLAG_UNDERLINE) && m_eUnderline )
    {
        pAttrList->AddAttribute(A2OUSTR("style:text-underline"), GetUnderlineName(m_eUnderline) );
        if( (m_nFlag & XFFONT_FLAG_UNDERLINECOLOR) )
        {
            pAttrList->AddAttribute( A2OUSTR("style:text-underline-color"), m_aUnderlineColor.ToString() );
        }
        else
            pAttrList->AddAttribute( A2OUSTR("style:text-underline-color"), A2OUSTR("font-color") );
    }

    //enumCrossoutType	m_eCrossout;
    if( (m_nFlag & XFFONT_FLAG_CROSSOUT) && m_eCrossout )
    {
        pAttrList->AddAttribute(A2OUSTR("style:text-crossing-out"), GetCrossoutName(m_eCrossout) );
    }

    if( m_nFlag & XFFONT_FLAG_UNDERLINE || m_nFlag & XFFONT_FLAG_CROSSOUT )
    {
        if( m_bWordByWord )
            pAttrList->AddAttribute(A2OUSTR("fo:score-spaces"), A2OUSTR("false") );
        else
            pAttrList->AddAttribute(A2OUSTR("fo:score-spaces"), A2OUSTR("true") );
    }

    if( (m_nFlag & XFFONT_FLAG_RELIEF) && m_eRelief )
    {
        pAttrList->AddAttribute(A2OUSTR("style:font-relief"), GetReliefName(m_eRelief) );
    }

    if( (m_nFlag & XFFONT_FLAG_TRANSFORM) && m_eTransform )
    {
        //enumTransformSmallCap is different:
        if( m_eTransform == enumXFTransformSmallCaps )
            pAttrList->AddAttribute(A2OUSTR("fo:font-variant"), GetTransformName(m_eTransform) );
        else
            pAttrList->AddAttribute(A2OUSTR("fo:text-transform"), GetTransformName(m_eTransform) );
    }

    if( (m_nFlag & XFFONT_FLAG_EMPHASIZE) && m_eEmphasize )
    {
        rtl::OUString empha = GetEmphasizeName(m_eEmphasize);
        empha += A2OUSTR(" ");
        if( m_bEmphasizeTop )
            empha += A2OUSTR("above");
        pAttrList->AddAttribute(A2OUSTR("style:text-emphasize"), empha );
    }

    if( (m_nFlag & XFFONT_FLAG_OUTLINE) && m_bOutline )
    {
        pAttrList->AddAttribute(A2OUSTR("style:text-outline"), A2OUSTR("true") );
    }

    if( (m_nFlag & XFFONT_FLAG_SHADOW) && m_bShadow )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:text-shadow"), A2OUSTR("1pt 1pt") );
    }

    if( (m_nFlag & XFFONT_FLAG_BLINK) && m_bBlink )
    {
        pAttrList->AddAttribute(A2OUSTR("style:text-blinking"), A2OUSTR("true") );
    }


    //position & sacle:
    if( ((m_nFlag & XFFONT_FLAG_SCALE) && m_nScale>0 ) ||
        ((m_nFlag & XFFONT_FLAG_POSITION) && m_nPosition != 0)
        )
    {
        rtl::OUString tmp;
        tmp = Int32ToOUString(m_nPosition) + A2OUSTR("% ");
        tmp += Int32ToOUString(m_nScale) + A2OUSTR("%");
        pAttrList->AddAttribute(A2OUSTR("style:text-position"), tmp );
    }

    //char space:
    if( (m_nFlag & XFFONT_FLAG_CHARSPACE) && m_fCharSpace != 0 )
    {
        pAttrList->AddAttribute(A2OUSTR("fo:letter-spacing"), DoubleToOUString(m_fCharSpace)+A2OUSTR("cm") );
    }

    if( (m_nFlag&XFFONT_FLAG_WIDTHSCALE) && m_nWidthScale != 100 )
    {
        pAttrList->AddAttribute(A2OUSTR("style:text-scale"), DoubleToOUString(m_nWidthScale)+A2OUSTR("%") );
    }

    //Color:
    if( (m_nFlag & XFFONT_FLAG_COLOR) )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:color"), m_aColor.ToString() );
    }

    if( (m_nFlag & XFFONT_FLAG_BGCOLOR) )
    {
        if (m_bTransparent)
            pAttrList->AddAttribute( A2OUSTR("style:text-background-color"), A2OUSTR("transparent"));
        else
            pAttrList->AddAttribute( A2OUSTR("style:text-background-color"), m_aBackColor.ToString() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
