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
/*************************************************************************
 * Change History
 * 2004-2-18 create this file.
 ************************************************************************/
#include	"xfdrawpolyline.hxx"

XFDrawPolyline::XFDrawPolyline()
{
}

void XFDrawPolyline::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    std::vector<XFPoint>::iterator it;

    pAttrList->Clear();
    //view-box:
    XFRect	rect = CalcViewBox();
    rtl::OUString strViewBox = A2OUSTR("0 0 ");
    strViewBox += DoubleToOUString(rect.GetWidth()*1000) + A2OUSTR(" ");
    strViewBox += DoubleToOUString(rect.GetHeight()*1000);
    pAttrList->AddAttribute( A2OUSTR("svg:viewBox"), strViewBox);

    //points
    rtl::OUString	strPoints;
    for( it = m_aPoints.begin(); it != m_aPoints.end(); it++ )
    {
        XFPoint pt = *it;
        double	x = (pt.GetX()-rect.GetX())*1000;
        double	y = (pt.GetY()-rect.GetY())*1000;
        strPoints += DoubleToOUString(x) + A2OUSTR(",") + DoubleToOUString(y) + A2OUSTR(" ");
    }
    strPoints = strPoints.trim();
    pAttrList->AddAttribute( A2OUSTR("draw:points"), strPoints);

    //
    SetPosition(rect.GetX(),rect.GetY(),rect.GetWidth(),rect.GetHeight());
    XFDrawObject::ToXml(pStrm);

    pStrm->StartElement( A2OUSTR("draw:polyline") );
    ContentToXml(pStrm);
    pStrm->EndElement( A2OUSTR("draw:polyline") );
}

XFRect	XFDrawPolyline::CalcViewBox()
{
    double	x1 = 0;
    double	y1 = 0;
    double	x2 = 0;
    double	y2 = 0;
    XFPoint	aPoint;
    std::vector<XFPoint>::iterator it = m_aPoints.begin();

    assert(m_aPoints.size()>0);

    aPoint = *it;
    x1 = aPoint.GetX();
    x2 = aPoint.GetX();
    y1 = aPoint.GetY();
    y2 = aPoint.GetY();
    for( ; it != m_aPoints.end(); it++ )
    {
        aPoint = *it;
        if( x1>aPoint.GetX() )
            x1 = aPoint.GetX();
        if( x2<aPoint.GetX() )
            x2 = aPoint.GetX();

        if( y1>aPoint.GetY() )
            y1 = aPoint.GetY();
        if( y2<aPoint.GetY() )
            y2 = aPoint.GetY();
    }
    return XFRect(x1,y1,x2-x1,y2-y1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
