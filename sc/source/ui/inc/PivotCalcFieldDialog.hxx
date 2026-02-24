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

#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/ComboBox.hxx>
#include <vcl/weld/Entry.hxx>

#include "viewdata.hxx"
#include <dpobject.hxx>
#include <pivot.hxx>

#include <memory>

class ScDocument;

class ScPivotCalcFieldDlg : public weld::GenericDialogController
{
public:
    SC_DLLPUBLIC ScPivotCalcFieldDlg(weld::Window* pParent, ScViewData& rData,
                                     const ScDPObject* pDPObj);
    virtual ~ScPivotCalcFieldDlg() override;

private:
    ScViewData& mrViewData;
    ScDocument& mrDocument;
    ScDPObject maPivotTableObject;

    OUString aStrAdd;
    OUString aStrModify;

    std::unique_ptr<weld::ComboBox> mxCalcNames;
    std::unique_ptr<weld::Entry> mxCalculation;

    std::unique_ptr<weld::Button> mxBtnAdd;
    std::unique_ptr<weld::Button> mxBtnMod;
    std::unique_ptr<weld::Button> mxBtnDel;

    std::unique_ptr<weld::TreeView> mxFieldList;
    std::unique_ptr<weld::Button> mxBtnInsert;

    std::unique_ptr<weld::Button> mxBtnOK;
    std::unique_ptr<weld::Button> mxBtnCancel;

    std::shared_ptr<weld::MessageDialog> m_xInfoBox;

    ScPivotParam maPivotParameters;
    std::vector<OUString> maDeletedFieldNames;

    void Init();
    void ErrorBox(const OUString& rString);
    bool IsExistingField(std::u16string_view rName) const;
    std::shared_ptr<ScTokenArray> ValidateFormula(const OUString& rFormula,
                                                  FormulaError* pError = nullptr);
    bool DoAddMod();
    void FillAllFields();
    void AddDummyNewField();
    void InsertSelectedField();
    void RemoveDeletedFieldsFromCache();
    void CompileCalcFieldFormulas();
    void ApplyChanges();
    void ApplySaveData(ScDPSaveData& rSaveData);
    void ApplyLabelData(const ScDPSaveData& rSaveData);

    DECL_LINK(CancelClicked, weld::Button&, void);
    DECL_LINK(OKClicked, weld::Button&, void);
    DECL_LINK(InsertClicked, weld::Button&, void);
    DECL_LINK(AddModClicked, weld::Button&, void);
    DECL_LINK(DeleteClicked, weld::Button&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(FieldListSelected, weld::TreeView&, void);
    DECL_LINK(CalcFieldNameSelected, weld::ComboBox&, void);
    DECL_LINK(CalcEntryChanged, weld::Entry&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
