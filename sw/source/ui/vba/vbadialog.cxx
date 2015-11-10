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
#include <ooo/vba/word/WdWordDialog.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

struct WordDialogTable
{
    sal_Int32 wdDialog;
    const sal_Char* ooDialog;
};

static const WordDialogTable aWordDialogTable[] =
{
    { word::WdWordDialog::wdDialogFileNew, ".uno:NewDoc" },
    { word::WdWordDialog::wdDialogFileOpen, ".uno:Open" },
    { word::WdWordDialog::wdDialogFilePrint, ".uno:Print" },
    { word::WdWordDialog::wdDialogFileSaveAs, ".uno:SaveAs" },
    { 0, nullptr }
};

OUString
SwVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(aWordDialogTable); ++i)
    {
        const WordDialogTable& rTable = aWordDialogTable[i];
        if( nIndex == rTable.wdDialog )
        {
            return OUString::createFromAscii( rTable.ooDialog );
        }
    }
    return OUString();
}

OUString
SwVbaDialog::getServiceImplName()
{
    return OUString("SwVbaDialog");
}

uno::Sequence< OUString >
SwVbaDialog::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Dialog";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
