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
 * Master page object which was used to apply a layout to the pages.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-12-23 create this file.
 ************************************************************************/
#include	"xfmasterpage.hxx"
#include	"xfpagemaster.hxx"
#include	"xfstylemanager.hxx"
#include	"xffooter.hxx"
#include	"xfheader.hxx"
#include "../lwpglobalmgr.hxx"
XFMasterPage::XFMasterPage()
{
    m_pHeader = NULL;
    m_pFooter = NULL;
}

XFMasterPage::~XFMasterPage()
{
    if( m_pHeader )
        delete m_pHeader;
    if( m_pFooter )
        delete m_pFooter;
}

void	XFMasterPage::SetHeader(XFHeader *pHeader)
{
    if( m_pHeader )
        delete m_pHeader;
    m_pHeader = pHeader;
}

void	XFMasterPage::SetFooter(XFFooter *pFooter)
{
    if( m_pFooter )
        delete m_pFooter;
    m_pFooter = pFooter;
}

enumXFStyle	XFMasterPage::GetStyleFamily()
{
    return enumXFStyleMasterPage;
}


void	XFMasterPage::SetPageMaster(rtl::OUString pm)
{
    m_strPageMaster = pm;
}

void	XFMasterPage::SetPageMaster(XFPageMaster *pPM)
{
    if( pPM->GetStyleName().getLength() > 0 )
        m_strPageMaster = pPM->GetStyleName();
    else
    {
        XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
        m_strPageMaster = pXFStyleManager->AddStyle(pPM)->GetStyleName();
    }
}

void	XFMasterPage::ToXml(IXFStream *pStrm)
{
    IXFAttrList		*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:name"), m_strStyleName );
    pAttrList->AddAttribute( A2OUSTR("style:page-master-name"), m_strPageMaster );
    pStrm->StartElement( A2OUSTR("style:master-page") );
    if( m_pHeader )
        m_pHeader->ToXml(pStrm);
    if( m_pFooter )
        m_pFooter->ToXml(pStrm);
    pStrm->EndElement( A2OUSTR("style:master-page") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
