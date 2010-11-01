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
    { 0, 0 }
};

rtl::OUString
SwVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    for( const WordDialogTable* pTable = aWordDialogTable; pTable != NULL; pTable++ )
    {
        if( nIndex == pTable->wdDialog )
        {
            return rtl::OUString::createFromAscii( pTable->ooDialog );
        }
    }
    return rtl::OUString();
}

rtl::OUString&
SwVbaDialog::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaDialog") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaDialog::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Dialog" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
