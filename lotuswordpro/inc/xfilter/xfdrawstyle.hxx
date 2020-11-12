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
 * Style for all draw object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFDRAWSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFDRAWSTYLE_HXX

#include <xfilter/xfstyle.hxx>
#include <xfilter/xfcolor.hxx>
#include <cassert>
#include <memory>

class XFDrawLineStyle;
class XFDrawAreaStyle;
class XFFontWorkStyle;

/**
 * @brief
 * Style for all drawing object, which can includes line style, area style, wrap style.
 * Line style includes line type, line width and line color.
 * Area style can be divided into 5 kinds, color fill, hatch, bitmap, gradient.I only support
 * color fill and hatch now.
 */
class XFDrawStyle : public XFStyle
{
public:
    XFDrawStyle();

    virtual ~XFDrawStyle() override;

public:
    /**
     * @descr   Set drawing object border line.
     */
    void SetLineStyle(double width, XFColor color);

    /**
     * @descr   Set drawing object dash border style.
     */
    void SetLineDashStyle(enumXFLineStyle style, double len1, double len2, double space);

    /**
     * @descr   Set drawing object area fill color.
     */
    void SetAreaColor(XFColor const& color);

    /**
     * @descr   Set drawing object area grid style.
     */
    void SetAreaLineStyle(enumXFAreaLineStyle style, sal_Int32 angle, double space,
                          XFColor lineColor);

    /**
     * @descr   Set drawing object arrow start style,only lines can have arrows.
     */
    void SetArrowStart(const OUString& start, double size);

    /**
     * @descr   Set drawing object arrow end style,only lines can have arrows.
     */
    void SetArrowEnd(const OUString& end, double size);

    void SetFontWorkStyle(enumXFFWStyle eStyle, enumXFFWAdjust eAdjust);

    virtual enumXFStyle GetStyleFamily() override;

    virtual void ToXml(IXFStream* pStrm) override;

private:
    std::unique_ptr<XFFontWorkStyle> m_pFontWorkStyle;
    XFDrawLineStyle* m_pLineStyle;
    XFDrawAreaStyle* m_pAreaStyle;
    OUString m_strArrowStart;
    OUString m_strArrowEnd;
    double m_fArrowStartSize;
    double m_fArrowEndSize;
    bool m_bArrowStartCenter;
    bool m_bArrowEndCenter;
};

inline void XFDrawStyle::SetArrowStart(const OUString& start, double size)
{
    assert(size > 0);
    m_strArrowStart = start;
    m_fArrowStartSize = size;
    m_bArrowStartCenter = true;
}

inline void XFDrawStyle::SetArrowEnd(const OUString& end, double size)
{
    assert(size > 0);
    m_strArrowEnd = end;
    m_fArrowEndSize = size;
    m_bArrowEndCenter = true;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
