/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTDIALOG_HXX

#include <svx/checklbx.hxx>
#include <vcl/lstbox.hxx>
#include "anyrefdg.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "document.hxx"
#include "viewdata.hxx"

#include "PivotLayoutTreeList.hxx"
#include "PivotLayoutTreeListData.hxx"
#include "PivotLayoutTreeListLabel.hxx"

class ScItemValue
{
public:
    OUString maName;
    ScPivotFuncData maFunctionData;
    ScItemValue* mpOriginalItemValue;

    ScItemValue(OUString const & aName, SCCOL nColumn, sal_uInt16 nFunctionMask);
    ScItemValue(ScItemValue* pInputItemValue);

    virtual ~ScItemValue();
};

class ScPivotLayoutDialog : public ScAnyRefDlg
{
public:
    ScDPObject maPivotTableObject;

private:
    ScViewData* mpViewData;
    ScDocument* mpDocument;

    bool mbNewPivotTable;

    VclPtr<ScPivotLayoutTreeListLabel> mpListBoxField;
    VclPtr<ScPivotLayoutTreeList>      mpListBoxPage;
    VclPtr<ScPivotLayoutTreeList>      mpListBoxColumn;
    VclPtr<ScPivotLayoutTreeList>      mpListBoxRow;
    VclPtr<ScPivotLayoutTreeListData>  mpListBoxData;

    VclPtr<CheckBox> mpCheckIgnoreEmptyRows;
    VclPtr<CheckBox> mpCheckTotalColumns;
    VclPtr<CheckBox> mpCheckAddFilter;
    VclPtr<CheckBox> mpCheckIdentifyCategories;
    VclPtr<CheckBox> mpCheckTotalRows;
    VclPtr<CheckBox> mpCheckDrillToDetail;

    VclPtr<RadioButton> mpSourceRadioNamedRange;
    VclPtr<RadioButton> mpSourceRadioSelection;

    VclPtr<ListBox>            mpSourceListBox;
    VclPtr<formula::RefEdit>   mpSourceEdit;
    VclPtr<formula::RefButton> mpSourceButton;

    VclPtr<RadioButton>        mpDestinationRadioNewSheet;
    VclPtr<RadioButton>        mpDestinationRadioNamedRange;
    VclPtr<RadioButton>        mpDestinationRadioSelection;

    VclPtr<ListBox>            mpDestinationListBox;
    VclPtr<formula::RefEdit>   mpDestinationEdit;
    VclPtr<formula::RefButton> mpDestinationButton;

    VclPtr<PushButton>       mpBtnOK;
    VclPtr<CancelButton>     mpBtnCancel;

    VclPtr<formula::RefEdit>   mpActiveEdit;
    ScAddress::Details  maAddressDetails;
    bool                mbDialogLostFocus;

    DECL_LINK_TYPED(CancelClicked,       Button*, void);
    DECL_LINK_TYPED(OKClicked,           Button*, void);
    DECL_LINK_TYPED(GetFocusHandler,     Control&, void);
    DECL_LINK_TYPED(LoseFocusHandler,    Control&, void);
    DECL_LINK_TYPED(ToggleSource,        RadioButton&, void);
    DECL_LINK_TYPED(ToggleDestination,   RadioButton&, void);
    DECL_LINK_TYPED(SourceListSelected,  ListBox&, void);
    DECL_LINK_TYPED(SourceEditModified,  Edit&, void);
    void ToggleSource();
    void ToggleDestination();
    virtual bool Close() override;

    ScPivotParam maPivotParameters;

    // UI
    void SetupSource();
    void SetupDestination();
    void FillValuesToListBoxes();

    // Other
    bool GetDestination(ScRange& aDestinationRange, bool& bToNewSheet);

public:
    ScPivotLayoutDialog(SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow, vcl::Window* pParent,
                             ScViewData* pViewData, const ScDPObject* pPivotTableObject, bool bCreateNewPivotTable);
    virtual ~ScPivotLayoutDialog();
    virtual void dispose() override;

    virtual void SetReference(const ScRange& rReferenceRange, ScDocument* pDocument) override;
    virtual void SetActive() override;
    virtual bool IsRefInputMode() const override;

    void ItemInserted(ScItemValue* pItemValue, ScPivotLayoutTreeList::SvPivotTreeListType eType);

    void UpdateSourceRange();

    bool ApplyChanges();
    void ApplySaveData(ScDPSaveData& rSaveData);
    void ApplyLabelData(ScDPSaveData& rSaveData);

    ScItemValue* GetItem(SCCOL nColumn);
    bool IsDataElement(SCCOL nColumn);

    ScDPLabelData& GetLabelData(SCCOL nColumn);
    ScDPLabelDataVector& GetLabelDataVector() { return maPivotParameters.maLabelArray;}
    void PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames);

    ScPivotLayoutTreeListBase* FindListBoxFor(SvTreeListEntry *pEntry);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
