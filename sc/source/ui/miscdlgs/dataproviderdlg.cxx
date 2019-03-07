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
#include <datamapper.hxx>
#include <dbdata.hxx>

#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>

#include <utility>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>

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

    Link<Window*, void> const maImportCallback;

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
    mpApplyBtn->SetModeImage(Image(StockImage::Yes, "sc/res/xml_element.png"));
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
    Invalidate();
    updateApplyBtn(bValid);
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
        mpApplyBtn->SetQuickHelpText("");
        return;
    }
    else
    {
        mpApplyBtn->Enable();
        mpApplyBtn->SetBackground(Wallpaper(COL_YELLOW));
        mpApplyBtn->SetQuickHelpText("Apply Changes");
    }
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ProviderSelectHdl, ListBox&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, IDEditHdl, Edit&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, URLEditHdl, Edit&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ApplyBtnHdl, Button*, void)
{
    updateApplyBtn(true);
    maImportCallback.Call(this);
}

namespace {

struct MenuData
{
    int const nMenuID;
    const char* aMenuName;
    std::function<void(ScDataProviderDlg*)> const maCallback;
};

MenuData aStartData[] = {
    { 0, "Apply & Quit", &ScDataProviderDlg::applyAndQuit },
    { 1, "Cancel & Quit", &ScDataProviderDlg::cancelAndQuit }
};

MenuData aColumnData[] = {
    { 0, "Delete Column", &ScDataProviderDlg::deleteColumn },
    { 1, "Split Column", &ScDataProviderDlg::splitColumn },
    { 2, "Merge Columns", &ScDataProviderDlg::mergeColumns },
    { 3, "Text Transformation", &ScDataProviderDlg::textTransformation },
    { 4, "Sort Columns", &ScDataProviderDlg::sortTransformation },
    { 5, "Aggregate Functions", &ScDataProviderDlg::aggregateFunction},
    { 6, "Number Transformations", &ScDataProviderDlg::numberTransformation },
    { 7, "Replace Null Transformations", &ScDataProviderDlg::replaceNullTransformation },
    { 8, "Date & Time Transformations", &ScDataProviderDlg::dateTimeTransformation }
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

class ScDeleteColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maColumnNums;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScDeleteColumnTransformationControl(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScDeleteColumnTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScDeleteColumnTransformationControl::ScDeleteColumnTransformationControl(
    vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/deletecolumnentry.ui")
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(maColumnNums, "ed_columns");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScDeleteColumnTransformationControl, DeleteHdl));
}

ScDeleteColumnTransformationControl::~ScDeleteColumnTransformationControl()
{
    disposeOnce();
}

void ScDeleteColumnTransformationControl::dispose()
{
    maColumnNums.clear();
    maDelete.clear();

    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScDeleteColumnTransformationControl::getTransformation()
{
    OUString aColumnString = maColumnNums->GetText();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> ColNums;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        ColNums.insert(nCol - 1);
    }

    return std::make_shared<sc::ColumnRemoveTransformation>(ColNums);
}

class ScSplitColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maSeparator;
    VclPtr<NumericField> maNumColumns;
    SCCOL mnCol;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScSplitColumnTransformationControl(vcl::Window* pParent, SCCOL nCol, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScSplitColumnTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScSplitColumnTransformationControl::ScSplitColumnTransformationControl(
    vcl::Window* pParent, SCCOL nCol, sal_uInt32 aIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/splitcolumnentry.ui")
    , mnCol(nCol)
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(maSeparator, "ed_separator");
    get(maNumColumns, "num_cols");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScSplitColumnTransformationControl, DeleteHdl));
}

ScSplitColumnTransformationControl::~ScSplitColumnTransformationControl()
{
    disposeOnce();
}

void ScSplitColumnTransformationControl::dispose()
{
    maSeparator.clear();
    maNumColumns.clear();
    maDelete.clear();
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
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScMergeColumnTransformationControl(vcl::Window* pParent, SCCOL nStartCol, SCCOL nEndCol, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScMergeColumnTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScMergeColumnTransformationControl::ScMergeColumnTransformationControl(
    vcl::Window* pParent, SCCOL nStartCol, SCCOL nEndCol, sal_uInt32 aIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/mergecolumnentry.ui")
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(mpSeparator, "ed_separator");
    get(mpEdColumns, "ed_columns");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScMergeColumnTransformationControl, DeleteHdl));

    OUStringBuffer aBuffer;

    // map from zero based to one based column numbers
    aBuffer.append(OUString::number(nStartCol + 1));
    for ( SCCOL nCol = nStartCol + 1; nCol <= nEndCol; ++nCol)
    {
        aBuffer.append(";").append(OUString::number(nCol + 1));
    }

    mpEdColumns->SetText(aBuffer.makeStringAndClear());
}

