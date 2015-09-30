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
 * Columns styles for section, or paragraph.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFCOLUMNS_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFCOLUMNS_HXX

#include "xfglobal.hxx"
#include "xfcolor.hxx"
#include <vector>

#define     XFCOLUMNS_FLAG_SEPARATOR    0X00000001
#define     XFCOLUMNS_FLAG_GAP          0X00000010

/**
 * @descr   Columns object for page or section.
 */
class XFColumn
{
public:
    XFColumn()
    {
        m_nRelWidth = 0;
        m_fMarginLeft = 0;
        m_fMarginRight = 0;
    }

public:
    /**
     * @descr   Set column relative width.
     */
    void    SetRelWidth(sal_Int32 width);

    /**
     * @descr   Set column margin left and margin right.
     */
    void    SetMargins(double left, double right);

    /**
     * @descr   output column style as an xml node.
     */
    void ToXml(IXFStream *pStrm);

    friend class XFColumns;

private:
    sal_Int32   m_nRelWidth;
    double      m_fMarginLeft;
    double      m_fMarginRight;
};

class XFColumnSep
{
public:
    XFColumnSep()
    {
        m_nRelHeight = 100;
        m_fWidth = 0;
        m_eVertAlign = enumXFAlignMiddle;
    }
public:
    void    SetRelHeight(sal_Int32 height);
    void    SetWidth(double width);
    void    SetColor(XFColor& color);
    void    SetVerticalAlign(enumXFAlignType align);

    void    ToXml(IXFStream *pStrm);

    friend class XFColumns;
private:
    sal_Int32   m_nRelHeight;
    double      m_fWidth;
    XFColor     m_aColor;
    enumXFAlignType m_eVertAlign;
};

class XFColumns
{
public:
    XFColumns()
        : m_nFlag(0)
        , m_nCount(0)
        , m_fGap(0.0)
        { }
public:
    void    SetSeparator(XFColumnSep& aSeparator);
    void    SetGap(double fGap);
    void    SetCount(sal_uInt16 nCount);
    void    AddColumn(XFColumn& column);

    void    ToXml(IXFStream *pStrm);
private:
    sal_Int32   m_nFlag;
    sal_uInt16  m_nCount;
    double      m_fGap;
    std::vector<XFColumn>   m_aColumns;
    XFColumnSep m_aSeparator;

};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
