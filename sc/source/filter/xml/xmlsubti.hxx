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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLSUBTI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLSUBTI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "XMLTableShapeResizer.hxx"
#include <formula/grammar.hxx>
#include "tabprotection.hxx"
#include "rangelst.hxx"

class ScXMLImport;

struct ScXMLTabProtectionData
{
    OUString maPassword;
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

    css::uno::Reference< css::sheet::XSpreadsheet > xCurrentSheet;
    css::uno::Reference< css::table::XCellRange > xCurrentCellRange;
    css::uno::Reference< css::drawing::XDrawPage > xDrawPage;
    css::uno::Reference < css::drawing::XShapes > xShapes;
    OUString                       sCurrentSheetName;
    ScAddress                           maCurrentCellPos;
    ScRangeList                         maMatrixRangeList;
    ScXMLTabProtectionData              maProtectionData;
    sal_Int32                           nCurrentColCount;
    sal_Int16                           nCurrentDrawPage;
    sal_Int16                           nCurrentXShapes;

    void                                SetTableStyle(const OUString& sStyleName);
public:
    explicit                            ScMyTables(ScXMLImport& rImport);
                                        ~ScMyTables();
    void                                NewSheet(const OUString& sTableName, const OUString& sStyleName,
                                                 const ScXMLTabProtectionData& rProtectData);
    void                                AddRow();
    void                                SetRowStyle(const OUString& rCellStyleName);
    void                                AddColumn(bool bIsCovered);
    void                                FixupOLEs() { aFixupOLEs.FixupOLEs(); }
    static bool                         IsOLE(css::uno::Reference< css::drawing::XShape >& rShape)
                                            { return ScMyOLEFixer::IsOLE(rShape); }
    void                                DeleteTable();
    const ScAddress&                    GetCurrentCellPos() const { return maCurrentCellPos; };
    void                                AddColStyle(const sal_Int32 nRepeat, const OUString& rCellStyleName);
    ScXMLTabProtectionData&             GetCurrentProtectionData() { return maProtectionData; }
    const OUString&                     GetCurrentSheetName() const { return sCurrentSheetName; }
    SCTAB                               GetCurrentSheet() const { return (maCurrentCellPos.Tab() >= 0) ? maCurrentCellPos.Tab() : 0; }
    SCCOL                               GetCurrentColCount() const { return std::min<sal_Int32>(nCurrentColCount, MAXCOL); }
    SCROW                               GetCurrentRow() const { return (maCurrentCellPos.Row() >= 0) ? maCurrentCellPos.Row() : 0; }
    const css::uno::Reference< css::sheet::XSpreadsheet >&
                                        GetCurrentXSheet() const { return xCurrentSheet; }
    css::uno::Reference< css::drawing::XDrawPage > const &
                                        GetCurrentXDrawPage();
    css::uno::Reference< css::drawing::XShapes > const &
                                        GetCurrentXShapes();
    bool                                HasDrawPage();
    bool                                HasXShapes();
    void                                AddOLE(css::uno::Reference <css::drawing::XShape>& rShape,
                                               const OUString &rRangeList);

    void                                AddMatrixRange( const SCCOL nStartColumn,
                                            const SCROW nStartRow,
                                            const SCCOL nEndColumn,
                                            const SCROW nEndRow,
                                            const OUString& rFormula,
                                            const OUString& rFormulaNmsp,
                                            const formula::FormulaGrammar::Grammar );
    bool                                IsPartOfMatrix( const ScAddress& rScAddress) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
