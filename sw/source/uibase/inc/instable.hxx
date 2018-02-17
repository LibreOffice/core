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

#include <vcl/button.hxx>
#include <sfx2/basedlgs.hxx>
#include <autoformatpreview.hxx>

class SwInsTableDlg : public SfxModalDialog
{
    TextFilter      m_aTextFilter;
    VclPtr<Edit>    m_pNameEdit;

    VclPtr<NumericField>    m_pColNF;
    VclPtr<NumericField>    m_pRowNF;

    VclPtr<CheckBox>        m_pHeaderCB;
    VclPtr<CheckBox>        m_pRepeatHeaderCB;
    VclPtr<NumericField>    m_pRepeatHeaderNF;
    VclPtr<VclContainer>    m_pRepeatGroup;

    VclPtr<CheckBox>        m_pDontSplitCB;
    VclPtr<CheckBox>        m_pBorderCB;

    VclPtr<PushButton>      m_pInsertBtn;

    VclPtr<ListBox>         m_pLbFormat;

    VclPtr<AutoFormatPreview> m_pWndPreview;

    SwWrtShell*     pShell;
    SwTableAutoFormatTable* pTableTable;
    SwTableAutoFormat*      pTAutoFormat;

    sal_uInt8       lbIndex;
    sal_uInt8       tbIndex;
    sal_uInt8       minTableIndexInLb;
    sal_uInt8       maxTableIndexInLb;
    sal_Int64       nEnteredValRepeatHeaderNF;

    // Returns 255 if mapping is not possible.
    // This means there cannot be more than 255 autotable style.
    sal_uInt8 lbIndexToTableIndex( const sal_uInt8 listboxIndex );
    void InitAutoTableFormat();
    void UpdateChecks( const SwTableAutoFormat& rFormat, bool bEnable );

    DECL_LINK( SelFormatHdl, ListBox&, void );
    DECL_LINK( CheckHdl, Button*, void );
    DECL_LINK( ModifyName, Edit&, void );
    DECL_LINK( ModifyRowCol, Edit&, void );
    DECL_LINK( OKHdl, Button*, void );
    DECL_LINK( CheckBoxHdl, Button*, void );
    DECL_LINK( ReapeatHeaderCheckBoxHdl, Button*, void );
    DECL_LINK( ModifyRepeatHeaderNF_Hdl, Edit&, void );

public:
    SwInsTableDlg( SwView& rView );
    virtual ~SwInsTableDlg() override;
    virtual void dispose() override;

    void GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTableOpts, OUString& rTableAutoFormatName,
                    SwTableAutoFormat *& prTAFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
