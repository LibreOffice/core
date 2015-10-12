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

#ifndef INCLUDED_SD_SOURCE_UI_INC_CUSTSDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_CUSTSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/dialog.hxx>

class SdDrawDocument;
class SdCustomShow;
class SdCustomShowList;

class SdCustomShowDlg : public ModalDialog
{
private:
    VclPtr<ListBox>         m_pLbCustomShows;
    VclPtr<CheckBox>        m_pCbxUseCustomShow;
    VclPtr<PushButton>      m_pBtnNew;
    VclPtr<PushButton>      m_pBtnEdit;
    VclPtr<PushButton>      m_pBtnRemove;
    VclPtr<PushButton>      m_pBtnCopy;
    VclPtr<HelpButton>      m_pBtnHelp;
    VclPtr<PushButton>      m_pBtnStartShow;
    VclPtr<OKButton>        m_pBtnOK;

    SdDrawDocument& rDoc;
    SdCustomShowList* pCustomShowList;
    SdCustomShow*   pCustomShow;
    bool            bModified;

    void            CheckState();

    DECL_LINK_TYPED( ClickButtonHdl, Button*, void );
    DECL_LINK_TYPED( SelectListBoxHdl, ListBox&, void );
    DECL_LINK_TYPED( StartShowHdl, Button*, void );
    void SelectHdl(void*);

public:
                SdCustomShowDlg( vcl::Window* pWindow, SdDrawDocument& rDrawDoc );
                virtual ~SdCustomShowDlg();
    virtual void dispose() override;
    bool         IsModified() const { return bModified; }
    bool         IsCustomShow() const;
};

class SdDefineCustomShowDlg : public ModalDialog
{
private:
    VclPtr<Edit>            m_pEdtName;
    VclPtr<ListBox>         m_pLbPages;
    VclPtr<PushButton>      m_pBtnAdd;
    VclPtr<PushButton>      m_pBtnRemove;
    VclPtr<SvTreeListBox>   m_pLbCustomPages;
    VclPtr<OKButton>        m_pBtnOK;
    VclPtr<CancelButton>    m_pBtnCancel;
    VclPtr<HelpButton>      m_pBtnHelp;

    SdDrawDocument& rDoc;
    SdCustomShow*&  rpCustomShow;
    bool            bModified;
    OUString        aOldName;

    void            CheckState();
    void            CheckCustomShow();

    DECL_LINK_TYPED( ClickButtonHdl, Button*, void );
    DECL_LINK( ClickButtonHdl2, void* );
    DECL_LINK_TYPED( ClickButtonHdl3, SvTreeListBox*, void );
    DECL_LINK_TYPED( ClickButtonHdl4, ListBox&, void );
    DECL_LINK_TYPED( OKHdl, Button*, void );

public:

                    SdDefineCustomShowDlg( vcl::Window* pWindow,
                            SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS );
                    virtual ~SdDefineCustomShowDlg();
    virtual void    dispose() override;

    bool            IsModified() const { return bModified; }
};

#endif // INCLUDED_SD_SOURCE_UI_INC_CUSTSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
