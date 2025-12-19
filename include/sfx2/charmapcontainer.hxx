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

#include <sfx2/charwin.hxx>
#include <sfx2/dllapi.h>
#include <vcl/virdev.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/customweld.hxx>
#include <vcl/weld/weld.hxx>
#include <deque>

class SFX2_DLLPUBLIC SfxCharmapContainer
{
    std::deque<CharAndFont> m_aRecentChars;
    std::deque<CharAndFont> m_aFavChars;

    SvxCharView m_aRecentCharView[16];
    SvxCharView m_aFavCharView[16];
    std::unique_ptr<weld::CustomWeld> m_xRecentCharView[16];
    std::unique_ptr<weld::CustomWeld> m_xFavCharView[16];

    std::unique_ptr<weld::Widget> m_xRecentGrid;
    std::unique_ptr<weld::Widget> m_xFavGrid;

    Link<void*, void> m_aUpdateFavHdl;
    Link<void*, void> m_aUpdateRecentHdl;

    DECL_DLLPRIVATE_LINK(RecentContextMenuHdl, const CommandEvent&, void);
    DECL_DLLPRIVATE_LINK(FavContextMenuHdl, const CommandEvent&, void);

    DECL_DLLPRIVATE_LINK(RecentClearClickHdl, SvxCharView&, void);
    DECL_DLLPRIVATE_LINK(FavClearClickHdl, SvxCharView&, void);
    DECL_DLLPRIVATE_LINK(RecentClearAllClickHdl, SvxCharView&, void);
    DECL_DLLPRIVATE_LINK(FavClearAllClickHdl, SvxCharView&, void);

    static void HandleContextMenu(std::span<SvxCharView> aCharViews,
                                  const Link<SvxCharView&, void>& rClearHdl,
                                  const Link<SvxCharView&, void>& rClearAllHdl,
                                  const CommandEvent& rCmdEvent);

    static void updateCharControl(std::span<SvxCharView> aCharViews,
                                  const std::deque<CharAndFont>& rChars);

    static bool GetDecimalValueAndCharName(std::u16string_view sCharText, sal_UCS4& rDecimalValue,
                                           OUString& rCharName);

public:
    SfxCharmapContainer(weld::Builder& rBuilder, const VclPtr<VirtualDevice>& rVirDev,
                        bool bLockGridSizes);

    void init(bool bHasInsert, const Link<const CharAndFont&, void>& rMouseClickHdl,
              const Link<void*, void>& rUpdateFavHdl, const Link<void*, void>& rUpdateRecentHdl,
              const Link<const CharAndFont&, void>& rFocusInHdl = Link<const CharAndFont&, void>());

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
