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
 * Polyline.
 ************************************************************************/
#include <xfilter/xfdrawpolyline.hxx>
#include <xfilter/ixfattrlist.hxx>
#include <rtl/ustrbuf.hxx>

XFDrawPolyline::XFDrawPolyline()
{
}

void XFDrawPolyline::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    //view-box:
    XFRect  rect = CalcViewBox();
    OUString strViewBox = "0 0 " +
        OUString::number(rect.GetWidth()*1000) + " " +
        OUString::number(rect.GetHeight()*1000);
    pAttrList->AddAttribute( "svg:viewBox", strViewBox);

    //points
    OUStringBuffer strPoints;
    for (auto const& point : m_aPoints)
    {
        double  x = (point.GetX()-rect.GetX())*1000;
        double  y = (point.GetY()-rect.GetY())*1000;
        strPoints.append(OUString::number(x)).append(",").append(OUString::number(y)).append(" ");
    }
    strPoints.stripEnd(' ');
    pAttrList->AddAttribute( "draw:points", strPoints.makeStringAndClear());

    SetPosition(rect.GetX(),rect.GetY(),rect.GetWidth(),rect.GetHeight());
    XFDrawObject::ToXml(pStrm);

    pStrm->StartElement( "draw:polyline" );
    ContentToXml(pStrm);
    pStrm->EndElement( "draw:polyline" );
}

XFRect  XFDrawPolyline::CalcViewBox()
{
    double  x1 = 0;
    double  y1 = 0;
    double  x2 = 0;
    double  y2 = 0;
    XFPoint aPoint;

    assert(!m_aPoints.empty());

    x1 = aPoint.GetX();
    x2 = aPoint.GetX();
    y1 = aPoint.GetY();
    y2 = aPoint.GetY();
    for (auto const& point : m_aPoints)
    {
        if( x1>point.GetX() )
            x1 = point.GetX();
        if( x2<point.GetX() )
            x2 = point.GetX();

        if( y1>point.GetY() )
            y1 = point.GetY();
        if( y2<point.GetY() )
            y2 = point.GetY();
    }
    return XFRect(x1,y1,x2-x1,y2-y1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
