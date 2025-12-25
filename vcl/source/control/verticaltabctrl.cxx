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

#include <verticaltabctrl.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <vcl/vclevent.hxx>

namespace
{
void collectUIInformation(const OUString& aID, const OUString& aPos)
{
    EventDescription aDescription;
    aDescription.aID = aID;
    aDescription.aParameters = { { "POS", aPos } };
    aDescription.aAction = "SELECT";
    aDescription.aKeyWord = "VerticalTab";
    UITestLogger::getInstance().logEvent(aDescription);
}
}

struct VerticalTabPageData
{
    OUString sId;
    SvxIconChoiceCtrlEntry* pEntry;
    VclPtr<vcl::Window> xPage; ///< the TabPage itself
};

VerticalTabControl::VerticalTabControl(vcl::Window* pParent, bool bWithIcons)
    : VclHBox(pParent)
    , m_xChooser(VclPtr<SvtIconChoiceCtrl>::Create(
          this, WB_3DLOOK | (bWithIcons ? WB_ICON : WB_SMALLICON) |
#ifdef MACOSX
                    WB_NOBORDER |
#else
                    WB_BORDER |
#endif
                    WB_NOCOLUMNHEADER | WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN
                    | WB_NOHSCROLL))
    , m_xBox(VclPtr<VclVBox>::Create(this))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    SetType(WindowType::VERTICALTABCONTROL);
    m_xChooser->SetClickHdl(LINK(this, VerticalTabControl, ChosePageHdl_Impl));
    m_xChooser->set_width_request(150);
    m_xChooser->set_height_request(400);
    m_xChooser->SetSizePixel(Size(150, 400));
    m_xBox->set_vexpand(true);
    m_xBox->set_hexpand(true);
    m_xBox->set_expand(true);
    m_xBox->Show();
    m_xChooser->Show();
}

VerticalTabControl::~VerticalTabControl() { disposeOnce(); }

void VerticalTabControl::dispose()
{
    m_xChooser.disposeAndClear();
    m_xBox.disposeAndClear();
    VclHBox::dispose();
}

IMPL_LINK_NOARG(VerticalTabControl, ChosePageHdl_Impl, SvtIconChoiceCtrl*, void)
{
    SvxIconChoiceCtrlEntry* pEntry = m_xChooser->GetSelectedEntry();
    if (!pEntry)
        pEntry = m_xChooser->GetCursor();

    VerticalTabPageData* pData = GetPageData(pEntry);

    if (pData->sId != m_sCurrentPageId)
        SetCurPageId(pData->sId);
}

bool VerticalTabControl::EventNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == NotifyEventType::KEYINPUT)
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
        if (nCode == KEY_PAGEUP || nCode == KEY_PAGEDOWN
            || (nCode == KEY_TAB && rNEvt.GetKeyEvent()->GetKeyCode().IsMod1()))
        {
            m_xChooser->DoKeyInput(*(rNEvt.GetKeyEvent()));
            m_xChooser->GrabFocus();
            return true;
        }
    }
    return VclHBox::EventNotify(rNEvt);
}

void VerticalTabControl::ActivatePage() { m_aActivateHdl.Call(this); }

bool VerticalTabControl::DeactivatePage()
{
    return !m_aDeactivateHdl.IsSet() || m_aDeactivateHdl.Call(this);
}

VerticalTabPageData* VerticalTabControl::GetPageData(const SvxIconChoiceCtrlEntry* pEntry) const
{
    VerticalTabPageData* pRet = nullptr;
    for (auto& pData : maPageList)
    {
        if (pData->pEntry == pEntry)
        {
            pRet = pData.get();
            break;
        }
    }
    return pRet;
}

VerticalTabPageData* VerticalTabControl::GetPageData(std::u16string_view rId) const
{
    VerticalTabPageData* pRet = nullptr;
    for (auto& pData : maPageList)
    {
        if (pData->sId == rId)
        {
            pRet = pData.get();
            break;
        }
    }
    return pRet;
}

void VerticalTabControl::SetCurPageId(const OUString& rId)
{
    OUString sOldPageId = GetCurPageId();
    if (sOldPageId == rId)
        return;

    VerticalTabPageData* pOldData = GetPageData(sOldPageId);
    if (pOldData && pOldData->xPage)
    {
        if (!DeactivatePage())
            return;
        pOldData->xPage->Hide();
    }

    m_sCurrentPageId = "";

    VerticalTabPageData* pNewData = GetPageData(rId);
    if (pNewData && pNewData->xPage)
    {
        m_sCurrentPageId = rId;
        m_xChooser->SetCursor(pNewData->pEntry);

        ActivatePage();
        pNewData->xPage->Show();
    }
    collectUIInformation(get_id(), m_sCurrentPageId);
}

const OUString& VerticalTabControl::GetPageId(sal_uInt16 nIndex) const
{
    return maPageList[nIndex]->sId;
}

