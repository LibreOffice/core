/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/svapp.hxx>

#include "structpg.hxx"
#include <formula/formula.hxx>
#include <formula/token.hxx>
#include <bitmaps.hlst>

namespace formula
{

void StructPage::SetActiveFlag(bool bFlag)
{
    bActiveFlag = bFlag;
}

StructPage::StructPage(weld::Container* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, u"formula/ui/structpage.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"StructPage"_ustr))
    , m_xTlbStruct(m_xBuilder->weld_tree_view(u"struct"_ustr))
    , maImgEnd(BMP_STR_END)
    , maImgError(BMP_STR_ERROR)
    , pSelectedToken(nullptr)
    , bActiveFlag(false)
{
    m_xTlbStruct->set_size_request(m_xTlbStruct->get_approximate_digit_width() * 20,
                                   m_xTlbStruct->get_height_rows(17));

    m_xTlbStruct->connect_changed(LINK( this, StructPage, SelectHdl ) );
}

StructPage::~StructPage()
{
}

void StructPage::ClearStruct()
{
    SetActiveFlag(false);
    m_xTlbStruct->clear();
}

bool StructPage::InsertEntry(const OUString& rText, const weld::TreeIter* pParent,
                             sal_uInt16 nFlag, int nPos,
                             const FormulaToken* pIFormulaToken,
                             weld::TreeIter& rRet)
{
    SetActiveFlag(false);

    OUString sId(weld::toId(pIFormulaToken));

    bool bEntry = false;
    switch (nFlag)
    {
        case STRUCT_FOLDER:
            m_xTlbStruct->insert(pParent, nPos, &rText, &sId, nullptr, nullptr,
                                 false, &rRet);
            m_xTlbStruct->set_image(rRet, BMP_STR_OPEN);
            bEntry = true;
            break;
        case STRUCT_END:
            m_xTlbStruct->insert(pParent, nPos, &rText, &sId, nullptr, nullptr,
                                 false, &rRet);
            m_xTlbStruct->set_image(rRet, maImgEnd);
            bEntry = true;
            break;
        case STRUCT_ERROR:
            m_xTlbStruct->insert(pParent, nPos, &rText, &sId, nullptr, nullptr,
                                 false, &rRet);
            m_xTlbStruct->set_image(rRet, maImgError);
            bEntry = true;
            break;
    }

    if (bEntry && pParent)
        m_xTlbStruct->expand_row(*pParent);
    return bEntry;
}

OUString StructPage::GetEntryText(const weld::TreeIter* pEntry) const
{
    OUString aString;
    if (pEntry)
        aString = m_xTlbStruct->get_text(*pEntry);
    return aString;
}

const FormulaToken* StructPage::GetFunctionEntry(const weld::TreeIter* pEntry)
{
    if (!pEntry)
        return nullptr;

    const FormulaToken * pToken = weld::fromId<const FormulaToken*>(m_xTlbStruct->get_id(*pEntry));
    if (pToken)
    {
        if ( !(pToken->IsFunction() || pToken->GetParamCount() > 1 ) )
        {
            std::unique_ptr<weld::TreeIter> xParent(m_xTlbStruct->make_iterator(pEntry));
            if (!m_xTlbStruct->iter_parent(*xParent))
                return nullptr;
            return GetFunctionEntry(xParent.get());
        }
        else
        {
            return pToken;
        }
    }
    return nullptr;
}

IMPL_LINK(StructPage, SelectHdl, weld::TreeView&, rTlb, void)
{
    if (!GetActiveFlag())
        return;

    if (&rTlb == m_xTlbStruct.get())
    {
        std::unique_ptr<weld::TreeIter> xCurEntry(m_xTlbStruct->make_iterator());
        if (m_xTlbStruct->get_cursor(xCurEntry.get()))
        {
            pSelectedToken = weld::fromId<const FormulaToken*>(m_xTlbStruct->get_id(*xCurEntry));
            if (pSelectedToken)
            {
                if ( !(pSelectedToken->IsFunction() || pSelectedToken->GetParamCount() > 1) )
                {
                    pSelectedToken = GetFunctionEntry(xCurEntry.get());
                }
            }
        }
    }

    aSelLink.Call(*this);
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
