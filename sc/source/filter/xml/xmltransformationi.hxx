/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLTRANSFORMATIONI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLTRANSFORMATIONI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

#include <datatransformation.hxx>

#include "xmlimprt.hxx"
#include "importcontext.hxx"

class ScXMLTransformationsContext : public ScXMLImportContext
{
public:
    ScXMLTransformationsContext(ScXMLImport& rImport);

    virtual ~ScXMLTransformationsContext() override;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

class ScXMLColumnRemoveContext : public ScXMLImportContext
{
    std::set<SCCOL> maColumns;

public:
    ScXMLColumnRemoveContext(ScXMLImport& rImport,
                             const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnRemoveContext() override;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

class ScXMLColumnSplitContext : public ScXMLImportContext
{
public:
    ScXMLColumnSplitContext(ScXMLImport& rImport,
                            const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnSplitContext() override;
};

class ScXMLColumnMergeContext : public ScXMLImportContext
{
    std::set<SCCOL> maColumns;
    OUString maMergeString;

public:
    ScXMLColumnMergeContext(ScXMLImport& rImport,
                            const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnMergeContext() override;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

class ScXMLColumnSortContext : public ScXMLImportContext
{
    ScSortParam maSortParam;

public:
    ScXMLColumnSortContext(ScXMLImport& rImport,
                           const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnSortContext() override;
    /*
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
    */
};

class ScXMLColumnTextContext : public ScXMLImportContext
{
    std::set<SCCOL> maColumns;
    sc::TEXT_TRANSFORM_TYPE maType;

public:
    ScXMLColumnTextContext(ScXMLImport& rImport,
                           const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnTextContext() override;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

class ScXMLColumnAggregateContext : public ScXMLImportContext
{
    std::set<SCCOL> maColumns;
    sc::AGGREGATE_FUNCTION maType;

public:
    ScXMLColumnAggregateContext(ScXMLImport& rImport,
                                const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnAggregateContext() override;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

class ScXMLColumnNumberContext : public ScXMLImportContext
{
    OUString aType;
    sc::NUMBER_TRANSFORM_TYPE maType;
    int maPrecision;
    std::set<SCCOL> maColumns;

public:
    ScXMLColumnNumberContext(ScXMLImport& rImport,
                             const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnNumberContext() override;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
