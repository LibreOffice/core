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
 * Drop cap of a paragraph.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFDROPCAP_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFDROPCAP_HXX

#include "xfglobal.hxx"

class XFDropcap
{
public:
    XFDropcap()
    {
        Reset();
    }

public:
    void    SetCharCount(sal_Int32 count);

    void    SetLines(sal_Int32 lines);

    void    SetDistance(double distance);

    void    Reset();

    void    ToXml(IXFStream *pStrm);

    friend bool operator==(XFDropcap& dc1, XFDropcap& dc2);
    friend bool operator!=(XFDropcap& dc1, XFDropcap& dc2);
    friend class XFParaStyle;
private:
    sal_Int32       m_nCharCount;
    sal_Int32       m_nLines;
    double          m_fDistance;
    OUString   m_strStyleName;
};

inline void XFDropcap::SetCharCount(sal_Int32 count)
{
    m_nCharCount = count;
}

inline void XFDropcap::SetLines(sal_Int32 lines)
{
    m_nLines = lines;
}

inline void XFDropcap::SetDistance(double distance)
{
    m_fDistance = distance;
}

inline void XFDropcap::Reset()
{
    m_nCharCount = 0;
    m_nLines = 0;
    m_fDistance = 0;
    m_strStyleName.clear();
}

inline void XFDropcap::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    if( m_nCharCount < 1 || m_nLines < 2 )
        return;
    pAttrList->AddAttribute("style:length", OUString::number(m_nCharCount) );
    pAttrList->AddAttribute("style:lines", OUString::number(m_nLines) );
    pAttrList->AddAttribute("style:distance", OUString::number(m_fDistance) + "cm" );
    if( !m_strStyleName.isEmpty() )
        pAttrList->AddAttribute("style:style-name", m_strStyleName );
    pStrm->StartElement( "style:drop-cap" );
    pStrm->EndElement( "style:drop-cap" );
}

inline bool operator==(XFDropcap& dc1, XFDropcap& dc2)
{
    return (
        (dc1.m_nCharCount == dc2.m_nCharCount)&&
        (dc1.m_nLines == dc2.m_nLines)&&
        (dc1.m_fDistance == dc2.m_fDistance)&&
        (dc1.m_strStyleName == dc2.m_strStyleName)
        );
}

inline bool operator!=(XFDropcap& dc1, XFDropcap& dc2)
{
    return !(dc1==dc2);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
