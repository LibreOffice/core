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
 *  For LWP filter architecture prototype
 ************************************************************************/

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfchange.hxx>

void XFChangeList::ToXml(IXFStream *pStrm)
{
    if (XFContentContainer::GetCount() == 0)
            return;
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    // Add for disable change tracking
    pAttrList->AddAttribute( u"text:track-changes"_ustr,u"false"_ustr);
    pStrm->StartElement( u"text:tracked-changes"_ustr );
    XFContentContainer::ToXml(pStrm);
    pStrm->EndElement(u"text:tracked-changes"_ustr);
}

void XFChangeRegion::ToXml(IXFStream * /*pStrm*/)
{
}

void XFChangeInsert::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if(m_sID.isEmpty())
        return;
    pAttrList->AddAttribute( u"text:id"_ustr,m_sID);

    pStrm->StartElement( u"text:changed-region"_ustr );
    pStrm->StartElement( u"text:insertion"_ustr );
    pAttrList->Clear();
    pAttrList->AddAttribute( u"office:chg-author"_ustr,m_sEditor);
    pAttrList->AddAttribute( u"office:chg-date-time"_ustr,u"0000-00-00T00:00:00"_ustr);
    pStrm->StartElement( u"office:change-info"_ustr );
    pStrm->EndElement( u"office:change-info"_ustr );
    pStrm->EndElement( u"text:insertion"_ustr );
    pStrm->EndElement( u"text:changed-region"_ustr );
}

void XFChangeDelete::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if(m_sID.isEmpty())
        return;
    pAttrList->AddAttribute( u"text:id"_ustr,m_sID);

    pStrm->StartElement( u"text:changed-region"_ustr );
    pStrm->StartElement( u"text:deletion"_ustr );
    pAttrList->Clear();
    pAttrList->AddAttribute( u"office:chg-author"_ustr,m_sEditor);
    pAttrList->AddAttribute( u"office:chg-date-time"_ustr,u"0000-00-00T00:00:00"_ustr);
    pStrm->StartElement( u"office:change-info"_ustr );
    pStrm->EndElement( u"office:change-info"_ustr );

    pStrm->EndElement( u"text:deletion"_ustr );
    pStrm->EndElement( u"text:changed-region"_ustr );
}

void XFChangeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if(m_sID.isEmpty())
        return;
    pAttrList->AddAttribute( u"text:change-id"_ustr,m_sID);

    pStrm->StartElement( u"text:change-start"_ustr );
    pStrm->EndElement( u"text:change-start"_ustr );
}

void XFChangeEnd::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if(m_sID.isEmpty())
        return;
    pAttrList->AddAttribute( u"text:change-id"_ustr,m_sID);

    pStrm->StartElement( u"text:change-end"_ustr );
    pStrm->EndElement( u"text:change-end"_ustr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
