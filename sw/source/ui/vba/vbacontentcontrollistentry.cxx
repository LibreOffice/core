/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vbacontentcontrollistentry.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaContentControlListEntry::SwVbaContentControlListEntry(
    const uno::Reference<ooo::vba::XHelperInterface>& rParent,
    const uno::Reference<uno::XComponentContext>& rContext, std::shared_ptr<SwContentControl> pCC,
    size_t nZIndex)
    : SwVbaContentControlListEntry_BASE(rParent, rContext)
    , m_pCC(std::move(pCC))
    , m_nZIndex(nZIndex)
{
}

SwVbaContentControlListEntry::~SwVbaContentControlListEntry() {}

sal_Int32 SwVbaContentControlListEntry::getIndex() { return m_nZIndex + 1; }

void SwVbaContentControlListEntry::setIndex(sal_Int32 nSet)
{
    if (nSet < 1 || static_cast<size_t>(nSet) == m_nZIndex + 1)
        return;

    // Given a one-based index to set to
    size_t nIndex = std::min(static_cast<size_t>(nSet), m_pCC->GetListItems().size());
    // change to zero-based index
    --nIndex;
    while (nIndex < m_nZIndex)
        MoveUp();
    while (m_nZIndex < nIndex)
        MoveDown();
}

OUString SwVbaContentControlListEntry::getText()
{
    assert(m_nZIndex < m_pCC->GetListItems().size());
    const SwContentControlListItem& rListItem = m_pCC->GetListItems()[m_nZIndex];
    return rListItem.ToString();
}

void SwVbaContentControlListEntry::setText(const OUString& rSet)
{
    std::vector<SwContentControlListItem> vListItems = m_pCC->GetListItems();
    assert(m_nZIndex < vListItems.size());

    // prevent duplicates
    for (size_t i = 0; i < vListItems.size(); ++i)
    {
        if (vListItems[i].ToString() == rSet)
            return;
    }

    const std::optional<size_t> oSel(m_pCC->GetSelectedListItem(/*bCheckDocModel=*/true));
    const bool bNeedsInvalidation = m_pCC->GetDropDown() && oSel && *oSel == m_nZIndex;

    vListItems[m_nZIndex].m_aDisplayText = rSet;
    m_pCC->SetListItems(vListItems);

    if (bNeedsInvalidation)
    {
        m_pCC->SetSelectedListItem(m_nZIndex);
        if (m_pCC->GetTextAttr())
            m_pCC->GetTextAttr()->Invalidate();
    }
}

OUString SwVbaContentControlListEntry::getValue()
{
    assert(m_nZIndex < m_pCC->GetListItems().size());
    const SwContentControlListItem& rListItem = m_pCC->GetListItems()[m_nZIndex];

    return rListItem.m_aValue;
}

void SwVbaContentControlListEntry::setValue(const OUString& rSet)
{
    assert(m_nZIndex < m_pCC->GetListItems().size());
    std::vector<SwContentControlListItem> vListItems = m_pCC->GetListItems();

    // LO may pull the display text from Value. Ensure changing Value doesn't alter display text.
    if (vListItems[m_nZIndex].m_aDisplayText.isEmpty())
        vListItems[m_nZIndex].m_aDisplayText = vListItems[m_nZIndex].ToString();

    vListItems[m_nZIndex].m_aValue = rSet;
    m_pCC->SetListItems(vListItems);
}

void SwVbaContentControlListEntry::Delete() { m_pCC->DeleteListItem(m_nZIndex); }

void SwVbaContentControlListEntry::MoveDown()
{
    // if already at last position, can't move down
    if (m_nZIndex >= m_pCC->GetListItems().size() - 1)
        return;

    const std::optional<size_t> oSelected = m_pCC->GetSelectedListItem(/*bCheckDocModel=*/false);
    if (oSelected)
    {
        if (*oSelected == m_nZIndex)
            m_pCC->SetSelectedListItem(m_nZIndex + 1);
        else if (*oSelected == m_nZIndex + 1)
            m_pCC->SetSelectedListItem(*oSelected - 1);
    }
    std::vector<SwContentControlListItem> vListItems = m_pCC->GetListItems();
    std::swap(vListItems[m_nZIndex], vListItems[m_nZIndex + 1]);
    m_pCC->SetListItems(vListItems);
    ++m_nZIndex;
}

void SwVbaContentControlListEntry::MoveUp()
{
    // if already at position 0, can't move up
    if (!m_nZIndex || m_nZIndex >= m_pCC->GetListItems().size())
        return;

    const std::optional<size_t> oSelected = m_pCC->GetSelectedListItem(/*bCheckDocModel=*/false);
    if (oSelected)
    {
        if (*oSelected == m_nZIndex)
            m_pCC->SetSelectedListItem(m_nZIndex - 1);
        else if (*oSelected == m_nZIndex - 1)
            m_pCC->SetSelectedListItem(*oSelected + 1);
    }
    std::vector<SwContentControlListItem> vListItems = m_pCC->GetListItems();
    std::swap(vListItems[m_nZIndex], vListItems[m_nZIndex - 1]);
    m_pCC->SetListItems(vListItems);
    --m_nZIndex;
}

void SwVbaContentControlListEntry::Select()
{
    assert(m_nZIndex < m_pCC->GetListItems().size());
    m_pCC->SetSelectedListItem(m_nZIndex);
    m_pCC->SetShowingPlaceHolder(false);
    if (m_pCC->GetTextAttr())
        m_pCC->GetTextAttr()->Invalidate();
}

// XHelperInterface
OUString SwVbaContentControlListEntry::getServiceImplName()
{
    return u"SwVbaContentControlListEntry"_ustr;
}

uno::Sequence<OUString> SwVbaContentControlListEntry::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{
        u"ooo.vba.word.ContentControlListEntry"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
