/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmltransformationi.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlerror.hxx>

#include <datamapper.hxx>
#include <document.hxx>
#include <dbdata.hxx>
#include <sax/tools/converter.hxx>

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
        rDataSources[0].AddDataTransformation(
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
            rDataSources[0].AddDataTransformation(
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
        rDataSources[0].AddDataTransformation(
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
        switch (aType)
        {
            case XML_CASEMAP_LOWERCASE:
                maType = sc::TEXT_TRANSFORM_TYPE::TO_LOWER;
                break;
            case XML_CASEMAP_UPPERCASE:
                maType = sc::TEXT_TRANSFORM_TYPE::TO_UPPER;
                break;
            case XML_CASEMAP_CAPITALIZE:
                maType = sc::TEXT_TRANSFORM_TYPE::CAPITALIZE;
                break;
            case XML_TRIM:
                maType = sc::TEXT_TRANSFORM_TYPE::TRIM;
                break;
        }
    }
}

ScXMLColumnTextContext::~ScXMLColumnTextContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[0].AddDataTransformation(
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
        switch (aType)
        {
            case XML_SUM:
                maType = sc::AGGREGATE_FUNCTION::SUM;
                break;
            case XML_AVERAGE:
                maType = sc::AGGREGATE_FUNCTION::AVERAGE;
                break;
            case XML_MIN:
                maType = sc::AGGREGATE_FUNCTION::MIN;
                break;
            case XML_MAX:
                maType = sc::AGGREGATE_FUNCTION::MAX;
                break;
        }
    }
}

ScXMLColumnAggregateContext::~ScXMLColumnAggregateContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[0].AddDataTransformation(
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
{
    OUString aType = OUString();
    SCCOL maCol;
    sc::NUMBER_TRANSFORM_TYPE maType;
    int maPrecision;

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
                case XML_ELEMENT(CALC_EXT, XML_COLUMN):
                {
                    maCol = aIter.toInt32();
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
        switch (aType)
        {
            case XML_ROUND:
                maType = sc::NUMBER_TRANSFORM_TYPE::ROUND;
                break;
                // TODO add all trnasformations
        }
    }

    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    if (!rDataSources.empty())
    {
        rDataSources[0].AddDataTransformation(
            std::make_shared<sc::NumberTransformation>(maCol, maType, maPrecision));
    }
}

ScXMLColumnNumberContext::~ScXMLColumnNumberContext() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
