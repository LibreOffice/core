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
#ifndef _INSTABLE_HXX
#define _INSTABLE_HXX

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <sfx2/basedlgs.hxx>
#include <actctrl.hxx>

class SwWrtShell;
class SwTableAutoFmt;
class SwView;
struct SwInsertTableOptions;


class SwInsTableDlg : public SfxModalDialog
{
    TableNameEdit*  m_pNameEdit;

    NumericField*   m_pColNF;
    NumericField*   m_pRowNF;

    CheckBox*       m_pHeaderCB;
    CheckBox*       m_pRepeatHeaderCB;
    NumericField*   m_pRepeatHeaderNF;
    VclContainer*   m_pRepeatGroup;

    CheckBox*       m_pDontSplitCB;
    CheckBox*       m_pBorderCB;

    PushButton*     m_pInsertBtn;
    PushButton*     m_pAutoFmtBtn;

    SwWrtShell*     pShell;
    SwTableAutoFmt* pTAutoFmt;
    sal_Int64       nEnteredValRepeatHeaderNF;

    DECL_LINK( ModifyName, Edit * );
    DECL_LINK( ModifyRowCol, NumericField * );
    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK(OKHdl, void*);
    DECL_LINK(CheckBoxHdl, void * = 0);
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );
    DECL_LINK( ModifyRepeatHeaderNF_Hdl, void* p = 0 );

public:
    SwInsTableDlg( SwView& rView );
    ~SwInsTableDlg();

    void GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTblOpts, OUString& rTableAutoFmtName,
                    SwTableAutoFmt *& prTAFmt );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
