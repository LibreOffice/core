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

static const OUString aStringList[]=
{
    OUString( ".uno:Open" ),
    OUString( ".uno:FormatCellDialog" ),
    OUString( ".uno:InsertCell" ),
    OUString( ".uno:Print" ),
    OUString( ".uno:PasteSpecial" ),
    OUString( ".uno:ToolProtectionDocument" ),
    OUString( ".uno:ColumnWidth" ),
    OUString( ".uno:DefineName" ),
    OUString( ".uno:ConfigureDialog" ),
    OUString( ".uno:HyperlinkDialog" ),
    OUString( ".uno:InsertGraphic" ),
    OUString( ".uno:InsertObject" ),
    OUString( ".uno:PageFormatDialog" ),
    OUString( ".uno:DataSort" ),
    OUString( ".uno:RowHeight" ),
    OUString( ".uno:AutoCorrectDlg" ),
    OUString( ".uno:ConditionalFormatDialog" ),
    OUString( ".uno:DataConsolidate" ),
    OUString( ".uno:CreateNames" ),
    OUString( ".uno:FillSeries" ),
    OUString( ".uno:Validation"),
    OUString( ".uno:DefineLabelRange" ),
    OUString( ".uno:DataFilterAutoFilter" ),
    OUString( ".uno:DataFilterSpecialFilter" ),
    OUString( ".uno:AutoFormat" )
};

const sal_Int32 nDialogSize = sizeof (aStringList) / sizeof (aStringList[0]);

OUString
ScVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    if( nIndex < nDialogSize )
        return aStringList[ nIndex ];
    return OUString();
}

OUString
ScVbaDialog::getServiceImplName()
{
    return OUString("ScVbaDialog");
}

uno::Sequence< OUString >
ScVbaDialog::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.Dialog" );
    }
    return aServiceNames;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
