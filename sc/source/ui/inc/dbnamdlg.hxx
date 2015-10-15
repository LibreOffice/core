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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DBNAMDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DBNAMDLG_HXX

#include <vector>

#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

#include "anyrefdg.hxx"
#include "dbdata.hxx"

class ScViewData;
class ScDocument;

class ScDbNameDlg : public ScAnyRefDlg
{
public:
                    ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                 ScViewData*    ptrViewData );
                    virtual ~ScDbNameDlg();
    virtual void    dispose() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual bool    Close() override;

private:
    VclPtr<ComboBox>           m_pEdName;

    VclPtr<VclFrame>           m_pAssignFrame;
    VclPtr<formula::RefEdit>   m_pEdAssign;
    VclPtr<formula::RefButton> m_pRbAssign;

    VclPtr<VclContainer>       m_pOptions;
    VclPtr<CheckBox>           m_pBtnHeader;
    VclPtr<CheckBox>           m_pBtnTotals;
    VclPtr<CheckBox>           m_pBtnDoSize;
    VclPtr<CheckBox>           m_pBtnKeepFmt;
    VclPtr<CheckBox>           m_pBtnStripData;
    VclPtr<FixedText>          m_pFTSource;
    VclPtr<FixedText>          m_pFTOperations;

    VclPtr<OKButton>           m_pBtnOk;
    VclPtr<CancelButton>       m_pBtnCancel;
    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnRemove;

    bool            bSaved;

    OUString        aStrAdd;
    OUString        aStrModify;
    OUString        aStrInvalid;

    OUString        aStrSource;
    OUString        aStrOperations;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    bool            bRefInputMode;
    ScAddress::Details aAddrDetails;

    ScDBCollection  aLocalDbCol;
    ScRange         theCurArea;
    std::vector<ScRange> aRemoveList;

private:
    void            Init();
    void            UpdateNames();
    void            UpdateDBData( const OUString& rStrName );
    void            SetInfoStrings( const ScDBData* pDBData );

    DECL_LINK_TYPED( CancelBtnHdl, Button*, void );
    DECL_LINK_TYPED( OkBtnHdl, Button*, void );
    DECL_LINK_TYPED( AddBtnHdl, Button*, void );
    DECL_LINK_TYPED( RemoveBtnHdl, Button*, void );
    DECL_LINK_TYPED( NameModifyHdl, Edit&, void );
    DECL_LINK_TYPED( AssModifyHdl, Edit&, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_DBNAMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
