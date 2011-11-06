/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SC_EXCEL_VBA_HELPER_HXX
#define SC_EXCEL_VBA_HELPER_HXX

#include <vbahelper/vbahelper.hxx>
#include "docsh.hxx"
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
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
public:
    static SfxItemSet* GetDataSet( ScCellRangesBase* pRangeObj );
};

// ============================================================================

} // namespace excel
} // namespace vba
} // namespace ooo

#endif
