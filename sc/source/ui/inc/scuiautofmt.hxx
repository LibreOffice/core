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
#ifndef INCLUDED_SC_SOURCE_UI_INC_SCUIAUTOFMT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCUIAUTOFMT_HXX
#include "autofmt.hxx"

class ScAutoFormatDlg : public ModalDialog
{
public:
    ScAutoFormatDlg(vcl::Window* pParent,
                    ScAutoFormat* pAutoFormat,
                    const ScAutoFormatData* pSelFormatData,
                    ScViewData *pViewData);
    virtual ~ScAutoFormatDlg();
    virtual void dispose() override;

    sal_uInt16 GetIndex() const { return nIndex; }
    OUString GetCurrFormatName();

private:
    VclPtr<ListBox>        m_pLbFormat;
    VclPtr<ScAutoFmtPreview> m_pWndPreview;
    VclPtr<OKButton>       m_pBtnOk;
    VclPtr<CancelButton>   m_pBtnCancel;
    VclPtr<PushButton>     m_pBtnAdd;
    VclPtr<PushButton>     m_pBtnRemove;
    VclPtr<PushButton>     m_pBtnRename;
    VclPtr<CheckBox>       m_pBtnNumFormat;
    VclPtr<CheckBox>       m_pBtnBorder;
    VclPtr<CheckBox>       m_pBtnFont;
    VclPtr<CheckBox>       m_pBtnPattern;
    VclPtr<CheckBox>       m_pBtnAlignment;
    VclPtr<CheckBox>       m_pBtnAdjust;
    OUString        aStrTitle;
    OUString        aStrLabel;
    OUString        aStrClose;
    OUString        aStrDelMsg;
    OUString        aStrRename;

    ScAutoFormat*           pFormat;
    const ScAutoFormatData* pSelFmtData;
    sal_uInt16                  nIndex;
    bool                    bCoreDataChanged;
    bool                    bFmtInserted;

    void Init           ();
    void UpdateChecks   ();

    DECL_LINK_TYPED( CheckHdl, Button*, void );
    DECL_LINK_TYPED( AddHdl, Button*, void );
    DECL_LINK_TYPED( RemoveHdl, Button*, void );
    DECL_LINK_TYPED( SelFmtHdl, ListBox&, void );
    DECL_LINK_TYPED( CloseHdl, Button *, void );
    DECL_LINK_TYPED( DblClkHdl, ListBox&, void );
    DECL_LINK_TYPED( RenameHdl, Button*, void );

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
