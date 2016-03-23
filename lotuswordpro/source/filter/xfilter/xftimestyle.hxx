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
 * Time style. The TIme format for time field.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFTIMESTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFTIMESTYLE_HXX

#include "xfglobal.hxx"
#include "xfstyle.hxx"
#include <vector>

class XFDateTimePart : public XFStyle
{
public:
    XFDateTimePart()
    {
        m_ePart = enumXFDateUnknown;
        m_bLongFmt = false;
    }

    virtual ~XFDateTimePart(){}
public:
    void    SetPartType(enumXFDatePart ePart);

    void    SetLongFmt(bool bLongFmt);

    void    SetText(const OUString& text);

protected:
    enumXFDatePart  m_ePart;
    bool        m_bLongFmt;
    OUString   m_strText;
};

class XFTimePart : public XFDateTimePart
{
public:
    XFTimePart();

    virtual ~XFTimePart(){}
public:
    void    SetDecimalPos(sal_Int32 pos);

    virtual void    ToXml(IXFStream *pStrm) override;

protected:
    sal_Int32       m_nDecimalPos;
};

class XFTimeStyle : public XFStyle
{
public:
    XFTimeStyle();

    virtual ~XFTimeStyle();

public:

    void    AddHour( bool bLongFmt = true );

    void    AddMinute( bool bLongFmt = true );

    void    AddSecond( bool bLongFmt = true, sal_Int16 nDecimalPos = 2);

    void    SetAmPm(bool bAmPm);

    void    AddText( const OUString& part );

    virtual enumXFStyle GetStyleFamily() override;

    virtual void    ToXml(IXFStream *pStrm) override;

    void    SetTruncate(bool bTrunc);

private:
    bool    m_bFixed;
    bool    m_bAmPm;
    std::vector<XFTimePart> m_aParts;
    bool    m_bTruncate;
};

inline void XFDateTimePart::SetPartType(enumXFDatePart ePart)
{
    m_ePart = ePart;
}

inline void XFDateTimePart::SetLongFmt(bool bLongFmt)
{
    m_bLongFmt = bLongFmt;
}

inline void XFDateTimePart::SetText(const OUString& text)
{
    m_strText = text;
}

inline void XFTimePart::SetDecimalPos(sal_Int32 pos)
{
    m_nDecimalPos = pos;
}

inline void XFTimeStyle::AddHour( bool bLongFmt )
{
    XFTimePart  part;
    part.SetPartType(enumXFDateHour);
    part.SetLongFmt(bLongFmt);
    m_aParts.push_back(part);
}

inline void XFTimeStyle::AddMinute( bool bLongFmt )
{
    XFTimePart  part;
    part.SetPartType(enumXFDateMinute);
    part.SetLongFmt(bLongFmt);
    m_aParts.push_back(part);
}

inline void XFTimeStyle::AddSecond( bool bLongFmt, sal_Int16 pos)
{
    XFTimePart  part;
    part.SetPartType(enumXFDateSecond);
    part.SetLongFmt(bLongFmt);
    part.SetDecimalPos(pos);
    m_aParts.push_back(part);
}

inline void XFTimeStyle::SetAmPm(bool bAmPm)
{
    m_bAmPm = bAmPm;
}

inline void XFTimeStyle::AddText( const OUString& text )
{
    XFTimePart  part;
    part.SetPartType(enumXFDateText);
    part.SetText(text);
    m_aParts.push_back(part);
}

inline  void    XFTimeStyle::SetTruncate(bool bTrunc)
{
    m_bTruncate = bTrunc;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
