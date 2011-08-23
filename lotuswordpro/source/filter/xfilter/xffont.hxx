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
 ************************************************************************/
#ifndef		_XFFONT_HXX
#define		_XFFONT_HXX

#include	"xfglobal.hxx"
#include	"xfcolor.hxx"

#define		XFFONT_FLAG_NAME			0X00000001
#define		XFFONT_FLAG_NAME_ASIA		0X00000002
#define		XFFONT_FLAG_NAME_COMPLEX	0X00000004

#define		XFFONT_FLAG_SIZE			0X00000008
#define		XFFONT_FLAG_SIZE_ASIA		0X00000010
#define		XFFONT_FLAG_SIZE_COMPLEX	0X00000020

#define		XFFONT_FLAG_ITALIC			0X00000040
#define		XFFONT_FLAG_ITALIC_ASIA		0X00000080
#define		XFFONT_FLAG_ITALIC_COMPLEX	0X00000100

#define		XFFONT_FLAG_BOLD			0X00000200
#define		XFFONT_FLAG_BOLD_ASIA		0X00000400
#define		XFFONT_FLAG_BOLD_COMPLEX	0X00000800

#define		XFFONT_FLAG_UNDERLINE		0X00001000
#define		XFFONT_FLAG_UNDERLINECOLOR	0X00002000

#define		XFFONT_FLAG_POSITION		0X00004000
#define		XFFONT_FLAG_SCALE			0X00008000
#define		XFFONT_FLAG_CHARSPACE		0x00010000

#define		XFFONT_FLAG_COLOR			0x00020000
#define		XFFONT_FLAG_BGCOLOR			0x00040000

#define		XFFONT_FLAG_CROSSOUT		0x00080000
#define		XFFONT_FLAG_RELIEF			0x00100000
#define		XFFONT_FLAG_TRANSFORM		0x00200000
#define		XFFONT_FLAG_EMPHASIZE		0x00400000
#define		XFFONT_FLAG_OUTLINE			0x00800000
#define		XFFONT_FLAG_SHADOW			0x01000000
#define		XFFONT_FLAG_BLINK			0x02000000
#define		XFFONT_FLAG_WIDTHSCALE		0x04000000

/**
 * @descr
 * The font struct of openoffice xml filter.
 * There should be some basic members,ie:

 * font name
 * font family
 * font size
 * font italic
 * font bold
 * font underline
 * font underline color
 */
class XFFont
{
public:
    XFFont();

public:
    /**
     * @descr	Set font name.
     */
    void	SetFontName(rtl::OUString name);

    /**
     * @descr	Set font name for asia locale.
     */
    void	SetFontNameAsia(rtl::OUString name);

    /**
     * @descr	Set font name for BIDI locale.
     */
    void	SetFontNameComplex(rtl::OUString name);

    /**
     * @descr	Set font size.
     */
    void	SetFontSize(sal_Int16 size);

    /**
     * @descr	Set font size for asia locale.
     */
    void	SetFontSizeAsia(sal_Int16 size);

    /**
     * @descr	Set font size for BIDI locale.
     */
    void	SetFontSizeComplex(sal_Int16 size);

    /**
     * @descr	Set whether font is italic.
     */
    void	SetItalic(sal_Bool italic = true);

    /**
     * @descr	Set whether font is italic for asia locale.
     */
    void	SetItalicAsia(sal_Bool italic);

    /**
     * @descr	Set whether font is italic for BIDI locale.
     */
    void	SetItalicComplex(sal_Bool italic);

    /**
     * @descr	Set font bold.
     */
    void	SetBold(sal_Bool bold = true);

    /**
     * @descr	Set font bold for asia locale.
     */
    void	SetBoldAsia(sal_Bool bold);

    /**
     * @descr	Set font bold for complex locale.
     */
    void	SetBoldComplex(sal_Bool bold);

    /**
     * @descr	Set underline style.
     */
    void	SetUnderline(enumXFUnderline underline,sal_Bool wordByWord=sal_False);

    /**
     * @descr	Set underline color.
     */
    void	SetUnderlineColor(XFColor color=XFColor(128,128,0));

    /**
     * @descr	Set crossout.
     */
    void	SetCrossout(enumXFCrossout cross,sal_Bool wordByWord=sal_False);

    /**
     * @descr	Set font relief.
     */
    void	SetRelief(enumXFRelief relief);

    /**
     * @descr	Set font transform type,pls refer to enumXFTransform.
     */
    void	SetTransform(enumXFTransform transform);

