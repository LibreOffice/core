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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLNEXPI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLNEXPI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include "address.hxx"
#include "xmlimprt.hxx"

#include <memory>

struct ScMyNamedExpression;
class ScRangeName;

class ScXMLNamedExpressionsContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

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
        GlobalInserter(ScXMLImport& rImport);
        virtual void insert(ScMyNamedExpression* pExp) SAL_OVERRIDE;
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
        virtual void insert(ScMyNamedExpression* pExp) SAL_OVERRIDE;
    private:
        ScXMLImport& mrImport;
        SCTAB mnTab;
    };

    ScXMLNamedExpressionsContext(
        ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        Inserter* pInserter );

    virtual ~ScXMLNamedExpressionsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;

private:
    std::shared_ptr<Inserter> mpInserter;
};

class ScXMLNamedRangeContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLNamedRangeContext(
        ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ScXMLNamedExpressionsContext::Inserter* pInserter );

    virtual ~ScXMLNamedRangeContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;

private:
    ScXMLNamedExpressionsContext::Inserter* mpInserter;
};

class ScXMLNamedExpressionContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLNamedExpressionContext(
        ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ScXMLNamedExpressionsContext::Inserter* pInserter );

    virtual ~ScXMLNamedExpressionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;

private:
    ScXMLNamedExpressionsContext::Inserter* mpInserter;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
