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
#ifndef SCUI_AUTOFMT_HXX
#define SCUI_AUTOFMT_HXX
#include "autofmt.hxx"

class ScAutoFormatDlg : public ModalDialog
{
public:
    ScAutoFormatDlg(Window* pParent,
                    ScAutoFormat* pAutoFormat,
                    const ScAutoFormatData* pSelFormatData,
                    ScViewData *pViewData);

    sal_uInt16 GetIndex() const { return nIndex; }
    OUString GetCurrFormatName();

private:
    ListBox*        m_pLbFormat;
    ScAutoFmtPreview* m_pWndPreview;
    OKButton*       m_pBtnOk;
    CancelButton*   m_pBtnCancel;
    PushButton*     m_pBtnAdd;
    PushButton*     m_pBtnRemove;
    PushButton*     m_pBtnRename;
    CheckBox*       m_pBtnNumFormat;
    CheckBox*       m_pBtnBorder;
    CheckBox*       m_pBtnFont;
    CheckBox*       m_pBtnPattern;
    CheckBox*       m_pBtnAlignment;
    CheckBox*       m_pBtnAdjust;
    OUString        aStrTitle;
    OUString        aStrLabel;
    OUString        aStrClose;
    OUString        aStrDelTitle;
    OUString        aStrDelMsg;
    OUString        aStrRename;


    ScAutoFormat*           pFormat;
    const ScAutoFormatData* pSelFmtData;
    sal_uInt16                  nIndex;
    bool                    bCoreDataChanged;
    bool                    bFmtInserted;

    void Init           ();
    void UpdateChecks   ();

    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( SelFmtHdl, void * );
    DECL_LINK( CloseHdl, PushButton * );
    DECL_LINK( DblClkHdl, void * );
    DECL_LINK( RenameHdl, void *);

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
