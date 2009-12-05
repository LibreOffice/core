/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbadialog.cxx,v $
 * $Revision: 1.4 $
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

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static const rtl::OUString aStringList[]=
{
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Open" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormatCellDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertCell" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Print" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PasteSpecial" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ToolProtectionDocument" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ColumnWidth" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DefineName" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConfigureDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:HyperlinkDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertGraphic" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertObject" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PageFormatDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataSort" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:RowHeight" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoCorrectDlg" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConditionalFormatDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataConsolidate" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CreateNames" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillSeries" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Validation") ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DefineLabelRange" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataFilterAutoFilter" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataFilterSpecialFilter" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoFormat" ) )
};

const sal_Int32 nDialogSize = sizeof( aStringList ) / sizeof( aStringList[ 0 ] );

rtl::OUString
ScVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    if( nIndex < nDialogSize )
        return aStringList[ nIndex ];
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
