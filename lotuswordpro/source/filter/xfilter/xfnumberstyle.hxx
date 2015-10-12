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
 * Number style for table cell.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFNUMBERSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFNUMBERSTYLE_HXX

#include "xfstyle.hxx"
#include "xfstylemanager.hxx"
#include "xfcolor.hxx"
#include <rtl/ustring.hxx>

class XFNumberStyle : public XFStyle
{
public:
    XFNumberStyle();

public:
    void    SetDecimalDigits(sal_Int32 decimal);

    void    SetGroup(bool group = true);

    void    SetColor(const XFColor& color);
    XFColor GetColor(){return m_aColor;}

    void    SetPrefix(const OUString& prefix);

    void    SetSurfix(const OUString& surfix);

    void    SetNegativeStyle(const OUString& prefix, const OUString& suffix, const XFColor& color=XFColor(255,0,0));

    void    SetNumberType(enumXFNumberType type);

    virtual enumXFStyle GetStyleFamily() override;

    virtual bool Equal(IXFStyle *pStyle) override;

    virtual void ToXml(IXFStream *pStrm) override;

protected:
    void    ToXml_StartElement(IXFStream *pStrm);

    void    ToXml_EndElement(IXFStream *pStrm);

    void    ToXml_Normal(IXFStream *pStrm);

    void    ToXml_Negative(IXFStream *pStrm);

    void    ToXml_Content(IXFStream *pStrm, bool nagetive);

protected:
    enumXFNumberType    m_eType;
    sal_Int32   m_nDecimalDigits;
    sal_Int32   m_nMinInteger;
    sal_Int32   m_nMinExponent;
    bool    m_bGroup;
    XFColor     m_aColor;
    bool    m_bCurrencySymbolPost;
    OUString   m_strCurrencySymbol;
    OUString   m_strPrefix;
    OUString   m_strSuffix;

    bool    m_bRedIfNegative;
    XFColor     m_aNegativeColor;
    OUString   m_strNegativePrefix;
    OUString   m_strNegativeSuffix;
};

inline void XFNumberStyle::SetDecimalDigits(sal_Int32 decimal)
{
    m_nDecimalDigits = decimal;
}

inline void XFNumberStyle::SetNegativeStyle(const OUString& prefix, const OUString& suffix, const XFColor& color)
{
    m_bRedIfNegative = true;
    m_aNegativeColor = color;
    m_strNegativePrefix = prefix;
    m_strNegativeSuffix = suffix;
}

inline void XFNumberStyle::SetGroup(bool group)
{
    m_bGroup = group;
}

inline void XFNumberStyle::SetColor(const XFColor& color)
{
    m_aColor = color;
}

inline void XFNumberStyle::SetNumberType(enumXFNumberType type)
{
    m_eType = type;
}

inline void XFNumberStyle::SetPrefix(const OUString& prefix)
{
    m_strPrefix = prefix;
}

inline void XFNumberStyle::SetSurfix(const OUString& surfix)
{
    m_strSuffix = surfix;
}



#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
