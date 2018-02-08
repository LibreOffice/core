/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataproviderdlg.hxx>

#include <document.hxx>
#include <dataprovider.hxx>
#include <datatransformation.hxx>

#include <comphelper/string.hxx>

#include <vcl/lstbox.hxx>

constexpr int MENU_START = 0;
constexpr int MENU_COLUMN = 1;

class ScDataProviderBaseControl : public VclContainer,
                                    public VclBuilderContainer
{
    VclPtr<VclContainer> maGrid;
    VclPtr<ListBox> maProviderList;
    VclPtr<Edit> maEditURL;
    VclPtr<Edit> maEditID;
    VclPtr<PushButton> mpApplyBtn;

    bool mbDirty;
    OUString maOldProvider;
    OUString maURL;
    OUString maID;

    Link<Window*, void> maImportCallback;

    DECL_LINK(ProviderSelectHdl, ListBox&, void);
    DECL_LINK(IDEditHdl, Edit&, void);
    DECL_LINK(URLEditHdl, Edit&, void);
    DECL_LINK(ApplyBtnHdl, Button*, void);

    void updateApplyBtn(bool bValidConfig);

public:
    ScDataProviderBaseControl(vcl::Window* pParent, const Link<Window*, void>& rImportCallback);
    ~ScDataProviderBaseControl() override;

    virtual void dispose() override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual Size calculateRequisition() const override;

    void isValid();

    sc::ExternalDataSource getDataSource(ScDocument* pDoc);
};

ScDataProviderBaseControl::ScDataProviderBaseControl(vcl::Window* pParent,
        const Link<Window*, void>& rImportCallback):
    VclContainer(pParent, WB_CLIPCHILDREN | WB_BORDER),
    mbDirty(false),
    maImportCallback(rImportCallback)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "modules/scalc/ui/dataproviderentry.ui"));

    get(maGrid, "grid");
    get(maProviderList, "provider_lst");
    get(maEditURL, "ed_url");
    get(maEditID, "ed_id");

    auto aDataProvider = sc::DataProviderFactory::getDataProviders();
    for (auto& rDataProvider : aDataProvider)
    {
        maProviderList->InsertEntry(rDataProvider);
    }

    maProviderList->SetSelectHdl(LINK(this, ScDataProviderBaseControl, ProviderSelectHdl));
    maEditID->SetModifyHdl(LINK(this, ScDataProviderBaseControl, IDEditHdl));
    maEditURL->SetModifyHdl(LINK(this, ScDataProviderBaseControl, URLEditHdl));

    mpApplyBtn = VclPtr<PushButton>::Create(maGrid, WB_FLATBUTTON);
    mpApplyBtn->set_grid_top_attach(1);
    mpApplyBtn->set_grid_left_attach(5);
    mpApplyBtn->SetQuickHelpText("Apply Changes");
    mpApplyBtn->SetControlForeground(COL_GREEN);
    mpApplyBtn->SetControlBackground(COL_GREEN);
    mpApplyBtn->SetBackground(Wallpaper(COL_LIGHTGREEN));
    mpApplyBtn->SetModeImage(Image(BitmapEx("sc/res/xml_element.png")));
    mpApplyBtn->Show();
    mpApplyBtn->SetClickHdl(LINK(this, ScDataProviderBaseControl, ApplyBtnHdl));
    SetSizePixel(GetOptimalSize());
    isValid();
}

ScDataProviderBaseControl::~ScDataProviderBaseControl()
{
    disposeOnce();
}

void ScDataProviderBaseControl::dispose()
{
    maEditID.clear();
    maEditURL.clear();
    maProviderList.clear();
    mpApplyBtn.disposeAndClear();
    maGrid.clear();
    disposeBuilder();
    VclContainer::dispose();
}

Size ScDataProviderBaseControl::calculateRequisition() const
{
    return getLayoutRequisition(*maGrid);
}

void ScDataProviderBaseControl::setAllocation(const Size &rAllocation)
{
    setLayoutPosSize(*maGrid, Point(0, 0), rAllocation);
}

