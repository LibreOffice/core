/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/container/XNameAccess.hpp>

#include <dlgname.hxx>
#include <i18nutil/searchopt.hxx>
#include <vcl/weld.hxx>

#include <vector>

struct Prop_Impl;
struct UserData;

struct prefBoxEntry
{
    OUString sProp;
    OUString sStatus;
    OUString sType;
    OUString sValue;
    UserData* pUserData;
};

class CuiAboutConfigTabPage : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Button> m_xResetBtn;
    std::unique_ptr<weld::Button> m_xEditBtn;
    std::unique_ptr<weld::Button> m_xSearchBtn;
    std::unique_ptr<weld::CheckButton> m_xModifiedCheckBtn;
    std::unique_ptr<weld::Entry> m_xSearchEdit;
    std::unique_ptr<weld::TreeView> m_xPrefBox;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;

    std::vector<std::unique_ptr<UserData>> m_vectorUserData;

    std::vector<prefBoxEntry> m_modifiedPrefBoxEntries;
    std::vector<std::shared_ptr<Prop_Impl>> m_vectorOfModified;

    //for search
    i18nutil::SearchOptions2 m_options;
    std::vector<prefBoxEntry> m_prefBoxEntries;

    bool m_bSorted;

    void AddToModifiedVector(const std::shared_ptr<Prop_Impl>& rProp);
    static std::vector<OUString> commaStringToSequence(std::u16string_view rCommaSepString);
    void InsertEntry(const prefBoxEntry& rEntry);

    DECL_LINK(QueryTooltip, const weld::TreeIter& rIter, OUString);
    DECL_LINK(StandardHdl_Impl, weld::Button&, void);
    DECL_LINK(DoubleClickHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(ResetBtnHdl_Impl, weld::Button&, void);
    DECL_LINK(SearchHdl_Impl, weld::Button&, void);
    DECL_LINK(ModifiedHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(ExpandingHdl_Impl, const weld::TreeIter&, bool);
    DECL_LINK(HeaderBarClick, int, void);
    DECL_STATIC_LINK(CuiAboutConfigTabPage, ValidNameHdl, SvxNameDialog&, bool);

public:
    explicit CuiAboutConfigTabPage(weld::Window* pParent);
    virtual ~CuiAboutConfigTabPage() override;
    void InsertEntry(const OUString& rPropertyPath, css::uno::Any aPropertyValue,
                     const OUString& rProp, const OUString& rStatus, const OUString& rType,
                     const OUString& rValue, const OUString& rTooltip,
                     const weld::TreeIter* pParentEntry, bool bInsertToPrefBox, bool bIsReadOnly,
                     bool bWasMOdified);
    void Reset();
    void InputChanged();
    void FillItems(const css::uno::Reference<css::container::XNameAccess>& xNameAccess,
                   const weld::TreeIter* pParentEntry = nullptr, int lineage = 0,
                   bool bLoadAll = false);
    static css::uno::Reference<css::container::XNameAccess>
    getConfigAccess(const OUString& sNodePath, bool bUpdate);
    void FillItemSet();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
