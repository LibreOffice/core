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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SRTDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SRTDLG_HXX

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
    VclPtr<FixedText>          m_pColLbl;
    VclPtr<FixedText>          m_pTypLbl;

    VclPtr<CheckBox>           m_pKeyCB1;
    VclPtr<NumericField>       m_pColEdt1;
    VclPtr<ListBox>            m_pTypDLB1;
    VclPtr<RadioButton>        m_pSortUp1RB;
    VclPtr<RadioButton>        m_pSortDn1RB;

    VclPtr<CheckBox>           m_pKeyCB2;
    VclPtr<NumericField>       m_pColEdt2;
    VclPtr<ListBox>            m_pTypDLB2;
    VclPtr<RadioButton>        m_pSortUp2RB;
    VclPtr<RadioButton>        m_pSortDn2RB;

    VclPtr<CheckBox>           m_pKeyCB3;
    VclPtr<NumericField>       m_pColEdt3;
    VclPtr<ListBox>            m_pTypDLB3;
    VclPtr<RadioButton>        m_pSortUp3RB;
    VclPtr<RadioButton>        m_pSortDn3RB;

    VclPtr<RadioButton>        m_pColumnRB;
    VclPtr<RadioButton>        m_pRowRB;

    VclPtr<RadioButton>        m_pDelimTabRB;
    VclPtr<RadioButton>        m_pDelimFreeRB;
    VclPtr<Edit>               m_pDelimEdt;
    VclPtr<PushButton>         m_pDelimPB;

    VclPtr<SvxLanguageBox>     m_pLangLB;

    VclPtr<CheckBox>           m_pCaseCB;

    OUString aColText;
    OUString aRowText;
    OUString aNumericText;

    SwWrtShell          &rSh;
    CollatorResource* pColRes;

    sal_uInt16 nX;
    sal_uInt16 nY;

    virtual void        Apply() SAL_OVERRIDE;
    sal_Unicode         GetDelimChar() const;

    DECL_LINK_TYPED( CheckHdl, Button*, void );
    DECL_LINK_TYPED( DelimHdl, Button*, void );
    DECL_LINK_TYPED( LanguageListBoxHdl, ListBox&, void );
    void LanguageHdl(ListBox*);
    DECL_LINK_TYPED(DelimCharHdl, Button*,void);

public:
    SwSortDlg(vcl::Window * pParent, SwWrtShell &rSh);
    virtual ~SwSortDlg();
    virtual void dispose() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