void ScDataProviderBaseControl::isValid()
{
    bool bValid = !maProviderList->GetSelectedEntry().isEmpty();
    bValid &= !maEditURL->GetText().isEmpty();

    if (bValid)
    {
        Color aColor = GetSettings().GetStyleSettings().GetDialogColor();
        SetBackground(aColor);
        maGrid->SetBackground(aColor);
        Invalidate();
        updateApplyBtn(true);
    }
    else
    {
        SetBackground(Wallpaper(COL_RED));
        maGrid->SetBackground(Wallpaper(COL_RED));
        Invalidate();
        updateApplyBtn(false);
    }
}

sc::ExternalDataSource ScDataProviderBaseControl::getDataSource(ScDocument* pDoc)
{
    OUString aURL = maEditURL->GetText();
    OUString aProvider = maProviderList->GetSelectedEntry();
    sc::ExternalDataSource aSource(aURL, aProvider, pDoc);

    OUString aID = maEditID->GetText();
    aSource.setID(aID);
    return aSource;
}

void ScDataProviderBaseControl::updateApplyBtn(bool bValidConfig)
{
    if (!bValidConfig)
    {
        mpApplyBtn->Disable();
        mpApplyBtn->SetBackground(Wallpaper(COL_RED));
        mpApplyBtn->SetQuickHelpText("");
        return;
    }

    if (mbDirty)
    {
        mpApplyBtn->Enable();
        mpApplyBtn->SetBackground(Wallpaper(COL_YELLOW));
        mpApplyBtn->SetQuickHelpText("Apply Changes");
    }
    else
    {
        mpApplyBtn->Disable();
        mpApplyBtn->SetBackground(Wallpaper(COL_GREEN));
        mpApplyBtn->SetQuickHelpText("Current Config Applied");
    }
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ProviderSelectHdl, ListBox&, void)
{
    isValid();
    mbDirty |= maOldProvider != maProviderList->GetSelectedEntry();
    maOldProvider = maProviderList->GetSelectedEntry();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, IDEditHdl, Edit&, void)
{
    isValid();
    mbDirty |= maEditID->GetText() != maID;
    maID = maEditID->GetText();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, URLEditHdl, Edit&, void)
{
    isValid();
    mbDirty |= maEditURL->GetText() != maURL;
    maURL = maEditURL->GetText();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ApplyBtnHdl, Button*, void)
{
    mbDirty = false;
    updateApplyBtn(true);
    maImportCallback.Call(this);
}

namespace {

struct MenuData
{
    int nMenuID;
    const char* aMenuName;
    std::function<void(ScDataProviderDlg*)> maCallback;
};

MenuData aStartData[] = {
    { 0, "Apply & Quit", &ScDataProviderDlg::applyAndQuit },
    { 1, "Cancel & Quit", &ScDataProviderDlg::cancelAndQuit }
};

MenuData aColumnData[] = {
    { 0, "Delete Column", &ScDataProviderDlg::deleteColumn },
    { 1, "Split Column", &ScDataProviderDlg::splitColumn },
    { 2, "Merge Columns", &ScDataProviderDlg::mergeColumns },
};

class ScDataTransformationBaseControl : public VclContainer,
                                    public VclBuilderContainer
{
    VclPtr<VclContainer> maGrid;

public:
    ScDataTransformationBaseControl(vcl::Window* pParent, const OUString& rUIFile);
    ~ScDataTransformationBaseControl() override;

    virtual void dispose() override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual Size calculateRequisition() const override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() = 0;
};

ScDataTransformationBaseControl::ScDataTransformationBaseControl(vcl::Window* pParent, const OUString& rUIFile):
    VclContainer(pParent, WB_BORDER | WB_CLIPCHILDREN)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), rUIFile));

    get(maGrid, "grid");
    SetSizePixel(GetOptimalSize());
}

ScDataTransformationBaseControl::~ScDataTransformationBaseControl()
{
    disposeOnce();
}

void ScDataTransformationBaseControl::dispose()
{
    maGrid.clear();

    VclContainer::dispose();
}

Size ScDataTransformationBaseControl::calculateRequisition() const
{
    return getLayoutRequisition(*maGrid);
}

void ScDataTransformationBaseControl::setAllocation(const Size &rAllocation)
{
    setLayoutPosSize(*maGrid, Point(0, 0), rAllocation);
}

class ScSplitColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maSeparator;
    VclPtr<NumericField> maNumColumns;
    SCCOL mnCol;

