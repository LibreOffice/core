/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INPUTDLG_HXX
#define INPUTDLG_HXX

#include <vcl/dialog.hxx>

class Edit;
class FixedText;
class PushButton;

class InputDialog : public ModalDialog
{
public:

    InputDialog (const OUString &labelText, Window *pParent = NULL);

    virtual ~InputDialog();

    OUString getEntryText () const;

private:

    DECL_LINK(ClickHdl, PushButton*);

private:

    Edit *mpEntry;
    FixedText *mpLabel;
    PushButton *mpOK;
    PushButton *mpCancel;
};

#endif // INPUTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
