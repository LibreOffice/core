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
 * List object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-17 create this file.
 ************************************************************************/
#include	"xflist.hxx"
#include	"xflistitem.hxx"
#include	"ixfstream.hxx"
#include	"ixfattrlist.hxx"

XFList::XFList()
{
    m_bOrdered = sal_True;
    m_bContinueNumber = sal_False;
    m_pHeader = NULL;
}

XFList::XFList(sal_Bool ordered)
{
    m_bOrdered = ordered;
    m_bContinueNumber = sal_False;
}

XFList::~XFList()
{
}

void	XFList::AddItem(XFListItem *pItem)
{
    assert(pItem);
    assert(pItem->GetContentType()!=enumXFContentUnknown);

    Add(pItem);
}

void	XFList::AddItem(rtl::OUString text)
{
    XFListItem *pItem = new XFListItem();
    pItem->Add(text);
    AddItem(pItem);
}

void	XFList::SetHeader(XFListItem *pItem)
{
    if( m_pHeader )
        delete m_pHeader;
    pItem->SetIsHeader(sal_True);
    m_pHeader = pItem;
}

void	XFList::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    assert(NULL!=pAttrList);

    pAttrList->Clear();
    if( GetStyleName().getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    if( m_bContinueNumber )
        pAttrList->AddAttribute( A2OUSTR("text:continue-numbering"), A2OUSTR("true") );

    if( m_bOrdered )
        pStrm->StartElement( A2OUSTR("text:ordered-list") );
    else
        pStrm->StartElement( A2OUSTR("text:unordered-list") );

    if( m_pHeader )
    {
        m_pHeader->ToXml(pStrm);
    }
    XFContentContainer::ToXml(pStrm);

    if( m_bOrdered )
        pStrm->EndElement( A2OUSTR("text:ordered-list") );
    else
        pStrm->EndElement( A2OUSTR("text:unordered-list") );
}

void	XFList::StartList(IXFStream *pStrm, sal_Bool bContinueNumber)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    assert(NULL!=pAttrList);

    pAttrList->Clear();
    if( GetStyleName().getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    if( bContinueNumber )
        pAttrList->AddAttribute( A2OUSTR("text:continue-numbering"), A2OUSTR("true") );

    if( m_bOrdered )
        pStrm->StartElement( A2OUSTR("text:ordered-list") );
    else
        pStrm->StartElement( A2OUSTR("text:unordered-list") );
}

void	XFList::EndList(IXFStream *pStrm)
{
    if( m_bOrdered )
        pStrm->EndElement( A2OUSTR("text:ordered-list") );
    else
        pStrm->EndElement( A2OUSTR("text:unordered-list") );
}

void	XFList::StartListHeader(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pStrm->StartElement( A2OUSTR("text:list-header") );
}

void	XFList::EndListHeader(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pStrm->EndElement( A2OUSTR("text:list-header") );
}

void	XFList::StartListItem(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    assert(NULL!=pAttrList);

    pAttrList->Clear();
    pStrm->StartElement( A2OUSTR("text:list-item") );
}

void	XFList::EndListItem(IXFStream *pStrm)
{
    pStrm->EndElement( A2OUSTR("text:list-item") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
