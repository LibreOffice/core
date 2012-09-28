/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SRTDLG_HXX
#define _SRTDLG_HXX

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

    String aColTxt;
    String aRowTxt;
    String aNumericTxt;

    SwWrtShell          &rSh;
    CollatorResource* pColRes;

    sal_uInt16 nX;
    sal_uInt16 nY;

    virtual void        Apply();
    sal_Unicode         GetDelimChar() const;

    DECL_LINK( CheckHdl, CheckBox * );
    DECL_LINK( DelimHdl, RadioButton* );
    DECL_LINK( LanguageHdl, ListBox* );
    DECL_LINK(DelimCharHdl, void *);

public:
    SwSortDlg(Window * pParent, SwWrtShell &rSh);
    ~SwSortDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
