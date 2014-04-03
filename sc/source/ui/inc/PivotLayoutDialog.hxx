/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef PIVOTLAYOUTDIALOG_HXX
#define PIVOTLAYOUTDIALOG_HXX

#include <boost/ptr_container/ptr_vector.hpp>

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

    ScPivotLayoutTreeListBase* mpPreviouslyFocusedListBox;
    ScPivotLayoutTreeListBase* mpCurrentlyFocusedListBox;

private:
    ScViewData* mpViewData;
    ScDocument* mpDocument;

    bool mbNewPivotTable;

    ScPivotLayoutTreeListLabel* mpListBoxField;
    ScPivotLayoutTreeList*      mpListBoxPage;
    ScPivotLayoutTreeList*      mpListBoxColumn;
    ScPivotLayoutTreeList*      mpListBoxRow;
    ScPivotLayoutTreeListData*  mpListBoxData;

    CheckBox* mpCheckIgnoreEmptyRows;
    CheckBox* mpCheckTotalColumns;
    CheckBox* mpCheckAddFilter;
    CheckBox* mpCheckIdentifyCategories;
    CheckBox* mpCheckTotalRows;
    CheckBox* mpCheckDrillToDetail;

    RadioButton* mpSourceRadioNamedRange;
    RadioButton* mpSourceRadioSelection;

    ListBox*            mpSourceListBox;
    formula::RefEdit*   mpSourceEdit;
    formula::RefButton* mpSourceButton;

    RadioButton*        mpDestinationRadioNewSheet;
    RadioButton*        mpDestinationRadioNamedRange;
    RadioButton*        mpDestinationRadioSelection;

    ListBox*            mpDestinationListBox;
    formula::RefEdit*   mpDestinationEdit;
    formula::RefButton* mpDestinationButton;

    PushButton*      mpButtonApply;
    OKButton*        mpButtonOk;
    CloseButton*     mpButtonClose;

    formula::RefEdit*   mpActiveEdit;
    ScAddress::Details  maAddressDetails;
    bool                mbDialogLostFocus;

    DECL_LINK(OkClicked,              PushButton*);
    DECL_LINK(CloseClicked,           PushButton*);
    DECL_LINK(ApplyClicked,           PushButton*);
    DECL_LINK(GetFocusHandler,        Control*);
    DECL_LINK(LoseFocusHandler,       void*);
    DECL_LINK(ToggleSource,           void*);
    DECL_LINK(ToggleDestination,      void*);
    DECL_LINK(SourceEditModified, void*);

    ScPivotParam maPivotParameters;

    // UI
    void SetupSource();
    void SetupDestination();
    void FillValuesToListBoxes();

    // Other
    bool GetDestination(ScRange& aDestinationRange, bool& bToNewSheet);

public:
    ScPivotLayoutDialog(SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow, Window* pParent,
                             ScViewData* pViewData, const ScDPObject* pPivotTableObject, bool bCreateNewPivotTable);
    virtual ~ScPivotLayoutDialog();

    virtual void SetReference(const ScRange& rReferenceRange, ScDocument* pDocument) SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual bool IsRefInputMode() const SAL_OVERRIDE;

    void ItemInserted(ScItemValue* pItemValue, ScPivotLayoutTreeList::SvPivotTreeListType eType);

    void UpdateSourceRange();

    bool ApplyChanges();
    void ApplySaveData(ScDPSaveData& rSaveData);
    void ApplyLabelData(ScDPSaveData& rSaveData);

    ScItemValue* GetItem(SCCOL nColumn);
    bool IsDataElement(SCCOL nColumn);

    ScDPLabelData* GetLabelData(SCCOL nColumn);
    ScDPLabelDataVector& GetLabelDataVector();
    void PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
