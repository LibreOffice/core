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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PFILTDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PFILTDLG_HXX

#include <vcl/morebtn.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include "address.hxx"
#include "queryparam.hxx"

class ScViewData;
class ScDocument;
class ScQueryItem;
class ScTypedStrData;

class ScPivotFilterDlg : public ModalDialog
{
public:
    ScPivotFilterDlg(vcl::Window* pParent, const SfxItemSet& rArgSet, SCTAB nSourceTab);
    virtual ~ScPivotFilterDlg();
    virtual void dispose() SAL_OVERRIDE;

    const ScQueryItem&  GetOutputItem();

private:
    VclPtr<ListBox>        m_pLbField1;
    VclPtr<ListBox>        m_pLbCond1;
    VclPtr<ComboBox>       m_pEdVal1;

    VclPtr<ListBox>        m_pLbConnect1;
    VclPtr<ListBox>        m_pLbField2;
    VclPtr<ListBox>        m_pLbCond2;
    VclPtr<ComboBox>       m_pEdVal2;

    VclPtr<ListBox>        m_pLbConnect2;
    VclPtr<ListBox>        m_pLbField3;
    VclPtr<ListBox>        m_pLbCond3;
    VclPtr<ComboBox>       m_pEdVal3;

    VclPtr<CheckBox>       m_pBtnCase;
    VclPtr<CheckBox>       m_pBtnRegExp;
    VclPtr<CheckBox>       m_pBtnUnique;
    VclPtr<FixedText>      m_pFtDbArea;
    const OUString aStrNone;
    const OUString aStrEmpty;
    const OUString aStrNotEmpty;
    const OUString aStrColumn;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    sal_uInt16              nFieldCount;
    VclPtr<ComboBox>           aValueEdArr[3];
    VclPtr<ListBox>            aFieldLbArr[3];
    VclPtr<ListBox>            aCondLbArr[3];

    std::vector<ScTypedStrData>* pEntryLists[MAXCOLCOUNT];

private:
    void    Init            ( const SfxItemSet& rArgSet );
    void    FillFieldLists  ();
    void    UpdateValueList ( sal_uInt16 nList );
    void    ClearValueList  ( sal_uInt16 nList );
    sal_uInt16  GetFieldSelPos  ( SCCOL nField );

    // Handler:
    DECL_LINK_TYPED( LbSelectHdl, ListBox&, void );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK_TYPED( CheckBoxHdl, Button*, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_PFILTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
