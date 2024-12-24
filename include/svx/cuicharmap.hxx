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
#include <svx/charmap.hxx>
#include <svx/searchcharmap.hxx>
#include <svx/ucsubset.hxx>
#include <sfx2/charwin.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/frame/XFrame.hpp>

#include <memory>

using namespace ::com::sun::star;

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

    ScopedVclPtr<VirtualDevice> m_xVirDev;
    vcl::Font aFont;
    std::unique_ptr<const SubsetMap> pSubsetMap;
    bool isSearchMode;
    css::uno::Reference<css::frame::XFrame> m_xFrame;

    SfxCharmapContainer m_aCharmapContents;

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
    std::unique_ptr<weld::CustomWeld> m_xShowChar;
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

    // inserts the character that is currently selected in the given SvxShowCharSet
    void insertSelectedCharacter(const SvxShowCharSet* pCharSet);

    DECL_DLLPRIVATE_LINK(FontSelectHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(SubsetSelectHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(CharDoubleClickHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(CharSelectHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(CharHighlightHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(CharPreSelectHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(ReturnKeypressOnCharHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(FavClickHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(SearchCharHighlightHdl, SvxShowCharSet*, void);
    DECL_DLLPRIVATE_LINK(DecimalCodeChangeHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(HexCodeChangeHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(CharClickHdl, SvxCharView*, void);
    DECL_DLLPRIVATE_LINK(InsertClickHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(FavSelectHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(SearchFieldGetFocusHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(UpdateFavHdl, void*, void);

    static void fillAllSubsets(weld::ComboBox& rListBox);
    void selectCharByCode(Radix radix);

public:
    SvxCharacterMap(weld::Widget* pParent, const SfxItemSet* pSet,
                    css::uno::Reference<css::frame::XFrame> xFrame);

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
