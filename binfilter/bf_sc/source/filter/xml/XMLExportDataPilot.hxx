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

#ifndef _SC_XMLEXPORTDATAPILOT_HXX
#define _SC_XMLEXPORTDATAPILOT_HXX

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

class ScXMLExport;
class ScDocument;

class ScXMLExportDataPilot
{
    ScXMLExport&		rExport;
    ScDocument*			pDoc;

    ::rtl::OUString getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
                                    const sal_Bool bIsString, const double dVal, const String& sVal) const;
    void WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
    void WriteDPFilter(const ScQueryParam& aQueryParam);
public:
    ScXMLExportDataPilot(ScXMLExport& rExport);
    ~ScXMLExportDataPilot();
    void WriteDataPilots(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument>& xSpreaDoc);
};

} //namespace binfilter
#endif

