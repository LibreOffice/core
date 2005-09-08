/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLExportDatabaseRanges.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:55:16 $
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

#ifndef SC_XMLEXPORTDATABASERANGES_HXX
#define SC_XMLEXPORTDATABASERANGES_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_FILTEROPERATOR_HPP_
#include <com/sun/star/sheet/FilterOperator.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_TABLEFILTERFIELD_HPP_
#include <com/sun/star/sheet/TableFilterField.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETFILTERDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif

class ScXMLExport;
class ScDocument;
class ScMyEmptyDatabaseRangesContainer;

class ScXMLExportDatabaseRanges
{
    ScXMLExport&    rExport;
    ScDocument*     pDoc;

    void WriteImportDescriptor(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue> aImportDescriptor);
    rtl::OUString getOperatorXML(const com::sun::star::sheet::FilterOperator aFilterOperator, const sal_Bool bUseRegularExpressions) const;
    void WriteCondition(const com::sun::star::sheet::TableFilterField& aFilterField, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
    void WriteFilterDescriptor(const com::sun::star::uno::Reference <com::sun::star::sheet::XSheetFilterDescriptor>& xSheetFilterDescriptor, const rtl::OUString sDatabaseRangeName);
    void WriteSortDescriptor(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue> aSortProperties);
    void WriteSubTotalDescriptor(const com::sun::star::uno::Reference <com::sun::star::sheet::XSubTotalDescriptor> xSubTotalDescriptor, const rtl::OUString sDatabaseRangeName);
public:
    ScXMLExportDatabaseRanges(ScXMLExport& rExport);
    ~ScXMLExportDatabaseRanges();
    ScMyEmptyDatabaseRangesContainer GetEmptyDatabaseRanges();
    void WriteDatabaseRanges(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
};

#endif

