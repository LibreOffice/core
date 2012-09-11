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

#include <vbahelper/vbahelper.hxx>
#include "docsh.hxx"
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <ooo/vba/XHelperInterface.hpp>

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

css::uno::Reference< css::sheet::XUnnamedDatabaseRanges > GetUnnamedDataBaseRanges( ScDocShell* pShell ) throw ( css::uno::RuntimeException );

css::uno::Reference< css::sheet::XDatabaseRange > GetAutoFiltRange( ScDocShell* pShell, sal_Int16 nSheet ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::sheet::XSpreadsheet >& xSheet ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::table::XCellRange >& xRange ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::table::XCell >& xCell ) throw ( css::uno::RuntimeException );
css::uno::Reference< ooo::vba::XHelperInterface > getUnoSheetModuleObj( const css::uno::Reference< css::frame::XModel >& xModel, SCTAB nTab ) throw ( css::uno::RuntimeException );

ScDocShell* GetDocShellFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );
ScDocShell* GetDocShellFromRanges( const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges ) throw ( css::uno::RuntimeException );
ScDocument* GetDocumentFromRange( const css::uno::Reference< css::uno::XInterface >& xRange ) throw ( css::uno::RuntimeException );
void setUpDocumentModules( const css::uno::Reference< css::sheet::XSpreadsheetDocument >& xDoc );

// ============================================================================

class ScVbaCellRangeAccess
{
public:
    static SfxItemSet* GetDataSet( ScCellRangesBase* pRangeObj );
};

// Extracts a implementation object ( via XUnoTunnel ) from an uno object
// by default will throw if unsuccessful.
template < typename ImplObject >
    ImplObject* getImplFromDocModuleWrapper( const css::uno::Reference< css::uno::XInterface >& rxWrapperIf, bool bThrow = true ) throw (css::uno::RuntimeException)
    {
        ImplObject* pObj = NULL;
        css::uno::Reference< css::lang::XUnoTunnel >  xTunnel( rxWrapperIf, css::uno::UNO_QUERY );
        if ( xTunnel.is() )
            pObj = reinterpret_cast<ImplObject*>( xTunnel->getSomething(ImplObject::getUnoTunnelId()));
        if ( bThrow && !pObj )
            throw css::uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Internal error, can't exctract implementation object" ) ), rxWrapperIf );
        return pObj;
    }
// ============================================================================

} // namespace excel
} // namespace vba
} // namespace ooo

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
