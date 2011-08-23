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
 * Outline style for the full document.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-02-2 create and implements.
 ************************************************************************/
#include	"xfoutlinestyle.hxx"

XFOutlineLevelStyle::XFOutlineLevelStyle()
{
    m_nLevel = 0;
    m_fIndent = 0;
    m_fMinLabelWidth = 0;
    m_fMinLabelDistance = 0;
    m_eAlign = enumXFAlignNone;
    m_pNumFmt = NULL;
}

XFOutlineLevelStyle::~XFOutlineLevelStyle()
{
    if( m_pNumFmt )
        delete m_pNumFmt;
}

void XFOutlineLevelStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    assert(m_nLevel>0);
    pAttrList->AddAttribute( A2OUSTR("text:level"), Int32ToOUString(m_nLevel) );
    if( GetStyleName().getLength() > 0 )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    if( m_pNumFmt )
        m_pNumFmt->ToXml(pStrm);
    else
        pAttrList->AddAttribute( A2OUSTR("style:num-format"), A2OUSTR("") );
    pStrm->StartElement( A2OUSTR("text:outline-level-style") );

    pAttrList->Clear();
    if( m_fIndent )
        pAttrList->AddAttribute( A2OUSTR("text:space-before"), DoubleToOUString(m_fIndent) + A2OUSTR("cm") );
    if( m_fMinLabelWidth )
        pAttrList->AddAttribute( A2OUSTR("text:min-label-width"), DoubleToOUString(m_fMinLabelWidth) + A2OUSTR("cm") );
    if( m_fMinLabelDistance )
        pAttrList->AddAttribute( A2OUSTR("text:min-label-distance"), DoubleToOUString(m_fMinLabelDistance) + A2OUSTR("cm") );
    if( enumXFAlignNone != m_eAlign )
        pAttrList->AddAttribute( A2OUSTR("fo:text-align"), GetAlignName(m_eAlign) );
    pStrm->StartElement( A2OUSTR("style:properties") );
    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("text:outline-level-style") );
}

XFOutlineStyle::XFOutlineStyle()
{
    for( int i=0; i<10; i++ )
        m_aLevels[i].SetLevel(i+1);
}

enumXFStyle	XFOutlineStyle::GetStyleFamily()
{
    return enumXFStyleOutline;
}

void XFOutlineStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pStrm->StartElement( A2OUSTR("text:outline-style") );
    for( int i=0; i<10; i++ )
        m_aLevels[i].ToXml(pStrm);
    pStrm->EndElement( A2OUSTR("text:outline-style") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
