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
#include "vbadialog.hxx"

#include <sal/macros.h>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

//solve the problem that "Application.Dialogs.Item(***).Show" and "Application.Dialogs.Count" cannot get the correct result
struct DialogMatch
{
    sal_Int32       nVbaDlgIndex;
    rtl::OUString   aOODlgName;
};

static const DialogMatch aDialogMatchList[] =
{
    { 1,    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Open" ) ) },                      // xlDialogOpen -> .uno:Open
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormatCellDialog" ) ) },          // ??? -> .uno:FormatCellDialog
    { 55,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertCell" ) ) },                // xlDialogInsert -> .uno:InsertCell
    { 8,    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Print" ) ) },                     // xlDialogPrint -> .uno:Print
    { 9,    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PrinterSetup" ) ) },              // xlDialogPrinterSetup -> .uno:PrinterSetup
    { 53,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PasteSpecial" ) ) },              // xlDialogPasteSpecial -> .uno:PasteSpecial
    { 28,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ToolProtectionDocument" ) ) },    // xlDialogProtectDocument -> uno:ToolProtectionDocument
    { 47,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ColumnWidth" ) ) },               // xlDialogColumnWidth -> .uno:ColumnWidth
    { 61,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DefineName" ) ) },                // xlDialogDefineName -> .uno:DefineName
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConfigureDialog" ) ) },           // ??? -> .uno:ConfigureDialog
    { 596,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:HyperlinkDialog" ) ) },           // xlDialogInsertHyperlink -> .uno:HyperlinkDialog
    { 342,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertGraphic" ) ) },             // xlDialogInsertPicture -> .uno:InsertGraphic
    { 259,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertObject" ) ) },              // xlDialogInsertObject -> .uno:InsertObject
    { 7,    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PageFormatDialog" ) ) },          // xlDialogPageSetup -> .uno:PageFormatDialog
    { 39,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataSort" ) ) },                  // xlDialogSort -> .uno:DataSort
    { 127,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:RowHeight" ) ) },                 // xlDialogRowHeight -> .uno:RowHeight
    { 485,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoCorrectDlg" ) ) },            // xlDialogAutoCorrect -> .uno:AutoCorrectDlg
    { 583,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConditionalFormatDialog" ) ) },   // xlDialogCondiationalFormatting -> .uno:ConditionalFormatDialog
    { 191,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataConsolidate" ) ) },           // xlDialogConsolidate -> .uno:DataConsolidate
    { 62,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CreateNames" ) ) },               // xlDialogCreateNames -> .uno:CreateNames
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillSeries" ) ) },                // ??? -> .uno:FillSeries
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Validation" ) ) },                // ??? -> .uno:Validation"
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DefineLabelRange" ) ) },          // ??? -> .uno:DefineLabelRange
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataFilterAutoFilter" ) ) },      // ??? -> .uno:DataFilterAutoFilter
    { -1,   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataFilterSpecialFilter" ) ) },   // ??? -> .uno:DataFilterSpecialFilter
    { 269,  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoFormat" ) ) }                 // xlDialogFormatAuto -> .uno:AutoFormat
};

const sal_Int32 nDialogSize = SAL_N_ELEMENTS( aDialogMatchList ) ;

rtl::OUString
ScVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    for (int i = 0; i < nDialogSize; i++)
    {
        if ( aDialogMatchList[i].nVbaDlgIndex == nIndex )
        {
            return aDialogMatchList[i].aOODlgName;
        }
    }

    return rtl::OUString();
}

rtl::OUString&
ScVbaDialog::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaDialog") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaDialog::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Dialog" ) );
    }
    return aServiceNames;
}

sal_Int32 ScVbaDialog::GetSupportedDialogCount()
{
    return nDialogSize;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
