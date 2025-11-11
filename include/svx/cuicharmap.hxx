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

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/itemset.hxx>
#include <svx/ucsubset.hxx>
#include <sfx2/charwin.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/frame/XFrame.hpp>

#include <string_view>
#include <memory>
#include <map>

using namespace ::com::sun::star;

struct ImplSVEvent;

/// Provides the show characters or texts in a drawing area with special sizes and fonts.
class SVX_DLLPUBLIC SvxShowText final : public weld::CustomWidgetController
{
private:
    ScopedVclPtr<VirtualDevice> m_xVirDev;
    OUString m_sText;
    tools::Long mnY;
    bool mbCenter;
    vcl::Font m_aFont;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void Resize() override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

public:
    SvxShowText(const VclPtr<VirtualDevice>& rVirDev);

    void SetFont(const vcl::Font& rFont);
    vcl::Font const& GetFont() const { return m_aFont; }
    void SetText(const OUString& rText);
    OUString const& GetText() const { return m_sText; }
    void SetCentered(bool bCenter) { mbCenter = bCenter; }

    Size get_preferred_size() const { return GetDrawingArea()->get_preferred_size(); }
};

/** The main purpose of this dialog is to enable the use of characters
    that are not easily accessible from the keyboard. */
class SVX_DLLPUBLIC SvxCharacterMap final : public SfxDialogController
{
private:
    void init();

    // Character bitmap generation and model population
    VclPtr<VirtualDevice> generateCharGraphic(sal_UCS4 cChar);
    void clearSearchCharModel();
    void populateShowCharModel();
    void populateSearchCharModel();

    // Character selection and retrieval
    void selectCharacter(sal_UCS4 cChar);
    void selectSearchSetCharFromSubset(const Subset* pSubset);
    static sal_UCS4 getCharacterFromId(std::u16string_view rId);
    static OUString getCharacterNameFromId(std::u16string_view sId);

    void modifyFavCharacterList(const OUString& sChar, const OUString& sFont);
    void rerenderCharacter(std::u16string_view favChar, std::u16string_view favCharFont);

    // Context menu and clipboard
    void createContextMenu(const Point& rPos, bool bSearchMode);
    void contextMenuSelect(std::u16string_view rIdent);
    void contextMenuHdl(weld::IconView& rIconView, const Point& pPos, bool bSearchMode);
    static void copyToClipboard(const OUString& rText);

    // Performance optimization via scheduling(lazy loading)
    void scheduleShowSetBackgroundRendering();
    void scheduleSearchSetBackgroundRendering();
    void renderShowSetBatch(sal_Int32 nStartPos, sal_Int32 nCount);
    void renderSearchSetBatch(sal_Int32 nStartPos, sal_Int32 nCount);

    sal_Int32 m_nShowSetRenderedCount = 0;
    sal_Int32 m_nSearchSetRenderedCount = 0;
    static constexpr sal_Int32 RENDER_BATCH_SIZE = 500;

    ImplSVEvent* m_nShowRenderIdleEvent = nullptr;
    ImplSVEvent* m_nSearchRenderIdleEvent = nullptr;

    ScopedVclPtr<VirtualDevice> m_xVirDev;
    vcl::Font aFont;
    std::unique_ptr<const SubsetMap> pSubsetMap;
    bool isSearchMode;
    css::uno::Reference<css::frame::XFrame> m_xFrame;

    SfxCharmapContainer m_aCharmapContents;

    SvxShowText m_aShowChar;

    FontCharMapRef mxFontCharMap;
    std::map<sal_Int32, sal_UCS4> m_aSearchItemList;

    std::unordered_map<OUString, sal_Int32> m_aShowCharPos;
    std::unordered_map<OUString, sal_Int32> m_aSearchCharPos;

    static sal_UCS4 m_cSelectedChar;

    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::Label> m_xFontText;
    std::unique_ptr<weld::ComboBox> m_xFontLB;
    std::unique_ptr<weld::Label> m_xSubsetText;
    std::unique_ptr<weld::ComboBox> m_xSubsetLB;
    std::unique_ptr<weld::Entry> m_xSearchText;
    std::unique_ptr<weld::Entry> m_xHexCodeText;
    std::unique_ptr<weld::Entry> m_xDecimalCodeText;
    std::unique_ptr<weld::Button> m_xFavouritesBtn;
    std::unique_ptr<weld::Label> m_xCharName;
    std::unique_ptr<weld::CustomWeld> m_xShowChar;
    std::unique_ptr<weld::ScrolledWindow> m_xShowSetArea;
    std::unique_ptr<weld::ScrolledWindow> m_xSearchSetArea;
    std::unique_ptr<weld::IconView> m_xShowSet;
    std::unique_ptr<weld::IconView> m_xSearchSet;

    std::unique_ptr<SfxAllItemSet> m_xOutputSet;

    enum class Radix : sal_Int16
    {
        decimal = 10,
        hexadecimal = 16
    };

    // inserts the character that is currently selected in the given IconView
    void insertSelectedCharacter(weld::IconView& rIconView);

    DECL_DLLPRIVATE_LINK(FontSelectHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(SubsetSelectHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(CharDoubleClickHdl, weld::IconView&, bool);
    DECL_DLLPRIVATE_LINK(CharSelectHdl, weld::IconView&, void);
    DECL_DLLPRIVATE_LINK(CharKeyPressHdl, const KeyEvent&, bool);
    DECL_DLLPRIVATE_LINK(ShowCharMousePressHdl, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(SearchCharMousePressHdl, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(ShowCharQueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_DLLPRIVATE_LINK(SearchCharQueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_DLLPRIVATE_LINK(DecimalCodeChangeHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(HexCodeChangeHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(CharClickHdl, SvxCharView*, void);
    DECL_DLLPRIVATE_LINK(InsertClickHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(FavSelectHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(SearchFieldGetFocusHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(UpdateFavHdl, SfxCharmapContainer::CharChange*, void);
    DECL_DLLPRIVATE_LINK(UpdateRecentHdl, SfxCharmapContainer::CharChange*, void);
    DECL_DLLPRIVATE_LINK(ShowSetScrollHdl, weld::ScrolledWindow&, void);
    DECL_DLLPRIVATE_LINK(ShowRenderIdleHdl, void*, void);
    DECL_DLLPRIVATE_LINK(SearchSetScrollHdl, weld::ScrolledWindow&, void);
    DECL_DLLPRIVATE_LINK(SearchRenderIdleHdl, void*, void);

    static void fillAllSubsets(weld::ComboBox& rListBox);
    void selectCharByCode(Radix radix);

public:
    SvxCharacterMap(weld::Widget* pParent, const SfxItemSet* pSet,
                    css::uno::Reference<css::frame::XFrame> xFrame);

    virtual ~SvxCharacterMap() override;

    // for explicit use before AsyncRun
    void prepForRun();

    virtual short run() override;

    void set_title(const OUString& rTitle) { m_xDialog->set_title(rTitle); }

    const SfxItemSet* GetOutputItemSet() const { return m_xOutputSet.get(); }

    void DisableFontSelection();

    const vcl::Font& GetCharFont() const { return aFont; }
    void SetCharFont(const vcl::Font& rFont);

    void SetChar(sal_UCS4);
    sal_UCS4 GetChar() const;

    void insertCharToDoc(const OUString& sChar);

    void setFavButtonState(std::u16string_view sTitle, std::u16string_view rFont);

    void setCharName(sal_UCS4 nDecimalValue);

    void toggleSearchView(bool state);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
