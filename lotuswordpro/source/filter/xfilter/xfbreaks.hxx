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
 * Breaks of paragraph.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-27 create this file.
 ************************************************************************/
#ifndef		_XFBREAKS_HXX
#define		_XFBREAKS_HXX

#include	"xfglobal.hxx"
#include	"ixfproperty.hxx"

/**
 * @descr	Page|Column break object.
 */
class XFBreaks : public IXFProperty
{
public:
    XFBreaks();

    XFBreaks(enumXFBreaks breaks);

public:
    /**
     * @descr	Set break type, pls refer to enumXFBreaks.
     */
    void	SetBreakType(enumXFBreaks breaks);

    /**
     * @descr	Output breaks object.
     */
    virtual void	ToXml(IXFStream *pStrm);

    friend bool operator==(XFBreaks& b1, XFBreaks& b2);
    friend bool operator!=(XFBreaks& b1, XFBreaks& b2);

private:
    enumXFBreaks	m_eBreaks;
};

inline XFBreaks::XFBreaks()
{
    m_eBreaks = enumXFBreakAuto;
}

inline XFBreaks::XFBreaks(enumXFBreaks breaks):m_eBreaks(breaks)
{
}

inline void	XFBreaks::SetBreakType(enumXFBreaks breaks)
{
    m_eBreaks = breaks;
}

inline void	XFBreaks::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    switch(m_eBreaks)
    {
    case enumXFBreakAuto:
        break;
    case enumXFBreakBefPage:
        pAttrList->AddAttribute( A2OUSTR("fo:break-before"), A2OUSTR("page") );
        break;
    case enumXFBreakBefColumn:
        pAttrList->AddAttribute( A2OUSTR("fo:break-before"), A2OUSTR("column") );
        break;
    case enumXFBreakAftPage:
        pAttrList->AddAttribute( A2OUSTR("fo:break-after"), A2OUSTR("page") );
        break;
    case enumXFBreakAftColumn:
        pAttrList->AddAttribute( A2OUSTR("fo:break-after"), A2OUSTR("column") );
        break;
    case enumXFBreakKeepWithNext:
        pAttrList->AddAttribute( A2OUSTR("fo:keep-with-next"), A2OUSTR("true") );
        break;
    }
}

inline bool operator==(XFBreaks& b1, XFBreaks& b2)
{
    return b1.m_eBreaks == b2.m_eBreaks;
}

inline bool operator!=(XFBreaks& b1, XFBreaks& b2)
{
    return !(b1==b2);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
