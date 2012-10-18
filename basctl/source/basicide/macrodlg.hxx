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
//
#ifndef BASCTL_MACRODLG_HXX
#define BASCTL_MACRODLG_HXX

#include <svheader.hxx>

#include <bastype2.hxx>
#include <bastype3.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

namespace basctl
{

enum MacroExitCode {
    Macro_Close = 10,
    Macro_OkRun = 11,
    Macro_New   = 12,
    Macro_Edit  = 14,
};

class MacroChooser : public SfxModalDialog
{
public:
    enum Mode {
        All = 1,
        ChooseOnly = 2,
        Recording = 3,
    };

private:
    FixedText               aMacroNameTxt;
    Edit                    aMacroNameEdit;
    FixedText               aMacroFromTxT;
    FixedText               aMacrosSaveInTxt;
    TreeListBox             aBasicBox;
    FixedText               aMacrosInTxt;
    String                  aMacrosInTxtBaseStr;
    SvTreeListBox           aMacroBox;

    PushButton              aRunButton;
    CancelButton            aCloseButton;
    PushButton              aAssignButton;
    PushButton              aEditButton;
    PushButton              aNewDelButton;
    PushButton              aOrganizeButton;
    HelpButton              aHelpButton;
    PushButton              aNewLibButton;
    PushButton              aNewModButton;

    bool                    bNewDelIsDel;
    bool                    bForceStoreBasic;

    Mode                    nMode;

    DECL_LINK( MacroSelectHdl, SvTreeListBox * );
    DECL_LINK(MacroDoubleClickHdl, void *);
    DECL_LINK( BasicSelectHdl, SvTreeListBox * );
    DECL_LINK( EditModifyHdl, Edit * );
    DECL_LINK( ButtonHdl, Button * );

    void                CheckButtons();
    void                SaveSetCurEntry( SvTreeListBox& rBox, SvTreeListEntry* pEntry );
    void                UpdateFields();

    void                EnableButton( Button& rButton, bool bEnable );

    String              GetInfo( SbxVariable* pVar );

    void                StoreMacroDescription();
    void                RestoreMacroDescription();

public:
                        MacroChooser( Window* pParent, bool bCreateEntries = true );
                        ~MacroChooser();

    SbMethod*           GetMacro();
    void                DeleteMacro();
    SbMethod*           CreateMacro();

    virtual short       Execute();

    void                SetMode (Mode);
    Mode                GetMode () const { return nMode; }
};

} // namespace basctl

#endif // BASCTL_MACRODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
