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
#ifndef _SCH_XMLTABLECONTEXT_HXX_
#define _SCH_XMLTABLECONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include "SchXMLImport.hxx"
// #include "SchXMLChartContext.hxx"
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include "transporttypes.hxx"

namespace com { namespace sun { namespace star {
    namespace frame {
        class XModel;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    }}
    namespace chart {
        class XChartDocument;
        struct ChartSeriesAddress;
}}}}

// ========================================

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
                        const rtl::OUString& rLocalName,
                        SchXMLTable& aTable );
    virtual ~SchXMLTableContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    void setRowPermutation( const ::com::sun::star::uno::Sequence< sal_Int32 > & rPermutation );
    void setColumnPermutation( const ::com::sun::star::uno::Sequence< sal_Int32 > & rPermutation );
};

// ----------------------------------------

class SchXMLTableHelper
{
private:
    static void GetCellAddress( const rtl::OUString& rStr, sal_Int32& rCol, sal_Int32& rRow );
    static sal_Bool GetCellRangeAddress( const rtl::OUString& rStr, SchNumericCellRangeAddress& rResult );
    static void PutTableContentIntoSequence(
        const SchXMLTable& rTable,
        SchNumericCellRangeAddress& rAddress,
        sal_Int32 nSeriesIndex,
        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< double > >& aSequence );
    static void AdjustMax( const SchNumericCellRangeAddress& rAddr,
                           sal_Int32& nRows, sal_Int32& nColumns );

public:
    static void applyTableToInternalDataProvider( const SchXMLTable& rTable,
                            com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > xChartDoc );

    /** This function reorders local data to fit the correct data structure.
        Call it after the data series got their styles set.
     */
    static void switchRangesFromOuterToInternalIfNecessary( const SchXMLTable& rTable,
                                  const tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                                  com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > xChartDoc,
                                  ::com::sun::star::chart::ChartDataRowSource eDataRowSource );
};

// ========================================

// ----------------------------------------
// classes for columns
// ----------------------------------------

/** With this context all column elements are parsed to
    determine the index of the column containing
    the row descriptions and probably get an estimate
    for the altogether number of columns
 */
class SchXMLTableColumnsContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

public:
    SchXMLTableColumnsContext( SchXMLImportHelper& rImpHelper,
                               SvXMLImport& rImport,
                               const rtl::OUString& rLocalName,
                               SchXMLTable& aTable );
    virtual ~SchXMLTableColumnsContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLTableColumnContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

public:
    SchXMLTableColumnContext( SchXMLImportHelper& rImpHelper,
                              SvXMLImport& rImport,
                              const rtl::OUString& rLocalName,
                              SchXMLTable& aTable );
    virtual ~SchXMLTableColumnContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------
// classes for rows
// ----------------------------------------

class SchXMLTableRowsContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

public:
    SchXMLTableRowsContext( SchXMLImportHelper& rImpHelper,
                            SvXMLImport& rImport,
                            const rtl::OUString& rLocalName,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableRowsContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLTableRowContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;

public:
    SchXMLTableRowContext( SchXMLImportHelper& rImpHelper,
                           SvXMLImport& rImport,
                           const rtl::OUString& rLocalName,
                           SchXMLTable& aTable );
    virtual ~SchXMLTableRowContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------
// classes for cells and their content
// ----------------------------------------

class SchXMLTableCellContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    SchXMLTable& mrTable;
    rtl::OUString maCellContent;
    rtl::OUString maRangeId;
    sal_Bool mbReadText;

public:
    SchXMLTableCellContext( SchXMLImportHelper& rImpHelper,
                            SvXMLImport& rImport,
                            const rtl::OUString& rLocalName,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableCellContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

#endif  // _SCH_XMLTABLECONTEXT_HXX_
