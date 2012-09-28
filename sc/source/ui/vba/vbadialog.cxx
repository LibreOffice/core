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

static const rtl::OUString aStringList[]=
{
    rtl::OUString( ".uno:Open" ),
    rtl::OUString( ".uno:FormatCellDialog" ),
    rtl::OUString( ".uno:InsertCell" ),
    rtl::OUString( ".uno:Print" ),
    rtl::OUString( ".uno:PasteSpecial" ),
    rtl::OUString( ".uno:ToolProtectionDocument" ),
    rtl::OUString( ".uno:ColumnWidth" ),
    rtl::OUString( ".uno:DefineName" ),
    rtl::OUString( ".uno:ConfigureDialog" ),
    rtl::OUString( ".uno:HyperlinkDialog" ),
    rtl::OUString( ".uno:InsertGraphic" ),
    rtl::OUString( ".uno:InsertObject" ),
    rtl::OUString( ".uno:PageFormatDialog" ),
    rtl::OUString( ".uno:DataSort" ),
    rtl::OUString( ".uno:RowHeight" ),
    rtl::OUString( ".uno:AutoCorrectDlg" ),
    rtl::OUString( ".uno:ConditionalFormatDialog" ),
    rtl::OUString( ".uno:DataConsolidate" ),
    rtl::OUString( ".uno:CreateNames" ),
    rtl::OUString( ".uno:FillSeries" ),
    rtl::OUString( ".uno:Validation"),
    rtl::OUString( ".uno:DefineLabelRange" ),
    rtl::OUString( ".uno:DataFilterAutoFilter" ),
    rtl::OUString( ".uno:DataFilterSpecialFilter" ),
    rtl::OUString( ".uno:AutoFormat" )
};

const sal_Int32 nDialogSize = sizeof (aStringList) / sizeof (aStringList[0]);

rtl::OUString
ScVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    if( nIndex < nDialogSize )
        return aStringList[ nIndex ];
    return rtl::OUString();
}

rtl::OUString
ScVbaDialog::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaDialog"));
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
