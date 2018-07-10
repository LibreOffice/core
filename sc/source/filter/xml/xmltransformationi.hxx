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
public:
    ScXMLColumnRemoveContext(ScXMLImport& rImport,
                             const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnRemoveContext() override;
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
public:
    ScXMLColumnMergeContext(ScXMLImport& rImport,
                            const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnMergeContext() override;
};

class ScXMLColumnSortContext : public ScXMLImportContext
{
public:
    ScXMLColumnSortContext(ScXMLImport& rImport,
                           const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnSortContext() override;
};

class ScXMLColumnTextContext : public ScXMLImportContext
{
public:
    ScXMLColumnTextContext(ScXMLImport& rImport,
                           const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnTextContext() override;
};

class ScXMLColumnAggregateContext : public ScXMLImportContext
{
public:
    ScXMLColumnAggregateContext(ScXMLImport& rImport,
                                const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnAggregateContext() override;
};

class ScXMLColumnNumberContext : public ScXMLImportContext
{
public:
    ScXMLColumnNumberContext(ScXMLImport& rImport,
                             const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLColumnNumberContext() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
