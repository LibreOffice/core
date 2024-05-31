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
#include <sfx2/filedlghelper.hxx>
#include <unotools/charclass.hxx>
#include <utility>
#include <vcl/svapp.hxx>

class ScDataTransformationBaseControl
{
protected:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxGrid;
    weld::Container* mpContainer;

    sal_uInt32 mnIndex;

public:
    ScDataTransformationBaseControl(weld::Container* pParent, const OUString& rUIFile, sal_uInt32 nIndex);
    virtual ~ScDataTransformationBaseControl();

    void updateIndex(sal_uInt32 nIndex) { mnIndex = nIndex; }

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() = 0;

    static SCROW getLastRow(const ScDocument& rDoc);
    static SCCOL getLastCol(const ScDocument& rDoc);
};

SCROW ScDataTransformationBaseControl::getLastRow(const ScDocument& rDoc)
{
    SCROW nEndRow = rDoc.MaxRow();
    return rDoc.GetLastDataRow(0, 0, 0, nEndRow);
}

SCCOL ScDataTransformationBaseControl::getLastCol(const ScDocument& rDoc)
{
    for (SCCOL nCol = 1; nCol <= rDoc.MaxCol(); ++nCol)
    {
        if (rDoc.GetCellType(nCol, 0, 0) == CELLTYPE_NONE)
        {
            return static_cast<SCCOL>(nCol - 1 );
        }
    }
    return rDoc.MaxCol();
}

ScDataTransformationBaseControl::ScDataTransformationBaseControl(weld::Container* pParent, const OUString& rUIFile, sal_uInt32 nIndex)
    : mxBuilder(Application::CreateBuilder(pParent, rUIFile))
    , mxGrid(mxBuilder->weld_container(u"grid"_ustr))
    , mpContainer(pParent)
    , mnIndex(nIndex)
{
}

ScDataTransformationBaseControl::~ScDataTransformationBaseControl()
{
    mpContainer->move(mxGrid.get(), nullptr);
}

namespace {

struct MenuData
{
    const char* aTransformationName;
    std::function<void(ScDataProviderDlg*)> maCallback;
};

MenuData aTransformationData[] = {
    { "Delete Column", &ScDataProviderDlg::deleteColumn },
    { "Delete Row", &ScDataProviderDlg::deleteRowTransformation},
    { "Swap Rows", &ScDataProviderDlg::swapRowsTransformation},
    { "Split Column", &ScDataProviderDlg::splitColumn },
    { "Merge Columns", &ScDataProviderDlg::mergeColumns },
    { "Text Transformation", &ScDataProviderDlg::textTransformation },
    { "Sort Columns", &ScDataProviderDlg::sortTransformation },
    { "Aggregate Functions", &ScDataProviderDlg::aggregateFunction},
    { "Number Transformations", &ScDataProviderDlg::numberTransformation },
    { "Replace Null Transformations", &ScDataProviderDlg::replaceNullTransformation },
    { "Date & Time Transformations", &ScDataProviderDlg::dateTimeTransformation },
    { "Find Replace Transformation", &ScDataProviderDlg::findReplaceTransformation}
};

class ScDeleteColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScDeleteColumnTransformationControl(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 aIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScDeleteColumnTransformationControl::ScDeleteColumnTransformationControl(
    const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/deletecolumnentry.ui"_ustr, nIndex)
    , mxColumnNums(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScDeleteColumnTransformationControl, DeleteHdl));
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        ColNums.insert(nCol - 1);
    }

    return std::make_shared<sc::ColumnRemoveTransformation>(std::move(ColNums));
}

class ScSplitColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxSeparator;
    std::unique_ptr<weld::Entry> mxNumColumns;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScSplitColumnTransformationControl(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScSplitColumnTransformationControl::ScSplitColumnTransformationControl(
    const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/splitcolumnentry.ui"_ustr, nIndex)
    , mxSeparator(mxBuilder->weld_entry(u"ed_separator"_ustr))
    , mxNumColumns(mxBuilder->weld_entry(u"num_cols"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScSplitColumnTransformationControl, DeleteHdl));
}

std::shared_ptr<sc::DataTransformation> ScSplitColumnTransformationControl::getTransformation()
{
    OUString aSeparator = mxSeparator->get_text();
    sal_Unicode cSeparator = aSeparator.isEmpty() ? ',' : aSeparator[0];
    OUString aColStr = mxNumColumns->get_text();
    SCCOL mnCol = -1;
    sal_Int32 nCol = aColStr.toInt32();
    if (nCol > 0 && nCol <= mpDoc->MaxCol())
        mnCol = nCol - 1;
    return std::make_shared<sc::SplitColumnTransformation>(mnCol, cSeparator);
}

class ScMergeColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxSeparator;
    std::unique_ptr<weld::Entry> mxEdColumns;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScMergeColumnTransformationControl(const ScDocument *pDoc, weld::Container* pParent, SCCOL nStartCol, SCCOL nEndCol, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScMergeColumnTransformationControl::ScMergeColumnTransformationControl(
    const ScDocument* pDoc, weld::Container* pParent, SCCOL nStartCol, SCCOL nEndCol, sal_uInt32 nIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/mergecolumnentry.ui"_ustr, nIndex)
    , mxSeparator(mxBuilder->weld_entry(u"ed_separator"_ustr))
    , mxEdColumns(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScMergeColumnTransformationControl, DeleteHdl));

    OUStringBuffer aBuffer;

    // map from zero based to one based column numbers
    aBuffer.append(static_cast<sal_Int32>(nStartCol + 1));
    for ( SCCOL nCol = nStartCol + 1; nCol <= nEndCol; ++nCol)
    {
        aBuffer.append(";" + OUString::number(nCol + 1));
    }

    mxEdColumns->set_text(aBuffer.makeStringAndClear());
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        aMergedColumns.insert(nCol - 1);
    }
    return std::make_shared<sc::MergeColumnTransformation>(std::move(aMergedColumns), mxSeparator->get_text());
}

class ScSortTransformationControl : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Entry> mxEdColumns;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScSortTransformationControl(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScSortTransformationControl::ScSortTransformationControl(
    const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/sorttransformationentry.ui"_ustr, nIndex)
    , mxType(mxBuilder->weld_combo_box(u"ed_ascending"_ustr))
    , mxEdColumns(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScSortTransformationControl, DeleteHdl));
}

std::shared_ptr<sc::DataTransformation> ScSortTransformationControl::getTransformation()
{
    OUString aColStr = mxEdColumns->get_text();
    bool aIsAscending = mxType->get_active();
    SCCOL aColumn = 0;
    sal_Int32 nCol = aColStr.toInt32();
    if (nCol > 0 && nCol <= mpDoc->MaxCol())
        aColumn = nCol - 1;     // translate from 1-based column notations to internal Calc one

    ScSortParam aSortParam;
    aSortParam.nRow1=0;
    aSortParam.nRow2=getLastRow(*mpDoc);
    aSortParam.nCol1=0;
    aSortParam.nCol2=getLastCol(*mpDoc);
    aSortParam.maKeyState[0].bDoSort = true;
    aSortParam.maKeyState[0].nField = aColumn;
    aSortParam.maKeyState[0].bAscending = aIsAscending;
    return std::make_shared<sc::SortTransformation>(aSortParam);
}

