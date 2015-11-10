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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_INSTABLE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_INSTABLE_HXX

#include <rtl/ustring.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <sfx2/basedlgs.hxx>
#include <actctrl.hxx>

class SwWrtShell;
class SwTableAutoFormat;
class SwView;
struct SwInsertTableOptions;

class SwInsTableDlg : public SfxModalDialog
{
    VclPtr<Edit>           m_pNameEdit;
    TextFilter      m_aTextFilter;

    VclPtr<NumericField>   m_pColNF;
    VclPtr<NumericField>   m_pRowNF;

    VclPtr<CheckBox>       m_pHeaderCB;
    VclPtr<CheckBox>       m_pRepeatHeaderCB;
    VclPtr<NumericField>   m_pRepeatHeaderNF;
    VclPtr<VclContainer>   m_pRepeatGroup;

    VclPtr<CheckBox>       m_pDontSplitCB;
    VclPtr<CheckBox>       m_pBorderCB;

    VclPtr<PushButton>     m_pInsertBtn;
    VclPtr<PushButton>     m_pAutoFormatBtn;

    SwWrtShell*     pShell;
    SwTableAutoFormat* pTAutoFormat;
    sal_Int64       nEnteredValRepeatHeaderNF;

    DECL_LINK_TYPED( ModifyName, Edit&, void );
    DECL_LINK_TYPED( ModifyRowCol, Edit&, void );
    DECL_LINK_TYPED( AutoFormatHdl, Button*, void );
    DECL_LINK_TYPED( OKHdl, Button*, void);
    DECL_LINK_TYPED( CheckBoxHdl, Button* = nullptr, void);
    DECL_LINK_TYPED( ReapeatHeaderCheckBoxHdl, Button* = nullptr, void);
    DECL_LINK_TYPED( ModifyRepeatHeaderNF_Hdl, Edit&, void );

public:
    SwInsTableDlg( SwView& rView );
    virtual ~SwInsTableDlg();
    virtual void dispose() override;

    void GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTableOpts, OUString& rTableAutoFormatName,
                    SwTableAutoFormat *& prTAFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
