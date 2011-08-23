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
/*************************************************************************
 * Change History
 * 2005-03-23 create this file.
 ************************************************************************/
#ifndef		_XFLINENUMBERCONFIG_HXX
#define		_XFLINENUMBERCONFIG_HXX

#include	"xfstyle.hxx"

class XFLineNumberConfig : public XFStyle
{
public:
    XFLineNumberConfig();

public:
    void	SetNumberOffset(double offset);

    void	SetNumberPosition(enumXFLineNumberPos pos);

    void	SetNumberIncrement(sal_Int32 increment);

    void	SetSeperator(sal_Int32 increment, rtl::OUString seperator);

    void	SetNumberFormat(rtl::OUString numfmt = A2OUSTR("1"));

    void	SetTextStyle(rtl::OUString style);

    void	SetRestartOnPage(sal_Bool restart = sal_True);

    void	SetCountEmptyLines(sal_Bool empty = sal_True);

    void	SetCountFrameLines(sal_Bool frame = sal_True);

    virtual void ToXml(IXFStream *pStrm);

private:
    enumXFLineNumberPos	m_ePosition;
    double		m_fOffset;
    sal_Int32	m_nIncrement;
    sal_Int32	m_nSepIncrement;
    rtl::OUString	m_strSeparator;
    rtl::OUString	m_strNumFmt;
    rtl::OUString	m_strTextStyle;
    sal_Bool	m_bRestartOnPage;
    sal_Bool	m_bCountEmptyLines;
    sal_Bool	m_bCountFrameLines;
};

inline XFLineNumberConfig::XFLineNumberConfig()
{
    m_fOffset = 0;
    m_nIncrement = 5;
    m_nSepIncrement = 3;
    m_bRestartOnPage = sal_False;
    m_bCountEmptyLines = sal_True;
    m_bCountFrameLines = sal_False;
}

inline void XFLineNumberConfig::SetNumberOffset(double offset)
{
    m_fOffset = offset;
}

inline void XFLineNumberConfig::SetNumberPosition(enumXFLineNumberPos	position)
{
    m_ePosition = position;
}

inline void XFLineNumberConfig::SetNumberIncrement(sal_Int32 increment)
{
    m_nIncrement = increment;
}

inline void XFLineNumberConfig::SetSeperator(sal_Int32 increment, rtl::OUString seperator)
{
    m_nSepIncrement = increment;
    m_strSeparator = seperator;
}

inline void XFLineNumberConfig::SetNumberFormat(rtl::OUString numfmt)
{
    m_strNumFmt = numfmt;
}

inline void XFLineNumberConfig::SetTextStyle(rtl::OUString style)
{
    m_strTextStyle = style;
}

inline void XFLineNumberConfig::SetRestartOnPage(sal_Bool restart)
{
    m_bRestartOnPage = restart;
}

inline void XFLineNumberConfig::SetCountEmptyLines(sal_Bool empty)
{
    m_bCountEmptyLines = empty;
}

inline void XFLineNumberConfig::SetCountFrameLines(sal_Bool frame)
{
    m_bCountFrameLines = frame;
}

inline void XFLineNumberConfig::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( m_strTextStyle.getLength() > 0 )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), m_strTextStyle );
    pAttrList->AddAttribute( A2OUSTR("text:offset"), DoubleToOUString(m_fOffset) + A2OUSTR("cm") );
    pAttrList->AddAttribute( A2OUSTR("style:num-format"), m_strNumFmt );
    //position:
    if( m_ePosition == enumXFLineNumberLeft )
        pAttrList->AddAttribute( A2OUSTR("text:number-position"), A2OUSTR("left") );
    else if( m_ePosition == enumXFLineNumberRight )
        pAttrList->AddAttribute( A2OUSTR("text:number-position"), A2OUSTR("right") );
    else if( m_ePosition == enumXFLineNumberInner )
        pAttrList->AddAttribute( A2OUSTR("text:number-position"), A2OUSTR("inner") );
    else if( m_ePosition == enumXFLineNumberOutter )
        pAttrList->AddAttribute( A2OUSTR("text:number-position"), A2OUSTR("outter") );

    pAttrList->AddAttribute( A2OUSTR("text:increment"), Int32ToOUString(m_nIncrement) );

    if( m_bRestartOnPage )
        pAttrList->AddAttribute( A2OUSTR("text:restart-on-page"), A2OUSTR("true") );
    else
        pAttrList->AddAttribute( A2OUSTR("text:restart-on-page"), A2OUSTR("false") );

    if( m_bCountEmptyLines )
        pAttrList->AddAttribute( A2OUSTR("text:count-empty-lines"), A2OUSTR("true") );
    else
        pAttrList->AddAttribute( A2OUSTR("text:count-empty-lines"), A2OUSTR("false") );

    if( m_bCountFrameLines )
        pAttrList->AddAttribute( A2OUSTR("text:count-in-floating-frames"), A2OUSTR("true") );
    else
        pAttrList->AddAttribute( A2OUSTR("text:count-in-floating-frames"), A2OUSTR("false") );


    pStrm->StartElement( A2OUSTR("text:linenumbering-configuration") );

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("text:increment"), Int32ToOUString(m_nSepIncrement) );
    pStrm->StartElement( A2OUSTR("text:linenumbering-separator") );
    pStrm->Characters(m_strSeparator);
    pStrm->EndElement( A2OUSTR("text:linenumbering-separator") );

    pStrm->EndElement( A2OUSTR("text:linenumbering-configuration") );
}


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
