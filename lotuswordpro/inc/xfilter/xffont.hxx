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
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFFONT_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFFONT_HXX

#include <sal/config.h>

#include <salhelper/simplereferenceobject.hxx>

#include <xfilter/xfglobal.hxx>
#include <xfilter/xfcolor.hxx>

#define     XFFONT_FLAG_NAME            0X00000001
#define     XFFONT_FLAG_NAME_ASIA       0X00000002
#define     XFFONT_FLAG_NAME_COMPLEX    0X00000004

#define     XFFONT_FLAG_SIZE            0X00000008
#define     XFFONT_FLAG_SIZE_ASIA       0X00000010
#define     XFFONT_FLAG_SIZE_COMPLEX    0X00000020

#define     XFFONT_FLAG_ITALIC          0X00000040
#define     XFFONT_FLAG_ITALIC_ASIA     0X00000080
#define     XFFONT_FLAG_ITALIC_COMPLEX  0X00000100

#define     XFFONT_FLAG_BOLD            0X00000200
#define     XFFONT_FLAG_BOLD_ASIA       0X00000400
#define     XFFONT_FLAG_BOLD_COMPLEX    0X00000800

#define     XFFONT_FLAG_UNDERLINE       0X00001000
#define     XFFONT_FLAG_UNDERLINECOLOR  0X00002000

#define     XFFONT_FLAG_POSITION        0X00004000
#define     XFFONT_FLAG_SCALE           0X00008000
#define     XFFONT_FLAG_CHARSPACE       0x00010000

#define     XFFONT_FLAG_COLOR           0x00020000
#define     XFFONT_FLAG_BGCOLOR         0x00040000

#define     XFFONT_FLAG_CROSSOUT        0x00080000
#define     XFFONT_FLAG_RELIEF          0x00100000
#define     XFFONT_FLAG_TRANSFORM       0x00200000
#define     XFFONT_FLAG_EMPHASIZE       0x00400000
#define     XFFONT_FLAG_OUTLINE         0x00800000
#define     XFFONT_FLAG_SHADOW          0x01000000
#define     XFFONT_FLAG_BLINK           0x02000000
#define     XFFONT_FLAG_WIDTHSCALE      0x04000000

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
class XFFont: public salhelper::SimpleReferenceObject
{
public:
    XFFont();

public:
    /**
     * @descr   Set font name.
     */
    void    SetFontName(const OUString& name);

    /**
     * @descr   Set font name for asia locale.
     */
    void    SetFontNameAsia(const OUString& name);

    /**
     * @descr   Set font name for BIDI locale.
     */
    void    SetFontNameComplex(const OUString& name);

    /**
     * @descr   Set font size.
     */
    void    SetFontSize(sal_Int16 size);

    /**
     * @descr   Set font size for asia locale.
     */
    void    SetFontSizeAsia(sal_Int16 size);

    /**
     * @descr   Set font size for BIDI locale.
     */
    void    SetFontSizeComplex(sal_Int16 size);

    /**
     * @descr   Set whether font is italic.
     */
    void    SetItalic(bool italic);

    /**
     * @descr   Set whether font is italic for asia locale.
     */
    void    SetItalicAsia(bool italic);

    /**
     * @descr   Set whether font is italic for BIDI locale.
     */
    void    SetItalicComplex(bool italic);

    /**
     * @descr   Set font bold.
     */
    void    SetBold(bool bold);

    /**
     * @descr   Set font bold for asia locale.
     */
    void    SetBoldAsia(bool bold);

    /**
     * @descr   Set font bold for complex locale.
     */
    void    SetBoldComplex(bool bold);

    /**
     * @descr   Set underline style.
     */
    void    SetUnderline(enumXFUnderline underline,bool wordByWord=false);

    /**
     * @descr   Set crossout.
     */
    void    SetCrossout(enumXFCrossout cross);

    /**
     * @descr   Set font transform type,pls refer to enumXFTransform.
     */
    void    SetTransform(enumXFTransform transform);

    /**
     * @descr   Set upper script or lower script.
     */
    void    SetPosition(bool bUpperScript = true, sal_Int16 pos = 33, sal_Int16 scale = 58);

    /**
     * @descr   Set font color.
     */
    void    SetColor(XFColor const & color);

    /**
     * @descr   Set font background color.
     */
    void    SetBackColor(XFColor const & color);
    void    SetBackColorTransparent();

    //getter:
    enumXFTransform GetTransform() const;

    sal_Int16 GetFontSize() const;
    const XFColor& GetColor() const;

    void    ToXml(IXFStream *pStrm);

    friend bool operator==(XFFont const & f1, XFFont const & f2);
    friend bool operator!=(XFFont const & f1, XFFont const & f2);
    friend class    XFFontFactory;
private:
    virtual ~XFFont() override {}

