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

#include <sal/config.h>

#include <string_view>

#include "vbadialog.hxx"

#include <sal/macros.h>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const std::u16string_view aStringList[]=
{
    u".uno:Open",
    u".uno:FormatCellDialog",
    u".uno:InsertCell",
    u".uno:Print",
    u".uno:PasteSpecial",
    u".uno:ToolProtectionDocument",
    u".uno:ColumnWidth",
    u".uno:DefineName",
    u".uno:ConfigureDialog",
    u".uno:HyperlinkDialog",
    u".uno:InsertGraphic",
    u".uno:InsertObject",
    u".uno:PageFormatDialog",
    u".uno:DataSort",
    u".uno:RowHeight",
    u".uno:AutoCorrectDlg",
    u".uno:ConditionalFormatDialog",
    u".uno:DataConsolidate",
    u".uno:CreateNames",
    u".uno:FillSeries",
    u".uno:Validation",
    u".uno:DefineLabelRange",
    u".uno:DataFilterAutoFilter",
    u".uno:DataFilterSpecialFilter",
    u".uno:AutoFormat"
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
    return "ScVbaDialog";
}

uno::Sequence< OUString >
ScVbaDialog::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Dialog"
    };
    return aServiceNames;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
