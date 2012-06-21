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
#ifndef _TAB_SPLIT_HXX
#define _TAB_SPLIT_HXX

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>

#include <vcl/button.hxx>

#include <vcl/button.hxx>

class SwWrtShell;

class SwSplitTableDlg : public SvxStandardDialog
{
    FixedLine            aCountFL;
    FixedText           aCountLbl;
    NumericField        aCountEdit;
    FixedLine            aDirFL;
    ImageRadioButton    aHorzBox;
    ImageRadioButton    aVertBox;
    CheckBox            aPropCB;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;
    SwWrtShell&         rSh;

protected:
    virtual void Apply();

public:
    SwSplitTableDlg(Window *pParent, SwWrtShell& rShell );
    DECL_LINK( ClickHdl, Button * );

    sal_Bool                IsHorizontal() const { return aHorzBox.IsChecked(); }
    sal_Bool                IsProportional() const { return aPropCB.IsChecked() && aHorzBox.IsChecked(); }
    long                GetCount() const { return sal::static_int_cast< long >(aCountEdit.GetValue()); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
