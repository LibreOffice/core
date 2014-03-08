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
 * index entry object.
 ************************************************************************/
#include "xfentry.hxx"

XFEntry::XFEntry()
{
    m_eType = enumXFEntryTOC;
    m_bMainEntry = sal_False;
    m_nOutlineLevel = 1;
}

void    XFEntry::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( "text:string-value", m_strValue );

    if( m_eType == enumXFEntryTOC )
    {
        pAttrList->AddAttribute( "text:outline-level", OUString::number(m_nOutlineLevel) );
        pStrm->StartElement( "text:toc-mark" );
//      pStrm->Characters(m_strDisplay);
        pStrm->EndElement( "text:toc-mark" );
    }
    else if( m_eType == enumXFEntryAlphabetical )
    {
        pAttrList->AddAttribute( "text:key1", m_strKey1 );
        if( !m_strKey2.isEmpty() )
            pAttrList->AddAttribute( "text:key2", m_strKey2 );

        if( m_bMainEntry )
            pAttrList->AddAttribute( "text:main-etry", "true" );

        pStrm->StartElement( "text:alphabetical-index-mark" );
//      pStrm->Characters(m_strDisplay);
        pStrm->EndElement( "text:alphabetical-index-mark" );
    }
    else if( m_eType == enumXFEntryUserIndex )
    {
        pAttrList->AddAttribute( "text:outline-level", OUString::number(m_nOutlineLevel) );
        pAttrList->AddAttribute( "text:index-name", m_strName );

        pStrm->StartElement( "text:user-index-mark" );
//      pStrm->Characters(m_strDisplay);
        pStrm->EndElement( "text:user-index-mark" );
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
