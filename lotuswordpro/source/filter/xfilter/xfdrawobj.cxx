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
 * Interfer for all Drawing object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-2-17 create this file.
 * 2005-4-1	revise for rotate.
 ************************************************************************/
#include	"xfdrawobj.hxx"

XFDrawObject::XFDrawObject()
{
    m_fRotate = 0;
    m_fScaleX = 0;
    m_fScaleY = 0;
    m_fSkewX = 0;
    m_fSkewY = 0;
    m_nFlag = 0;
    m_eType = enumXFFrameDrawing;
}

void XFDrawObject::ContentToXml(IXFStream *pStrm)
{
    XFContentContainer::ToXml(pStrm);
}

void XFDrawObject::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    if( GetStyleName().getLength() )
        pAttrList->AddAttribute( A2OUSTR("draw:style-name"), GetStyleName() );

    assert(m_strName.getLength()>0);	//name should not be null.
    if( m_strName.getLength() )
        pAttrList->AddAttribute( A2OUSTR("draw:name"), m_strName );
    //anchor type:
    switch( m_eAnchor )
    {
    case enumXFAnchorPara:
        pAttrList->AddAttribute( A2OUSTR("text:anchor-type"), A2OUSTR("paragraph") );
        break;
    case enumXFAnchorPage:
        pAttrList->AddAttribute( A2OUSTR("text:anchor-type"), A2OUSTR("page") );
        break;
    case enumXFAnchorChar:
        pAttrList->AddAttribute( A2OUSTR("text:anchor-type"), A2OUSTR("char") );
        break;
    case enumXFAnchorAsChar:
        pAttrList->AddAttribute( A2OUSTR("text:anchor-type"), A2OUSTR("as-char") );
        break;
    case enumXFAnchorFrame:
        pAttrList->AddAttribute( A2OUSTR("text:anchor-type"), A2OUSTR("frame") );
        break;
    default:
        break;
    }
    pAttrList->AddAttribute( A2OUSTR("draw:z-index"), Int32ToOUString(m_nZIndex) );

    pAttrList->AddAttribute( A2OUSTR("svg:x"), DoubleToOUString(m_aRect.GetX()) + A2OUSTR("cm") );
    pAttrList->AddAttribute( A2OUSTR("svg:y"), DoubleToOUString(m_aRect.GetY()) + A2OUSTR("cm") );
    pAttrList->AddAttribute( A2OUSTR("svg:width"), DoubleToOUString(m_aRect.GetWidth()) + A2OUSTR("cm") );
    pAttrList->AddAttribute( A2OUSTR("svg:height"), DoubleToOUString(m_aRect.GetHeight()) + A2OUSTR("cm") );

    //transform
    rtl::OUString	strTransform;
    if( m_nFlag&XFDRAWOBJECT_FLAG_ROTATE )
        strTransform = A2OUSTR("rotate (") + DoubleToOUString(m_fRotate) + A2OUSTR(") ");
    if( m_nFlag&XFDRAWOBJECT_FLAG_TRANLATE )
        strTransform += A2OUSTR("translate (") + DoubleToOUString(m_aRotatePoint.GetX()) + A2OUSTR("cm ") + DoubleToOUString(m_aRotatePoint.GetY()) + A2OUSTR("cm) ");
    if( m_nFlag&XFDRAWOBJECT_FLAG_SCALE )
        strTransform += A2OUSTR("scale (") + DoubleToOUString(m_fScaleX) + A2OUSTR("cm ") + DoubleToOUString(m_fScaleY) + A2OUSTR("cm) ");
    if( m_nFlag&XFDRAWOBJECT_FLAG_SKEWX )
        strTransform += A2OUSTR("skewX (") + DoubleToOUString(m_fSkewX) + A2OUSTR(" ");
    if( m_nFlag&XFDRAWOBJECT_FLAG_SKEWY )
        strTransform += A2OUSTR("skewY (") + DoubleToOUString(m_fSkewY) + A2OUSTR(" ");
    strTransform = strTransform.trim();

    if( strTransform.getLength() > 0 )
        pAttrList->AddAttribute( A2OUSTR("draw:transform"), strTransform );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
