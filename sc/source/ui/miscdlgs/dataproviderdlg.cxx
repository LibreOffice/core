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

class ScDataProviderBaseControl
{
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxGrid;
    std::unique_ptr<weld::ComboBox> mxProviderList;
    std::unique_ptr<weld::Entry> mxEditURL;
    std::unique_ptr<weld::Entry> mxEditID;
    std::unique_ptr<weld::Button> mxApplyBtn;

    OUString msApplyTooltip;

    Link<ScDataProviderBaseControl*, void> const maImportCallback;

    DECL_LINK(ProviderSelectHdl, weld::ComboBox&, void);
    DECL_LINK(IDEditHdl, weld::Entry&, void);
    DECL_LINK(URLEditHdl, weld::Entry&, void);
    DECL_LINK(ApplyBtnHdl, weld::Button&, void);

    void updateApplyBtn(bool bValidConfig);

public:
    ScDataProviderBaseControl(weld::Widget* pParent, const Link<ScDataProviderBaseControl*, void>& rImportCallback);
    ~ScDataProviderBaseControl();

//TODO    virtual void setAllocation(const Size &rAllocation) override;
//TODO    virtual Size calculateRequisition() const override;

    void isValid();

    sc::ExternalDataSource getDataSource(ScDocument* pDoc);
};

ScDataProviderBaseControl::ScDataProviderBaseControl(weld::Widget* pParent,
        const Link<ScDataProviderBaseControl*, void>& rImportCallback)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/dataproviderentry.ui"))
    , mxGrid(mxBuilder->weld_container("grid"))
    , mxProviderList(mxBuilder->weld_combo_box("provider_lst"))
    , mxEditURL(mxBuilder->weld_entry("ed_url"))
    , mxEditID(mxBuilder->weld_entry("ed_id"))
    , mxApplyBtn(mxBuilder->weld_button("apply"))
    , maImportCallback(rImportCallback)
{
    auto aDataProvider = sc::DataProviderFactory::getDataProviders();
    for (const auto& rDataProvider : aDataProvider)
    {
        mxProviderList->append_text(rDataProvider);
    }

    mxProviderList->connect_changed(LINK(this, ScDataProviderBaseControl, ProviderSelectHdl));
    mxEditID->connect_changed(LINK(this, ScDataProviderBaseControl, IDEditHdl));
    mxEditURL->connect_changed(LINK(this, ScDataProviderBaseControl, URLEditHdl));

    msApplyTooltip = mxApplyBtn->get_tooltip_text();
#if 0
    mxApplyBtn->SetControlForeground(COL_GREEN);
    mxApplyBtn->SetControlBackground(COL_GREEN);
    mxApplyBtn->SetBackground(Wallpaper(COL_LIGHTGREEN));
#endif
    mxApplyBtn->connect_clicked(LINK(this, ScDataProviderBaseControl, ApplyBtnHdl));
//TODO    SetSizePixel(GetOptimalSize());
    isValid();
}

ScDataProviderBaseControl::~ScDataProviderBaseControl()
{
}

void ScDataProviderBaseControl::isValid()
{
    bool bValid = !mxProviderList->get_active_text().isEmpty();
    bValid &= !mxEditURL->get_text().isEmpty();
//TODO    Invalidate();
    updateApplyBtn(bValid);
}

sc::ExternalDataSource ScDataProviderBaseControl::getDataSource(ScDocument* pDoc)
{
    OUString aURL = mxEditURL->get_text();
    OUString aProvider = mxProviderList->get_active_text();
    sc::ExternalDataSource aSource(aURL, aProvider, pDoc);

    OUString aID = mxEditID->get_text();
    aSource.setID(aID);
    return aSource;
}

void ScDataProviderBaseControl::updateApplyBtn(bool bValidConfig)
{
    if (!bValidConfig)
    {
        mxApplyBtn->set_sensitive(false);
        mxApplyBtn->set_tooltip_text(OUString());
        return;
    }

    mxApplyBtn->set_sensitive(true);
#if 0
    mxApplyBtn->SetBackground(Wallpaper(COL_YELLOW));
#endif
    mxApplyBtn->set_tooltip_text(msApplyTooltip);
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ProviderSelectHdl, weld::ComboBox&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, IDEditHdl, weld::Entry&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, URLEditHdl, weld::Entry&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ApplyBtnHdl, weld::Button&, void)
{
    updateApplyBtn(true);
    maImportCallback.Call(this);
}

class ScDataTransformationBaseControl
{
protected:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxGrid;

public:
    ScDataTransformationBaseControl(weld::Widget* pParent, const OUString& rUIFile);
    virtual ~ScDataTransformationBaseControl();

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() = 0;
};

