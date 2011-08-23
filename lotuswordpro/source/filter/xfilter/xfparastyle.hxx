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
/*************************************************************************
 * Change History
 * 2005-01-10 create this file.
 * 2005-01-20 move some structure out of this file.
 ************************************************************************/
#ifndef		_XFPARASTYLE_HXX
#define		_XFPARASTYLE_HXX

#include	"xfglobal.hxx"
#include	"xfstyle.hxx"
#include	"xfcolor.hxx"
#include	"xfmargins.hxx"
#include	"xfbreaks.hxx"
#include	"xfpadding.hxx"
#include	"xfshadow.hxx"
#include	"xfdropcap.hxx"
#include	"xfstylecont.hxx"
#include	"xflineheight.hxx"

#define		XFPARA_FLAG_FONT		0X00000001
#define		XFPARA_FLAG_DROPCAP		0X00000002
#define		XFPARA_FLAG_BACKCOLOR	0X00000004

class XFFont;
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

    XFParaStyle(XFParaStyle& other);

    XFParaStyle& operator=(XFParaStyle& other);

    virtual ~XFParaStyle();

public:
    /**
     * @descr	Set layout for the paragraph.When such property was setted, this paragraph will
     *			start at an new page.
     */
    void	SetMasterPage(rtl::OUString master);

    rtl::OUString GetMasterPage();

    /**
     * @descr	set the paragraph defaut font.
     * @param	font font obejct to be setted.Font object are deleted by font-factory,so
     *			dont't delete it in the destructure function of para style.
     */
    void	SetFont(XFFont *font);

    /**
     * @descr	get the font object.
     */
    XFFont*	GetFont(){ return m_pFont; }

    /**
     * @descr	Set the indent of the paragraph.This is the indent for
                the first line.
     * @param	indent value of the first-line indent.
     */
    void	SetIndent(double indent );

    /**
     * @descr	Set line number style.
     */
    void	SetLineNumber(sal_Bool show, sal_Int32 restart=1);
    /**
     * @descr	Set the pading of the paragraph.This is the distance
                between the border and the top of the text.
     * @param	indent value of the padding.
     */
    void	SetPadding(double left, double right = -1, double top = -1, double bottom = -1);

    /**
     * @descr	Set the Margins of the paragraph.
     * @param	-1:		don't change.
                other:	set value.
     */
    void	SetMargins(double left, double right=-1,double top=-1, double bottom=-1);

    /**
     * @descr	Set alignment property of the paragraph.
     * @param	eAlign alignment type,left,right,center or justify.
     */
    void	SetAlignType(enumXFAlignType eAlign);

    enumXFAlignType GetAlighType();


    /**
     * @descr	Set last line alignment property of the paragraph.
     * @param	eAlign alignment type,left,right,center or justify.
                eJustSingleWord If chars of the last-line'word should be
                stretched.
     */
    void	SetLastLineAlign(enumXFAlignType align,sal_Bool	bJustSingleWord = sal_False);

    /**
     * @descr	Set the shadow of the paragraph.there is 4 postions, you
                can find it in the definition of enumShadowPos.
     * @param	pos
     * @param	offset the distance between the paragraph border and the shadow.
     * @param	color color to fill the shadow.
     */
    void	SetShadow(enumXFShadowPos pos, double offset, XFColor& color);

    /**
     * @descr	The borders is complex,so you have to create one before use.
                Very few paragraphs will readly have borders property,this way
                we can save much memory.
     * @param	pBorders borders of the paragraph,please reference the XFBorders.
     */
    void	SetBorders(XFBorders *pBorders);

    /**
     * @descr	Set drop caption of the paragraph.
     * @param	nLength number of chars to be droped.
     * @param	nLines line of which the droped chars will occupy.
     */
    void	SetDropCap(sal_Int16 nLength = 1,sal_Int16 nLines = 3,double fDistance = 0);

    /**
     * @descr	Set line height of the paragraph.
     * @param	type type of line height,fixed,space,percent or cm.
     * @param	value value of the line height
     */
    void	SetLineHeight(enumLHType type, double value);

    /**
     * @descr	Set background color of the paragraph.
     * @param	color value of the back color.
     */
    void	SetBackColor(XFColor& color);

    /**
     * @descr	Set background image of the paragraph.
     * @param	image the background image to set.
     */
    void	SetBackImage(XFBGImage *image);

    /**
     * descr	You can only set one break property for every para style object.
     */
    void	SetBreaks(enumXFBreaks breaks);

    /**
     * @descr	For paragraph numbering.
     */
    void	SetPageNumber(sal_Int32 num);

    /**
     * @descr	Add a tab style.
     */
    void	AddTabStyle(enumXFTab type, double len, sal_Unicode leader = '*', sal_Unicode delimiter='.');

    /**
     * @descr	for para style copy operator,sometimes you may need to override tab styles.
     */
    void	ClearTabStyles();

    /**
     * descr	set the paragraph to be in the same page with the next paragraph.
     *			If that can't be insured,the paragraph will start with a new page.
     */
    void	SetKeepWithNext(sal_Bool keepWithNext);

    sal_uInt32 GetFlag(){ return m_nFlag; }

    /**
     * @descr	return margins.
     */
    XFMargins GetMargins(){return m_aMargin;}

    virtual enumXFStyle	GetStyleFamily();

    virtual sal_Bool Equal(IXFStyle *pStyle);

    virtual void	ToXml(IXFStream *strm);

    void SetNumberRight(sal_Bool bFlag){m_bNumberRight = bFlag;}
    sal_Bool GetNumberRight(){return m_bNumberRight;}

