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
#ifndef INCLUDED_SW_SOURCE_UI_INC_TAUTOFMT_HXX
#define INCLUDED_SW_SOURCE_UI_INC_TAUTOFMT_HXX
#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include <vcl/morebtn.hxx>

#include <vcl/virdev.hxx>

class SwTableAutoFmt;
class AutoFmtPreview;
class SwTableAutoFmtTbl;
class SwWrtShell;

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

class SwAutoFormatDlg : public SfxModalDialog
{
    ListBox*        m_pLbFormat;
    VclContainer*   m_pFormatting;
    CheckBox*       m_pBtnNumFormat;
    CheckBox*       m_pBtnBorder;
    CheckBox*       m_pBtnFont;
    CheckBox*       m_pBtnPattern;
    CheckBox*       m_pBtnAlignment;
    OKButton*       m_pBtnOk;
    CancelButton*   m_pBtnCancel;
    PushButton*     m_pBtnAdd;
    PushButton*     m_pBtnRemove;
    PushButton*     m_pBtnRename;
    OUString        aStrTitle;
    OUString        aStrLabel;
    OUString        aStrClose;
    OUString        aStrDelTitle;
    OUString        aStrDelMsg;
    OUString        aStrRenameTitle;
    OUString        aStrInvalidFmt;
    AutoFmtPreview* m_pWndPreview;

    SwWrtShell*             pShell;
    SwTableAutoFmtTbl*      pTableTbl;
    sal_uInt8                   nIndex;
    sal_uInt8                   nDfltStylePos;
    sal_Bool                    bCoreDataChanged : 1;
    sal_Bool                    bSetAutoFmt : 1;

    void Init( const SwTableAutoFmt* pSelFmt );
    void UpdateChecks( const SwTableAutoFmt&, sal_Bool bEnableBtn );

    DECL_LINK( CheckHdl, Button * );
    DECL_LINK(OkHdl, void *);
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( RenameHdl, void * );
    DECL_LINK( SelFmtHdl, void * );

public:
    SwAutoFormatDlg( Window* pParent, SwWrtShell* pShell,
                        sal_Bool bSetAutoFmt = sal_True,
                        const SwTableAutoFmt* pSelFmt = 0 );
    virtual ~SwAutoFormatDlg();

    void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const;
};

#endif // SW_AUTOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
