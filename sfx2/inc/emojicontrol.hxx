/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svtools/toolbarmenu.hxx>
#include <vcl/customweld.hxx>

#define TAB_FONT_SIZE 15

namespace com::sun::star::frame { class XFrame; }

class EmojiPopup;
class EmojiView;
enum class FILTER_CATEGORY;

class SfxEmojiControl final : public WeldToolbarPopup

{
public:
    explicit SfxEmojiControl(EmojiPopup* pControl, weld::Widget* pParent);
    virtual ~SfxEmojiControl() override;

    virtual void GrabFocus() override;

private:
    void ConvertLabelToUnicode(const OString& rPageId);

    /// Return filter according to the currently selected tab page.
    FILTER_CATEGORY getCurrentFilter() const;

    DECL_LINK(ActivatePageHdl, const OString&, void);
//TODO    DECL_STATIC_LINK(SfxEmojiControl, InsertHdl, ThumbnailViewItem*, void);

    std::unique_ptr<weld::Notebook> mxTabControl;
    std::unique_ptr<EmojiView> mxPeopleView;
    std::unique_ptr<weld::CustomWeld> mxPeopleWeld;
//    VclPtr<EmojiView>    mpEmojiView;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