protected:
    rtl::OUString	m_strMasterPage;
    enumXFAlignType	m_eAlignType;
    enumXFAlignType	m_eLastLineAlign;
    sal_Bool		m_bJustSingleWord;
    sal_Bool		m_bKeepWithNext;

    double		m_fTextIndent;
    XFColor		m_aBackColor;
    XFMargins	m_aMargin;
    XFPadding	m_aPadding;
    XFStyleContainer m_aTabs;
    XFFont		*m_pFont;
    XFShadow	m_aShadow;
    XFBorders	*m_pBorders;
    XFBGImage	*m_pBGImage;
    XFDropcap	m_aDropcap;
    XFLineHeight m_aLineHeight;
    XFBreaks	m_aBreaks;
    sal_Int32	m_nPageNumber;
    sal_Bool	m_bNumberLines;
    sal_Int32	m_nLineNumberRestart;

    sal_uInt32	m_nFlag;
    sal_Bool m_bNumberRight;
};

inline void XFParaStyle::SetMasterPage(rtl::OUString master)
{
    m_strMasterPage = master;
}

inline void	XFParaStyle::SetBreaks(enumXFBreaks breaks)
{
    m_aBreaks.SetBreakType(breaks);
}

inline void	XFParaStyle::SetPageNumber(sal_Int32 num)
{
    assert(num>0);
    m_nPageNumber = num;
}

inline void	XFParaStyle::SetLineNumber(sal_Bool show, sal_Int32 restart)
{
    m_bNumberLines = show;
    m_nLineNumberRestart = restart;
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

inline void	XFParaStyle::SetAlignType(enumXFAlignType eAlign)
{
    m_eAlignType = eAlign;
}

inline void XFParaStyle::SetLastLineAlign(enumXFAlignType align, sal_Bool	bJustSingleWord)
{
    m_eLastLineAlign = align;
    m_bJustSingleWord = bJustSingleWord;
}

inline void XFParaStyle::SetKeepWithNext(sal_Bool keep)
{
    m_bKeepWithNext = keep;
}

inline void XFParaStyle::ClearTabStyles()
{
    m_aTabs.Reset();
}

inline 	rtl::OUString XFParaStyle::GetMasterPage()
{
    return m_strMasterPage;
}

inline enumXFAlignType XFParaStyle::GetAlighType()
{
    return m_eAlignType;
}

class XFDefaultParaStyle : public XFStyle
{
public:
    XFDefaultParaStyle();
    void SetTabDistance(double len);
    enumXFStyle	GetStyleFamily();
    virtual void	ToXml(IXFStream *pStrm);
private:
    double m_fTabDistance;
};

inline void XFDefaultParaStyle::SetTabDistance(double len)
{
    m_fTabDistance = len;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
