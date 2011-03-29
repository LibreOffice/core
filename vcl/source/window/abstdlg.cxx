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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/abstdlg.hxx>

#include <osl/module.hxx>
#include <tools/string.hxx>

typedef VclAbstractDialogFactory* (__LOADONCALLAPI *FuncPtrCreateDialogFactory)();

extern "C" { static void SAL_CALL thisModule() {} }

VclAbstractDialogFactory* VclAbstractDialogFactory::Create()
{
    FuncPtrCreateDialogFactory fp = 0;
    static ::osl::Module aDialogLibrary;
    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, String( RTL_CONSTASCII_USTRINGPARAM( CUI_DLL_NAME ) ) ) )
        fp = ( VclAbstractDialogFactory* (__LOADONCALLAPI*)() )
            aDialogLibrary.getFunctionSymbol( ::rtl::OUString::createFromAscii("CreateDialogFactory") );
    if ( fp )
        return fp();
    return 0;
}

VclAbstractDialog::~VclAbstractDialog()
{
}

// virtual
VclAbstractDialog2::~VclAbstractDialog2()
{
}

VclAbstractDialogFactory::~VclAbstractDialogFactory()
{
}
