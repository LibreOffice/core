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

namespace com::sun::star::frame { class XFrame; }

class EmojiPopup;
class EmojiView;
class ThumbnailViewItem;
enum class FILTER_CATEGORY;

class SfxEmojiControl final : public WeldToolbarPopup

{
public:
    explicit SfxEmojiControl(const EmojiPopup* pControl, weld::Widget* pParent);
    virtual ~SfxEmojiControl() override;

    virtual void GrabFocus() override;

private:
    static void ConvertLabelToUnicode(weld::ToggleButton& rBtn);

    FILTER_CATEGORY getFilter(const weld::Button& rBtn) const;

    DECL_LINK(ActivatePageHdl, weld::Button&, void);
    DECL_STATIC_LINK(SfxEmojiControl, InsertHdl, ThumbnailViewItem*, void);

    std::unique_ptr<weld::ToggleButton> mxPeopleBtn;
    std::unique_ptr<weld::ToggleButton> mxNatureBtn;
    std::unique_ptr<weld::ToggleButton> mxFoodBtn;
    std::unique_ptr<weld::ToggleButton> mxActivityBtn;
    std::unique_ptr<weld::ToggleButton> mxTravelBtn;
    std::unique_ptr<weld::ToggleButton> mxObjectsBtn;
    std::unique_ptr<weld::ToggleButton> mxSymbolsBtn;
    std::unique_ptr<weld::ToggleButton> mxFlagsBtn;
    std::unique_ptr<weld::ToggleButton> mxUnicode9Btn;
    std::unique_ptr<EmojiView> mxEmojiView;
    std::unique_ptr<weld::CustomWeld> mxEmojiWeld;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