    /**
     * @descr	Set font emphasize effect.
     */
    void	SetEmphasize(enumXFEmphasize emphasize, sal_Bool top = sal_True);

    /**
     * @descr	Set outline effect.
     */
    void	SetOutline(sal_Bool outline);

    /**
     * @descr	Set font shadow.
     */
    void	SetShadow(sal_Bool shadow);

    /**
     * @descr	Set font blink effect.
     */
    void	SetBlink(sal_Bool blink);

    /**
     * @descr	Set upper script or lower script.
     */
    void	SetPosition(sal_Bool bUpperScript = sal_True, sal_Int16 pos = 33, sal_Int16 scale = 58);

    /**
     * @descr	Set space between chars in percent.
     */
    void	SetCharSpace(sal_Int16 space);

    /**
     * @descr	Set char width scale in percent.
     */
    void	SetWidthScale(sal_Int16 widthScale);

    /**
     * @descr	Set font color.
     */
    void	SetColor(XFColor& color);

    /**
     * @descr	Set font background color.
     */
    void	SetBackColor(XFColor& color);
    void	SetBackColorTransparent();

    //getter:
    enumXFTransform GetTransform();

    sal_Int16 GetFontSize();
    XFColor GetColor();

    sal_uInt32	GetFlag(){ return m_nFlag; }

    void	ToXml(IXFStream *pStrm);

    friend bool operator==(XFFont& f1, XFFont& f2);
    friend bool operator!=(XFFont& f1, XFFont& f2);
    friend class	XFFontFactory;
private:
    rtl::OUString	m_strFontName;
    rtl::OUString	m_strFontNameAsia;
    rtl::OUString	m_strFontNameComplex;
    sal_Int16	m_nFontSize;
    sal_Int16	m_nFontSizeAsia;
    sal_Int16	m_nFontSizeComplex;
    sal_Bool	m_bItalic;
    sal_Bool	m_bItalicAsia;
    sal_Bool	m_bItalicComplex;
    sal_Bool	m_bBold;
    sal_Bool	m_bBoldAsia;
    sal_Bool	m_bBoldComplex;
    XFColor		m_aUnderlineColor;

    enumXFUnderline	m_eUnderline;
    enumXFCrossout	m_eCrossout;
    enumXFRelief	m_eRelief;
    enumXFTransform	m_eTransform;
    enumXFEmphasize	m_eEmphasize;
    sal_Bool	m_bWordByWord;
    sal_Bool	m_bEmphasizeTop;
    sal_Bool	m_bOutline;
    sal_Bool	m_bShadow;
    sal_Bool	m_bBlink;
    sal_Int16	m_nPosition;
    sal_Int16	m_nScale;
    double		m_fCharSpace;
    sal_Int16	m_nWidthScale;
    XFColor		m_aColor;
    XFColor		m_aBackColor;
    //The flag defines which variable will be funtional.
    sal_uInt32	m_nFlag;
    sal_Bool	m_bTransparent;
};


inline void	XFFont::SetFontName(rtl::OUString name)
{
    m_strFontName = name;
    m_nFlag |= XFFONT_FLAG_NAME;

    SetFontNameAsia(name);
    SetFontNameComplex(name);
}

inline void	XFFont::SetFontNameAsia(rtl::OUString name)
{
    m_strFontNameAsia = name;
    m_nFlag |= XFFONT_FLAG_NAME_ASIA;
}

inline void	XFFont::SetFontNameComplex(rtl::OUString name)
{
    m_strFontNameComplex = name;
    m_nFlag |= XFFONT_FLAG_NAME_COMPLEX;
}

//
inline void	XFFont::SetFontSize(sal_Int16 size)
{
    m_nFontSize = size;
    m_nFlag |= XFFONT_FLAG_SIZE;

    SetFontSizeAsia(size);
    SetFontSizeComplex(size);
}

inline void	XFFont::SetFontSizeAsia(sal_Int16 size)
{
    m_nFontSizeAsia = size;
    m_nFlag |= XFFONT_FLAG_SIZE_ASIA;
}

inline void	XFFont::SetFontSizeComplex(sal_Int16 size)
{
    m_nFontSizeComplex = size;
    m_nFlag |= XFFONT_FLAG_SIZE_COMPLEX;
}

//---------Italic:
inline void	XFFont::SetItalic(sal_Bool italic)
{
    m_bItalic = italic;
    m_nFlag |= XFFONT_FLAG_ITALIC;

    SetItalicAsia(italic);
    SetItalicComplex(italic);
}

inline void	XFFont::SetItalicAsia(sal_Bool italic)
{
    m_bItalicAsia = italic;
    m_nFlag |= XFFONT_FLAG_ITALIC_ASIA;
}