ScMergeColumnTransformationControl::~ScMergeColumnTransformationControl()
{
    disposeOnce();
}

void ScMergeColumnTransformationControl::dispose()
{
    mpSeparator.clear();
    mpEdColumns.clear();
    maDelete.clear();

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

class ScSortTransformationControl : public ScDataTransformationBaseControl
{
private:

    VclPtr<CheckBox> mpAscending;
    VclPtr<Edit> mpEdColumns;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScSortTransformationControl(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScSortTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScSortTransformationControl::ScSortTransformationControl(
    vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/sorttransformationentry.ui")
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(mpAscending, "ed_ascending");
    get(mpEdColumns, "ed_columns");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScSortTransformationControl, DeleteHdl));
}

ScSortTransformationControl::~ScSortTransformationControl()
{
    disposeOnce();
}

void ScSortTransformationControl::dispose()
{
    mpAscending.clear();
    mpEdColumns.clear();
    maDelete.clear();

    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScSortTransformationControl::getTransformation()
{
    OUString aColStr = mpEdColumns->GetText();
    bool aIsAscending = mpAscending->IsChecked();
    SCCOL aColumn = 0;
    sal_Int32 nCol = aColStr.toInt32();
    if (nCol > 0 && nCol <= MAXCOL)
        aColumn = nCol - 1;     // translate from 1-based column notations to internal Calc one

    ScSortParam aSortParam;
    ScSortKeyState aSortKey;
    aSortKey.bDoSort = true;
    aSortKey.nField = aColumn;
    aSortKey.bAscending = aIsAscending;
    aSortParam.maKeyState.push_back(aSortKey);
    return std::make_shared<sc::SortTransformation>(aSortParam);
}

class ScColumnTextTransformation : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maColumnNums;
    VclPtr<ListBox> maType;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScColumnTextTransformation(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScColumnTextTransformation() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScColumnTextTransformation::ScColumnTextTransformation(
    vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/texttransformationentry.ui")
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(maColumnNums, "ed_columns");
    get(maType, "ed_lst");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScColumnTextTransformation, DeleteHdl));
}

ScColumnTextTransformation::~ScColumnTextTransformation()
{
    disposeOnce();
}

void ScColumnTextTransformation::dispose()
{
    maColumnNums.clear();
    maType.clear();
    maDelete.clear();
    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScColumnTextTransformation::getTransformation()
{
    OUString aColumnString = maColumnNums->GetText();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }

    sal_Int32 nPos = maType->GetSelectedEntryPos();
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::TextTransformation>(aColumns,sc::TEXT_TRANSFORM_TYPE::TO_LOWER);
        case 1:
            return std::make_shared<sc::TextTransformation>(aColumns,sc::TEXT_TRANSFORM_TYPE::TO_UPPER);
        case 2:
            return std::make_shared<sc::TextTransformation>(aColumns,sc::TEXT_TRANSFORM_TYPE::CAPITALIZE);
        case 3:
            return std::make_shared<sc::TextTransformation>(aColumns,sc::TEXT_TRANSFORM_TYPE::TRIM);
        default:
            assert(false);
    }

    return nullptr;
}

class ScAggregateFunction : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maColumnNums;
    VclPtr<ListBox> maType;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScAggregateFunction(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScAggregateFunction() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScAggregateFunction::ScAggregateFunction(vcl::Window* pParent, sal_uInt32 aIndex,
                                         std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/aggregatefunctionentry.ui")
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(maColumnNums, "ed_columns");
    get(maType, "ed_lst");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScAggregateFunction, DeleteHdl));
}

ScAggregateFunction::~ScAggregateFunction()
{
    disposeOnce();
}

void ScAggregateFunction::dispose()
{
    maColumnNums.clear();
    maType.clear();
    maDelete.clear();
    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScAggregateFunction::getTransformation()
{
    OUString aColumnString = maColumnNums->GetText();
    sal_Int32 nPos = maType->GetSelectedEntryPos();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::AggregateFunction>(aColumns,sc::AGGREGATE_FUNCTION::SUM);
        case 1:
            return std::make_shared<sc::AggregateFunction>(aColumns,sc::AGGREGATE_FUNCTION::AVERAGE);
        case 2:
            return std::make_shared<sc::AggregateFunction>(aColumns,sc::AGGREGATE_FUNCTION::MIN);
        case 3:
            return std::make_shared<sc::AggregateFunction>(aColumns,sc::AGGREGATE_FUNCTION::MAX);
        default:
            assert(false);
    }

    return nullptr;
}

