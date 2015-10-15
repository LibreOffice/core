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
#include <sfx2/basedlgs.hxx>

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
    VclPtr<Edit>                   m_pMacroNameEdit;
    VclPtr<FixedText>              m_pMacroFromTxT;
    VclPtr<FixedText>              m_pMacrosSaveInTxt;
    VclPtr<TreeListBox>            m_pBasicBox;
    VclPtr<FixedText>              m_pMacrosInTxt;
    OUString                m_aMacrosInTxtBaseStr;
    VclPtr<SvTreeListBox>          m_pMacroBox;

    VclPtr<PushButton>             m_pRunButton;
    VclPtr<CloseButton>            m_pCloseButton;
    VclPtr<PushButton>             m_pAssignButton;
    VclPtr<PushButton>             m_pEditButton;
    VclPtr<PushButton>             m_pDelButton;
    VclPtr<PushButton>             m_pOrganizeButton;
    VclPtr<PushButton>             m_pNewLibButton;
    VclPtr<PushButton>             m_pNewModButton;

    bool                    bNewDelIsDel;
    bool                    bForceStoreBasic;

    Mode                    nMode;

    DECL_LINK_TYPED( MacroSelectHdl, SvTreeListBox *, void );
    DECL_LINK_TYPED( MacroDoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( BasicSelectHdl, SvTreeListBox *, void );
    DECL_LINK_TYPED( EditModifyHdl, Edit&, void );
    DECL_LINK_TYPED( ButtonHdl, Button *, void );

    void                CheckButtons();
    void                SaveSetCurEntry( SvTreeListBox& rBox, SvTreeListEntry* pEntry );
    void                UpdateFields();

    void                EnableButton( Button& rButton, bool bEnable );

    static OUString     GetInfo( SbxVariable* pVar );

    void                StoreMacroDescription();
    void                RestoreMacroDescription();

public:
                        MacroChooser( vcl::Window* pParent, bool bCreateEntries = true );
                        virtual ~MacroChooser();
    virtual void        dispose() override;

    SbMethod*           GetMacro();
    void                DeleteMacro();
    SbMethod*           CreateMacro();

    virtual short       Execute() override;

    void                SetMode (Mode);
    Mode                GetMode () const { return nMode; }
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_MACRODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
