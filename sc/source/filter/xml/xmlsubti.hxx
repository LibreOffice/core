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
#ifndef SC_XMLSUBTI_HXX
#define SC_XMLSUBTI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "XMLTableShapeResizer.hxx"
#include "formula/grammar.hxx"
#include "tabprotection.hxx"
#include "rangelst.hxx"

class ScXMLImport;

struct ScXMLTabProtectionData
{
    ::rtl::OUString maPassword;
    ScPasswordHash  meHash1;
    ScPasswordHash  meHash2;
    bool            mbProtected;
    bool            mbSelectProtectedCells;
    bool            mbSelectUnprotectedCells;

    ScXMLTabProtectionData();
};

class ScMyTables
{
private:
    ScXMLImport&                        rImport;

    ScMyOLEFixer                        aFixupOLEs;

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > xCurrentSheet;
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > xCurrentCellRange;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage;
    ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > xShapes;
    rtl::OUString                       sCurrentSheetName;
    ScAddress                           maCurrentCellPos;
    ScRangeList                         maMatrixRangeList;
    ScXMLTabProtectionData              maProtectionData;
    sal_Int32                           nCurrentColStylePos;
    sal_Int16                           nCurrentDrawPage;
    sal_Int16                           nCurrentXShapes;

    void                                NewRow();
    void                                NewColumn(bool bIsCovered);

    void                                SetTableStyle(const rtl::OUString& sStyleName);
public:
                                        ScMyTables(ScXMLImport& rImport);
                                        ~ScMyTables();
    void                                NewSheet(const rtl::OUString& sTableName, const rtl::OUString& sStyleName,
                                                 const ScXMLTabProtectionData& rProtectData);
    void                                AddRow();
    void                                SetRowStyle(const rtl::OUString& rCellStyleName);
    void                                AddColumn(bool bIsCovered);
    void                                FixupOLEs() { aFixupOLEs.FixupOLEs(); }
    bool                                IsOLE(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape) const
                                            { return ScMyOLEFixer::IsOLE(rShape); }
    void                                DeleteTable();
    ScAddress                           GetCurrentCellPos() const { return maCurrentCellPos; };
    void                                AddColStyle(const sal_Int32 nRepeat, const rtl::OUString& rCellStyleName);
    ScXMLTabProtectionData&             GetCurrentProtectionData() { return maProtectionData; }
    rtl::OUString                       GetCurrentSheetName() const { return sCurrentSheetName; }
    SCTAB                               GetCurrentSheet() const { return (maCurrentCellPos.Tab() >= 0) ? maCurrentCellPos.Tab() : 0; }
    SCCOL                               GetCurrentColumn() const { return (maCurrentCellPos.Col() >= 0) ? maCurrentCellPos.Col() : 0; }
    SCROW                               GetCurrentRow() const { return (maCurrentCellPos.Row() >= 0) ? maCurrentCellPos.Row() : 0; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                                        GetCurrentXSheet() const { return xCurrentSheet; }
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                                        GetCurrentXCellRange() const { return xCurrentCellRange; }
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                                        GetCurrentXDrawPage();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                                        GetCurrentXShapes();
    bool                                HasDrawPage();
    bool                                HasXShapes();
    void                                AddOLE(com::sun::star::uno::Reference <com::sun::star::drawing::XShape>& rShape,
                                               const rtl::OUString &rRangeList);

    void                                AddMatrixRange( const SCCOL nStartColumn,
                                            const SCROW nStartRow,
                                            const SCCOL nEndColumn,
                                            const SCROW nEndRow,
                                            const rtl::OUString& rFormula,
                                            const rtl::OUString& rFormulaNmsp,
                                            const formula::FormulaGrammar::Grammar );
    bool                                IsPartOfMatrix( const ScAddress& rScAddress) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