class ScColumnTextTransformation : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScColumnTextTransformation(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScColumnTextTransformation::ScColumnTextTransformation(
    const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/texttransformationentry.ui"_ustr, nIndex)
    , mxColumnNums(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxType(mxBuilder->weld_combo_box(u"ed_lst"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScColumnTextTransformation, DeleteHdl));
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }

    sal_Int32 nPos = mxType->get_active();
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::TextTransformation>(std::move(aColumns),sc::TEXT_TRANSFORM_TYPE::TO_LOWER);
        case 1:
            return std::make_shared<sc::TextTransformation>(std::move(aColumns),sc::TEXT_TRANSFORM_TYPE::TO_UPPER);
        case 2:
            return std::make_shared<sc::TextTransformation>(std::move(aColumns),sc::TEXT_TRANSFORM_TYPE::CAPITALIZE);
        case 3:
            return std::make_shared<sc::TextTransformation>(std::move(aColumns),sc::TEXT_TRANSFORM_TYPE::TRIM);
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
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScAggregateFunction(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScAggregateFunction::ScAggregateFunction(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex,
                                         std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/aggregatefunctionentry.ui"_ustr, nIndex)
    , mxColumnNums(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxType(mxBuilder->weld_combo_box(u"ed_lst"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScAggregateFunction, DeleteHdl));
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::AggregateFunction>(std::move(aColumns),sc::AGGREGATE_FUNCTION::SUM);
        case 1:
            return std::make_shared<sc::AggregateFunction>(std::move(aColumns),sc::AGGREGATE_FUNCTION::AVERAGE);
        case 2:
            return std::make_shared<sc::AggregateFunction>(std::move(aColumns),sc::AGGREGATE_FUNCTION::MIN);
        case 3:
            return std::make_shared<sc::AggregateFunction>(std::move(aColumns),sc::AGGREGATE_FUNCTION::MAX);
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
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScNumberTransformation(const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScNumberTransformation::ScNumberTransformation(
    const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/numbertransformationentry.ui"_ustr, nIndex)
    , mxColumnNums(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxType(mxBuilder->weld_combo_box(u"ed_lst"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScNumberTransformation, DeleteHdl));
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::SIGN);
        case 1:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::ROUND);
        case 2:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::ROUND_UP);
        case 3:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::ROUND_DOWN);
        case 4:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::ABSOLUTE);
        case 5:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::LOG_E);
        case 6:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::LOG_10);
        case 7:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::CUBE);
        case 8:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::SQUARE);
        case 9:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::SQUARE_ROOT);
        case 10:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::EXPONENT);
        case 11:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::IS_EVEN);
        case 12:
            return std::make_shared<sc::NumberTransformation>(std::move(aColumns),sc::NUMBER_TRANSFORM_TYPE::IS_ODD);
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
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument *mpDoc;

public:

    ScReplaceNullTransformation(const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScReplaceNullTransformation::ScReplaceNullTransformation(const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent,u"modules/scalc/ui/replacenulltransformationentry.ui"_ustr, nIndex)
    , mxColumnNums(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxReplaceString(mxBuilder->weld_entry(u"ed_str"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScReplaceNullTransformation, DeleteHdl));
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }

    return std::make_shared<sc::ReplaceNullTransformation>(std::move(aColumns),aReplaceWithString);
}

class ScDateTimeTransformation : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxColumnNums;
    std::unique_ptr<weld::ComboBox> mxType;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:

    ScDateTimeTransformation(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScDateTimeTransformation::ScDateTimeTransformation(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent,u"modules/scalc/ui/datetimetransformationentry.ui"_ustr, nIndex)
    , mxColumnNums(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxType(mxBuilder->weld_combo_box(u"ed_lst"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this,ScDateTimeTransformation, DeleteHdl));
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

        if (nCol > mpDoc->MaxCol())
            continue;

        // translate from 1-based column notations to internal Calc one
        aColumns.insert(nCol - 1);
    }
    switch (nPos)
    {
        case 0:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::DATE_STRING);
        case 1:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::YEAR);
        case 2:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR);
        case 3:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR);
        case 4:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::MONTH);
        case 5:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::MONTH_NAME);
        case 6:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH);
        case 7:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH);
        case 8:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::DAY);
        case 9:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK);
        case 10:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR);
        case 11:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::QUARTER);
        case 12:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::START_OF_QUARTER);
        case 13:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::END_OF_QUARTER);
        case 14:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::HOUR);
        case 15:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::MINUTE);
        case 16:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::SECOND);
        case 17:
            return std::make_shared<sc::DateTimeTransformation>(std::move(aColumns),sc::DATETIME_TRANSFORMATION_TYPE::TIME);
        default:
            assert(false);
    }

    return nullptr;
}

class ScFindReplaceTransformation : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxFindString;
    std::unique_ptr<weld::Entry> mxReplaceString;
    std::unique_ptr<weld::Entry> mxEdColumns;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScFindReplaceTransformation(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScFindReplaceTransformation::ScFindReplaceTransformation(
    const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/findreplaceentry.ui"_ustr, nIndex)
    , mxFindString(mxBuilder->weld_entry(u"ed_find"_ustr))
    , mxReplaceString(mxBuilder->weld_entry(u"ed_replace"_ustr))
    , mxEdColumns(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this, ScFindReplaceTransformation, DeleteHdl));
}

std::shared_ptr<sc::DataTransformation> ScFindReplaceTransformation::getTransformation()
{
    OUString aColStr = mxEdColumns->get_text();
    SCCOL aColumn = -1;
    sal_Int32 nCol = aColStr.toInt32();
    if (nCol > 0 && nCol <= mpDoc->MaxCol())
        aColumn = nCol - 1;
    return std::make_shared<sc::FindReplaceTransformation>(aColumn, mxFindString->get_text(), mxReplaceString->get_text());
}

