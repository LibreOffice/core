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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TAUTOFMT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TAUTOFMT_HXX

#include <tools/link.hxx>
#include <sfx2/basedlgs.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vcl/vclreferencebase.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/window.hxx>

#include "wrtsh.hxx"
#include "autoformatpreview.hxx"
#include <tblafmt.hxx>

class SwTableAutoFormat;
class SwTableAutoFormatTable;
class SwWrtShell;

enum AutoFormatLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

class SwAutoFormatDlg : public SfxModalDialog
{
    VclPtr<ListBox>        m_pLbFormat;
    VclPtr<VclContainer>   m_pFormatting;
    VclPtr<CheckBox>       m_pBtnNumFormat;
    VclPtr<CheckBox>       m_pBtnBorder;
    VclPtr<CheckBox>       m_pBtnFont;
    VclPtr<CheckBox>       m_pBtnPattern;
    VclPtr<CheckBox>       m_pBtnAlignment;
    VclPtr<OKButton>       m_pBtnOk;
    VclPtr<CancelButton>   m_pBtnCancel;
    VclPtr<PushButton>     m_pBtnAdd;
    VclPtr<PushButton>     m_pBtnRemove;
    VclPtr<PushButton>     m_pBtnRename;
    OUString        aStrTitle;
    OUString        aStrLabel;
    OUString        aStrClose;
    OUString        aStrDelTitle;
    OUString        aStrDelMsg;
    OUString        aStrRenameTitle;
    OUString        aStrInvalidFormat;
    VclPtr<AutoFormatPreview> m_pWndPreview;

    SwWrtShell*             pShell;
    SwTableAutoFormatTable*     pTableTable;
    sal_uInt8                   nIndex;
    sal_uInt8                   nDfltStylePos;
    bool                    bCoreDataChanged : 1;
    bool                    bSetAutoFormat : 1;

    void Init( const SwTableAutoFormat* pSelFormat );
    void UpdateChecks( const SwTableAutoFormat&, bool bEnableBtn );

    DECL_LINK( CheckHdl, Button*, void );
    DECL_LINK(OkHdl, Button*, void);
    DECL_LINK( AddHdl, Button*, void );
    DECL_LINK( RemoveHdl, Button*, void );
    DECL_LINK( RenameHdl, Button*, void );
    DECL_LINK( SelFormatHdl, ListBox&, void );

public:
    SwAutoFormatDlg( vcl::Window* pParent, SwWrtShell* pShell,
                        bool bSetAutoFormat,
                        const SwTableAutoFormat* pSelFormat );
    virtual ~SwAutoFormatDlg() override;
    virtual void dispose() override;

    void FillAutoFormatOfIndex( SwTableAutoFormat*& rToFill ) const;
};

#endif // SW_AUTOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