ScDataTransformationBaseControl::ScDataTransformationBaseControl(weld::Widget* pParent, const OUString& rUIFile)
    : mxBuilder(Application::CreateBuilder(pParent, rUIFile))
    , mxGrid(mxBuilder->weld_container("grid"))
{
}

ScDataTransformationBaseControl::~ScDataTransformationBaseControl()
{
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

class ScDeleteColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScDeleteColumnTransformationControl(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScDeleteColumnTransformationControl() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScDeleteColumnTransformationControl::ScDeleteColumnTransformationControl(
    weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/deletecolumnentry.ui")
    , mxColumnNums(mxBuilder->weld_entry("ed_columns"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScDeleteColumnTransformationControl, DeleteHdl));
}

ScDeleteColumnTransformationControl::~ScDeleteColumnTransformationControl()
{
}

std::shared_ptr<sc::DataTransformation> ScDeleteColumnTransformationControl::getTransformation()
{
    OUString aColumnString = mxColumnNums->get_text();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> ColNums;
    for (const auto& rColStr : aSplitColumns)
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
    std::unique_ptr<weld::Entry> mxSeparator;
    std::unique_ptr<weld::SpinButton> mxNumColumns;
    std::unique_ptr<weld::Button> mxDelete;
    SCCOL mnCol;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScSplitColumnTransformationControl(weld::Widget* pParent, SCCOL nCol, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScSplitColumnTransformationControl() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScSplitColumnTransformationControl::ScSplitColumnTransformationControl(
    weld::Widget* pParent, SCCOL nCol, sal_uInt32 aIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/splitcolumnentry.ui")
    , mxSeparator(mxBuilder->weld_entry("ed_separator"))
    , mxNumColumns(mxBuilder->weld_spin_button("num_cols"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , mnCol(nCol)
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScSplitColumnTransformationControl, DeleteHdl));
}

ScSplitColumnTransformationControl::~ScSplitColumnTransformationControl()
{
}

std::shared_ptr<sc::DataTransformation> ScSplitColumnTransformationControl::getTransformation()
{
    OUString aSeparator = mxSeparator->get_text();
    sal_Unicode cSeparator = aSeparator.isEmpty() ? ',' : aSeparator[0];
    return std::make_shared<sc::SplitColumnTransformation>(mnCol, cSeparator);
}

class ScMergeColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxSeparator;
    std::unique_ptr<weld::Entry> mxEdColumns;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScMergeColumnTransformationControl(weld::Widget* pParent, SCCOL nStartCol, SCCOL nEndCol, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScMergeColumnTransformationControl() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScMergeColumnTransformationControl::ScMergeColumnTransformationControl(
    weld::Widget* pParent, SCCOL nStartCol, SCCOL nEndCol, sal_uInt32 aIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/mergecolumnentry.ui")
    , mxSeparator(mxBuilder->weld_entry("ed_separator"))
    , mxEdColumns(mxBuilder->weld_entry("ed_columns"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScMergeColumnTransformationControl, DeleteHdl));

    OUStringBuffer aBuffer;

    // map from zero based to one based column numbers
    aBuffer.append(OUString::number(nStartCol + 1));
    for ( SCCOL nCol = nStartCol + 1; nCol <= nEndCol; ++nCol)
    {
        aBuffer.append(";").append(OUString::number(nCol + 1));
    }

    mxEdColumns->set_text(aBuffer.makeStringAndClear());
}

ScMergeColumnTransformationControl::~ScMergeColumnTransformationControl()
{
}

std::shared_ptr<sc::DataTransformation> ScMergeColumnTransformationControl::getTransformation()
{
    OUString aColumnString = mxEdColumns->get_text();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aMergedColumns;
    for (const auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aMergedColumns.insert(nCol - 1);
    }
    return std::make_shared<sc::MergeColumnTransformation>(aMergedColumns, mxSeparator->get_text());
}

class ScSortTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::CheckButton> mxAscending;
    std::unique_ptr<weld::Entry> mxEdColumns;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScSortTransformationControl(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScSortTransformationControl() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScSortTransformationControl::ScSortTransformationControl(
    weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/sorttransformationentry.ui")
    , mxAscending(mxBuilder->weld_check_button("ed_ascending"))
    , mxEdColumns(mxBuilder->weld_entry("ed_columns"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScSortTransformationControl, DeleteHdl));
}

ScSortTransformationControl::~ScSortTransformationControl()
{
}

std::shared_ptr<sc::DataTransformation> ScSortTransformationControl::getTransformation()
{
    OUString aColStr = mxEdColumns->get_text();
    bool aIsAscending = mxAscending->get_active();
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
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScColumnTextTransformation(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScColumnTextTransformation() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScColumnTextTransformation::ScColumnTextTransformation(
    weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/texttransformationentry.ui")
    , mxColumnNums(mxBuilder->weld_entry("ed_columns"))
    , mxType(mxBuilder->weld_combo_box("ed_lst"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScColumnTextTransformation, DeleteHdl));
}

ScColumnTextTransformation::~ScColumnTextTransformation()
{
}

std::shared_ptr<sc::DataTransformation> ScColumnTextTransformation::getTransformation()
{
    OUString aColumnString = mxColumnNums->get_text();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (const auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }

    sal_Int32 nPos = mxType->get_active();
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
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScAggregateFunction(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScAggregateFunction() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScAggregateFunction::ScAggregateFunction(weld::Widget* pParent, sal_uInt32 aIndex,
                                         std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/aggregatefunctionentry.ui")
    , mxColumnNums(mxBuilder->weld_entry("ed_columns"))
    , mxType(mxBuilder->weld_combo_box("ed_lst"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScAggregateFunction, DeleteHdl));
}

ScAggregateFunction::~ScAggregateFunction()
{
}

std::shared_ptr<sc::DataTransformation> ScAggregateFunction::getTransformation()
{
    OUString aColumnString = mxColumnNums->get_text();
    sal_Int32 nPos = mxType->get_active();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (const auto& rColStr : aSplitColumns)
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
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:
    ScNumberTransformation(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScNumberTransformation() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScNumberTransformation::ScNumberTransformation(
    weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, "modules/scalc/ui/numbertransformationentry.ui")
    , mxColumnNums(mxBuilder->weld_entry("ed_columns"))
    , mxType(mxBuilder->weld_combo_box("ed_lst"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(std::move(aDeleteTransformation))
{
    mxDelete->connect_clicked(LINK(this,ScNumberTransformation, DeleteHdl));
}

ScNumberTransformation::~ScNumberTransformation()
{
}

std::shared_ptr<sc::DataTransformation> ScNumberTransformation::getTransformation()
{
    OUString aColumnString = mxColumnNums->get_text();
    sal_Int32 nPos = mxType->get_active();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (const auto& rColStr : aSplitColumns)
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
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::Entry> mxReplaceString;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScReplaceNullTransformation(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScReplaceNullTransformation() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScReplaceNullTransformation::ScReplaceNullTransformation(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent,"modules/scalc/ui/replacenulltransformationentry.ui")
    , mxColumnNums(mxBuilder->weld_entry("ed_columns"))
    , mxReplaceString(mxBuilder->weld_entry("ed_str"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(aDeleteTransformation)
{
    mxDelete->connect_clicked(LINK(this,ScReplaceNullTransformation, DeleteHdl));
}

ScReplaceNullTransformation::~ScReplaceNullTransformation()
{
}

std::shared_ptr<sc::DataTransformation> ScReplaceNullTransformation::getTransformation()
{
    OUString aColumnString = mxColumnNums->get_text();
    OUString aReplaceWithString = mxReplaceString->get_text();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (const auto& rColStr : aSplitColumns)
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
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Button> mxDelete;
    sal_uInt32 maIndex;
    std::function<void(sal_uInt32&)> maDeleteTransformation;

public:

    ScDateTimeTransformation(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);
    ~ScDateTimeTransformation() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScDateTimeTransformation::ScDateTimeTransformation(weld::Widget* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent,"modules/scalc/ui/datetimetransformationentry.ui")
    , mxColumnNums(mxBuilder->weld_entry("ed_columns"))
    , mxType(mxBuilder->weld_combo_box("ed_lst"))
    , mxDelete(mxBuilder->weld_button("ed_delete"))
    , maIndex(aIndex)
    , maDeleteTransformation(aDeleteTransformation)
{
    mxDelete->connect_clicked(LINK(this,ScDateTimeTransformation, DeleteHdl));
}

ScDateTimeTransformation::~ScDateTimeTransformation()
{
}

std::shared_ptr<sc::DataTransformation> ScDateTimeTransformation::getTransformation()
{
    OUString aColumnString = mxColumnNums->get_text();
    sal_Int32 nPos = mxType->get_active();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aColumns;
    for (const auto& rColStr : aSplitColumns)
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

ScDataProviderDlg::ScDataProviderDlg(weld::Window* pParent, std::shared_ptr<ScDocument> pDoc,
                                     const ScDocument* pDocument)
    : GenericDialogController(pParent, "modules/scalc/ui/dataproviderdlg.ui", "dataproviderdlg")
    , mxDoc(std::move(pDoc))
    , mxStartMenu(m_xBuilder->weld_menu("start"))
    , mxColumnMenu(m_xBuilder->weld_menu("column"))
//TODO    , mxTable, "data_table");
    , mxList(m_xBuilder->weld_container("operation_ctrl"))
    , mxDataProviderCtrl(new ScDataProviderBaseControl(mxList.get(), LINK(this, ScDataProviderDlg, ImportHdl)))
    , mxDBRanges(m_xBuilder->weld_combo_box("select_db_range"))
{
    mxTable->Init(mxDoc);
    mpIndex = 0;
    ScDBCollection* pDBCollection = pDocument->GetDBCollection();
    auto& rNamedDBs = pDBCollection->getNamedDBs();
    for (auto& rNamedDB : rNamedDBs)
    {
        mxDBRanges->append_text(rNamedDB->GetName());
    }

    mpIndex++;
    pDBData = new ScDBData("data", 0, 0, 0, MAXCOL, MAXROW);
    bool bSuccess = mxDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    SAL_WARN_IF(!bSuccess, "sc", "temporary warning");

    InitMenu();
}

ScDataProviderDlg::~ScDataProviderDlg()
{
}

void ScDataProviderDlg::InitMenu()
{
    for (const auto& itrStartData : aStartData)
        mxStartMenu->append(OUString::number(itrStartData.nMenuID), OUString::createFromAscii(itrStartData.aMenuName));
//TODO    pPopup->SetSelectHdl(LINK(this, ScDataProviderDlg, StartMenuHdl));

    for (const auto& itrColumnData : aColumnData)
        mxColumnMenu->append(OUString::number(itrColumnData.nMenuID), OUString::createFromAscii(itrColumnData.aMenuName));
//TODO    pColumnMenu->SetSelectHdl(LINK(this, ScDataProviderDlg, ColumnMenuHdl));
}

#if 0
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
#endif

IMPL_LINK(ScDataProviderDlg, ImportHdl, ScDataProviderBaseControl*, pCtrl, void)
{
    if (pCtrl == mxDataProviderCtrl.get())
    {
        import(mxDoc.get(), true);
    }
}

void ScDataProviderDlg::applyAndQuit()
{
    m_xDialog->response(RET_OK);
}

void ScDataProviderDlg::cancelAndQuit()
{
    m_xDialog->response(RET_CANCEL);
}

void ScDataProviderDlg::deleteColumn()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScDeleteColumnTransformationControl>(mxList.get(), mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::splitColumn()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mxTable->getColRange(nStartCol, nEndCol);
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScSplitColumnTransformationControl>(mxList.get(), nStartCol, mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::mergeColumns()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mxTable->getColRange(nStartCol, nEndCol);
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScMergeColumnTransformationControl>(mxList.get(), nStartCol, nEndCol, mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::textTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScColumnTextTransformation>(mxList.get(), mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::sortTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScSortTransformationControl>(mxList.get(), mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::aggregateFunction()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScAggregateFunction>(mxList.get(), mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::numberTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScNumberTransformation>(mxList.get(), mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::replaceNullTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScReplaceNullTransformation>(mxList.get(), mpIndex++, adeleteTransformation));
}

void ScDataProviderDlg::dateTimeTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScDateTimeTransformation>(mxList.get(), mpIndex++, adeleteTransformation));
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
    sc::ExternalDataSource aSource = mxDataProviderCtrl->getDataSource(pDoc);

    for (size_t i = 0; i < maControls.size(); ++i)
    {
        ScDataTransformationBaseControl* pTransformationCtrl = maControls[i].get();
        aSource.AddDataTransformation(pTransformationCtrl->getTransformation());
    }
    if (bInternal)
        aSource.setDBData(pDBData->GetName());
    else
    {
        aSource.setDBData(mxDBRanges->get_active_text());
        if (!hasDBName(aSource.getDBName(), pDoc->GetDBCollection()))
            return;
        pDoc->GetExternalDataMapper().insertDataSource(aSource);
    }
    aSource.refresh(pDoc, true);
    mxTable->Invalidate();
}

void ScDataProviderDlg::deletefromList(sal_uInt32 nIndex)
{
    maControls.erase(maControls.begin() + nIndex);
}

IMPL_LINK_NOARG(ScDeleteColumnTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScSplitColumnTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScMergeColumnTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScNumberTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScAggregateFunction, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScSortTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScColumnTextTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScReplaceNullTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}

IMPL_LINK_NOARG(ScDateTimeTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(maIndex);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