void VerticalTabControl::InsertPage(const rtl::OUString& rIdent, const rtl::OUString& rLabel,
                                    const Image& rImage, const rtl::OUString& rTooltip,
                                    VclPtr<vcl::Window> xPage, int nPos)
{
    SvxIconChoiceCtrlEntry* pEntry = m_xChooser->InsertEntry(rLabel, rImage);
    pEntry->SetQuickHelpText(rTooltip);
    m_xChooser->ArrangeIcons();
    VerticalTabPageData* pNew;
    if (nPos == -1)
    {
        maPageList.emplace_back(new VerticalTabPageData);
        pNew = maPageList.back().get();
    }
    else
    {
        maPageList.emplace(maPageList.begin() + nPos, new VerticalTabPageData);
        pNew = maPageList[nPos].get();
    }
    pNew->sId = rIdent;
    pNew->pEntry = pEntry;
    pNew->xPage = xPage;
    Size aOrigPrefSize(m_xBox->get_preferred_size());
    Size aPagePrefSize(xPage->get_preferred_size());
    m_xBox->set_width_request(std::max(aOrigPrefSize.Width(), aPagePrefSize.Width()));
    m_xBox->set_height_request(std::max(aOrigPrefSize.Height(), aPagePrefSize.Height()));
    pNew->xPage->Hide();
}

void VerticalTabControl::RemovePage(std::u16string_view rPageId)
{
    for (auto it = maPageList.begin(), end = maPageList.end(); it != end; ++it)
    {
        VerticalTabPageData* pData = it->get();
        if (pData->sId == rPageId)
        {
            sal_Int32 nEntryListPos = m_xChooser->GetEntryListPos(pData->pEntry);
            assert(nEntryListPos >= 0);
            m_xChooser->RemoveEntry(nEntryListPos);
            m_xChooser->ArrangeIcons();
            maPageList.erase(it);
            break;
        }
    }
}

sal_uInt16 VerticalTabControl::GetPagePos(std::u16string_view rPageId) const
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return TAB_PAGE_NOTFOUND;
    return m_xChooser->GetEntryListPos(pData->pEntry);
}

VclPtr<vcl::Window> VerticalTabControl::GetPage(std::u16string_view rPageId) const
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return nullptr;
    return pData->xPage;
}

OUString VerticalTabControl::GetPageText(std::u16string_view rPageId) const
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return OUString();
    return pData->pEntry->GetText();
}

void VerticalTabControl::SetPageText(std::u16string_view rPageId, const OUString& rText)
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return;
    pData->pEntry->SetText(rText);
}

Size VerticalTabControl::GetOptimalSize() const
{
    // re-calculate size - we might have replaced dummy tab pages with
    // actual content
    Size aOptimalPageSize(m_xBox->get_preferred_size());

    for (auto const& item : maPageList)
    {
        Size aPagePrefSize(item->xPage->get_preferred_size());
        if (aPagePrefSize.Width() > aOptimalPageSize.Width())
            aOptimalPageSize.setWidth(aPagePrefSize.Width());
        if (aPagePrefSize.Height() > aOptimalPageSize.Height())
            aOptimalPageSize.setHeight(aPagePrefSize.Height());
    }

    Size aChooserSize(m_xChooser->get_preferred_size());
    return Size(aChooserSize.Width() + aOptimalPageSize.Width(),
                std::max(aChooserSize.Height(), aOptimalPageSize.Height()));
}

void VerticalTabControl::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    rJsonWriter.put("id", get_id());
    rJsonWriter.put("type", "tabcontrol");
    rJsonWriter.put("vertical", true);
    rJsonWriter.put("selected", GetCurPageId());

    {
        auto childrenNode = rJsonWriter.startArray("children");
        for (int i = 0; i < GetPageCount(); i++)
        {
            VclPtr<vcl::Window> pChild = GetPage(GetPageId(i));

            if (pChild)
            {
                if (!pChild->GetChildCount())
                    continue;

                auto aChildNode = rJsonWriter.startStruct();
                pChild->DumpAsPropertyTree(rJsonWriter);
            }
        }
    }
    {
        auto tabsNode = rJsonWriter.startArray("tabs");
        for (int i = 0; i < GetPageCount(); i++)
        {
            VclPtr<vcl::Window> pChild = GetPage(GetPageId(i));

            if (pChild)
            {
                if (!pChild->GetChildCount())
                    continue;

                auto aTabNode = rJsonWriter.startStruct();
                auto sId = GetPageId(i);
                rJsonWriter.put("text", GetPageText(sId));
                rJsonWriter.put("id", sId);
                rJsonWriter.put("name", GetPageText(sId));
            }
        }
    }
}

FactoryFunction VerticalTabControl::GetUITestFactory() const
{
    return VerticalTabControlUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
