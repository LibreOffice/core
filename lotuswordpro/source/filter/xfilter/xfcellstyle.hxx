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
 * Table cell style. Number format, string value, and so on...
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFCELLSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFCELLSTYLE_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>

#include "xfstyle.hxx"
#include "xfcolor.hxx"
#include "xfmargins.hxx"
#include "xfpadding.hxx"
#include "xfshadow.hxx"

class XFBorders;
class XFFont;
class XFBorders;
class XFMargins;
class XFBGImage;

/**
 * @descr   Style object for cell.
 */
class XFCellStyle : public XFStyle
{
public:
    XFCellStyle();

    virtual ~XFCellStyle();

public:
    /**
     * @descr   Set cell data format style name.
     */
    void    SetDataStyle(const OUString& style);

    /**
     * @descr:  Set the padding of the paragraph.This is the distance
                between the border and the top of the text.
     * @param:  indent value of the padding.
     */
    void    SetPadding(double left, double right = -1, double top = -1, double bottom = -1);

    /**
     * @descr:  Set alignment property of the cell.
     * @param:  eAlign alignment type,left,right,center or justify.
     */
    void    SetAlignType(enumXFAlignType hori=enumXFAlignNone, enumXFAlignType vert = enumXFAlignBottom);

    /**
     * @descr:  The borders is complex,so you have to create one before use.
                Very few paragraphs will readly have borders property,this way
                we can save much memory.
     * @param:  pBorders borders of the paragraph,please reference the XFBorders.
     */
    void    SetBorders(XFBorders *pBorders);

    /**
     * @descr:  Set background color of the cell.
     * @param:  color value of the back color.
     */
    void    SetBackColor(XFColor& color);

    /**
     * descr:   set cell background image.
     */
    void    SetBackImage(XFBGImage *pImage);

    virtual enumXFStyle GetStyleFamily() override;

    virtual bool Equal(IXFStyle *pStyle) override;

    virtual void ToXml(IXFStream *pStrm) override;

private:
    OUString   m_strDataStyle;
    OUString   m_strParentStyleName;
    enumXFAlignType m_eHoriAlign;
    enumXFAlignType m_eVertAlign;

    double      m_fTextIndent;
    XFColor     m_aBackColor;
    XFBGImage   *m_pBackImage;
    XFMargins   m_aMargin;
    XFPadding   m_aPadding;
    rtl::Reference<XFFont> m_pFont;
    XFShadow    m_aShadow;
    XFBorders   *m_pBorders;
    bool    m_bWrapText;
};

inline void XFCellStyle::SetAlignType(enumXFAlignType hori, enumXFAlignType vert)
{
    m_eHoriAlign = hori;
    m_eVertAlign = vert;
}

inline void XFCellStyle::SetDataStyle(const OUString& style)
{
    m_strDataStyle = style;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
