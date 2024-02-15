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

#ifndef INCLUDED_SFX2_INC_CHARWIN_HXX
#define INCLUDED_SFX2_INC_CHARWIN_HXX

#include <sfx2/dllapi.h>
#include <vcl/customweld.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <deque>

class SAL_DLLPUBLIC_RTTI SvxCharView final : public weld::CustomWidgetController
{
private:
    VclPtr<VirtualDevice> mxVirDev;
    tools::Long            mnY;
    vcl::Font       maFont;
    bool            maHasInsert;
    OUString        m_sText;

    Link<SvxCharView*, void> maFocusInHdl;
    Link<SvxCharView*, void> maMouseClickHdl;
    Link<SvxCharView*, void> maClearClickHdl;
    Link<SvxCharView*, void> maClearAllClickHdl;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent&) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual OUString RequestHelp(tools::Rectangle&rHelpRect) override;
    virtual bool KeyInput(const KeyEvent&) override;
    virtual bool Command(const CommandEvent&) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    bool GetDecimalValueAndCharName(sal_UCS4& rDecimalValue, OUString& rCharName);
public:
    SvxCharView(const VclPtr<VirtualDevice>& rVirDev);
    SFX2_DLLPUBLIC virtual ~SvxCharView() override;

    void            SetFont( const vcl::Font& rFont );
    vcl::Font const & GetFont() const { return maFont; }
    void            SetText( const OUString& rText );
    OUString const & GetText() const { return m_sText; }
    OUString GetCharInfoText();
    void            SetHasInsert( bool bInsert );
    void            InsertCharToDoc();

    void            createContextMenu(const Point& rPosition);

    Size            get_preferred_size() const { return GetDrawingArea()->get_preferred_size(); }

    void setFocusInHdl(const Link<SvxCharView*,void> &rLink);
    void setMouseClickHdl(const Link<SvxCharView*,void> &rLink);
    void setClearClickHdl(const Link<SvxCharView*,void> &rLink);
    void setClearAllClickHdl(const Link<SvxCharView*,void> &rLink);

    void ContextMenuSelect(std::u16string_view rIdent);
};

class SFX2_DLLPUBLIC SfxCharmapContainer
{
    std::deque<OUString>   m_aRecentCharList;
    std::deque<OUString>   m_aRecentCharFontList;
    std::deque<OUString>   m_aFavCharList;
    std::deque<OUString>   m_aFavCharFontList;

    SvxCharView m_aRecentCharView[16];
    SvxCharView m_aFavCharView[16];
    std::unique_ptr<weld::CustomWeld> m_xRecentCharView[16];
    std::unique_ptr<weld::CustomWeld> m_xFavCharView[16];

    std::unique_ptr<weld::Widget> m_xRecentGrid;
    std::unique_ptr<weld::Widget> m_xFavGrid;

    Link<void*, void> m_aUpdateFavHdl;
    Link<void*, void> m_aUpdateRecentHdl;

    DECL_DLLPRIVATE_LINK(RecentClearClickHdl, SvxCharView*, void);
    DECL_DLLPRIVATE_LINK(FavClearClickHdl, SvxCharView*, void);
    DECL_DLLPRIVATE_LINK(RecentClearAllClickHdl, SvxCharView*, void);
    DECL_DLLPRIVATE_LINK(FavClearAllClickHdl, SvxCharView*, void);

public:
    SfxCharmapContainer(weld::Builder& rBuilder, const VclPtr<VirtualDevice>& rVirDev, bool bLockGridSizes);

    void            init(bool bHasInsert, const Link<SvxCharView*,void> &rMouseClickHdl,
                         const Link<void*,void> &rUpdateFavHdl,
                         const Link<void*,void> &rUpdateRecentHdl,
                         const Link<SvxCharView *,void> &rFocusInHdl = Link<SvxCharView *,void>());

    void            getFavCharacterList();
    void            updateFavCharControl();

    void            getRecentCharacterList(); //gets both recent char and recent char font list
    void            updateRecentCharControl();

    void            updateRecentCharacterList(const OUString& sTitle, const OUString& rFont);
    void            updateFavCharacterList(const OUString& sTitle, const OUString& rFont);
    void            deleteFavCharacterFromList(std::u16string_view sTitle, std::u16string_view rFont);

    bool            isFavChar(std::u16string_view sTitle, std::u16string_view rFont);
    bool            hasRecentChars() const;

    bool            FavCharListIsFull() const;

    void            GrabFocusToFirstFavorite();

private:
    std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
    getRecentChar(std::u16string_view sTitle, std::u16string_view rFont) const;

    std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
    getFavChar(std::u16string_view sTitle, std::u16string_view rFont) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