public:
    ScSplitColumnTransformationControl(vcl::Window* pParent, SCCOL nCol);
    ~ScSplitColumnTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
};

ScSplitColumnTransformationControl::ScSplitColumnTransformationControl(vcl::Window* pParent, SCCOL nCol):
    ScDataTransformationBaseControl(pParent, "modules/scalc/ui/splitcolumnentry.ui"),
    mnCol(nCol)
{
    get(maSeparator, "ed_separator");
    get(maNumColumns, "num_cols");
}

ScSplitColumnTransformationControl::~ScSplitColumnTransformationControl()
{
    disposeOnce();
}

void ScSplitColumnTransformationControl::dispose()
{
    maSeparator.clear();
    maNumColumns.clear();

    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScSplitColumnTransformationControl::getTransformation()
{
    OUString aSeparator = maSeparator->GetText();
    sal_Unicode cSeparator = aSeparator.isEmpty() ? ',' : aSeparator[0];
    return std::make_shared<sc::SplitColumnTransformation>(mnCol, cSeparator);
}

class ScMergeColumnTransformationControl : public ScDataTransformationBaseControl
{
private:

    VclPtr<Edit> mpSeparator;
    VclPtr<Edit> mpEdColumns;

public:
    ScMergeColumnTransformationControl(vcl::Window* pParent, SCCOL nStartCol, SCCOL nEndCol);

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
};

ScMergeColumnTransformationControl::ScMergeColumnTransformationControl(vcl::Window* pParent, SCCOL nStartCol, SCCOL nEndCol):
    ScDataTransformationBaseControl(pParent, "modules/scalc/ui/mergecolumnentry.ui")
{
    get(mpSeparator, "ed_separator");
    get(mpEdColumns, "ed_columns");

    OUStringBuffer aBuffer;

    // map from zero based to one based column numbers
    aBuffer.append(OUString::number(nStartCol + 1));
    for ( SCCOL nCol = nStartCol + 1; nCol <= nEndCol; ++nCol)
    {
        aBuffer.append(";").append(OUString::number(nCol + 1));
    }

    mpEdColumns->SetText(aBuffer.makeStringAndClear());
}

void ScMergeColumnTransformationControl::dispose()
{
    mpSeparator.clear();
    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScMergeColumnTransformationControl::getTransformation()
{
    OUString aColumnString = mpEdColumns->GetText();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aMergedColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aMergedColumns.insert(nCol - 1);
    }
    return std::make_shared<sc::MergeColumnTransformation>(aMergedColumns, mpSeparator->GetText());
}

}

ScDataProviderDlg::ScDataProviderDlg(vcl::Window* pParent, std::shared_ptr<ScDocument> pDoc, ScDocument* pDocument):
    ModalDialog(pParent, "dataproviderdlg", "modules/scalc/ui/dataproviderdlg.ui", true),
    mpDoc(pDoc),
    mpBar(VclPtr<MenuBar>::Create()),
    mpDocument(pDocument)
{
    get(mpTable, "data_table");
    get(mpList, "operation_ctrl");
    get(mpDBRanges, "select_db_range");
    mpTable->Init(mpDoc);

    ScDBCollection* pDBCollection = pDocument->GetDBCollection();
    auto& rNamedDBs = pDBCollection->getNamedDBs();
    for (auto& rNamedDB : rNamedDBs)
    {
        mpDBRanges->InsertEntry(rNamedDB->GetName());
    }

    mpDataProviderCtrl = VclPtr<ScDataProviderBaseControl>::Create(mpList, LINK(this, ScDataProviderDlg, ImportHdl));
    mpList->addEntry(mpDataProviderCtrl);

    pDBData = new ScDBData("data", 0, 0, 0, MAXCOL, MAXROW);
    bool bSuccess = mpDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    SAL_WARN_IF(!bSuccess, "sc", "temporary warning");

    InitMenu();
}

ScDataProviderDlg::~ScDataProviderDlg()
{
    disposeOnce();
}

void ScDataProviderDlg::dispose()
{
    mpDataProviderCtrl.clear();
    mpTable.clear();
    mpList.clear();
    mpBar.disposeAndClear();

    ModalDialog::dispose();
}

