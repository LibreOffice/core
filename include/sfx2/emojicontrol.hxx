/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_INC_EMOJICONTROL_HXX
#define INCLUDED_SFX2_INC_EMOJICONTROL_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>

class EmojiView;
class ThumbnailViewItem;
enum class FILTER_CATEGORY;

class SFX2_DLLPUBLIC SfxEmojiControl : public SfxPopupWindow
{
public:
    explicit SfxEmojiControl(sal_uInt16 nId, const css::uno::Reference< css::frame::XFrame >& rFrame);

    virtual ~SfxEmojiControl();

    virtual void dispose() override;

private:
    void ConvertLabelToUnicode(sal_uInt16 nPageId);

    /// Return filter according to the currently selected tab page.
    FILTER_CATEGORY getCurrentFilter();

    DECL_LINK_TYPED(ActivatePageHdl, TabControl*, void);
    DECL_LINK_TYPED(InsertHdl, ThumbnailViewItem*, void);

    VclPtr<TabControl>   mpTabControl;
    VclPtr<EmojiView>    mpEmojiView;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
