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
 * Section style,whose family is "section"
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-10	create this file.
 ************************************************************************/
#include	"xfsectionstyle.hxx"
#include	"xfcolumns.hxx"

XFSectionStyle::XFSectionStyle()
{
    m_fMarginLeft = 0;
    m_fMarginRight = 0;
    m_pBackImage = NULL;
    m_pColumns = NULL;
}

XFSectionStyle::~XFSectionStyle()
{
    if( m_pColumns )
        delete m_pColumns;
    if( m_pBackImage )
        delete m_pBackImage;
}

enumXFStyle XFSectionStyle::GetStyleFamily()
{
    return enumXFStyleSection;
}

void XFSectionStyle::SetMarginLeft(double left)
{
    m_fMarginLeft = left;
}

void XFSectionStyle::SetMarginRight(double right)
{
    m_fMarginRight = right;
}

void XFSectionStyle::SetBackColor(const XFColor& color)
{
    m_aBackColor = color;
}

void	XFSectionStyle::SetColumns(XFColumns *pColumns)
{
    if( m_pColumns )
        delete m_pColumns;
    m_pColumns = pColumns;
}

void	XFSectionStyle::SetBackImage(XFBGImage *image)
{
    if( m_pBackImage )
        delete m_pBackImage;
    m_pBackImage = image;
}

void XFSectionStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("section") );

    pStrm->StartElement( A2OUSTR("style:style") );

    pAttrList->Clear();
    if( m_fMarginLeft != 0 )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:margin-left"), DoubleToOUString(m_fMarginLeft) + A2OUSTR("cm") );
    }
    if( m_fMarginRight != 0 )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:margin-right"), DoubleToOUString(m_fMarginRight) + A2OUSTR("cm") );
    }
    if( m_aBackColor.IsValid() && !m_pBackImage )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:background-color"), m_aBackColor.ToString() );
    }
    else
    {
        pAttrList->AddAttribute( A2OUSTR("fo:background-color"), A2OUSTR("transparent") );
    }

    pStrm->StartElement( A2OUSTR("style:properties") );

    if( m_pColumns )
        m_pColumns->ToXml(pStrm);
    if( m_pBackImage )
        m_pBackImage->ToXml(pStrm);

    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("style:style") );


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
