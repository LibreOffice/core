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

#ifndef SC_XMLEXPORTDATABASERANGES_HXX
#define SC_XMLEXPORTDATABASERANGES_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
namespace binfilter {

class ScXMLExport;
class ScDocument;
class ScMyEmptyDatabaseRangesContainer;

class ScXMLExportDatabaseRanges
{
    ScXMLExport&	rExport;
    ScDocument*		pDoc;

    void WriteImportDescriptor(const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue> aImportDescriptor);
    ::rtl::OUString getOperatorXML(const ::com::sun::star::sheet::FilterOperator aFilterOperator, const sal_Bool bUseRegularExpressions) const;
    void WriteCondition(const ::com::sun::star::sheet::TableFilterField& aFilterField, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
    void WriteFilterDescriptor(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSheetFilterDescriptor>& xSheetFilterDescriptor, const ::rtl::OUString sDatabaseRangeName);
    void WriteSortDescriptor(const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue> aSortProperties);
    void WriteSubTotalDescriptor(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSubTotalDescriptor> xSubTotalDescriptor, const ::rtl::OUString sDatabaseRangeName);
public:
    ScXMLExportDatabaseRanges(ScXMLExport& rExport);
    ~ScXMLExportDatabaseRanges();
    ScMyEmptyDatabaseRangesContainer GetEmptyDatabaseRanges();
    void WriteDatabaseRanges(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