class ScNumberTransformation : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maColumnNums;
    VclPtr<ListBox> maType;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScNumberTransformation(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScNumberTransformation() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScNumberTransformation::ScNumberTransformation(
    vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/numbertransformationentry.ui")
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    get(maColumnNums, "ed_columns");
    get(maType, "ed_lst");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScNumberTransformation, DeleteHdl));
}

ScNumberTransformation::~ScNumberTransformation()
{
    disposeOnce();
}

void ScNumberTransformation::dispose()
{
    maColumnNums.clear();
    maType.clear();
    maDelete.clear();
    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScNumberTransformation::getTransformation()
{
    OUString aColumnString = maColumnNums->GetText();
    sal_Int32 nPos = maType->GetSelectedEntryPos();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::SIGN);
        case 1:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::ROUND);
        case 2:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::ROUND_UP);
        case 3:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::ROUND_DOWN);
        case 4:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::ABSOLUTE);
        case 5:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::LOG_E);
        case 6:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::LOG_10);
        case 7:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::CUBE);
        case 8:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::SQUARE);
        case 9:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::SQUARE_ROOT);
        case 10:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::EXPONENT);
        case 11:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::IS_EVEN);
        case 12:
            return std::make_shared<sc::NumberTransformation>(aColumns,sc::NUMBER_TRANSFORM_TYPE::IS_ODD);
        default:
            assert(false);
    }

    return nullptr;
}

class ScReplaceNullTransformation : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maColumnNums;
    VclPtr<Edit> maReplaceString;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScReplaceNullTransformation(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScReplaceNullTransformation() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScReplaceNullTransformation::ScReplaceNullTransformation(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation):
    ScDataTransformationBaseControl(pParent,"modules/scalc/ui/replacenulltransformationentry.ui"),
    maIndex(aIndex),
    maDeleteTransformation(aDeleteTransformation)
{
    get(maColumnNums, "ed_columns");
    get(maReplaceString, "ed_str");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScReplaceNullTransformation, DeleteHdl));
}

ScReplaceNullTransformation::~ScReplaceNullTransformation()
{
    disposeOnce();
}

void ScReplaceNullTransformation::dispose()
{
    maColumnNums.clear();
    maReplaceString.clear();
    maDelete.clear();
    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScReplaceNullTransformation::getTransformation()
{
    OUString aColumnString = maColumnNums->GetText();
    OUString aReplaceWithString = maReplaceString->GetText();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }

    return std::make_shared<sc::ReplaceNullTransformation>(aColumns,aReplaceWithString);
}

class ScDateTimeTransformation : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maColumnNums;
    VclPtr<ListBox> maType;
    VclPtr<PushButton> maDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScDateTimeTransformation(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScDateTimeTransformation() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, Button*, void);
};

ScDateTimeTransformation::ScDateTimeTransformation(vcl::Window* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation):
    ScDataTransformationBaseControl(pParent,"modules/scalc/ui/datetimetransformationentry.ui"),
    maIndex(aIndex),
    maDeleteTransformation(aDeleteTransformation)
{
    get(maColumnNums, "ed_columns");
    get(maType, "ed_lst");
    get(maDelete, "ed_delete");
    maDelete->SetClickHdl(LINK(this,ScDateTimeTransformation, DeleteHdl));
}

ScDateTimeTransformation::~ScDateTimeTransformation()
{
    disposeOnce();
}

void ScDateTimeTransformation::dispose()
{
    maColumnNums.clear();
    maType.clear();
    maDelete.clear();
    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScDateTimeTransformation::getTransformation()
{
    OUString aColumnString = maColumnNums->GetText();
    sal_Int32 nPos = maType->GetSelectedEntryPos();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
}
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::DATE_STRING);
        case 1:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::YEAR);
        case 2:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR);
        case 3:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR);
        case 4:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::MONTH);
        case 5:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::MONTH_NAME);
        case 6:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH);
        case 7:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH);
        case 8:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::DAY);
        case 9:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK);
        case 10:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR);
        case 11:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::QUARTER);
        case 12:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::START_OF_QUARTER);
        case 13:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::END_OF_QUARTER);
        case 14:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::HOUR);
        case 15:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::MINUTE);
        case 16:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::SECOND);
        case 17:
            return std::make_shared<sc::DateTimeTransformation>(aColumns,sc::DATETIME_TRANSFORMATION_TYPE::TIME);
        default:
            assert(false);
    }

    return nullptr;
}

}

