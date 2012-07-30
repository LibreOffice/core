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
    FixedText           aColLbl;
    FixedText           aTypLbl;
    FixedText           aDirLbl;

    FixedLine           aSortFL;

    CheckBox            aKeyCB1;
    NumericField        aColEdt1;
    ListBox             aTypDLB1;
    RadioButton         aSortUpRB;
    RadioButton         aSortDnRB;

    CheckBox            aKeyCB2;
    NumericField        aColEdt2;
    ListBox             aTypDLB2;
    RadioButton         aSortUp2RB;
    RadioButton         aSortDn2RB;

    CheckBox            aKeyCB3;
    NumericField        aColEdt3;
    ListBox             aTypDLB3;
    RadioButton         aSortUp3RB;
    RadioButton         aSortDn3RB;

    FixedLine           aDirFL;
    RadioButton         aColumnRB;
    RadioButton         aRowRB;

    FixedLine           aDelimFL;
    RadioButton         aDelimTabRB;
    RadioButton         aDelimFreeRB;
    Edit                aDelimEdt;
    PushButton          aDelimPB;

    FixedLine           aLangFL;
    SvxLanguageBox      aLangLB;

    FixedLine           aSortOptFL;
    CheckBox            aCaseCB;

    OKButton            aOkBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

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
