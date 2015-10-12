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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONVERT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONVERT_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <sfx2/basedlgs.hxx>

class SwTableAutoFormat;
class SwView;
class SwWrtShell;
class NumericField;
struct SwInsertTableOptions;

class SwConvertTableDlg: public SfxModalDialog
{
    VclPtr<RadioButton>    mpTabBtn;
    VclPtr<RadioButton>    mpSemiBtn;
    VclPtr<RadioButton>    mpParaBtn;
    VclPtr<RadioButton>    mpOtherBtn;
    VclPtr<Edit>           mpOtherEd;
    VclPtr<CheckBox>       mpKeepColumn;

    VclPtr<VclContainer>   mpOptions;

    VclPtr<CheckBox>       mpHeaderCB;
    VclPtr<CheckBox>       mpRepeatHeaderCB;

    VclPtr<VclContainer>   mpRepeatRows;
    VclPtr<NumericField>   mpRepeatHeaderNF;

    VclPtr<CheckBox>       mpDontSplitCB;
    VclPtr<CheckBox>       mpBorderCB;
    VclPtr<PushButton>     mpAutoFormatBtn;

    OUString        sConvertTextTable;
    SwTableAutoFormat* pTAutoFormat;
    SwWrtShell*     pShell;

    DECL_LINK_TYPED( AutoFormatHdl, Button*, void );
    DECL_LINK_TYPED( BtnHdl, Button*, void );
    DECL_LINK_TYPED( CheckBoxHdl, Button* = 0, void );
    DECL_LINK_TYPED( ReapeatHeaderCheckBoxHdl, Button* = 0, void );

public:
    SwConvertTableDlg( SwView& rView, bool bToTable );
    virtual ~SwConvertTableDlg();
    virtual void dispose() override;

    void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTableOpts,
                    SwTableAutoFormat const*& prTAFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