ScDataProviderDlg::ScDataProviderDlg(vcl::Window* pParent, std::shared_ptr<ScDocument> pDoc,
                                     const ScDocument* pDocument)
    : ModalDialog(pParent, "dataproviderdlg", "modules/scalc/ui/dataproviderdlg.ui", true)
    , mpDoc(std::move(pDoc))
    , mpBar(VclPtr<MenuBar>::Create())
{
    get(mpTable, "data_table");
    get(mpList, "operation_ctrl");
    get(mpDBRanges, "select_db_range");
    mpTable->Init(mpDoc);
    mpIndex = 0;
    ScDBCollection* pDBCollection = pDocument->GetDBCollection();
    auto& rNamedDBs = pDBCollection->getNamedDBs();
    for (auto& rNamedDB : rNamedDBs)
    {
        mpDBRanges->InsertEntry(rNamedDB->GetName());
    }

    mpDataProviderCtrl = VclPtr<ScDataProviderBaseControl>::Create(mpList, LINK(this, ScDataProviderDlg, ImportHdl));
    mpList->addEntry(mpDataProviderCtrl);
    mpIndex++;
    pDBData = new ScDBData("data", 0, 0, 0, MAXCOL, MAXROW);
    bool bSuccess = mpDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
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
    mpDBRanges.clear();
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
    mpIndex++;
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
        import(mpDoc.get(), true);
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
{   std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScDeleteColumnTransformationControl> pDeleteColumnEntry = VclPtr<ScDeleteColumnTransformationControl>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pDeleteColumnEntry);
}

void ScDataProviderDlg::splitColumn()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mpTable->getColRange(nStartCol, nEndCol);
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScSplitColumnTransformationControl> pSplitColumnEntry = VclPtr<ScSplitColumnTransformationControl>::Create(mpList, nStartCol, mpIndex++, adeleteTransformation);
    mpList->addEntry(pSplitColumnEntry);
}

void ScDataProviderDlg::mergeColumns()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mpTable->getColRange(nStartCol, nEndCol);
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScMergeColumnTransformationControl> pMergeColumnEntry = VclPtr<ScMergeColumnTransformationControl>::Create(mpList, nStartCol, nEndCol, mpIndex++, adeleteTransformation);
    mpList->addEntry(pMergeColumnEntry);
}

void ScDataProviderDlg::textTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScColumnTextTransformation> pTextTransforamtionEntry = VclPtr<ScColumnTextTransformation>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pTextTransforamtionEntry);
}

void ScDataProviderDlg::sortTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScSortTransformationControl> pSortTransforamtionEntry = VclPtr<ScSortTransformationControl>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pSortTransforamtionEntry);
}

void ScDataProviderDlg::aggregateFunction()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScAggregateFunction> pAggregateFuntionEntry = VclPtr<ScAggregateFunction>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pAggregateFuntionEntry);
}

void ScDataProviderDlg::numberTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScNumberTransformation> pNumberTransformationEntry = VclPtr<ScNumberTransformation>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pNumberTransformationEntry);
}

void ScDataProviderDlg::replaceNullTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScReplaceNullTransformation> pReplaceNullTransformationEntry = VclPtr<ScReplaceNullTransformation>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pReplaceNullTransformationEntry);
}

void ScDataProviderDlg::dateTimeTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    VclPtr<ScDateTimeTransformation> pDateTimeTransformationEntry = VclPtr<ScDateTimeTransformation>::Create(mpList, mpIndex++, adeleteTransformation);
    mpList->addEntry(pDateTimeTransformationEntry);
}

namespace {

bool hasDBName(const OUString& rName, ScDBCollection* pDBCollection)
{
    if (pDBCollection->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rName)))
        return true;

    return false;
}

}

void ScDataProviderDlg::import(ScDocument* pDoc, bool bInternal)
{
    sc::ExternalDataSource aSource = mpDataProviderCtrl->getDataSource(pDoc);

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
    if (bInternal)
        aSource.setDBData(pDBData->GetName());
    else
    {
        aSource.setDBData(mpDBRanges->GetSelectedEntry());
        if (!hasDBName(aSource.getDBName(), pDoc->GetDBCollection()))
            return;
        pDoc->GetExternalDataMapper().insertDataSource(aSource);
    }
    aSource.refresh(pDoc, true);
    mpTable->Invalidate();
}

void ScDataProviderDlg::deletefromList(sal_uInt32 nIndex)
{
    mpList->deleteEntry(nIndex);
}

IMPL_LINK_NOARG(ScDeleteColumnTransformationControl, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScSplitColumnTransformationControl, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScMergeColumnTransformationControl, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScNumberTransformation, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScAggregateFunction, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScSortTransformationControl, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScColumnTextTransformation, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScReplaceNullTransformation, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScDateTimeTransformation, DeleteHdl, Button*, void)
{
   maDeleteTransformation(maIndex);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
