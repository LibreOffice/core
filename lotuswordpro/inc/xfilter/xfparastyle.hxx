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
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPARASTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPARASTYLE_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>

#include <xfilter/xffont.hxx>
#include <xfilter/xfstyle.hxx>
#include <xfilter/xfcolor.hxx>
#include <xfilter/xfmargins.hxx>
#include <xfilter/xfbreaks.hxx>
#include <xfilter/xfpadding.hxx>
#include <xfilter/xfshadow.hxx>
#include <xfilter/xfdropcap.hxx>
#include <xfilter/xfstylecont.hxx>
#include <xfilter/xflineheight.hxx>
#include <memory>

enum XFParaFlags {
    XFPARA_FLAG_FONT      = 0X00000001,
    XFPARA_FLAG_DROPCAP   = 0X00000002,
    XFPARA_FLAG_BACKCOLOR = 0X00000004
};

class XFBorders;
class XFBGImage;

/**
 * @brief
 * Style object for aragraph.
 */
class XFParaStyle : public XFStyle
{
public:
    XFParaStyle();

    XFParaStyle(const XFParaStyle& other);

    XFParaStyle& operator=(const XFParaStyle& other);

    virtual ~XFParaStyle() override;

public:
    /**
     * @descr   Set layout for the paragraph. When such property was set, this paragraph will
     *          start at a new page.
     */
    void    SetMasterPage(const OUString& master);

    const OUString& GetMasterPage() const;

    /**
     * @descr   set the paragraph default font.
     * @param   font object to be set. Font object is deleted by font-factory, so
     *          don't delete it in the destructure function of para style.
     */
    void    SetFont(rtl::Reference<XFFont> const & font);

    /**
     * @descr   get the font object.
     */
    const rtl::Reference<XFFont>& GetFont() const { return m_pFont; }

    /**
     * @descr   Set the indent of the paragraph. This is the indent for
                the first line.
     * @param   indent value of the first-line indent.
     */
    void    SetIndent(double indent );

    /**
     * @descr   Set the padding of the paragraph. This is the distance
                between the border and the top of the text.
     * @param   indent value of the padding.
     */
    void    SetPadding(double left, double right, double top, double bottom);

    /**
     * @descr   Set the Margins of the paragraph.
     * @param   -1:     don't change.
                other:  set value.
     */
    void    SetMargins(double left, double right,double top=-1, double bottom=-1);

    /**
     * @descr   Set alignment property of the paragraph.
     * @param   eAlign alignment type,left,right,center or justify.
     */
    void    SetAlignType(enumXFAlignType eAlign);

    /**
     * @descr   Set the shadow of the paragraph. There are 4 positions, you
                can find it in the definition of enumShadowPos.
     * @param   pos
     * @param   offset the distance between the paragraph border and the shadow.
     * @param   color color to fill the shadow.
     */
    void    SetShadow(enumXFShadowPos pos, double offset, XFColor const & color);

    /**
     * @descr   The borders is complex, so you have to create one before use.
                Very few paragraphs will already have borders property, this way
                we can save much memory.
     * @param   pBorders borders of the paragraph, please reference the XFBorders.
     */
    void    SetBorders(XFBorders *pBorders);

    /**
     * @descr   Set drop caption of the paragraph.
     * @param   nLength number of chars to be dropped.
     * @param   nLines line of which the dropped chars will occupy.
     */
    void    SetDropCap(sal_Int16 nLength, sal_Int16 nLines, double fDistance = 0);

    /**
     * @descr   Set line height of the paragraph.
     * @param   type type of line height,fixed,space,percent or cm.
     * @param   value value of the line height
     */
    void    SetLineHeight(enumLHType type, double value);

    /**
     * @descr   Set background color of the paragraph.
     * @param   color value of the back color.
     */
    void    SetBackColor(XFColor const & color);

    /**
     * @descr   Set background image of the paragraph.
     * @param   image the background image to set.
     */
    void    SetBackImage(std::unique_ptr<XFBGImage>& rImage);

    /**
     * descr    You can only set one break property for every para style object.
     */
    void    SetBreaks(enumXFBreaks breaks);

    /**
     * @descr   Add a tab style.
     */
    void    AddTabStyle(enumXFTab type, double len, sal_Unicode leader, sal_Unicode delimiter);

    /**
     * @descr   for para style copy operator, sometimes you may need to override tab styles.
     */
    void    ClearTabStyles();

    XFMargins& GetMargins(){return m_aMargin;}

    virtual enumXFStyle GetStyleFamily() override;

    virtual bool Equal(IXFStyle *pStyle) override;

    virtual void    ToXml(IXFStream *strm) override;

    void SetNumberRight(bool bFlag){m_bNumberRight = bFlag;}
    bool GetNumberRight() const {return m_bNumberRight;}

private:
    OUString   m_strMasterPage;
    enumXFAlignType m_eAlignType;

    double      m_fTextIndent;
    XFColor     m_aBackColor;
    XFMargins   m_aMargin;
    XFPadding   m_aPadding;
    XFStyleContainer m_aTabs;
    rtl::Reference<XFFont> m_pFont;
    XFShadow    m_aShadow;
    std::unique_ptr<XFBorders> m_pBorders;
    std::unique_ptr<XFBGImage> m_pBGImage;
    XFDropcap   m_aDropcap;
    XFLineHeight m_aLineHeight;
    XFBreaks    m_aBreaks;

    sal_uInt32  m_nFlag;
    bool m_bNumberRight;
};

inline void XFParaStyle::SetMasterPage(const OUString& master)
{
    m_strMasterPage = master;
}

inline void XFParaStyle::SetBreaks(enumXFBreaks breaks)
{
    m_aBreaks.SetBreakType(breaks);
}

inline void XFParaStyle::SetPadding(double left, double right, double top, double bottom)
{
    m_aPadding.SetLeft(left);
    if( right == -1 )
        m_aPadding.SetRight(left);
    else
        m_aPadding.SetRight(right);
    if( top == -1 )
        m_aPadding.SetTop(left);
    else
        m_aPadding.SetTop(top);
    if( bottom == -1 )
        m_aPadding.SetBottom(left);
    else
        m_aPadding.SetBottom(bottom);
}

inline void XFParaStyle::SetAlignType(enumXFAlignType eAlign)
{
    m_eAlignType = eAlign;
}

inline void XFParaStyle::ClearTabStyles()
{
    m_aTabs.Reset();
}

inline const OUString& XFParaStyle::GetMasterPage() const
{
    return m_strMasterPage;
}


class XFDefaultParaStyle : public XFStyle
{
public:
    XFDefaultParaStyle();
    void SetTabDistance(double len);
    enumXFStyle GetStyleFamily() override;
    virtual void    ToXml(IXFStream *pStrm) override;
private:
    double m_fTabDistance;
};

inline void XFDefaultParaStyle::SetTabDistance(double len)
{
    m_fTabDistance = len;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
