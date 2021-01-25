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

#pragma once

#include <memory>
#include <string_view>

#include <vcl/dllapi.h>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/layout.hxx>

struct VerticalTabPageData;

class VerticalTabControl final : public VclHBox
{
    VclPtr<SvtIconChoiceCtrl> m_xChooser;
    VclPtr<VclVBox> m_xBox;

    std::vector<std::unique_ptr<VerticalTabPageData>> maPageList;
    OString m_sCurrentPageId;

    Link<VerticalTabControl*, void> m_aActivateHdl;
    Link<VerticalTabControl*, bool> m_aDeactivateHdl;

    DECL_LINK(ChosePageHdl_Impl, SvtIconChoiceCtrl*, void);

    void ActivatePage();
    bool DeactivatePage();

    VerticalTabPageData* GetPageData(std::string_view rId) const;
    VerticalTabPageData* GetPageData(const SvxIconChoiceCtrlEntry* pEntry) const;

public:
    VerticalTabControl(vcl::Window* pParent);
    virtual ~VerticalTabControl() override;
    virtual void dispose() override;

    sal_uInt16 GetPageCount() const { return m_xChooser->GetEntryCount(); }

    const OString& GetCurPageId() const { return m_sCurrentPageId; }
    void SetCurPageId(const OString& rId);

    sal_uInt16 GetPagePos(std::string_view rPageId) const;
    const OString& GetPageId(sal_uInt16 nIndex) const;
    VclPtr<vcl::Window> GetPage(std::string_view rPageId);

    void RemovePage(std::string_view rPageId);
    void InsertPage(const OString& rPageId, const OUString& rLabel, const Image& rImage,
                    const OUString& rTooltip, VclPtr<vcl::Window> xPage, int nPos = -1);

    void SetActivatePageHdl(const Link<VerticalTabControl*, void>& rLink)
    {
        m_aActivateHdl = rLink;
    }
    void SetDeactivatePageHdl(const Link<VerticalTabControl*, bool>& rLink)
    {
        m_aDeactivateHdl = rLink;
    }

    OUString GetPageText(std::string_view rPageId) const;
    void SetPageText(std::string_view rPageId, const OUString& rText);

    vcl::Window* GetPageParent() { return m_xBox.get(); }

    virtual FactoryFunction GetUITestFactory() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
