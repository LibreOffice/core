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
 * Line style of Drawing object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-02-21 create this file.
 ************************************************************************/
#include	"xfdrawlinestyle.hxx"

XFDrawLineStyle::XFDrawLineStyle()
{
    m_eLineStyle = enumXFLineSolid;

    m_fWidth = 0;
    m_nTransparency = 0;
    m_nNumber1 = 1;
    m_nNumber2 = 1;
    m_fSpace = 0;
    m_fLength1 = 0;
    m_fLength2 = 0;
}

enumXFStyle	XFDrawLineStyle::GetStyleFamily()
{
    return enumXFStyleStrokeDash;
}

void	XFDrawLineStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("draw:name"), GetStyleName());
    //simple place here:
    pAttrList->AddAttribute( A2OUSTR("draw:style"), A2OUSTR("rect"));

    if( m_eLineStyle == enumXFLineDot )
    {

    }
    else if( m_eLineStyle == enumXFLineDash )
    {
        pAttrList->AddAttribute( A2OUSTR("draw:dots1"), Int32ToOUString(m_nNumber1));
        pAttrList->AddAttribute( A2OUSTR(" draw:dots1-length"), DoubleToOUString(m_fLength1) + A2OUSTR("cm"));

        pAttrList->AddAttribute( A2OUSTR("draw:dots2"), Int32ToOUString(m_nNumber2));
        pAttrList->AddAttribute( A2OUSTR(" draw:dots2-length"), DoubleToOUString(m_fLength2) + A2OUSTR("cm"));
    }
    else if( m_eLineStyle == enumXFLineDotDash )
    {
        pAttrList->AddAttribute( A2OUSTR("draw:dots1"), Int32ToOUString(m_nNumber1));

        pAttrList->AddAttribute( A2OUSTR("draw:dots2"), Int32ToOUString(m_nNumber2));
        pAttrList->AddAttribute( A2OUSTR(" draw:dots2-length"), DoubleToOUString(m_fLength2) + A2OUSTR("cm"));
    }
    else if( m_eLineStyle == enumXFLineDashDot )
    {
        pAttrList->AddAttribute( A2OUSTR("draw:dots1"), Int32ToOUString(m_nNumber1));
        pAttrList->AddAttribute( A2OUSTR(" draw:dots1-length"), DoubleToOUString(m_fLength1) + A2OUSTR("cm"));

        pAttrList->AddAttribute( A2OUSTR("draw:dots2"), Int32ToOUString(m_nNumber2));
    }

    pAttrList->AddAttribute( A2OUSTR("draw:distance"), DoubleToOUString(m_fSpace) + A2OUSTR("cm") );

    pStrm->StartElement( A2OUSTR("draw:stroke-dash") );
    pStrm->EndElement( A2OUSTR("draw:stroke-dash") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
