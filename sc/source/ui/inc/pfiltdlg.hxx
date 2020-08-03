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

#pragma once

#include <vcl/weld.hxx>
#include <address.hxx>
#include <queryparam.hxx>
#include <array>
#include <memory>

class ScViewData;
class ScDocument;
class ScQueryItem;
class SfxItemSet;
struct ScFilterEntries;

class ScPivotFilterDlg : public weld::GenericDialogController
{
public:
    ScPivotFilterDlg(weld::Window* pParent, const SfxItemSet& rArgSet, SCTAB nSourceTab);
    virtual ~ScPivotFilterDlg() override;

    const ScQueryItem&  GetOutputItem();

private:
    const OUString aStrNone;
    const OUString aStrEmpty;
    const OUString aStrNotEmpty;
    const OUString aStrColumn;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    std::unique_ptr<ScQueryItem> pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    std::unique_ptr<weld::ComboBox> m_xLbField1;
    std::unique_ptr<weld::ComboBox> m_xLbCond1;
    std::unique_ptr<weld::ComboBox> m_xEdVal1;

    std::unique_ptr<weld::ComboBox> m_xLbConnect1;
    std::unique_ptr<weld::ComboBox> m_xLbField2;
    std::unique_ptr<weld::ComboBox> m_xLbCond2;
    std::unique_ptr<weld::ComboBox> m_xEdVal2;

    std::unique_ptr<weld::ComboBox> m_xLbConnect2;
    std::unique_ptr<weld::ComboBox> m_xLbField3;
    std::unique_ptr<weld::ComboBox> m_xLbCond3;
    std::unique_ptr<weld::ComboBox> m_xEdVal3;

    std::unique_ptr<weld::CheckButton> m_xBtnCase;
    std::unique_ptr<weld::CheckButton> m_xBtnRegExp;
    std::unique_ptr<weld::CheckButton> m_xBtnUnique;
    std::unique_ptr<weld::Label> m_xFtDbArea;

    weld::ComboBox* aValueEdArr[3];
    weld::ComboBox* aFieldLbArr[3];
    weld::ComboBox* aCondLbArr[3];

    std::array<std::unique_ptr<ScFilterEntries>, MAXCOLCOUNT> m_pEntryLists;

private:
    void    Init            ( const SfxItemSet& rArgSet );
    void    FillFieldLists  ();
    void    UpdateValueList ( sal_uInt16 nList );
    void    ClearValueList  ( sal_uInt16 nList );
    sal_uInt16  GetFieldSelPos  ( SCCOL nField );

    // Handler:
    DECL_LINK( LbSelectHdl, weld::ComboBox&, void );
    DECL_LINK( ValModifyHdl, weld::ComboBox&, void );
    DECL_LINK( CheckBoxHdl, weld::Button&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
