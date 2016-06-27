/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_INC_EMOJIDLG_HXX
#define INCLUDED_SFX2_INC_EMOJIDLG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>

class EmojiView;

class SFX2_DLLPUBLIC SfxEmojiDlg : public ModalDialog
{
public:
    SfxEmojiDlg(vcl::Window *parent = nullptr);

    virtual ~SfxEmojiDlg();

    virtual void dispose() override;

private:
    VclPtr<PushButton> mpCategory1;
    VclPtr<PushButton> mpCategory2;
    VclPtr<PushButton> mpCategory3;
    VclPtr<PushButton> mpCategory4;
    VclPtr<PushButton> mpCategory5;
    VclPtr<PushButton> mpCategory6;
    VclPtr<PushButton> mpCategory7;
    VclPtr<PushButton> mpCategory8;
    VclPtr<PushButton> mpCategory9;
    VclPtr<PushButton> mpCategory10;

    VclPtr<EmojiView> mpEmojiView;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
