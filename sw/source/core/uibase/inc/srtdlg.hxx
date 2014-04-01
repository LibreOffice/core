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
#ifndef INCLUDED_SW_SOURCE_UI_INC_SRTDLG_HXX
#define INCLUDED_SW_SOURCE_UI_INC_SRTDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svx/stddlg.hxx>
#include <svx/langbox.hxx>

class SwWrtShell;
class CollatorResource;

class SwSortDlg : public SvxStandardDialog
{
    FixedText*          m_pColLbl;
    FixedText*          m_pTypLbl;

    CheckBox*           m_pKeyCB1;
    NumericField*       m_pColEdt1;
    ListBox*            m_pTypDLB1;
    RadioButton*        m_pSortUp1RB;
    RadioButton*        m_pSortDn1RB;

    CheckBox*           m_pKeyCB2;
    NumericField*       m_pColEdt2;
    ListBox*            m_pTypDLB2;
    RadioButton*        m_pSortUp2RB;
    RadioButton*        m_pSortDn2RB;

    CheckBox*           m_pKeyCB3;
    NumericField*       m_pColEdt3;
    ListBox*            m_pTypDLB3;
    RadioButton*        m_pSortUp3RB;
    RadioButton*        m_pSortDn3RB;

    RadioButton*        m_pColumnRB;
    RadioButton*        m_pRowRB;

    RadioButton*        m_pDelimTabRB;
    RadioButton*        m_pDelimFreeRB;
    Edit*               m_pDelimEdt;
    PushButton*         m_pDelimPB;

    SvxLanguageBox*     m_pLangLB;

    CheckBox*           m_pCaseCB;

    OUString aColTxt;
    OUString aRowTxt;
    OUString aNumericTxt;

    SwWrtShell          &rSh;
    CollatorResource* pColRes;

    sal_uInt16 nX;
    sal_uInt16 nY;

    virtual void        Apply() SAL_OVERRIDE;
    sal_Unicode         GetDelimChar() const;

    DECL_LINK( CheckHdl, CheckBox * );
    DECL_LINK( DelimHdl, RadioButton* );
    DECL_LINK( LanguageHdl, ListBox* );
    DECL_LINK(DelimCharHdl, void *);

public:
    SwSortDlg(Window * pParent, SwWrtShell &rSh);
    virtual ~SwSortDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
