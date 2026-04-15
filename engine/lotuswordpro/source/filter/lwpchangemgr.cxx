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

#include <lwpchangemgr.hxx>
#include <rtl/ustring.hxx>
#include <lwpfrib.hxx>
#include <xfilter/ixfattrlist.hxx>

LwpChangeMgr::LwpChangeMgr()
{
    m_nCounter = 0;
    m_DocFribMap.clear();
    m_HeadFootFribMap.clear();
    m_pFribMap = &m_DocFribMap;
    m_ChangeList.clear();
}

LwpChangeMgr::~LwpChangeMgr()
{
    m_pFribMap = nullptr;
    m_DocFribMap.clear();
    m_HeadFootFribMap.clear();
    m_ChangeList.clear();
}

void LwpChangeMgr::AddChangeFrib(LwpFrib* pFrib)
{
    m_nCounter++;
    OUString sID = "ct" + OUString::number(m_nCounter);
    m_pFribMap->insert(std::pair<LwpFrib*, OUString>(pFrib, sID));
    pFrib->Register(m_pFribMap);
}

OUString LwpChangeMgr::GetChangeID(LwpFrib* pFrib)
{
    auto iter = m_pFribMap->find(pFrib);
    if (iter == m_pFribMap->end())
        return OUString();
    else
        return iter->second;
}

void LwpChangeMgr::ConvertAllChange(IXFStream* pStream)
{
    for (auto const& docFrib : m_DocFribMap)
    {
        if (docFrib.first->GetRevisionType() == LwpFrib::REV_INSERT)
        {
            XFChangeInsert* pInsert = new XFChangeInsert;
            pInsert->SetChangeID(docFrib.second);
            pInsert->SetEditor(docFrib.first->GetEditor());
            m_ChangeList.push_back(pInsert);
        }
        else if (docFrib.first->GetRevisionType() == LwpFrib::REV_DELETE)
        {
            XFChangeDelete* pDelete = new XFChangeDelete;
            pDelete->SetChangeID(docFrib.second);
            pDelete->SetEditor(docFrib.first->GetEditor());
            m_ChangeList.push_back(pDelete);
        }
    }

    pStream->GetAttrList()->Clear();
    if (m_ChangeList.empty())
        return;
    // Add for disable change tracking
    pStream->GetAttrList()->AddAttribute(u"text:track-changes"_ustr, u"false"_ustr);
    pStream->StartElement(u"text:tracked-changes"_ustr);
    for (auto const& elem : m_ChangeList)
        elem->ToXml(pStream);

    pStream->EndElement(u"text:tracked-changes"_ustr);

    m_ChangeList.clear();
}
void LwpChangeMgr::SetHeadFootFribMap(bool bFlag)
{
    if (bFlag)
        m_pFribMap = &m_HeadFootFribMap;
    else
    {
        m_HeadFootFribMap.clear();
        m_pFribMap = &m_DocFribMap;
    }
}

void LwpChangeMgr::SetHeadFootChange(XFContentContainer* pCont)
{
    XFChangeList* pChangeList = new XFChangeList;

    for (auto const& headFootFrib : m_HeadFootFribMap)
    {
        if (headFootFrib.first->GetRevisionType() == LwpFrib::REV_INSERT)
        {
            XFChangeInsert* pInsert = new XFChangeInsert;
            pInsert->SetChangeID(headFootFrib.second);
            pInsert->SetEditor(headFootFrib.first->GetEditor());
            pChangeList->Add(pInsert);
        }
        else if (headFootFrib.first->GetRevisionType() == LwpFrib::REV_DELETE)
        {
            XFChangeDelete* pDelete = new XFChangeDelete;
            pDelete->SetChangeID(headFootFrib.second);
            pDelete->SetEditor(headFootFrib.first->GetEditor());
            pChangeList->Add(pDelete);
        }
    }

    pCont->Add(pChangeList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
