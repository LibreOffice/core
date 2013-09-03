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


#ifndef _SD_CUSTSDLG_HXX
#define _SD_CUSTSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/dialog.hxx>

class SdDrawDocument;
class SdCustomShow;
class SdCustomShowList;

//------------------------------------------------------------------------

class SdCustomShowDlg : public ModalDialog
{
private:
    ListBox*         m_pLbCustomShows;
    CheckBox*        m_pCbxUseCustomShow;
    PushButton*      m_pBtnNew;
    PushButton*      m_pBtnEdit;
    PushButton*      m_pBtnRemove;
    PushButton*      m_pBtnCopy;
    HelpButton*      m_pBtnHelp;
    PushButton*      m_pBtnStartShow;
    OKButton*        m_pBtnOK;

    SdDrawDocument& rDoc;
    SdCustomShowList* pCustomShowList;
    SdCustomShow*   pCustomShow;
    sal_Bool            bModified;

    void            CheckState();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( StartShowHdl, void* );

public:
                SdCustomShowDlg( Window* pWindow, SdDrawDocument& rDrawDoc );
                ~SdCustomShowDlg();

    sal_Bool        IsModified() const { return( bModified ); }
    sal_Bool        IsCustomShow() const;
};


//------------------------------------------------------------------------

class SdDefineCustomShowDlg : public ModalDialog
{
private:
    Edit*            m_pEdtName;
    ListBox*         m_pLbPages;
    PushButton*      m_pBtnAdd;
    PushButton*      m_pBtnRemove;
    SvTreeListBox*   m_pLbCustomPages;
    OKButton*        m_pBtnOK;
    CancelButton*    m_pBtnCancel;
    HelpButton*      m_pBtnHelp;

    SdDrawDocument& rDoc;
    SdCustomShow*&  rpCustomShow;
    sal_Bool            bModified;
    OUString        aOldName;

    void            CheckState();
    void            CheckCustomShow();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( OKHdl, void * );

public:

                    SdDefineCustomShowDlg( Window* pWindow,
                            SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS );
                    ~SdDefineCustomShowDlg();

    sal_Bool            IsModified() const { return( bModified ); }
};

#endif // _SD_CUSTSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
