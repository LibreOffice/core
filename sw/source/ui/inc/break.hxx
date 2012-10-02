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
#ifndef _BREAK_HXX
#define _BREAK_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/field.hxx>

class SwWrtShell;

class SwBreakDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;
    RadioButton*    m_pLineBtn;
    RadioButton*    m_pColumnBtn;
    RadioButton*    m_pPageBtn;
    FixedText*      m_pPageCollText;
    ListBox*        m_pPageCollBox;
    CheckBox*       m_pPageNumBox;
    NumericField*   m_pPageNumEdit;

    String          aTemplate;
    sal_uInt16          nKind;
    sal_uInt16          nPgNum;

    sal_Bool            bHtmlMode;

    DECL_LINK( ClickHdl, void * );
    DECL_LINK( PageNumHdl, CheckBox * );
    DECL_LINK(PageNumModifyHdl, void *);
    DECL_LINK(OkHdl, void *);

    void CheckEnable();

protected:
    virtual void Apply();

public:
    SwBreakDlg( Window *pParent, SwWrtShell &rSh );
    ~SwBreakDlg();

    String  GetTemplateName() { return aTemplate; }
    sal_uInt16  GetKind() { return nKind; }
    sal_uInt16  GetPageNumber() { return nPgNum; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
