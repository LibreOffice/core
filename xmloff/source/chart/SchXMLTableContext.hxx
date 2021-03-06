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

#include <xmloff/xmlictxt.hxx>
#include <xmloff/SchXMLImportHelper.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include "transporttypes.hxx"

namespace com::sun::star {
    namespace xml::sax {
        class XAttributeList;
    }
    namespace chart {
        class XChartDocument;
    }
}

class SchXMLTableContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;

    bool mbHasRowPermutation;
    bool mbHasColumnPermutation;
    css::uno::Sequence< sal_Int32 > maRowPermutation;
    css::uno::Sequence< sal_Int32 > maColumnPermutation;

public:
    SchXMLTableContext( SvXMLImport& rImport,
                        SchXMLTable& aTable );
    virtual ~SchXMLTableContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    void setRowPermutation( const css::uno::Sequence< sal_Int32 > & rPermutation );
    void setColumnPermutation( const css::uno::Sequence< sal_Int32 > & rPermutation );
};

class SchXMLTableHelper
{
public:
    static void applyTableToInternalDataProvider( const SchXMLTable& rTable,
                            const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc );

    /** This function reorders local data to fit the correct data structure.
        Call it after the data series got their styles set.
     */
    static void switchRangesFromOuterToInternalIfNecessary( const SchXMLTable& rTable,
                                  const tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                                  const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc,
                                  css::chart::ChartDataRowSource eDataRowSource );
};

// classes for columns

/** With this context all column elements are parsed to
    determine the index of the column containing
    the row descriptions and probably get an estimate
    for the altogether number of columns
 */
class SchXMLTableColumnsContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;

public:
    SchXMLTableColumnsContext( SvXMLImport& rImport,
                               SchXMLTable& aTable );
    virtual ~SchXMLTableColumnsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

class SchXMLTableColumnContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;

public:
    SchXMLTableColumnContext( SvXMLImport& rImport,
                              SchXMLTable& aTable );
    virtual ~SchXMLTableColumnContext() override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

// classes for rows

class SchXMLTableRowsContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;

public:
    SchXMLTableRowsContext( SvXMLImport& rImport,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableRowsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

class SchXMLTableRowContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;

public:
    SchXMLTableRowContext( SvXMLImport& rImport,
                           SchXMLTable& aTable );
    virtual ~SchXMLTableRowContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

// classes for cells and their content

class SchXMLTableCellContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;
    OUString maCellContent;
    OUString maRangeId;
    bool mbReadText;

public:
    SchXMLTableCellContext( SvXMLImport& rImport,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableCellContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
