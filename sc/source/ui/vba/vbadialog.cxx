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
    return rtl::OUString("ScVbaDialog");
}

uno::Sequence< rtl::OUString >
ScVbaDialog::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.Dialog" );
    }
    return aServiceNames;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
