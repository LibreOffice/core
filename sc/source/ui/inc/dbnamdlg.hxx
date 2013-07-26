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

#ifndef SC_DBNAMDLG_HXX
#define SC_DBNAMDLG_HXX

#include <vector>

#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

#include "anyrefdg.hxx"
#include "dbdata.hxx"
#include "expftext.hxx"

class ScViewData;
class ScDocument;


//============================================================================

class ScDbNameDlg : public ScAnyRefDlg
{
public:
                    ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScDbNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    ComboBox*           m_pEdName;

    VclFrame*           m_pAssignFrame;
    formula::RefEdit*   m_pEdAssign;
    formula::RefButton* m_pRbAssign;

    VclContainer*       m_pOptions;
    CheckBox*           m_pBtnHeader;
    CheckBox*           m_pBtnDoSize;
    CheckBox*           m_pBtnKeepFmt;
    CheckBox*           m_pBtnStripData;
    FixedText*          m_pFTSource;
    FixedText*          m_pFTOperations;

    OKButton*           m_pBtnOk;
    CancelButton*       m_pBtnCancel;
    PushButton*         m_pBtnAdd;
    PushButton*         m_pBtnRemove;

    sal_Bool            bSaved;

    OUString        aStrAdd;
    OUString        aStrModify;
    OUString        aStrInvalid;

    OUString        aStrSource;
    OUString        aStrOperations;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    sal_Bool            bRefInputMode;
    ScAddress::Details aAddrDetails;

    ScDBCollection  aLocalDbCol;
    ScRange         theCurArea;
    std::vector<ScRange> aRemoveList;

private:
    void            Init();
    void            UpdateNames();
    void            UpdateDBData( const String& rStrName );
    void            SetInfoStrings( const ScDBData* pDBData );

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( NameModifyHdl, void * );
    DECL_LINK( AssModifyHdl, void * );
};



#endif // SC_DBNAMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
