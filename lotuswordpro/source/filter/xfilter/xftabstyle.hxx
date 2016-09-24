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
 * Tab style for paragraph.include tab offset and tab char properties.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFTABSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFTABSTYLE_HXX

#include "xfglobal.hxx"
#include "xfstyle.hxx"
#include <vector>

class XFTabStyle : public XFStyle
{
public:
    XFTabStyle();

public:
    void    SetTabType(enumXFTab type);

    void    SetLength(double len);

    void    SetDelimiter(sal_Unicode delimiter);

    void    SetLeaderChar(sal_Unicode leader);

    virtual void    ToXml(IXFStream *pStrm) override;

private:
    enumXFTab   m_eType;
    double      m_fLength;
    OUString   m_strDelimiter;
    OUString   m_strLeader;
};

inline void XFTabStyle::SetTabType(enumXFTab type)
{
    m_eType = type;
}

inline void XFTabStyle::SetLength(double len)
{
    m_fLength = len;
}

inline void XFTabStyle::SetDelimiter(sal_Unicode delimiter)
{
    sal_Unicode     chs[2];
    chs[0] = delimiter;
    chs[1] = 0;

    m_strDelimiter = OUString(chs);
}

inline void XFTabStyle::SetLeaderChar(sal_Unicode leader)
{
    sal_Unicode     chs[2];
    chs[0] = leader;
    chs[1] = 0;

    m_strLeader = OUString(chs);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
