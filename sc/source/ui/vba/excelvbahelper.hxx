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
#ifndef SC_EXCEL_VBA_HELPER_HXX
#define SC_EXCEL_VBA_HELPER_HXX

#include<vbahelper/vbahelper.hxx>
#include <docsh.hxx>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <ooo/vba/XHelperInterface.hpp>
#include <formula/grammar.hxx>

class ScCellRangesBase;

namespace ooo {
namespace vba {
namespace excel {

// ============================================================================

// nTabs empty means apply zoom to all sheets
void implSetZoom( const css::uno::Reference< css::frame::XModel >& xModel, sal_Int16 nZoom, std::vector< SCTAB >& nTabs );
void implnCopy( const css::uno::Reference< css::frame::XModel>& xModel );
void implnPaste ( const css::uno::Reference< css::frame::XModel>& xModel );
void implnCut( const css::uno::Reference< css::frame::XModel>& xModel );
void implnPasteSpecial( const css::uno::Reference< css::frame::XModel>& xModel, sal_uInt16 nFlags,sal_uInt16 nFunction,sal_Bool bSkipEmpty, sal_Bool bTranspose);
ScTabViewShell* getBestViewShell( const css::uno::Reference< css::frame::XModel>& xModel ) ;
ScDocShell* getDocShell( const css::uno::Reference< css::frame::XModel>& xModel ) ;
ScTabViewShell* getCurrentBestViewShell( const css::uno::Reference< css::uno::XComponentContext >& xContext );
SfxViewFrame* getViewFrame( const css::uno::Reference< css::frame::XModel >& xModel );

css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::sheet::XSpreadsheet >& xSheet ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::table::XCellRange >& xRange ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::table::XCell >& xCell ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::frame::XModel >& xModel, SCTAB nTab ) throw ( css::uno::RuntimeException );

ScDocShell* GetDocShellFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );
ScDocument* GetDocumentFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );
css::uno::Reference< css::frame::XModel > GetModelFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );

// ============================================================================

class ScVbaCellRangeAccess
{
    namespace vba
    {
        namespace excel
        {
            // nTabs empty means apply zoom to all sheets
            void implSetZoom( const css::uno::Reference< css::frame::XModel >& xModel, sal_Int16 nZoom, std::vector< SCTAB >& nTabs );
            void implnCopy( const css::uno::Reference< css::frame::XModel>& xModel );
            void implnPaste ( const css::uno::Reference< css::frame::XModel>& xModel );
            void implnCut( const css::uno::Reference< css::frame::XModel>& xModel );
            void implnPasteSpecial( const css::uno::Reference< css::frame::XModel>& xModel, sal_uInt16 nFlags,sal_uInt16 nFunction,sal_Bool bSkipEmpty, sal_Bool bTranspose);
            void implnCopyRange( const css::uno::Reference< css::frame::XModel>& xModel, const ScRange& rRange );
            ScTabViewShell* getBestViewShell( const css::uno::Reference< css::frame::XModel>& xModel ) ;
            ScDocShell* getDocShell( const css::uno::Reference< css::frame::XModel>& xModel ) ;
            ScTabViewShell* getCurrentBestViewShell( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            SfxViewFrame* getViewFrame( const css::uno::Reference< css::frame::XModel >& xModel );
            sal_Bool IsR1C1ReferFormat( ScDocument* pDoc, const ::rtl::OUString& sRangeStr );
            formula::FormulaGrammar::Grammar GetFormulaGrammar( ScDocument* pDoc, const ScAddress& sAddress, const css::uno::Any& aFormula );
            void CompileExcelFormulaToODF( ScDocument* pDoc, const String& rOldFormula, String& rNewFormula );
            void CompileODFFormulaToExcel( ScDocument* pDoc, const String& rOldFormula, String& rNewFormula, const formula::FormulaGrammar::Grammar eGrammar );
            css::uno::Reference< css::sheet::XDatabaseRanges > GetDataBaseRanges( ScDocShell* pShell ) throw ( css::uno::RuntimeException );

            css::uno::Reference< css::sheet::XDatabaseRange > GetAutoFiltRange( ScDocShell* pShell, sal_Int16 nSheet, rtl::OUString& sName ) throw ( css::uno::RuntimeException );
            css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges ) throw ( css::uno::RuntimeException );
            css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::table::XCellRange >& xRange ) throw ( css::uno::RuntimeException );
             ScDocShell* GetDocShellFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );
            ScDocShell* GetDocShellFromRanges( const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges ) throw ( css::uno::RuntimeException );
             ScDocument* GetDocumentFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );
             css::uno::Reference< css::frame::XModel > GetModelFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );

            class ScVbaCellRangeAccess
            {
            public:
                static SfxItemSet* GetDataSet( ScCellRangesBase* pRangeObj );
            };
        }
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
