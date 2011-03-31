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