    OUString   m_strFontName;
    OUString   m_strFontNameAsia;
    OUString   m_strFontNameComplex;
    sal_Int16   m_nFontSize;
    sal_Int16   m_nFontSizeAsia;
    sal_Int16   m_nFontSizeComplex;
    bool    m_bItalic;
    bool    m_bItalicAsia;
    bool    m_bItalicComplex;
    bool    m_bBold;
    bool    m_bBoldAsia;
    bool    m_bBoldComplex;
    XFColor     m_aUnderlineColor;

    enumXFUnderline m_eUnderline;
    enumXFCrossout  m_eCrossout;
    enumXFTransform m_eTransform;
    bool    m_bWordByWord;
    sal_Int16   m_nPosition;
    sal_Int16   m_nScale;
    XFColor     m_aColor;
    XFColor     m_aBackColor;
    //The flag defines which variable will be functional
    sal_uInt32  m_nFlag;
    bool    m_bTransparent;
};

inline void XFFont::SetFontName(const OUString& name)
{
    m_strFontName = name;
    m_nFlag |= XFFONT_FLAG_NAME;

    SetFontNameAsia(name);
    SetFontNameComplex(name);
}

inline void XFFont::SetFontNameAsia(const OUString& name)
{
    m_strFontNameAsia = name;
    m_nFlag |= XFFONT_FLAG_NAME_ASIA;
}

inline void XFFont::SetFontNameComplex(const OUString& name)
{
    m_strFontNameComplex = name;
    m_nFlag |= XFFONT_FLAG_NAME_COMPLEX;
}

inline void XFFont::SetFontSize(sal_Int16 size)
{
    m_nFontSize = size;
    m_nFlag |= XFFONT_FLAG_SIZE;

    SetFontSizeAsia(size);
    SetFontSizeComplex(size);
}

inline void XFFont::SetFontSizeAsia(sal_Int16 size)
{
    m_nFontSizeAsia = size;
    m_nFlag |= XFFONT_FLAG_SIZE_ASIA;
}

inline void XFFont::SetFontSizeComplex(sal_Int16 size)
{
    m_nFontSizeComplex = size;
    m_nFlag |= XFFONT_FLAG_SIZE_COMPLEX;
}

//---------Italic:
inline void XFFont::SetItalic(bool italic)
{
    m_bItalic = italic;
    m_nFlag |= XFFONT_FLAG_ITALIC;

    SetItalicAsia(italic);
    SetItalicComplex(italic);
}

inline void XFFont::SetItalicAsia(bool italic)
{
    m_bItalicAsia = italic;
    m_nFlag |= XFFONT_FLAG_ITALIC_ASIA;
}

inline void XFFont::SetItalicComplex(bool italic)
{
    m_bItalicComplex = italic;
    m_nFlag |= XFFONT_FLAG_ITALIC_COMPLEX;
}

//--------Bold
inline void XFFont::SetBold(bool bold)
{
    m_bBold = bold;
    m_nFlag |= XFFONT_FLAG_BOLD;
    SetBoldAsia(bold);
    SetBoldComplex(bold);
}

inline void XFFont::SetBoldAsia(bool bold)
{
    m_bBoldAsia = bold;
    m_nFlag |= XFFONT_FLAG_BOLD_ASIA;
}

inline void XFFont::SetBoldComplex(bool bold)
{
    m_bBoldComplex = bold;
    m_nFlag |= XFFONT_FLAG_BOLD_COMPLEX;
}

//--------Underline:
inline void XFFont::SetUnderline(enumXFUnderline underline, bool wordByWord)
{
    m_eUnderline = underline;
    m_bWordByWord = wordByWord;
    m_nFlag |= XFFONT_FLAG_UNDERLINE;
}

inline void XFFont::SetCrossout(enumXFCrossout cross)
{
    m_eCrossout = cross;
    m_bWordByWord = false;
    m_nFlag |= XFFONT_FLAG_CROSSOUT;
}

inline void XFFont::SetTransform(enumXFTransform transform)
{
    m_eTransform = transform;
    m_nFlag |= XFFONT_FLAG_TRANSFORM;
}

inline void XFFont::SetPosition(bool bUpperScript, sal_Int16 pos, sal_Int16 scale)
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

inline void XFFont::SetColor(XFColor const & color)
{
    m_nFlag |= XFFONT_FLAG_COLOR;
    m_aColor = color;
}

inline void XFFont::SetBackColor(XFColor const & color)
{
    m_bTransparent = false;
    m_nFlag |= XFFONT_FLAG_BGCOLOR;
    m_aBackColor = color;
}

inline void XFFont::SetBackColorTransparent()
{
    m_bTransparent = true;
    m_nFlag |= XFFONT_FLAG_BGCOLOR;
}

inline enumXFTransform XFFont::GetTransform() const
{
    return m_eTransform;
}

inline sal_Int16 XFFont::GetFontSize() const
{
    return m_nFontSize;
}

inline const XFColor& XFFont::GetColor() const
{
    return m_aColor;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
