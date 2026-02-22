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

#include <sfx2/dllapi.h>
#include <vcl/vclenum.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/IconView.hxx>
#include <vcl/weld/weld.hxx>
#include <deque>

struct CharAndFont
{
    OUString sChar;
    OUString sFont;

    CharAndFont(const OUString& rChar, const OUString& rFont)
        : sChar(rChar)
        , sFont(rFont)
    {
    }

    bool operator==(const CharAndFont& rOther) const = default;
};

class SFX2_DLLPUBLIC SfxCharmapContainer
{
    std::deque<CharAndFont> m_aRecentChars;
    std::deque<CharAndFont> m_aFavChars;

    std::unique_ptr<weld::IconView> m_xRecentIconView;
    std::unique_ptr<weld::IconView> m_xFavIconView;

    Link<const CharAndFont&, void> m_aCharActivateHdl;
    Link<const CharAndFont&, void> m_aCharSelectedHdl;

    Link<void*, void> m_aUpdateFavHdl;
    Link<void*, void> m_aUpdateRecentHdl;

    DECL_DLLPRIVATE_LINK(IconViewSelectionChangedHdl, weld::IconView&, void);
    DECL_DLLPRIVATE_LINK(ItemActivatedHdl, weld::IconView&, bool);
    DECL_DLLPRIVATE_LINK(ItemViewFocusInHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_STATIC_LINK(SfxCharmapContainer, ItemViewFocusOutHdl, weld::Widget&, void);

    DECL_DLLPRIVATE_LINK(RecentContextMenuHdl, const CommandEvent&, bool);
    DECL_DLLPRIVATE_LINK(FavContextMenuHdl, const CommandEvent&, bool);

    DECL_DLLPRIVATE_LINK(RecentClearClickHdl, const CharAndFont&, void);
    DECL_DLLPRIVATE_LINK(FavClearClickHdl, const CharAndFont&, void);
    DECL_DLLPRIVATE_LINK(RecentClearAllClickHdl, weld::IconView&, void);
    DECL_DLLPRIVATE_LINK(FavClearAllClickHdl, weld::IconView&, void);

    static bool HandleContextMenu(weld::IconView& rIconView, std::deque<CharAndFont>& rChars,
                                  const Link<const CharAndFont&, void>& rClearHdl,
                                  const Link<weld::IconView&, void>& rClearAllHdl,
                                  const CommandEvent& rCmdEvent);

    static void updateCharControl(weld::IconView& rIconView, const std::deque<CharAndFont>& rChars);

    static bool GetDecimalValueAndCharName(std::u16string_view sCharText, sal_UCS4& rDecimalValue,
                                           OUString& rCharName);

public:
    SfxCharmapContainer(weld::Builder& rBuilder);

    void init(const Link<const CharAndFont&, void>& rActivateHdl,
              const Link<void*, void>& rUpdateFavHdl, const Link<void*, void>& rUpdateRecentHdl,
              const Link<const CharAndFont&, void>& rCharSelectedHdl);

    static VclPtr<VirtualDevice> CreateIcon(weld::IconView& rIconView, const OUString& rFont,
                                            const OUString& rText);

    void getFavCharacterList();
    void updateFavCharControl();

    void getRecentCharacterList(); //gets both recent char and recent char font list
    void updateRecentCharControl();

    void updateRecentCharacterList(const OUString& rTitle, const OUString& rFont);
    void updateFavCharacterList(const OUString& rTitle, const OUString& rFont);
    void deleteFavCharacterFromList(const OUString& rTitle, const OUString& rFont);

    bool isFavChar(const OUString& rTitle, const OUString& rFont);
    bool hasRecentChars() const;

    bool FavCharListIsFull() const;

    void GrabFocusToFirstFavorite();

    static OUString GetCharInfoText(std::u16string_view sCharText);
    static void InsertCharToDoc(const CharAndFont& rChar);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
