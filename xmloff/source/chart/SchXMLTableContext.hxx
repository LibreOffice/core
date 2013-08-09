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
#ifndef _SCH_XMLTABLECONTEXT_HXX_
#define _SCH_XMLTABLECONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include "SchXMLImport.hxx"
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include "transporttypes.hxx"

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {
        class XAttributeList;
    }}
    namespace chart {
        class XChartDocument;
}}}}

class SchXMLTableContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

    bool mbHasRowPermutation;
    bool mbHasColumnPermutation;
    ::com::sun::star::uno::Sequence< sal_Int32 > maRowPermutation;
    ::com::sun::star::uno::Sequence< sal_Int32 > maColumnPermutation;

public:
    SchXMLTableContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport,
                        const OUString& rLocalName,
                        SchXMLTable& aTable );
    virtual ~SchXMLTableContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    void setRowPermutation( const ::com::sun::star::uno::Sequence< sal_Int32 > & rPermutation );
    void setColumnPermutation( const ::com::sun::star::uno::Sequence< sal_Int32 > & rPermutation );
};

class SchXMLTableHelper
{
private:
    static void GetCellAddress( const OUString& rStr, sal_Int32& rCol, sal_Int32& rRow );
    static sal_Bool GetCellRangeAddress( const OUString& rStr, SchNumericCellRangeAddress& rResult );
    static void AdjustMax( const SchNumericCellRangeAddress& rAddr,
                           sal_Int32& nRows, sal_Int32& nColumns );

public:
    static void applyTableToInternalDataProvider( SchXMLTable rTable,
                            com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > xChartDoc,
                            com::sun::star::chart::ChartDataRowSource eDataRowSource );

    /** This function reorders local data to fit the correct data structure.
        Call it after the data series got their styles set.
     */
    static void switchRangesFromOuterToInternalIfNecessary( const SchXMLTable& rTable,
                                  const tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                                  com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > xChartDoc,
                                  ::com::sun::star::chart::ChartDataRowSource eDataRowSource );
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
                               const OUString& rLocalName,
                               SchXMLTable& aTable );
    virtual ~SchXMLTableColumnsContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

class SchXMLTableColumnContext : public SvXMLImportContext
{
private:
    SchXMLTable& mrTable;

public:
    SchXMLTableColumnContext( SvXMLImport& rImport,
                              const OUString& rLocalName,
                              SchXMLTable& aTable );
    virtual ~SchXMLTableColumnContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// classes for rows

class SchXMLTableRowsContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

public:
    SchXMLTableRowsContext( SchXMLImportHelper& rImpHelper,
                            SvXMLImport& rImport,
                            const OUString& rLocalName,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableRowsContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

class SchXMLTableRowContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

public:
    SchXMLTableRowContext( SchXMLImportHelper& rImpHelper,
                           SvXMLImport& rImport,
                           const OUString& rLocalName,
                           SchXMLTable& aTable );
    virtual ~SchXMLTableRowContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// classes for cells and their content

class SchXMLTableCellContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;
    OUString maCellContent;
    OUString maRangeId;
    sal_Bool mbReadText;

public:
    SchXMLTableCellContext( SchXMLImportHelper& rImpHelper,
                            SvXMLImport& rImport,
                            const OUString& rLocalName,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableCellContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

#endif  // _SCH_XMLTABLECONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
