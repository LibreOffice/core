/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "inputdlg.hxx"

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

#define LABEL_TEXT_SPACE 10
#define DIALOG_BORDER 10
#define MAX_FOLDER_NAME_LENGTH 20

InputDialog::InputDialog (const rtl::OUString &rLabelText, Window *pParent)
    : ModalDialog(pParent),
      mpEntry(new Edit(this)),
      mpLabel(new FixedText(this))
{
    SetStyle(GetStyle() | WB_CENTER | WB_VCENTER);

    Point aPos(DIALOG_BORDER,DIALOG_BORDER);

    Size aTextSize = mpLabel->CalcMinimumTextSize(mpLabel,100);
    Size aEntrySize = mpEntry->CalcSize(MAX_FOLDER_NAME_LENGTH);

    aTextSize.setWidth(aEntrySize.getHeight());

    mpLabel->SetPosPixel(Point(DIALOG_BORDER,DIALOG_BORDER));
    mpLabel->SetSizePixel(aTextSize);
    mpLabel->SetText(String("Enter name"));

    aPos.setX(DIALOG_BORDER + aTextSize.getWidth() + LABEL_TEXT_SPACE + DIALOG_BORDER);

    mpEntry->SetPosPixel(aPos);
    mpEntry->SetSizePixel(aEntrySize);

    // Set windows correct size
    SetSizePixel(Size(aTextSize.getWidth() + aEntrySize.getWidth() + 2*DIALOG_BORDER,
                      aTextSize.getHeight()+2*DIALOG_BORDER));

    mpEntry->Show();
    mpLabel->Show();
}

rtl::OUString InputDialog::getEntryText () const
{
    return mpEntry->GetText();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


