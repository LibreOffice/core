/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "anyrefdg.hxx"
#include <dpobject.hxx>
#include "viewdata.hxx"

#include "PivotLayoutTreeList.hxx"
#include "PivotLayoutTreeListData.hxx"
#include "PivotLayoutTreeListLabel.hxx"

class ScItemValue final
{
public:
    OUString maName;
    ScPivotFuncData maFunctionData;
    ScItemValue* mpOriginalItemValue;

    ScItemValue(OUString aName, SCCOL nColumn, PivotFunc nFunctionMask);
    ScItemValue(const ScItemValue* pInputItemValue);

    ~ScItemValue();
};

class ScPivotLayoutDialog : public ScAnyRefDlgController
{
public:
    ScDPObject maPivotTableObject;

    ScPivotLayoutTreeListBase* mpPreviouslyFocusedListBox;

private:
    ScViewData* mpViewData;
    ScDocument& mrDocument;

    bool        mbNewPivotTable;

    ScAddress::Details  maAddressDetails;
    bool                mbDialogLostFocus;

    formula::RefEdit* mpActiveEdit;
    std::unique_ptr<ScPivotLayoutTreeListLabel> mxListBoxField;
    std::unique_ptr<ScPivotLayoutTreeList> mxListBoxPage;
    std::unique_ptr<ScPivotLayoutTreeList> mxListBoxColumn;
    std::unique_ptr<ScPivotLayoutTreeList> mxListBoxRow;
    std::unique_ptr<ScPivotLayoutTreeListData>  mxListBoxData;

    std::unique_ptr<weld::CheckButton> mxCheckIgnoreEmptyRows;
    std::unique_ptr<weld::CheckButton> mxCheckTotalColumns;
    std::unique_ptr<weld::CheckButton> mxCheckAddFilter;
    std::unique_ptr<weld::CheckButton> mxCheckIdentifyCategories;
    std::unique_ptr<weld::CheckButton> mxCheckTotalRows;
    std::unique_ptr<weld::CheckButton> mxCheckDrillToDetail;
    std::unique_ptr<weld::CheckButton> mxCheckExpandCollapse;

    std::unique_ptr<weld::RadioButton> mxSourceRadioNamedRange;
    std::unique_ptr<weld::RadioButton> mxSourceRadioSelection;

    std::unique_ptr<weld::ComboBox> mxSourceListBox;
    std::unique_ptr<formula::RefEdit> mxSourceEdit;
    std::unique_ptr<formula::RefButton> mxSourceButton;

    std::unique_ptr<weld::RadioButton> mxDestinationRadioNewSheet;
    std::unique_ptr<weld::RadioButton> mxDestinationRadioNamedRange;
    std::unique_ptr<weld::RadioButton> mxDestinationRadioSelection;

    std::unique_ptr<weld::ComboBox> mxDestinationListBox;
    std::unique_ptr<formula::RefEdit> mxDestinationEdit;
    std::unique_ptr<formula::RefButton> mxDestinationButton;

    std::unique_ptr<weld::Button> mxBtnOK;
    std::unique_ptr<weld::Button> mxBtnCancel;

    std::unique_ptr<weld::Frame> mxSourceFrame;
    std::unique_ptr<weld::Label> mxSourceLabel;
    std::unique_ptr<weld::Frame> mxDestFrame;
    std::unique_ptr<weld::Label> mxDestLabel;

    std::unique_ptr<weld::Expander> mxOptions;
    std::unique_ptr<weld::Expander> mxMore;

    DECL_LINK(CancelClicked, weld::Button&, void);
    DECL_LINK(OKClicked, weld::Button&, void);
    DECL_LINK(GetEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(GetButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(LoseEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(LoseButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(ToggleSource, weld::Toggleable&, void);
    DECL_LINK(ToggleDestination, weld::Toggleable&, void);
    DECL_LINK(SourceListSelected, weld::ComboBox&, void);
    DECL_LINK(SourceEditModified, formula::RefEdit&, void);
    void ToggleSource();
    void ToggleDestination();
    virtual void Close() override;

    ScPivotParam maPivotParameters;

    // UI
    void SetupSource();
    void SetupDestination();
    void FillValuesToListBoxes();

    // Other
    bool GetDestination(ScRange& aDestinationRange, bool& bToNewSheet);

public:
    ScPivotLayoutDialog(SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow, weld::Window* pParent,
                             ScViewData* pViewData, const ScDPObject* pPivotTableObject, bool bCreateNewPivotTable);
    virtual ~ScPivotLayoutDialog() override;

    virtual void SetReference(const ScRange& rReferenceRange, ScDocument& rDocument) override;
    virtual void SetActive() override;
    virtual bool IsRefInputMode() const override;

    void ItemInserted(const ScItemValue* pItemValue, ScPivotLayoutTreeList::SvPivotTreeListType eType);

    void UpdateSourceRange();

    void ApplyChanges();
    void ApplySaveData(ScDPSaveData& rSaveData);
    void ApplyLabelData(const ScDPSaveData& rSaveData);

    ScItemValue* GetItem(SCCOL nColumn);
    bool IsDataElement(SCCOL nColumn);

    ScDPLabelData& GetLabelData(SCCOL nColumn);
    ScDPLabelDataVector& GetLabelDataVector() { return maPivotParameters.maLabelArray;}
    void PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

