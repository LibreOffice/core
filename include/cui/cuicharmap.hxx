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
#ifndef INCLUDED_CUI_CUICHARMAP_HXX
#define INCLUDED_CUI_CUICHARMAP_HXX

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/itemset.hxx>
#include <svx/charmap.hxx>
#include <svx/searchcharmap.hxx>
#include <svx/ucsubset.hxx>
#include <sfx2/charwin.hxx>
#include <cui/cuidllapi.h>
#include <com/sun/star/frame/XFrame.hpp>

#include <deque>
#include <memory>
#include <utility>

using namespace ::com::sun::star;
class SubsetMap;

#define CHARMAP_MAXLEN 32

namespace svx
{
struct SvxShowCharSetItem;
}

/// Provides the show characters or texts in a drawing area with special sizes and fonts.
class CUI_DLLPUBLIC SvxShowText final : public weld::CustomWidgetController
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
class CUI_DLLPUBLIC SvxCharacterMap final : public SfxDialogController
{
private:
    void init();

    ScopedVclPtr<VirtualDevice> m_xVirDev;
    vcl::Font aFont;
    std::unique_ptr<const SubsetMap> pSubsetMap;
    bool isSearchMode;
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    std::deque<OUString> maRecentCharList;
    std::deque<OUString> maRecentCharFontList;
    std::deque<OUString> maFavCharList;
    std::deque<OUString> maFavCharFontList;
    uno::Reference<uno::XComponentContext> mxContext;

    SvxCharView m_aRecentCharView[16];
    SvxCharView m_aFavCharView[16];
    SvxShowText m_aShowChar;

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
    std::unique_ptr<weld::Widget> m_xRecentGrid;
    std::unique_ptr<weld::Widget> m_xFavGrid;
    std::unique_ptr<weld::CustomWeld> m_xShowChar;
    std::unique_ptr<weld::CustomWeld> m_xRecentCharView[16];
    std::unique_ptr<weld::CustomWeld> m_xFavCharView[16];
    std::unique_ptr<SvxShowCharSet> m_xShowSet;
    std::unique_ptr<weld::CustomWeld> m_xShowSetArea;
    std::unique_ptr<SvxSearchCharSet> m_xSearchSet;
    std::unique_ptr<weld::CustomWeld> m_xSearchSetArea;

    std::unique_ptr<SfxAllItemSet> m_xOutputSet;

    enum class Radix : sal_Int16
    {
        decimal = 10,
        hexadecimal = 16
    };

    DECL_LINK(FontSelectHdl, weld::ComboBox&, void);
    DECL_LINK(SubsetSelectHdl, weld::ComboBox&, void);
    DECL_LINK(CharDoubleClickHdl, SvxShowCharSet*, void);
    DECL_LINK(CharSelectHdl, SvxShowCharSet*, void);
    DECL_LINK(CharHighlightHdl, SvxShowCharSet*, void);
    DECL_LINK(CharPreSelectHdl, SvxShowCharSet*, void);
    DECL_LINK(FavClickHdl, SvxShowCharSet*, void);
    DECL_LINK(SearchCharDoubleClickHdl, SvxShowCharSet*, void);
    DECL_LINK(SearchCharSelectHdl, SvxShowCharSet*, void);
    DECL_LINK(SearchCharHighlightHdl, SvxShowCharSet*, void);
    DECL_LINK(SearchCharPreSelectHdl, SvxShowCharSet*, void);
    DECL_LINK(DecimalCodeChangeHdl, weld::Entry&, void);
    DECL_LINK(HexCodeChangeHdl, weld::Entry&, void);
    DECL_LINK(CharClickHdl, SvxCharView*, void);
    DECL_LINK(RecentClearClickHdl, SvxCharView*, void);
    DECL_LINK(FavClearClickHdl, SvxCharView*, void);
    DECL_LINK(RecentClearAllClickHdl, SvxCharView*, void);
    DECL_LINK(FavClearAllClickHdl, SvxCharView*, void);
    DECL_LINK(InsertClickHdl, weld::Button&, void);
    DECL_LINK(FavSelectHdl, weld::Button&, void);
    DECL_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_LINK(SearchFieldGetFocusHdl, weld::Widget&, void);

    static void fillAllSubsets(weld::ComboBox& rListBox);
    void selectCharByCode(Radix radix);

public:
    SvxCharacterMap(weld::Widget* pParent, const SfxItemSet* pSet,
                    const css::uno::Reference<css::frame::XFrame>& rFrame);
    virtual short run() override;

    void set_title(const OUString& rTitle) { m_xDialog->set_title(rTitle); }

    const SfxItemSet* GetOutputItemSet() const { return m_xOutputSet.get(); }

    void DisableFontSelection();

    const vcl::Font& GetCharFont() const { return aFont; }
    void SetCharFont(const vcl::Font& rFont);

    void SetChar(sal_UCS4);
    sal_UCS4 GetChar() const;

    void getRecentCharacterList(); //gets both recent char and recent char font list
    void updateRecentCharacterList(const OUString& rChar, const OUString& rFont);

    void getFavCharacterList(); //gets both Fav char and Fav char font list
    void updateFavCharacterList(const OUString& rChar, const OUString& rFont);
    void deleteFavCharacterFromList(std::u16string_view rChar, std::u16string_view rFont);
    bool isFavChar(std::u16string_view sTitle, std::u16string_view rFont);

    void updateRecentCharControl();
    void insertCharToDoc(const OUString& sChar);

    void updateFavCharControl();
    void setFavButtonState(std::u16string_view sTitle, std::u16string_view rFont);

    void setCharName(sal_UCS4 nDecimalValue);

    void toggleSearchView(bool state);

private:
    std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
    getRecentChar(std::u16string_view sTitle, std::u16string_view rFont) const;

    std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
    getFavChar(std::u16string_view sTitle, std::u16string_view rFont) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
