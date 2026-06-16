/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <vcl/toolkit/svlbitm.hxx>

#include <map>
#include <memory>

class SvLBoxButton;

class SvLBoxButtonData
{
private:
    Link<SvLBoxButtonData*, void> m_aLink;
    Size m_aSize;
    std::map<SvBmp, Image> m_aBmps;

    SvTreeListEntry* m_pEntry;
    SvLBoxButton* m_pBox;
    bool m_bShowRadioButton;

public:
    // include creating default images (CheckBox or RadioButton)
    SvLBoxButtonData(const Control& rControlForSettings, bool _bRadioBtn);

    ~SvLBoxButtonData();

    static SvBmp GetIndex(SvItemStateFlags nItemState);
    const Size& GetSize();
    void SetLink(const Link<SvLBoxButtonData*, void>& rLink) { m_aLink = rLink; }
    void CallLink(SvTreeListEntry* pActEntry, SvLBoxButton* pActBox);
    bool IsRadio() const;

    static SvButtonState ConvertToButtonState(SvItemStateFlags nItemFlags);

    SvTreeListEntry* GetActEntry() const;
    SvLBoxButton* GetActBox() const;

    const Image& GetImage(SvBmp eIndex) const { return m_aBmps.at(eIndex); }

    void SetDefaultImages(const Control& rControlForSettings);
    // set images according to the color scheme of the Control
};

class SvLBoxButton final : public SvLBoxItem
{
    bool m_bIsVis;
    SvLBoxButtonData& m_rData;
    SvItemStateFlags m_nItemFlags;

    static void ImplAdjustBoxSize(Size& io_rCtrlSize, ControlType i_eType,
                                  vcl::RenderContext const& pRenderContext);

public:
    SvLBoxButton(SvLBoxButtonData& rBData, bool bIsVis = true);
    virtual ~SvLBoxButton() override;
    virtual void InitViewData(SvTreeListBox& rView, SvTreeListEntry& rEntry,
                              SvViewDataItem* pViewData = nullptr) override;

    virtual SvLBoxItemType GetType() const override;
    void ClickHdl(SvTreeListEntry*);

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev,
                       vcl::RenderContext& rRenderContext, const SvViewDataEntry* pView,
                       const SvTreeListEntry& rEntry) override;

    virtual std::unique_ptr<SvLBoxItem> Clone(SvLBoxItem const* pSource) const override;

    SvItemStateFlags GetButtonFlags() const { return m_nItemFlags; }
    bool IsStateChecked() const { return bool(m_nItemFlags & SvItemStateFlags::CHECKED); }
    bool IsStateUnchecked() const { return bool(m_nItemFlags & SvItemStateFlags::UNCHECKED); }
    bool IsStateTristate() const { return bool(m_nItemFlags & SvItemStateFlags::TRISTATE); }
    bool IsStateHilighted() const { return bool(m_nItemFlags & SvItemStateFlags::HIGHLIGHTED); }
    void SetStateChecked();
    void SetStateUnchecked();
    void SetStateTristate();
    void SetStateHilighted(bool bHilight);
};

inline void SvLBoxButton::SetStateChecked()
{
    m_nItemFlags &= SvItemStateFlags::HIGHLIGHTED;
    m_nItemFlags |= SvItemStateFlags::CHECKED;
}

inline void SvLBoxButton::SetStateUnchecked()
{
    m_nItemFlags &= SvItemStateFlags::HIGHLIGHTED;
    m_nItemFlags |= SvItemStateFlags::UNCHECKED;
}
inline void SvLBoxButton::SetStateTristate()
{
    m_nItemFlags &= SvItemStateFlags::HIGHLIGHTED;
    m_nItemFlags |= SvItemStateFlags::TRISTATE;
}
inline void SvLBoxButton::SetStateHilighted(bool bHilight)
{
    if (bHilight)
        m_nItemFlags |= SvItemStateFlags::HIGHLIGHTED;
    else
        m_nItemFlags &= ~SvItemStateFlags::HIGHLIGHTED;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