void ScDataProviderDlg::InitMenu()
{
    mpBar->InsertItem(MENU_START, "Start");
    VclPtrInstance<PopupMenu> pPopup;
    for (auto& itrStartData : aStartData)
    {
        pPopup->InsertItem(itrStartData.nMenuID, OUString::createFromAscii(itrStartData.aMenuName));
    }

    mpBar->SetPopupMenu(MENU_START, pPopup);
    pPopup->SetSelectHdl(LINK(this, ScDataProviderDlg, StartMenuHdl));

    mpBar->InsertItem(MENU_COLUMN, "Column");
    VclPtrInstance<PopupMenu> pColumnMenu;
    for (auto& itrColumnData : aColumnData)
 {
        pColumnMenu->InsertItem(itrColumnData.nMenuID, OUString::createFromAscii(itrColumnData.aMenuName));
    }
    pColumnMenu->SetSelectHdl(LINK(this, ScDataProviderDlg, ColumnMenuHdl));

    mpBar->SetPopupMenu(MENU_COLUMN, pColumnMenu);

    SetMenuBar(mpBar.get());
}

void ScDataProviderDlg::MouseButtonUp(const MouseEvent& rMEvt)
{
    VclPtr<FixedText> mpText = VclPtr<FixedText>::Create(mpList);
    mpText->SetText("Some Text " + OUString::number(rMEvt.GetPosPixel().X()) + "x" + OUString::number(rMEvt.GetPosPixel().getY()));
    mpText->SetSizePixel(Size(400, 20));
    mpList->addEntry(mpText);
}

IMPL_LINK(ScDataProviderDlg, StartMenuHdl, Menu*, pMenu, bool)
{
    for (auto& i: aStartData)
    {
        if (i.nMenuID == pMenu->GetCurItemId())
        {
            i.maCallback(this);
            return true;
        }
    }
    return true;
}

IMPL_LINK(ScDataProviderDlg, ColumnMenuHdl, Menu*, pMenu, bool)
{
    for (auto& i: aColumnData)
    {
        if (i.nMenuID == pMenu->GetCurItemId())
        {
            i.maCallback(this);
            return true;
        }
    }
    return true;
}

IMPL_LINK(ScDataProviderDlg, ImportHdl, Window*, pCtrl, void)
{
    if (pCtrl == mpDataProviderCtrl.get())
    {
        import();
    }
}

void ScDataProviderDlg::applyAndQuit()
{
    EndDialog(RET_OK);
}

void ScDataProviderDlg::cancelAndQuit()
{
    EndDialog(RET_CANCEL);
}

void ScDataProviderDlg::deleteColumn()
{
    VclPtr<FixedText> mpText = VclPtr<FixedText>::Create(mpList);
    mpText->SetText("Delete Column");
    mpText->SetSizePixel(Size(400, 20));
    mpList->addEntry(mpText);
}

void ScDataProviderDlg::splitColumn()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mpTable->getColRange(nStartCol, nEndCol);
    VclPtr<ScSplitColumnTransformationControl> pSplitColumnEntry = VclPtr<ScSplitColumnTransformationControl>::Create(mpList, nStartCol);
    mpList->addEntry(pSplitColumnEntry);
}

void ScDataProviderDlg::mergeColumns()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mpTable->getColRange(nStartCol, nEndCol);
    VclPtr<ScMergeColumnTransformationControl> pMergeColumnEntry = VclPtr<ScMergeColumnTransformationControl>::Create(mpList, nStartCol, nEndCol);
    mpList->addEntry(pMergeColumnEntry);
}

void ScDataProviderDlg::import()
{
    sc::ExternalDataSource aSource = mpDataProviderCtrl->getDataSource(mpDoc.get());
    std::vector<VclPtr<vcl::Window>> aListEntries = mpList->getEntries();
    for (size_t i = 1; i < aListEntries.size(); ++i)
    {
        ScDataTransformationBaseControl* pTransformationCtrl = dynamic_cast<ScDataTransformationBaseControl*>(aListEntries[i].get());
        if (!pTransformationCtrl)
        {
            SAL_WARN("sc", "all children except the provider should inherit from the base control");
            continue;
        }
        aSource.AddDataTransformation(pTransformationCtrl->getTransformation());
    }
    aSource.setDBData(pDBData);
    aSource.refresh(mpDoc.get(), true);
    mpTable->Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
