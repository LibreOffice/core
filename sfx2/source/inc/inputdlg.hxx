/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_INPUTDLG_HXX
#define INCLUDED_SFX2_SOURCE_INC_INPUTDLG_HXX

#include <vcl/dialog.hxx>

class Edit;
class FixedText;
class PushButton;

class InputDialog : public ModalDialog
{
public:
    InputDialog (const OUString &labelText, Window *pParent = NULL);
    OUString getEntryText () const;

private:

    DECL_LINK(ClickHdl, PushButton*);

private:

    Edit *m_pEntry;
    FixedText *m_pLabel;
    PushButton *m_pOK;
    PushButton *m_pCancel;
};

#endif // INCLUDED_SFX2_SOURCE_INC_INPUTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
