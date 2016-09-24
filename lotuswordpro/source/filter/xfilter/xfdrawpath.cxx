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
 * Draw path object.
 ************************************************************************/
#include "xfdrawpath.hxx"

XFSvgPathEntry::XFSvgPathEntry()
{
}

OUString XFSvgPathEntry::ToString()
{
    assert(!m_strCommand.isEmpty());
    OUString str = m_strCommand;
    std::vector<XFPoint>::iterator it;

    for( it = m_aPoints.begin(); it != m_aPoints.end(); ++it )
    {
        XFPoint aPt= *it;
        str += OUString::number(aPt.GetX()*1000) + " " + OUString::number(aPt.GetY()*1000) + " ";
    }
    return str.trim();
}

XFDrawPath::XFDrawPath()
{
}

void    XFDrawPath::MoveTo(XFPoint pt)
{
    XFSvgPathEntry  entry;

    entry.SetCommand("M");
    entry.AddPoint(pt);
    m_aPaths.push_back(entry);
}

void    XFDrawPath::LineTo(XFPoint pt)
{
    XFSvgPathEntry  entry;

    entry.SetCommand("L");
    entry.AddPoint(pt);
    m_aPaths.push_back(entry);
}

void    XFDrawPath::CurveTo(XFPoint dest, XFPoint ctrl1, XFPoint ctrl2)
{
    XFSvgPathEntry  entry;

    entry.SetCommand("C");
    entry.AddPoint(ctrl1);
    entry.AddPoint(ctrl2);
    entry.AddPoint(dest);

    m_aPaths.push_back(entry);
}

void    XFDrawPath::ClosePath()
{
    XFSvgPathEntry  entry;

    entry.SetCommand("Z");

    m_aPaths.push_back(entry);
}

void    XFDrawPath::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    //view-box:
    XFRect  rect = m_aRect;

    OUString strViewBox = "0 0 ";
    strViewBox += OUString::number(rect.GetWidth()*1000) + " ";
    strViewBox += OUString::number(rect.GetHeight()*1000);
    pAttrList->AddAttribute( "svg:viewBox", strViewBox);

    //points
    OUString   strPath;
    std::vector<XFSvgPathEntry>::iterator it;
    for( it = m_aPaths.begin(); it != m_aPaths.end(); ++it )
    {
        XFSvgPathEntry  aSvg = *it;
        strPath += aSvg.ToString();
    }
    strPath = strPath.trim();
    pAttrList->AddAttribute( "svg:d", strPath);

    SetPosition(rect);
    XFDrawObject::ToXml(pStrm);

    pStrm->StartElement( "draw:path" );
    ContentToXml(pStrm);
    pStrm->EndElement( "draw:path" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
