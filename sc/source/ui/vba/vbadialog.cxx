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

static const OUStringLiteral aStringList[]=
{
    OUStringLiteral( ".uno:Open" ),
    OUStringLiteral( ".uno:FormatCellDialog" ),
    OUStringLiteral( ".uno:InsertCell" ),
    OUStringLiteral( ".uno:Print" ),
    OUStringLiteral( ".uno:PasteSpecial" ),
    OUStringLiteral( ".uno:ToolProtectionDocument" ),
    OUStringLiteral( ".uno:ColumnWidth" ),
    OUStringLiteral( ".uno:DefineName" ),
    OUStringLiteral( ".uno:ConfigureDialog" ),
    OUStringLiteral( ".uno:HyperlinkDialog" ),
    OUStringLiteral( ".uno:InsertGraphic" ),
    OUStringLiteral( ".uno:InsertObject" ),
    OUStringLiteral( ".uno:PageFormatDialog" ),
    OUStringLiteral( ".uno:DataSort" ),
    OUStringLiteral( ".uno:RowHeight" ),
    OUStringLiteral( ".uno:AutoCorrectDlg" ),
    OUStringLiteral( ".uno:ConditionalFormatDialog" ),
    OUStringLiteral( ".uno:DataConsolidate" ),
    OUStringLiteral( ".uno:CreateNames" ),
    OUStringLiteral( ".uno:FillSeries" ),
    OUStringLiteral( ".uno:Validation"),
    OUStringLiteral( ".uno:DefineLabelRange" ),
    OUStringLiteral( ".uno:DataFilterAutoFilter" ),
    OUStringLiteral( ".uno:DataFilterSpecialFilter" ),
    OUStringLiteral( ".uno:AutoFormat" )
};

const sal_Int32 nDialogSize = SAL_N_ELEMENTS(aStringList);

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
        aServiceNames[ 0 ] = "ooo.vba.excel.Dialog";
    }
    return aServiceNames;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
