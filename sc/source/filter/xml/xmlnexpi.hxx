/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_XMLNEXPI_HXX
#define SC_XMLNEXPI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

#include <boost/shared_ptr.hpp>

class ScXMLImport;
struct ScMyNamedExpression;
class ScRangeName;
class ScDocument;

class ScXMLNamedExpressionsContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    class Inserter
    {
    public:
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
        virtual void insert(ScMyNamedExpression* pExp);
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
        SheetLocalInserter(ScDocument* pDoc, ScRangeName& rRangeName);
        virtual void insert(ScMyNamedExpression* pExp);
    private:
        ScDocument* mpDoc;
        ScRangeName& mrRangeName;
    };

    ScXMLNamedExpressionsContext(
        ScXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        Inserter* pInserter );

    virtual ~ScXMLNamedExpressionsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

private:
    ::boost::shared_ptr<Inserter> mpInserter;
};

class ScXMLNamedRangeContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLNamedRangeContext(
        ScXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ScXMLNamedExpressionsContext::Inserter* pInserter );

    virtual ~ScXMLNamedRangeContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

private:
    ScXMLNamedExpressionsContext::Inserter* mpInserter;
};

class ScXMLNamedExpressionContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLNamedExpressionContext(
        ScXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        ScXMLNamedExpressionsContext::Inserter* pInserter );

    virtual ~ScXMLNamedExpressionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

private:
    ScXMLNamedExpressionsContext::Inserter* mpInserter;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
