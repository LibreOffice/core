/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "inputdlg.hxx"

#include "inputdlg.hrc"

#include <sfx2/sfxresid.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

#define LABEL_TEXT_SPACE 5

InputDialog::InputDialog (const OUString &rLabelText, Window *pParent)
    : ModalDialog(pParent,SfxResId(DLG_INPUT_BOX)),
      mpEntry(new Edit(this,SfxResId(EDT_INPUT_FIELD))),
      mpLabel(new FixedText(this,SfxResId(LABEL_INPUT_TEXT))),
      mpOK(new PushButton(this,SfxResId(BTN_INPUT_OK))),
      mpCancel(new PushButton(this,SfxResId(BTN_INPUT_CANCEL)))
{
    SetStyle(GetStyle() | WB_CENTER | WB_VCENTER);

    mpLabel->SetText(rLabelText);

    // Fit label size to text and reposition edit box
    Size aLabelSize = mpLabel->CalcMinimumSize();
    Size aEditSize = mpEntry->GetSizePixel();
    Size aBtnSize = mpOK->GetSizePixel();

    Point aLabelPos = mpLabel->GetPosPixel();
    Point aEditPos = mpEntry->GetPosPixel();

    aEditPos.setX(aLabelPos.getX() + aLabelSize.getWidth() + LABEL_TEXT_SPACE);

    mpLabel->SetPosSizePixel(aLabelPos,aLabelSize);
    mpEntry->SetPosSizePixel(aEditPos,aEditSize);

    // Resize window if needed
    Size aWinSize = GetOutputSize();
    aWinSize.setWidth(aEditPos.getX() + aEditSize.getWidth() + LABEL_TEXT_SPACE);
    SetSizePixel(aWinSize);

    // Align buttons
    Point aBtnPos = mpCancel->GetPosPixel();

    aBtnPos.setX(aWinSize.getWidth() - aBtnSize.getWidth() - LABEL_TEXT_SPACE);
    mpCancel->SetPosPixel(aBtnPos);

    aBtnPos.setX(aBtnPos.getX() - aBtnSize.getWidth() - LABEL_TEXT_SPACE);
    mpOK->SetPosPixel(aBtnPos);

    mpOK->SetClickHdl(LINK(this,InputDialog,ClickHdl));
    mpCancel->SetClickHdl(LINK(this,InputDialog,ClickHdl));
}

InputDialog::~InputDialog()
{
    delete mpEntry;
    delete mpLabel;
    delete mpOK;
    delete mpCancel;
}

OUString InputDialog::getEntryText () const
{
    return mpEntry->GetText();
}

IMPL_LINK(InputDialog,ClickHdl,PushButton*, pButton)
{
    EndDialog(pButton == mpOK ? true : false);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


