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

#include "oox/xls/excelvbaproject.hxx"
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include "properties.hxx"
#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"

using ::rtl::OUString;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::sheet::XSpreadsheetDocument;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;

namespace oox {
namespace xls {

// ============================================================================

VbaProject::VbaProject( const Reference< XMultiServiceFactory >& rxGlobalFactory, const Reference< XSpreadsheetDocument >& rxDocument ) :
    ::oox::ole::VbaProject( rxGlobalFactory, Reference< XModel >( rxDocument, UNO_QUERY ), CREATE_OUSTRING( "Calc" ) ),
    mxDocument( rxDocument )
{
}

void VbaProject::attachToEvents()
{
    // do nothing is code is not executable
    if( !isImportVbaExecutable() )
        return;

    // document events
    PropertySet aDocProp( mxDocument );
    OUString aCodeName;
    aDocProp.getProperty( aCodeName, PROP_CodeName );
    attachToDocumentEvents( aCodeName );

    // sheet events
    if( mxDocument.is() ) try
    {
        Reference< XEnumerationAccess > xSheetsEA( mxDocument->getSheets(), UNO_QUERY_THROW );
        Reference< XEnumeration > xSheetsEnum( xSheetsEA->createEnumeration(), UNO_SET_THROW );
        // own try/catch for every sheet
        while( xSheetsEnum->hasMoreElements() ) try
        {
            // TODO: once we have chart sheets we need a switch/case on sheet type
            Reference< XEventsSupplier > xEventsSupp( xSheetsEnum->nextElement(), UNO_QUERY_THROW );
            PropertySet aSheetProp( xEventsSupp );
            aSheetProp.getProperty( aCodeName, PROP_CodeName );
            attachToSheetEvents( xEventsSupp, aCodeName );
        }
        catch( Exception& )
        {
        }
    }
    catch( Exception& )
    {
    }
}

// private --------------------------------------------------------------------

void VbaProject::attachToDocumentEvents( const OUString& rCodeName )
{
    if( (rCodeName.getLength() == 0) || !hasModule( rCodeName ) )
        return;

    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnLoad" ),          rCodeName, CREATE_OUSTRING( "Workbook_Open" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnFocus" ),         rCodeName, CREATE_OUSTRING( "Workbook_Activate" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnUnfocus" ),       rCodeName, CREATE_OUSTRING( "Workbook_Deactivate" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnSave" ),          rCodeName, CREATE_OUSTRING( "Workbook_BeforeSave" ),  OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO False, False" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnSaveAs" ),        rCodeName, CREATE_OUSTRING( "Workbook_BeforeSave" ),  OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO True, False" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnSaveDone" ),      rCodeName, CREATE_OUSTRING( "Workbook_AfterSave" ),   OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO True" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnSaveAsDone" ),    rCodeName, CREATE_OUSTRING( "Workbook_AfterSave" ),   OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO True" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnSaveFailed" ),    rCodeName, CREATE_OUSTRING( "Workbook_AfterSave" ),   OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO False" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnSaveAsFailed" ),  rCodeName, CREATE_OUSTRING( "Workbook_AfterSave" ),   OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO False" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnPrint" ),         rCodeName, CREATE_OUSTRING( "Workbook_BeforePrint" ), OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO False" ) );
    attachMacroToDocumentEvent( CREATE_OUSTRING( "OnPrepareUnload" ), rCodeName, CREATE_OUSTRING( "Workbook_BeforeClose" ), OUString(), OUString(), CREATE_OUSTRING( "\t$MACRO False" ) );
}

