/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLTableContext.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 16:02:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SCH_XMLTABLECONTEXT_HXX_
#define _SCH_XMLTABLECONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef SCH_XMLIMPORT_HXX_
#include "SchXMLImport.hxx"
#endif
// #include "SchXMLChartContext.hxx"

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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
        class XChartDataArray;
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
        USHORT nPrefix,
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
    /// The data for the ChartDocument is applied linearly
    static void applyTableSimple(
        const SchXMLTable& rTable,
        const com::sun::star::uno::Reference< com::sun::star::chart::XChartDataArray > & xData );

    /** The data for the ChartDocument is applied by reading the
        table, the addresses of series, the addresses of labels,
        the cell-range-address for the categories
     */
    static void applyTable( const SchXMLTable& rTable,
                            com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > xChartDoc );

    /** Second part of applyTable that has to be called after the data series
        got their styles set.  This function reorders local data to fit the
        correct data structure.
     */
    static void postProcessTable( const SchXMLTable& rTable,
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
        USHORT nPrefix,
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
        USHORT nPrefix,
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
        USHORT nPrefix,
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
    sal_Bool mbReadPara;

public:
    SchXMLTableCellContext( SchXMLImportHelper& rImpHelper,
                            SvXMLImport& rImport,
                            const rtl::OUString& rLocalName,
                            SchXMLTable& aTable );
    virtual ~SchXMLTableCellContext();

    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

#endif  // _SCH_XMLTABLECONTEXT_HXX_
