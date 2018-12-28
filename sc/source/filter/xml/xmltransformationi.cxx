/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlimprt.hxx"
#include "xmltransformationi.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include <datamapper.hxx>
#include <document.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTransformationsContext::ScXMLTransformationsContext(ScXMLImport& rImport)
    : ScXMLImportContext(rImport)
{
}

ScXMLTransformationsContext::~ScXMLTransformationsContext() {}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLTransformationsContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);

    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_REMOVE_TRANSFORMATION):
        {
            pContext = new ScXMLColumnRemoveContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_SPLIT_TRANSFORMATION):
        {
            pContext = new ScXMLColumnSplitContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_MERGE_TRANSFORMATION):
        {
            pContext = new ScXMLColumnMergeContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_SORT_TRANSFORMATION):
        {
            pContext = new ScXMLColumnSortContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_TEXT_TRANSFORMATION):
        {
            pContext = new ScXMLColumnTextContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_AGGREGATE_TRANSFORMATION):
        {
            pContext = new ScXMLColumnAggregateContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_NUMBER_TRANSFORMATION):
        {
            pContext = new ScXMLColumnNumberContext(GetScImport(), pAttribList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_COLUMN_REPLACENULL_TRANSFORMATION):
        {
            pContext = new ScXMLColumnRemoveNullContext(GetScImport(), pAttribList);
        }
        break;
    }

    if (!pContext)
        pContext = new SvXMLImportContext(GetImport());

    return pContext;
}

ScXMLColumnRemoveContext::ScXMLColumnRemoveContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& /*rAttrList*/)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnRemoveContext::~ScXMLColumnRemoveContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::ColumnRemoveTransformation>(maColumns));
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnRemoveContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);

    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;

    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }

    return new SvXMLImportContext(GetImport());
}

ScXMLColumnSplitContext::ScXMLColumnSplitContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
    SCCOL mnCol = 0;
    OUString cSeparator;

    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                {
                    mnCol = aIter.toInt32();
                }
                break;
                case XML_ELEMENT(CALC_EXT, XML_SEPARATOR):
                {
                    cSeparator = aIter.toString();
                }
                break;
            }
        }
    }

    if (mnCol > 0)
    {
        ScDocument* pDoc = GetScImport().GetDocument();
        auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
        if (!rDataSources.empty())
        {
            rDataSources[rDataSources.size() - 1].AddDataTransformation(
                std::make_shared<sc::SplitColumnTransformation>(mnCol, cSeparator.toChar()));
        }
    }
}

ScXMLColumnSplitContext::~ScXMLColumnSplitContext() {}

ScXMLColumnMergeContext::ScXMLColumnMergeContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_MERGE_STRING):
                {
                    maMergeString = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLColumnMergeContext::~ScXMLColumnMergeContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::MergeColumnTransformation>(maColumns, maMergeString));
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnMergeContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;
    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return new SvXMLImportContext(GetImport());
}

ScXMLColumnSortContext::ScXMLColumnSortContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& /*rAttrList*/)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnSortContext::~ScXMLColumnSortContext() {}

/*
uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnSortContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{

}
*/

ScXMLColumnTextContext::ScXMLColumnTextContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
    , maType(sc::TEXT_TRANSFORM_TYPE::TO_LOWER)
{
    OUString aType = OUString();

    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_TYPE):
                {
                    aType = aIter.toString();
                }
                break;
            }
        }
    }

    if (!aType.isEmpty())
    {
        if (aType == "lowercase")
            maType = sc::TEXT_TRANSFORM_TYPE::TO_LOWER;
        else if (aType == "uppercase")
            maType = sc::TEXT_TRANSFORM_TYPE::TO_UPPER;
        else if (aType == "capitalize")
            maType = sc::TEXT_TRANSFORM_TYPE::CAPITALIZE;
        else if (aType == "trim")
            maType = sc::TEXT_TRANSFORM_TYPE::TRIM;
    }
}

ScXMLColumnTextContext::~ScXMLColumnTextContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::TextTransformation>(maColumns, maType));
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnTextContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);

    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;

    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return new SvXMLImportContext(GetImport());
}

ScXMLColumnAggregateContext::ScXMLColumnAggregateContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
    , maType(sc::AGGREGATE_FUNCTION::SUM)
{
    OUString aType = OUString();

    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_TYPE):
                {
                    aType = aIter.toString();
                }
                break;
            }
        }
    }

    if (!aType.isEmpty())
    {
        if (aType == "sum")
            maType = sc::AGGREGATE_FUNCTION::SUM;
        else if (aType == "average")
            maType = sc::AGGREGATE_FUNCTION::AVERAGE;
        else if (aType == "min")
            maType = sc::AGGREGATE_FUNCTION::MIN;
        else if (aType == "max")
            maType = sc::AGGREGATE_FUNCTION::MAX;
    }
}