inline void	XFFont::SetItalicComplex(sal_Bool italic)
{
    m_bItalicComplex = italic;
    m_nFlag |= XFFONT_FLAG_ITALIC_COMPLEX;
}

//--------Bold
inline void	XFFont::SetBold(sal_Bool bold)
{
    m_bBold = bold;
    m_nFlag |= XFFONT_FLAG_BOLD;
    SetBoldAsia(bold);
    SetBoldComplex(bold);
}

inline void	XFFont::SetBoldAsia(sal_Bool bold)
{
    m_bBoldAsia = bold;
    m_nFlag |= XFFONT_FLAG_BOLD_ASIA;
}

inline void	XFFont::SetBoldComplex(sal_Bool bold)
{
    m_bBoldComplex = bold;
    m_nFlag |= XFFONT_FLAG_BOLD_COMPLEX;
}

//--------Underline:
inline void	XFFont::SetUnderline(enumXFUnderline underline, sal_Bool wordByWord)
{
    m_eUnderline = underline;
    m_bWordByWord = wordByWord;
    m_nFlag |= XFFONT_FLAG_UNDERLINE;
}

inline void	XFFont::SetUnderlineColor(XFColor color)
{
    m_aUnderlineColor = color;
    m_nFlag |= XFFONT_FLAG_UNDERLINECOLOR;
}

inline void	XFFont::SetCrossout(enumXFCrossout cross, sal_Bool wordByWord)
{
    m_eCrossout = cross;
    m_bWordByWord = wordByWord;
    m_nFlag |= XFFONT_FLAG_CROSSOUT;
}
inline void	XFFont::SetRelief(enumXFRelief relief)
{
    m_eRelief = relief;
    m_nFlag |= XFFONT_FLAG_RELIEF;
}
inline void	XFFont::SetTransform(enumXFTransform transform)
{
    m_eTransform = transform;
    m_nFlag |= XFFONT_FLAG_TRANSFORM;
}

inline void	XFFont::SetEmphasize(enumXFEmphasize emphasize, sal_Bool top)
{
    m_eEmphasize = emphasize;
    m_bEmphasizeTop = top;
    m_nFlag |= XFFONT_FLAG_EMPHASIZE;
}
inline void	XFFont::SetOutline(sal_Bool outline)
{
    m_bOutline = outline;
    m_nFlag |= XFFONT_FLAG_OUTLINE;
}
inline void	XFFont::SetShadow(sal_Bool shadow)
{
    m_bShadow = shadow;
    m_nFlag |= XFFONT_FLAG_SHADOW;
}
inline void	XFFont::SetBlink(sal_Bool blink)
{
    m_bBlink = blink;
    m_nFlag |= XFFONT_FLAG_BLINK;
}

inline void	XFFont::SetPosition(sal_Bool bUpperScript, sal_Int16 pos, sal_Int16 scale)
{
    assert(pos>=0&&pos<=100);
    assert(scale>0&&scale<=100);
    m_nFlag |= XFFONT_FLAG_POSITION;
    m_nPosition = pos;
    m_nScale = scale;
    if( bUpperScript )
    {
        if( m_nPosition <0 )
            m_nPosition = -m_nPosition;
    }
    else
    {
        if( m_nPosition >0 )
            m_nPosition = -m_nPosition;
    }
}

inline void	XFFont::SetCharSpace(sal_Int16 space)
{
    m_nFlag |= XFFONT_FLAG_CHARSPACE;
    m_fCharSpace = space;
}

inline void	XFFont::SetWidthScale(sal_Int16 widthScale)
{
    m_nFlag |= XFFONT_FLAG_WIDTHSCALE;
    m_nWidthScale = widthScale;
}

inline void	XFFont::SetColor(XFColor& color)
{
    m_nFlag |= XFFONT_FLAG_COLOR;
    m_aColor = color;
}

inline void	XFFont::SetBackColor(XFColor& color)
{
    m_bTransparent = sal_False;
    m_nFlag |= XFFONT_FLAG_BGCOLOR;
    m_aBackColor = color;
}

inline void	XFFont::SetBackColorTransparent()
{
    m_bTransparent = sal_True;
    m_nFlag |= XFFONT_FLAG_BGCOLOR;
}

inline enumXFTransform XFFont::GetTransform()
{
    return m_eTransform;
}

inline sal_Int16 XFFont::GetFontSize()
{
    return m_nFontSize;
}

inline XFColor XFFont::GetColor()
{
    return m_aColor;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
