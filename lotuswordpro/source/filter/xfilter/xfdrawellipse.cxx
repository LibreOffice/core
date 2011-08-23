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
 * Eclipse object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-2-18 create this file.
 ************************************************************************/
#include	"xfdrawellipse.hxx"
#include	<cmath>

XFDrawEllipse::XFDrawEllipse()
{
    m_fRadiusX = 0;
    m_fRadiusY = 0;
    m_fStartAngle = 0;
    m_fEndAngle = 360;
    m_eDrawKind = enumXFDrawKindFull;
    m_bUseAngle = sal_False;
}

void XFDrawEllipse::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    //not support yet.
    if( m_bUseAngle )
    {
        double	x1 = m_aCenterPoint.GetX() + m_fRadiusX*cos(m_fStartAngle*2*PI/360);
        double  x2 = m_aCenterPoint.GetX() + m_fRadiusX*cos(m_fEndAngle*2*PI/360);
        double  y1 = m_aCenterPoint.GetY() + m_fRadiusY*sin(m_fStartAngle*2*PI/360);
        double  y2 = m_aCenterPoint.GetY() + m_fRadiusY*sin(m_fEndAngle*2*PI/360);
        double	width;
        double	height;

        double x11 = MIN3(m_aCenterPoint.GetX()-m_fRadiusX,x1,x2);
        double x12 = MAX3(m_aCenterPoint.GetX()+m_fRadiusX,x1,x2);
        double y11 = MIN3(m_aCenterPoint.GetY()-m_fRadiusY,y1,y2);
        double y12 = MAX3(m_aCenterPoint.GetY()+m_fRadiusY,y1,y2);
        width = x12-x11;
        height = y12-y11;
        SetPosition(x11,y11,width,height);

        //
        pAttrList->AddAttribute( A2OUSTR("draw:kind"), GetDrawKind(m_eDrawKind) );
        pAttrList->AddAttribute( A2OUSTR("draw:start-angle"), DoubleToOUString(m_fStartAngle) );
        pAttrList->AddAttribute( A2OUSTR("draw:end-angle"), DoubleToOUString(m_fEndAngle) );
    }
    else
    {
        double	x = m_aCenterPoint.GetX() - m_fRadiusX;
        double  y = m_aCenterPoint.GetY() - m_fRadiusY;
        double	width = m_fRadiusX*2;
        double	height = m_fRadiusY*2;
        SetPosition(x,y,width,height);
    }

    XFDrawObject::ToXml(pStrm);

    pStrm->StartElement( A2OUSTR("draw:ellipse") );
    ContentToXml(pStrm);
    pStrm->EndElement( A2OUSTR("draw:ellipse") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
