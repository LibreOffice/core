/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/emojiview.hxx>
#include <sfx2/emojiviewitem.hxx>
#include <sfx2/emojidlg.hxx>

SfxEmojiDlg::SfxEmojiDlg(vcl::Window *parent)
    : ModalDialog(parent, "EmojiWindow", "sfx/ui/emojiwindow.ui")
{
    get(mpCategory1, "category1");
    get(mpCategory2, "category2");
    get(mpCategory3, "category3");
    get(mpCategory4, "category4");
    get(mpCategory5, "category5");
    get(mpCategory6, "category6");
    get(mpCategory7, "category7");
    get(mpCategory8, "category8");
    get(mpCategory9, "category9");
    get(mpCategory10, "category10");

    get(mpEmojiView, "emoji_view");

    mpEmojiView->Show();
}

SfxEmojiDlg::~SfxEmojiDlg()
{
    disposeOnce();
}


void SfxEmojiDlg::dispose()
{
    mpCategory1.clear();
    mpCategory2.clear();
    mpCategory3.clear();
    mpCategory4.clear();
    mpCategory5.clear();
    mpCategory6.clear();
    mpCategory7.clear();
    mpCategory8.clear();
    mpCategory9.clear();
    mpCategory10.clear();

    mpEmojiView.clear();

    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
