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
/*************************************************************************
 * Change History
 * 2004-2-18 create this file.
 ************************************************************************/
#include	"xfdrawpath.hxx"

XFSvgPathEntry::XFSvgPathEntry()
{
}

XFSvgPathEntry::XFSvgPathEntry(rtl::OUString cmd)
{
    m_strCommand = cmd;
}

rtl::OUString XFSvgPathEntry::ToString()
{
    assert(m_strCommand.getLength()>0);
    rtl::OUString str = m_strCommand;
    std::vector<XFPoint>::iterator it;

    for( it = m_aPoints.begin(); it != m_aPoints.end(); it++ )
    {
        XFPoint aPt= *it;
        str += DoubleToOUString(aPt.GetX()*1000) + A2OUSTR(" ") + DoubleToOUString(aPt.GetY()*1000) + A2OUSTR(" ");
    }
    return str.trim();
}

XFDrawPath::XFDrawPath()
{
}

void	XFDrawPath::MoveTo(XFPoint pt, sal_Bool absPosition)
{
    XFSvgPathEntry	entry;

    if( absPosition )
        entry.SetCommand(A2OUSTR("M"));
    else
        entry.SetCommand(A2OUSTR("m"));
    entry.AddPoint(pt);
    m_aPaths.push_back(entry);
}

void	XFDrawPath::LineTo(XFPoint pt, sal_Bool absPosition)
{
    XFSvgPathEntry	entry;

    if( absPosition )
        entry.SetCommand(A2OUSTR("L"));
    else
        entry.SetCommand(A2OUSTR("l"));
    entry.AddPoint(pt);
    m_aPaths.push_back(entry);
}

void	XFDrawPath::CurveTo(XFPoint dest, XFPoint ctrl1, XFPoint ctrl2, sal_Bool absPosition)
{
    XFSvgPathEntry	entry;

    if( absPosition )
        entry.SetCommand(A2OUSTR("C"));
    else
        entry.SetCommand(A2OUSTR("c"));
    entry.AddPoint(ctrl1);
    entry.AddPoint(ctrl2);
    entry.AddPoint(dest);

    m_aPaths.push_back(entry);
}

void	XFDrawPath::SmoothCurveTo(XFPoint dest, XFPoint ctrl, sal_Bool absPosition)
{
    XFSvgPathEntry	entry;

    if( absPosition )
        entry.SetCommand(A2OUSTR("C"));
    else
        entry.SetCommand(A2OUSTR("c"));
    entry.AddPoint(ctrl);
    entry.AddPoint(dest);

    m_aPaths.push_back(entry);
}

void	XFDrawPath::ClosePath(sal_Bool absPosition)
{
    XFSvgPathEntry	entry;

    if( absPosition )
        entry.SetCommand(A2OUSTR("Z"));
    else
        entry.SetCommand(A2OUSTR("z"));

    m_aPaths.push_back(entry);
}

void	XFDrawPath::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    //view-box:
    XFRect	rect = m_aRect;

    rtl::OUString strViewBox = A2OUSTR("0 0 ");
    strViewBox += DoubleToOUString(rect.GetWidth()*1000) + A2OUSTR(" ");
    strViewBox += DoubleToOUString(rect.GetHeight()*1000);
    pAttrList->AddAttribute( A2OUSTR("svg:viewBox"), strViewBox);

    //points
    rtl::OUString	strPath;
    std::vector<XFSvgPathEntry>::iterator it;
    for( it = m_aPaths.begin(); it != m_aPaths.end(); it++ )
    {
        XFSvgPathEntry	aSvg = *it;
        strPath += aSvg.ToString();
    }
    strPath = strPath.trim();
    pAttrList->AddAttribute( A2OUSTR("svg:d"), strPath);

    SetPosition(rect);
    XFDrawObject::ToXml(pStrm);

    pStrm->StartElement( A2OUSTR("draw:path") );
    ContentToXml(pStrm);
    pStrm->EndElement( A2OUSTR("draw:path") );
}


XFRect	XFDrawPath::CalcViewBox()
{
    double	x1 = 65536;
    double	y1 = 65536;
    double	x2 = -1;	//not quite safe.()
    double	y2 = -1;
    XFSvgPathEntry	aPath;
    XFPoint	aPoint;
    std::vector<XFSvgPathEntry>::iterator itPath = m_aPaths.begin();
    std::vector<XFPoint> points;
    std::vector<XFPoint>::iterator itPoint;
    do{
        aPath = *itPath;
        points = aPath.m_aPoints;

        for( itPoint = points.begin(); itPoint != points.end(); itPoint++ )
        {
            aPoint = *itPoint;
            if( x1>aPoint.GetX() )
                x1 = aPoint.GetX();
            if( x2<aPoint.GetX() )
                x2 = aPoint.GetX();

            if( y1>aPoint.GetY() )
                y1 = aPoint.GetY();
            if( y2<aPoint.GetY() )
                y2 = aPoint.GetY();
        }
        itPath++;
    }while(itPath!=m_aPaths.end());

    return XFRect(x1,y1,x2-x1,y2-y1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