ScXMLColumnAggregateContext::~ScXMLColumnAggregateContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::AggregateFunction>(maColumns, maType));
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnAggregateContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;

    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return new SvXMLImportContext(GetImport());
}

ScXMLColumnNumberContext::ScXMLColumnNumberContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
    , maType(sc::NUMBER_TRANSFORM_TYPE::ROUND)
    , maPrecision(0)
{
    OUString aType;
    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_TYPE):
                {
                    aType = aIter.toString();
                }
                break;
                case XML_ELEMENT(CALC_EXT, XML_PRECISION):
                {
                    maPrecision = aIter.toInt32();
                }
                break;
            }
        }
    }

    if (!aType.isEmpty())
    {
        if (aType == "round")
            maType = sc::NUMBER_TRANSFORM_TYPE::ROUND;
        else if (aType == "round-up")
            maType = sc::NUMBER_TRANSFORM_TYPE::ROUND_UP;
        else if (aType == "round-down")
            maType = sc::NUMBER_TRANSFORM_TYPE::ROUND_DOWN;
        else if (aType == "abs")
            maType = sc::NUMBER_TRANSFORM_TYPE::ABSOLUTE;
        else if (aType == "log")
            maType = sc::NUMBER_TRANSFORM_TYPE::LOG_E;
        else if (aType == "log-base-10")
            maType = sc::NUMBER_TRANSFORM_TYPE::LOG_10;
        else if (aType == "cube")
            maType = sc::NUMBER_TRANSFORM_TYPE::CUBE;
        else if (aType == "number-square")
            maType = sc::NUMBER_TRANSFORM_TYPE::SQUARE;
        else if (aType == "square-root")
            maType = sc::NUMBER_TRANSFORM_TYPE::SQUARE_ROOT;
        else if (aType == "exponential")
            maType = sc::NUMBER_TRANSFORM_TYPE::EXPONENT;
        else if (aType == "even")
            maType = sc::NUMBER_TRANSFORM_TYPE::IS_EVEN;
        else if (aType == "odd")
            maType = sc::NUMBER_TRANSFORM_TYPE::IS_ODD;
        else if (aType == "sign")
            maType = sc::NUMBER_TRANSFORM_TYPE::SIGN;
    }
}

ScXMLColumnNumberContext::~ScXMLColumnNumberContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::NumberTransformation>(maColumns, maType, maPrecision));
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnNumberContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;

    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return new SvXMLImportContext(GetImport());
}

ScXMLColumnRemoveNullContext::ScXMLColumnRemoveNullContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_REPLACE_STRING):
                {
                    maReplaceString = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLColumnRemoveNullContext::~ScXMLColumnRemoveNullContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::ReplaceNullTransformation>(maColumns, maReplaceString));
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLColumnRemoveNullContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;
    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return new SvXMLImportContext(GetImport());
}

ScXMLDateTimeContext::ScXMLDateTimeContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
    , maType(sc::DATETIME_TRANSFORMATION_TYPE::DATE_STRING)
{
    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CALC_EXT, XML_TYPE):
                {
                    aType = aIter.toString();
                }
                break;
            }
        }
    }

    if (!aType.isEmpty())
    {
        if (aType == "date-string")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::DATE_STRING;
        else if (aType == "year")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::YEAR;
        else if (aType == "start-of-year")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR;
        else if (aType == "end-of-year")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR;
        else if (aType == "month")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::MONTH;
        else if (aType == "month-name")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::MONTH_NAME;
        else if (aType == "start-of-month")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH;
        else if (aType == "end-of-month")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH;
        else if (aType == "day")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::DAY;
        else if (aType == "day-of-week")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK;
        else if (aType == "day-of-year")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR;
        else if (aType == "quarter")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::QUARTER;
        else if (aType == "start-of-quarter")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::START_OF_QUARTER;
        else if (aType == "end-of-quarter")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::END_OF_QUARTER;
        else if (aType == "time")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::TIME;
        else if (aType == "hour")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::HOUR;
        else if (aType == "minute")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::MINUTE;
        else if (aType == "seconds")
            maType = sc::DATETIME_TRANSFORMATION_TYPE::SECOND;
    }
}

ScXMLDateTimeContext::~ScXMLDateTimeContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[rDataSources.size() - 1].AddDataTransformation(
            std::make_shared<sc::DateTimeTransformation>(maColumns, maType));
    }
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL ScXMLDateTimeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = sax_fastparser::FastAttributeList::castToFastAttributeList(xAttrList);
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList = pAttribList;

    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
        {
            if (rAttrList.is())
            {
                for (auto& aIter : *rAttrList)
                {
                    switch (aIter.getToken())
                    {
                        case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                        {
                            maColumns.insert(aIter.toInt32());
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return new SvXMLImportContext(GetImport());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
