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
#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/weld.hxx>

namespace basctl
{

enum MacroExitCode {
    Macro_Close = 110,
    Macro_OkRun = 111,
    Macro_New   = 112,
    Macro_Edit  = 114,
};

class MacroChooser : public SfxDialogController
{
public:
    enum Mode {
        All = 1,
        ChooseOnly = 2,
        Recording = 3,
    };

private:
    OUString                       m_aMacrosInTxtBaseStr;

    // For forwarding to Assign dialog
    ::css::uno::Reference< ::css::frame::XFrame > m_xDocumentFrame;

    bool                    bForceStoreBasic;

    Mode                    nMode;

    DECL_LINK(MacroSelectHdl, weld::TreeView&, void);
    DECL_LINK(MacroDoubleClickHdl, weld::TreeView&, void);
    DECL_LINK(BasicSelectHdl, weld::TreeView&, void);
    DECL_LINK(EditModifyHdl, weld::Entry&, void);
    DECL_LINK(ButtonHdl, weld::Button&, void);

    void                CheckButtons();
    void                SaveSetCurEntry(weld::TreeView& rBox, const weld::TreeIter& rEntry);
    void                UpdateFields();

    void                EnableButton(weld::Button& rButton, bool bEnable);

    static OUString     GetInfo( SbxVariable* pVar );

    void                StoreMacroDescription();
    void                RestoreMacroDescription();

    std::unique_ptr<weld::Entry> m_xMacroNameEdit;
    std::unique_ptr<weld::Label> m_xMacroFromTxT;
    std::unique_ptr<weld::Label> m_xMacrosSaveInTxt;
    std::unique_ptr<SbTreeListBox> m_xBasicBox;
    std::unique_ptr<weld::TreeIter> m_xBasicBoxIter;
    std::unique_ptr<weld::Label> m_xMacrosInTxt;
    std::unique_ptr<weld::TreeView> m_xMacroBox;
    std::unique_ptr<weld::TreeIter> m_xMacroBoxIter;
    std::unique_ptr<weld::Button> m_xRunButton;
    std::unique_ptr<weld::Button> m_xCloseButton;
    std::unique_ptr<weld::Button> m_xAssignButton;
    std::unique_ptr<weld::Button> m_xEditButton;
    std::unique_ptr<weld::Button> m_xDelButton;
    std::unique_ptr<weld::Button> m_xNewButton;
    std::unique_ptr<weld::Button> m_xOrganizeButton;
    std::unique_ptr<weld::Button> m_xNewLibButton;
    std::unique_ptr<weld::Button> m_xNewModButton;
public:
    MacroChooser(weld::Window *pParent, const ::css::uno::Reference< ::css::frame::XFrame >& xDocFrame);
    virtual ~MacroChooser() override;

    SbMethod*           GetMacro();
    void                DeleteMacro();
    SbMethod*           CreateMacro();

    virtual short       run() override;

    void                SetMode (Mode);
    Mode                GetMode () const { return nMode; }
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_MACRODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