class ScDeleteRowTransformation : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxFindString;
    std::unique_ptr<weld::Entry> mxEdColumns;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScDeleteRowTransformation(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScDeleteRowTransformation::ScDeleteRowTransformation(
    const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/deleterowentry.ui"_ustr, nIndex)
    , mxFindString(mxBuilder->weld_entry(u"ed_find"_ustr))
    , mxEdColumns(mxBuilder->weld_entry(u"ed_columns"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this, ScDeleteRowTransformation, DeleteHdl));
}

std::shared_ptr<sc::DataTransformation> ScDeleteRowTransformation::getTransformation()
{
    OUString aColStr = mxEdColumns->get_text();
    SCCOL aColumn = -1;
    sal_Int32 nCol = aColStr.toInt32();
    if (nCol > 0 && nCol <= mpDoc->MaxCol())
        aColumn = nCol - 1;
    return std::make_shared<sc::DeleteRowTransformation>(aColumn, mxFindString->get_text());
}

class ScSwapRowsTransformation : public ScDataTransformationBaseControl
{
private:
    std::unique_ptr<weld::Entry> mxRow;
    std::unique_ptr<weld::Entry> nxRow;
    std::unique_ptr<weld::Button> mxDelete;
    std::function<void(sal_uInt32&)> maDeleteTransformation;
    const ScDocument* mpDoc;

public:
    ScSwapRowsTransformation(const ScDocument* pDoc, weld::Container* pParent, sal_uInt32 nIndex, std::function<void(sal_uInt32&)> aDeleteTransformation);

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

ScSwapRowsTransformation::ScSwapRowsTransformation(
    const ScDocument *pDoc, weld::Container* pParent, sal_uInt32 nIndex,
    std::function<void(sal_uInt32&)> aDeleteTransformation)
    : ScDataTransformationBaseControl(pParent, u"modules/scalc/ui/swaprowsentry.ui"_ustr, nIndex)
    , mxRow(mxBuilder->weld_entry(u"ed_row1"_ustr))
    , nxRow(mxBuilder->weld_entry(u"ed_row2"_ustr))
    , mxDelete(mxBuilder->weld_button(u"ed_delete"_ustr))
    , maDeleteTransformation(std::move(aDeleteTransformation))
    , mpDoc(pDoc)
{
    mxDelete->connect_clicked(LINK(this, ScSwapRowsTransformation, DeleteHdl));
}

std::shared_ptr<sc::DataTransformation> ScSwapRowsTransformation::getTransformation()
{
    OUString aRowStr = mxRow->get_text();
    OUString bRowStr = nxRow->get_text();
    SCROW aRow = -1;
    SCROW bRow = -1;
    sal_Int32 mRow = aRowStr.toInt32();
    sal_Int32 nRow = bRowStr.toInt32();
    if (mRow > 0 && mRow <= mpDoc->MaxRow())
        aRow = mRow - 1;
    if (nRow > 0 && nRow <= mpDoc->MaxRow())
        bRow = nRow - 1;
    return std::make_shared<sc::SwapRowsTransformation>(aRow, bRow);
}

}

ScDataProviderDlg::ScDataProviderDlg(weld::Window* pParent, std::shared_ptr<ScDocument> pDoc,
                                     const ScDocument* pDocument)
    : GenericDialogController(pParent, u"modules/scalc/ui/dataproviderdlg.ui"_ustr, u"dataproviderdlg"_ustr)
    , mxDoc(std::move(pDoc))
    , mxBox(m_xBuilder->weld_container(u"data_table"_ustr))
    , m_xTableParent(mxBox->CreateChildFrame())
    , mxTable(VclPtr<ScDataTableView>::Create(m_xTableParent, mxDoc))
    , mxDBRanges(m_xBuilder->weld_combo_box(u"select_db_range"_ustr))
    , mxOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , mxCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr))
    , mxAddTransformationBtn(m_xBuilder->weld_button(u"add_transformation"_ustr))
    , mxScroll(m_xBuilder->weld_scrolled_window(u"scroll"_ustr))
    , mxTransformationList(m_xBuilder->weld_container(u"transformation_ctrl"_ustr))
    , mxTransformationBox(m_xBuilder->weld_combo_box(u"transformation_box"_ustr))
    , mxProviderList(m_xBuilder->weld_combo_box(u"provider_lst"_ustr))
    , mxEditURL(m_xBuilder->weld_entry(u"ed_url"_ustr))
    , mxEditID(m_xBuilder->weld_entry(u"ed_id"_ustr))
    , mxApplyBtn(m_xBuilder->weld_button(u"apply"_ustr))
    , mxBrowseBtn(m_xBuilder->weld_button(u"browse"_ustr))
    , maIdle("ScDataProviderDlg maIdle")
    , mnIndex(0)
{
    Size aPrefSize = mxTable->GetOptimalSize();
    mxBox->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    mxTable->Show();

    ScDBCollection* pDBCollection = pDocument->GetDBCollection();
    auto& rNamedDBs = pDBCollection->getNamedDBs();
    for (auto& rNamedDB : rNamedDBs)
    {
        mxDBRanges->append_text(rNamedDB->GetName());
    }

    for (const auto& i : aTransformationData)
    {
         mxTransformationBox->append_text(OUString::createFromAscii(i.aTransformationName));
    }

    pDBData = new ScDBData(u"data"_ustr, 0, 0, 0, mxDoc->MaxCol(), mxDoc->MaxRow());
    bool bSuccess = mxDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    SAL_WARN_IF(!bSuccess, "sc", "temporary warning");

    auto aDataProvider = sc::DataProviderFactory::getDataProviders();
    for (const auto& rDataProvider : aDataProvider)
    {
        mxProviderList->append_text(rDataProvider);
    }

    mxOKBtn->connect_clicked(LINK(this, ScDataProviderDlg, ApplyQuitHdl));
    mxCancelBtn->connect_clicked(LINK(this, ScDataProviderDlg, CancelQuitHdl));
    mxAddTransformationBtn->connect_clicked(LINK(this, ScDataProviderDlg, TransformationListHdl));
    mxApplyBtn->connect_clicked(LINK(this, ScDataProviderDlg, ApplyBtnHdl));
    mxBrowseBtn->connect_clicked(LINK(this, ScDataProviderDlg, BrowseBtnHdl));
    mxTransformationBox->connect_changed(LINK(this, ScDataProviderDlg, TransformationSelectHdl));
    mxProviderList->connect_changed(LINK(this, ScDataProviderDlg, ProviderSelectHdl));
    mxEditID->connect_changed(LINK(this, ScDataProviderDlg, IDEditHdl));
    mxEditURL->connect_changed(LINK(this, ScDataProviderDlg, URLEditHdl));

    msApplyTooltip = mxApplyBtn->get_tooltip_text();
    msAddTransformationToolTip = mxAddTransformationBtn->get_tooltip_text();
    mxAddTransformationBtn->set_sensitive(false);
    mxAddTransformationBtn->set_tooltip_text(OUString());
    isValid();

    maIdle.SetPriority( TaskPriority::LOWEST );
    maIdle.SetInvokeHandler( LINK( this, ScDataProviderDlg, ScrollToEnd) );
}

