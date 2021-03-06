/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <types.hxx>
#include "importcontext.hxx"

#include <memory>

namespace sax_fastparser { class FastAttributeList; }

struct ScMyNamedExpression;

class ScXMLNamedExpressionsContext : public ScXMLImportContext
{
public:

    class Inserter
    {
    public:
        virtual ~Inserter() {}
        virtual void insert(ScMyNamedExpression* pExp) = 0;
    };

    /**
     * Global named expressions are inserted into ScXMLImport, which does the
     * bulk insertion at the end of the import.
     */
    class GlobalInserter : public Inserter
    {
    public:
        explicit GlobalInserter(ScXMLImport& rImport);
        virtual void insert(ScMyNamedExpression* pExp) override;
    private:
        ScXMLImport& mrImport;
    };

    /**
     * Sheet local named expressions are inserted directly into ScRangeName
     * instance of that sheet.  TODO: the global ones should be inserted the
     * same way for efficiency.
     */
    class SheetLocalInserter : public Inserter
    {
    public:
        SheetLocalInserter(ScXMLImport& rImport, SCTAB nTab);
        virtual void insert(ScMyNamedExpression* pExp) override;
    private:
        ScXMLImport& mrImport;
        SCTAB mnTab;
    };

    ScXMLNamedExpressionsContext(
        ScXMLImport& rImport,
        std::shared_ptr<Inserter> pInserter );

    virtual ~ScXMLNamedExpressionsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    std::shared_ptr<Inserter> mpInserter;
};

class ScXMLNamedRangeContext : public ScXMLImportContext
{
public:

    ScXMLNamedRangeContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
        ScXMLNamedExpressionsContext::Inserter* pInserter );

    virtual ~ScXMLNamedRangeContext() override;
};

class ScXMLNamedExpressionContext : public ScXMLImportContext
{
public:

    ScXMLNamedExpressionContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
        ScXMLNamedExpressionsContext::Inserter* pInserter );

    virtual ~ScXMLNamedExpressionContext() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
