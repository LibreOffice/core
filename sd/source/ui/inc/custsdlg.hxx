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

#pragma once

#include <vcl/weld.hxx>

class SdDrawDocument;
class SdCustomShow;
class SdCustomShowList;

class SdCustomShowDlg : public weld::GenericDialogController
{
private:
    SdDrawDocument& rDoc;
    SdCustomShowList* pCustomShowList;
    bool            bModified;

    std::unique_ptr<weld::TreeView> m_xLbCustomShows;
    std::unique_ptr<weld::CheckButton> m_xCbxUseCustomShow;
    std::unique_ptr<weld::Button> m_xBtnNew;
    std::unique_ptr<weld::Button> m_xBtnEdit;
    std::unique_ptr<weld::Button> m_xBtnRemove;
    std::unique_ptr<weld::Button> m_xBtnCopy;
    std::unique_ptr<weld::Button> m_xBtnHelp;
    std::unique_ptr<weld::Button> m_xBtnStartShow;
    std::unique_ptr<weld::Button> m_xBtnOK;

    void            CheckState();

    DECL_LINK( ClickButtonHdl, weld::Button&, void );
    DECL_LINK( SelectListBoxHdl, weld::TreeView&, void );
    DECL_LINK( StartShowHdl, weld::Button&, void );
    void SelectHdl(void const *);

public:
    SdCustomShowDlg(weld::Window* pWindow, SdDrawDocument& rDrawDoc);
    virtual ~SdCustomShowDlg() override;
    bool         IsModified() const { return bModified; }
    bool         IsCustomShow() const;
};

class SdDefineCustomShowDlg : public weld::GenericDialogController
{
private:
    SdDrawDocument& rDoc;
    std::unique_ptr<SdCustomShow>& rpCustomShow;
    bool            bModified;
    OUString        aOldName;

    std::unique_ptr<weld::Entry> m_xEdtName;
    std::unique_ptr<weld::TreeView> m_xLbPages;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;
    std::unique_ptr<weld::TreeView> m_xLbCustomPages;
    std::unique_ptr<weld::Button> m_xBtnOK;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnHelp;

    void            CheckState();
    void            CheckCustomShow();

    DECL_LINK( ClickButtonHdl, weld::Button&, void );
    DECL_LINK( ClickButtonEditHdl, weld::Entry&, void );
    DECL_LINK( ClickButtonHdl3, weld::TreeView&, void );
    DECL_LINK( ClickButtonHdl4, weld::TreeView&, void );
    DECL_LINK( OKHdl, weld::Button&, void );
    void ClickButtonHdl2(void const *);

public:

    SdDefineCustomShowDlg(weld::Window* pWindow, SdDrawDocument& rDrawDoc, std::unique_ptr<SdCustomShow>& rpCS);
    virtual ~SdDefineCustomShowDlg() override;

    bool            IsModified() const { return bModified; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