void VbaProject::attachToSheetEvents( const Reference< XEventsSupplier >& rxEventsSupp, const OUString& rCodeName )
{
    if( !rxEventsSupp.is() || (rCodeName.getLength() == 0) || !hasModule( rCodeName ) )
        return;

    // attach macros to simple sheet events directly
    attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnFocus" ),     rCodeName, CREATE_OUSTRING( "Worksheet_Activate" ) );
    attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnUnfocus" ),   rCodeName, CREATE_OUSTRING( "Worksheet_Deactivate" ) );
    attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnCalculate" ), rCodeName, CREATE_OUSTRING( "Worksheet_Calculate" ) );

    /*  Attach macros to complex sheet events. The events pass a cell range or
        a collection of cell ranges depending on the event type and sheet
        selection. The generated proxy macros need to convert these UNO renges
        to VBA compatible ranges.
     */

#define VBA_MACRONAME_RANGECONV "Local_GetVbaRangeFromUnoRange"
#define VBA_MACRONAME_TARGETCONV "Local_GetVbaTargetFromUnoTarget"

    /*  If this variable turns to true, the macros that convert UNO cell ranges
        to VBA Range objects have to be inserted.
     */
    bool bNeedsTargetHelper = false;

    /*  Insert the proxy macros attached to sheet events that notify something
        has changed (changed selection and changed cell contents). These events
        cannot be cancelled. The proxy macro converts the passed UNO cell range
        or collection of cell ranges to a VBA Range object, and calls the VBA
        event handler.
     */
    OUString aChangeProxyArgs = CREATE_OUSTRING( "ByVal unoTarget As Object" );
    OUString aChangeProxyCode = CREATE_OUSTRING(
        "\tDim vbaTarget As Range : Set vbaTarget = " VBA_MACRONAME_TARGETCONV "( unoTarget )\n"
        "\tIf Not vbaTarget Is Nothing Then $MACRO vbaTarget" );
    bNeedsTargetHelper |= attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnChange" ), rCodeName, CREATE_OUSTRING( "Worksheet_Change" ),          aChangeProxyArgs, OUString(), aChangeProxyCode );
    bNeedsTargetHelper |= attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnSelect" ), rCodeName, CREATE_OUSTRING( "Worksheet_SelectionChange" ), aChangeProxyArgs, OUString(), aChangeProxyCode );

    /*  Insert the proxy macros attached to sheet events that notify an ongoing
        mouse click event (double click and right click). These events can be
        cancelled by returning false (in VBA: as a Boolean output parameter, in
        UNO: as return value of the Basic function). The proxy macro converts
        the passed UNO cell range or collection of cell ranges to a VBA Range
        object, calls the VBA event handler, and returns the Boolean value
        provided by the VBA event handler.
     */
    OUString aClickProxyArgs = CREATE_OUSTRING( "ByVal unoTarget As Object" );
    OUString aClickProxyRetT = CREATE_OUSTRING( "Boolean" );
    OUString aClickProxyCode = CREATE_OUSTRING(
        "\tDim Cancel As Boolean : Cancel = False\n"
        "\tDim vbaTarget As Range : Set vbaTarget = " VBA_MACRONAME_TARGETCONV "( unoTarget )\n"
        "\tIf Not vbaTarget Is Nothing Then $MACRO vbaTarget, Cancel\n"
        "\t$PROXY = Cancel" );
    bNeedsTargetHelper |= attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnDoubleClick" ), rCodeName, CREATE_OUSTRING( "Worksheet_BeforeDoubleClick" ), aClickProxyArgs, aClickProxyRetT, aClickProxyCode );
    bNeedsTargetHelper |= attachMacroToEvent( rxEventsSupp, CREATE_OUSTRING( "OnRightClick" ),  rCodeName, CREATE_OUSTRING( "Worksheet_BeforeRightClick" ),  aClickProxyArgs, aClickProxyRetT, aClickProxyCode );

    if( bNeedsTargetHelper )
    {
        /*  Generate a helper function that converts a
            com.sun.star.sheet.SheetCellRange object to a VBA Range object.
         */
        OUString aRangeConvName = CREATE_OUSTRING( VBA_MACRONAME_RANGECONV );
        OUString aRangeConvArgs = CREATE_OUSTRING( "ByVal unoRange As com.sun.star.sheet.SheetCellRange" );
        OUString aRangeConvRetT = CREATE_OUSTRING( "Range" );
        OUString aRangeConvCode = CREATE_OUSTRING(
            "\tDim unoAddress As com.sun.star.table.CellRangeAddress : Set unoAddress = unoRange.RangeAddress\n"
            "\tDim vbaSheet As Worksheet : Set vbaSheet = Application.ThisWorkbook.Sheets( unoAddress.Sheet + 1 )\n"
            "\tSet $MACRO = vbaSheet.Range( vbaSheet.Cells( unoAddress.StartRow + 1, unoAddress.StartColumn + 1 ), vbaSheet.Cells( unoAddress.EndRow + 1, unoAddress.EndColumn + 1 ) )" );
        insertMacro( rCodeName, aRangeConvName, aRangeConvArgs, aRangeConvRetT, aRangeConvCode );

        /*  Generate a helper function that converts a generic range selection
            object (com.sun.star.sheet.SheetCellRange or
            com.sun.star.sheet.SheetCellRanges) to a VBA Range object.
         */
        OUString aTargetConvName = CREATE_OUSTRING( VBA_MACRONAME_TARGETCONV );
        OUString aTargetConvArgs = CREATE_OUSTRING( "ByVal unoTarget As Object" );
        OUString aTargetConvRetT = CREATE_OUSTRING( "Range" );
        OUString aTargetConvCode = CREATE_OUSTRING(
            "\tDim vbaTarget As Range\n"
            "\tIf unoTarget.supportsService( \"com.sun.star.sheet.SheetCellRange\" ) Then\n"
            "\t\tSet vbaTarget = " VBA_MACRONAME_RANGECONV "( unoTarget )\n"
            "\tElseIf unoTarget.supportsService( \"com.sun.star.sheet.SheetCellRanges\" ) Then\n"
            "\t\tDim unoRangeEnum As Object : Set unoRangeEnum = unoTarget.createEnumeration\n"
            "\t\tIf unoRangeEnum.hasMoreElements Then Set vbaTarget = " VBA_MACRONAME_RANGECONV "( unoRangeEnum.nextElement )\n"
            "\t\tWhile unoRangeEnum.hasMoreElements\n"
            "\t\t\tSet vbaTarget = Application.Union( vbaTarget, " VBA_MACRONAME_RANGECONV "( unoRangeEnum.nextElement ) )\n"
            "\t\tWend\n"
            "\tEnd If\n"
            "\tSet $MACRO = vbaTarget" );
        insertMacro( rCodeName, aTargetConvName, aTargetConvArgs, aTargetConvRetT, aTargetConvCode );
    }
#undef VBA_MACRONAME_RANGECONV
#undef VBA_MACRONAME_TARGETCONV
}

// ============================================================================

} // namespace xls
} // namespace oox
