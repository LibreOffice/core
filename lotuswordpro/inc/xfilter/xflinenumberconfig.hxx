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
 * Line numbering config for the full document.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFLINENUMBERCONFIG_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFLINENUMBERCONFIG_HXX

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfstyle.hxx>

class XFLineNumberConfig : public XFStyle
{
public:
    XFLineNumberConfig();

public:
    void    SetNumberOffset(double offset);

    void    SetNumberPosition(enumXFLineNumberPos pos);

    void    SetNumberIncrement(sal_Int32 increment);

    void    SetRestartOnPage(bool restart = true);

    void    SetCountEmptyLines(bool empty = true);

    virtual void ToXml(IXFStream *pStrm) override;

private:
    enumXFLineNumberPos m_ePosition;
    double     m_fOffset;
    sal_Int32  m_nIncrement;
    static const sal_Int32  m_nSepIncrement = 3;
    OUString   m_strSeparator;
    OUString   m_strNumFmt;
    OUString   m_strTextStyle;
    bool       m_bRestartOnPage;
    bool       m_bCountEmptyLines;
};

inline XFLineNumberConfig::XFLineNumberConfig()
    : m_ePosition(enumXFLineNumberLeft)
    , m_fOffset(0)
    , m_nIncrement(5)
    , m_bRestartOnPage(false)
    , m_bCountEmptyLines(true)
{}

inline void XFLineNumberConfig::SetNumberOffset(double offset)
{
    m_fOffset = offset;
}

inline void XFLineNumberConfig::SetNumberPosition(enumXFLineNumberPos   position)
{
    m_ePosition = position;
}

inline void XFLineNumberConfig::SetNumberIncrement(sal_Int32 increment)
{
    m_nIncrement = increment;
}

inline void XFLineNumberConfig::SetRestartOnPage(bool restart)
{
    m_bRestartOnPage = restart;
}

inline void XFLineNumberConfig::SetCountEmptyLines(bool empty)
{
    m_bCountEmptyLines = empty;
}

inline void XFLineNumberConfig::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( !m_strTextStyle.isEmpty() )
        pAttrList->AddAttribute( u"text:style-name"_ustr, m_strTextStyle );
    pAttrList->AddAttribute( u"text:offset"_ustr, OUString::number(m_fOffset) + "cm" );
    pAttrList->AddAttribute( u"style:num-format"_ustr, m_strNumFmt );
    //position:
    if( m_ePosition == enumXFLineNumberLeft )
        pAttrList->AddAttribute( u"text:number-position"_ustr, u"left"_ustr );
    else if( m_ePosition == enumXFLineNumberRight )
        pAttrList->AddAttribute( u"text:number-position"_ustr, u"right"_ustr );
    else if( m_ePosition == enumXFLineNumberInner )
        pAttrList->AddAttribute( u"text:number-position"_ustr, u"inner"_ustr );
    else if( m_ePosition == enumXFLineNumberOuter )
        pAttrList->AddAttribute( u"text:number-position"_ustr, u"outer"_ustr );

    pAttrList->AddAttribute( u"text:increment"_ustr, OUString::number(m_nIncrement) );

    if( m_bRestartOnPage )
        pAttrList->AddAttribute( u"text:restart-on-page"_ustr, u"true"_ustr );
    else
        pAttrList->AddAttribute( u"text:restart-on-page"_ustr, u"false"_ustr );

    if( m_bCountEmptyLines )
        pAttrList->AddAttribute( u"text:count-empty-lines"_ustr, u"true"_ustr );
    else
        pAttrList->AddAttribute( u"text:count-empty-lines"_ustr, u"false"_ustr );

    pAttrList->AddAttribute( u"text:count-in-floating-frames"_ustr, u"false"_ustr );

    pStrm->StartElement( u"text:linenumbering-configuration"_ustr );

    pAttrList->Clear();
    pAttrList->AddAttribute( u"text:increment"_ustr, OUString::number(m_nSepIncrement) );
    pStrm->StartElement( u"text:linenumbering-separator"_ustr );
    pStrm->Characters(m_strSeparator);
    pStrm->EndElement( u"text:linenumbering-separator"_ustr );

    pStrm->EndElement( u"text:linenumbering-configuration"_ustr );
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