ScDataProviderDlg::~ScDataProviderDlg()
{
    mxTable.disposeAndClear();
    m_xTableParent->dispose();
    m_xTableParent.clear();
}

IMPL_LINK_NOARG(ScDataProviderDlg, ScrollToEnd, Timer*, void)
{
    mxScroll->vadjustment_set_value(mxScroll->vadjustment_get_upper());
}

IMPL_LINK_NOARG(ScDataProviderDlg, ApplyQuitHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ScDataProviderDlg, CancelQuitHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(ScDataProviderDlg, TransformationListHdl, weld::Button&, void)
{
    OUString transformation_string = mxTransformationBox->get_active_text();
    for (auto& i: aTransformationData)
    {
        if (transformation_string == OUString::createFromAscii(i.aTransformationName))
        {
            i.maCallback(this);
            maIdle.Start();
            return;
        }
    }
}

IMPL_LINK_NOARG(ScDataProviderDlg, ProviderSelectHdl, weld::ComboBox&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderDlg, IDEditHdl, weld::Entry&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderDlg, URLEditHdl, weld::Entry&, void)
{
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderDlg, ApplyBtnHdl, weld::Button&, void)
{
    updateApplyBtn(true);
    import(*mxDoc, true);
}

IMPL_LINK_NOARG(ScDataProviderDlg, BrowseBtnHdl, weld::Button&, void)
{
    sfx2::FileDialogHelper aFileDialog(0, FileDialogFlags::NONE, m_xDialog.get());
    aFileDialog.SetContext(sfx2::FileDialogHelper::CalcDataProvider);
    if (aFileDialog.Execute() != ERRCODE_NONE)
        return;

    mxEditURL->set_text(aFileDialog.GetPath());
    isValid();
}

IMPL_LINK_NOARG(ScDataProviderDlg, TransformationSelectHdl, weld::ComboBox&, void)
{
    mxAddTransformationBtn->set_sensitive(true);
    mxAddTransformationBtn->set_tooltip_text(msAddTransformationToolTip);
}

sc::ExternalDataSource ScDataProviderDlg::getDataSource(ScDocument* pDoc)
{
    sc::ExternalDataSource aSource(mxEditURL->get_text(), mxProviderList->get_active_text(), pDoc);

    OUString aID = mxEditID->get_text();
    aSource.setID(aID);
    return aSource;
}

void ScDataProviderDlg::isValid()
{
    bool bValid = !mxProviderList->get_active_text().isEmpty();
    bValid &= !mxEditURL->get_text().isEmpty();
    updateApplyBtn(bValid);
}

void ScDataProviderDlg::updateApplyBtn(bool bValidConfig)
{
    if (!bValidConfig)
    {
        mxApplyBtn->set_sensitive(false);
        mxApplyBtn->set_tooltip_text(OUString());
        return;
    }

    mxApplyBtn->set_sensitive(true);
    mxApplyBtn->set_tooltip_text(msApplyTooltip);
}

void ScDataProviderDlg::deleteColumn()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScDeleteColumnTransformationControl>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::splitColumn()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScSplitColumnTransformationControl>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::mergeColumns()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mxTable->getColRange(nStartCol, nEndCol);
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScMergeColumnTransformationControl>(mxDoc.get(), mxTransformationList.get(), nStartCol, nEndCol, mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::textTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScColumnTextTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::sortTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScSortTransformationControl>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::aggregateFunction()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScAggregateFunction>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::numberTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScNumberTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::replaceNullTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScReplaceNullTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::dateTimeTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList,this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScDateTimeTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::findReplaceTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList, this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScFindReplaceTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::deleteRowTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList, this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScDeleteRowTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

void ScDataProviderDlg::swapRowsTransformation()
{
    std::function<void(sal_uInt32&)> adeleteTransformation = std::bind(&ScDataProviderDlg::deletefromList, this, std::placeholders::_1);
    maControls.emplace_back(std::make_unique<ScSwapRowsTransformation>(mxDoc.get(), mxTransformationList.get(), mnIndex++, adeleteTransformation));
}

namespace {

bool hasDBName(const OUString& rName, ScDBCollection* pDBCollection)
{
    if (pDBCollection->getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(rName)))
        return true;

    return false;
}

}

void ScDataProviderDlg::import(ScDocument& rDoc, bool bInternal)
{
    sc::ExternalDataSource aSource = getDataSource(&rDoc);

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
        if (!hasDBName(aSource.getDBName(), rDoc.GetDBCollection()))
            return;
        rDoc.GetExternalDataMapper().insertDataSource(aSource);
    }
    aSource.refresh(&rDoc, true);
    mxTable->Invalidate();
}

void ScDataProviderDlg::deletefromList(sal_uInt32 nIndex)
{
    auto itr = maControls.erase(maControls.begin() + nIndex);
    while (itr != maControls.end())
    {
        (*itr)->updateIndex(nIndex++);
        ++itr;
    }
    --mnIndex;
}

IMPL_LINK_NOARG(ScDeleteColumnTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScSplitColumnTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScMergeColumnTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScNumberTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScAggregateFunction, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScSortTransformationControl, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScColumnTextTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScReplaceNullTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScDateTimeTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScFindReplaceTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScDeleteRowTransformation, DeleteHdl, weld::Button&, void)
{
    maDeleteTransformation(mnIndex);
}

IMPL_LINK_NOARG(ScSwapRowsTransformation, DeleteHdl, weld::Button&, void)
{
   maDeleteTransformation(mnIndex);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
