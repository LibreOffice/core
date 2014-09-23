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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_MACRODLG_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_MACRODLG_HXX

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
    Edit*                   m_pMacroNameEdit;
    FixedText*              m_pMacroFromTxT;
    FixedText*              m_pMacrosSaveInTxt;
    TreeListBox*            m_pBasicBox;
    FixedText*              m_pMacrosInTxt;
    OUString                m_aMacrosInTxtBaseStr;
    SvTreeListBox*          m_pMacroBox;

    PushButton*             m_pRunButton;
    CloseButton*            m_pCloseButton;
    PushButton*             m_pAssignButton;
    PushButton*             m_pEditButton;
    PushButton*             m_pDelButton;
    PushButton*             m_pOrganizeButton;
    PushButton*             m_pNewLibButton;
    PushButton*             m_pNewModButton;

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

    OUString            GetInfo( SbxVariable* pVar );

    void                StoreMacroDescription();
    void                RestoreMacroDescription();

public:
                        MacroChooser( vcl::Window* pParent, bool bCreateEntries = true );
                        virtual ~MacroChooser();

    SbMethod*           GetMacro();
    void                DeleteMacro();
    SbMethod*           CreateMacro();

    virtual short       Execute() SAL_OVERRIDE;

    void                SetMode (Mode);
    Mode                GetMode () const { return nMode; }
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_MACRODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
