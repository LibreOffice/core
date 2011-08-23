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
 * Date style. The date format for date field.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-20 create this file.
 ************************************************************************/
#ifndef		_XFDATESTYLE_HXX
#define		_XFDATESTYLE_HXX

#include	"xfglobal.hxx"
#include	"xfstyle.hxx"
#include	"ixfproperty.hxx"
#include	"xftimestyle.hxx"
#include	"xfstylecont.hxx"
#include	<vector>

class XFDatePart : public XFTimePart
{
public:
    XFDatePart();
public:
    void	SetTexture(sal_Bool bTexture);

    virtual void	ToXml(IXFStream *pStrm);

private:
    sal_Bool		m_bTexture;
};

class XFDateStyle : public XFStyle
{
public:
    XFDateStyle();

    virtual ~XFDateStyle();

public:

    void	SetFixed(sal_Bool fixed);

    void	SetLanguage(rtl::OUString lang);

    void	SetCountry(rtl::OUString country);

    void	SetAutoOrder(sal_Bool bAutoOrder);

    void	AddYear( sal_Bool bLongFmt = sal_True );

    void	AddMonth( sal_Bool bLongFmt = sal_True, sal_Bool bTexture = sal_False  );

    void	AddMonthDay( sal_Bool bLongFmt = sal_True );

    void	AddWeekDay( sal_Bool bLongFmt = sal_True );

    void	AddYearWeek();

    void	AddEra(sal_Bool bLongFmt = sal_False);

    void	AddQuarter(sal_Bool bLongFmt = sal_False);

    void	AddHour( sal_Bool bLongFmt = sal_True );

    void	AddMinute( sal_Bool bLongFmt = sal_True );

    void	AddSecond( sal_Bool bLongFmt = sal_True, sal_Int16 nDecimalPos = 2);

    void	AddAmPm(sal_Bool bAmPm);

    void	AddText( rtl::OUString part );

    virtual enumXFStyle GetStyleFamily();

    virtual void	ToXml(IXFStream *pStrm);

private:
    sal_Bool	m_bFixed;
    sal_Bool	m_bAutoOrder;
    rtl::OUString	m_strLanguage;
    rtl::OUString	m_strCountry;
    XFStyleContainer	m_aParts;
};

inline void	XFDatePart::SetTexture(sal_Bool bTexture)
{
    m_bTexture = bTexture;
}

inline void	XFDateStyle::SetFixed(sal_Bool fixed)
{
    m_bFixed = fixed;
}

inline void	XFDateStyle::SetLanguage(rtl::OUString lang)
{
    m_strLanguage = lang;
}

inline void	XFDateStyle::SetCountry(rtl::OUString country)
{
    m_strCountry = country;
}

inline void	XFDateStyle::SetAutoOrder(sal_Bool bAutoOrder)
{
    m_bAutoOrder = bAutoOrder;
}

inline void	XFDateStyle::AddYear( sal_Bool bLongFmt )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateYear);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddMonth( sal_Bool bLongFmt, sal_Bool bTexture  )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateMonth);
    part->SetLongFmt(bLongFmt);
    part->SetTexture(bTexture);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddMonthDay( sal_Bool bLongFmt )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateMonthDay);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddWeekDay( sal_Bool bLongFmt )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateWeekDay);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddYearWeek()
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateYearWeek);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddEra(sal_Bool bLongFmt)
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateEra);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddQuarter(sal_Bool bLongFmt)
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateQuarter);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddText( rtl::OUString text )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateText);
    part->SetText(text);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddHour( sal_Bool bLongFmt )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateHour);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddMinute( sal_Bool bLongFmt )
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateMinute);
    part->SetLongFmt(bLongFmt);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddSecond( sal_Bool bLongFmt, sal_Int16 pos)
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateSecond);
    part->SetLongFmt(bLongFmt);
    part->SetDecimalPos(pos);
    m_aParts.AddStyle(part);
}

inline void	XFDateStyle::AddAmPm(sal_Bool /*bAmPm*/)
{
    XFDatePart	*part = new XFDatePart();
    part->SetPartType(enumXFDateAmPm);
    m_aParts.AddStyle(part);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
