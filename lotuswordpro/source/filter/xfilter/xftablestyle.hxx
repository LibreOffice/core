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
 * Table style.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFTABLESTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFTABLESTYLE_HXX

#include "xfstyle.hxx"
#include "xfmargins.hxx"
#include "xfshadow.hxx"
#include "xfcolor.hxx"
#include "xfbreaks.hxx"

class XFBGImage;
class XFTableStyle : public XFStyle
{
public:
    XFTableStyle();

    virtual ~XFTableStyle();

public:
    void    SetWidth(double width);

    void    SetAlign(enumXFAlignType eAlign, double offset = 0);

    void    SetShadow(enumXFShadowPos pos, double offset = 0.18, XFColor color=XFColor(128,128,0) );

    void    SetBackColor(XFColor& color);

    void    SetBackImage(XFBGImage *pImage);

    virtual void    ToXml(IXFStream *pStrm) override;

    virtual enumXFStyle GetStyleFamily() override;

private:
    double  m_fWidth;
    XFColor m_aBackColor;
    XFBGImage   *m_pBGImage;
    XFShadow    m_aShadow;
    XFMargins   m_aMargins;
    XFBreaks    m_aBreaks;
    enumXFAlignType m_eAlign;
};

inline void XFTableStyle::SetWidth(double width)
{
    m_fWidth = width;
}

inline void XFTableStyle::SetAlign(enumXFAlignType eAlign, double offset)
{
    m_eAlign = eAlign;
    if( m_eAlign == enumXFAlignStart )
    {
        m_aMargins.SetLeft(offset);
        m_aMargins.SetRight(0);
        m_aMargins.SetTop(0);
        m_aMargins.SetBottom(0);
    }
}

inline void XFTableStyle::SetShadow(enumXFShadowPos pos, double offset, XFColor color)
{
    m_aShadow.SetColor(color);
    m_aShadow.SetOffset(offset);
    m_aShadow.SetPosition(pos);
}

inline void XFTableStyle::SetBackColor(XFColor& color)
{
    m_aBackColor = color;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
