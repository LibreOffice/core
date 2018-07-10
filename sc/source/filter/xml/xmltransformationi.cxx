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
#include <datatransformation.hxx>
#include <sax/tools/converter.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTransformationsContext::ScXMLTransformationsContext(ScXMLImport& rImport)
    : ScXMLImportContext(rImport)
{
    // has no attributes
    rImport.LockSolarMutex();
}

ScXMLTransformationsContext::~ScXMLTransformationsContext() { GetScImport().UnlockSolarMutex(); }

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
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
    std::set<SCCOL> aColumns;
}

ScXMLColumnRemoveContext::~ScXMLColumnRemoveContext() {}

ScXMLColumnSplitContext::ScXMLColumnSplitContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
    SCCOL mnCol = 0;
    sal_Unicode cSeparator;

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
                std::make_shared<sc::SplitColumnTransformation>(mnCol, cSeparator));
        }
    }
}

ScXMLColumnSplitContext::~ScXMLColumnSplitContext() {}

ScXMLColumnMergeContext::ScXMLColumnMergeContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnMergeContext::~ScXMLColumnMergeContext() {}

ScXMLColumnSortContext::ScXMLColumnSortContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnSortContext::~ScXMLColumnSortContext() {}

ScXMLColumnTextContext::ScXMLColumnTextContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnTextContext::~ScXMLColumnTextContext() {}

ScXMLColumnAggregateContext::ScXMLColumnAggregateContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnAggregateContext::~ScXMLColumnAggregateContext() {}

ScXMLColumnNumberContext::ScXMLColumnNumberContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
}

ScXMLColumnNumberContext::~ScXMLColumnNumberContext() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
