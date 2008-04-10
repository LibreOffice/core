/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scabstdlg.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_sc.hxx"

#include "scabstdlg.hxx"

#include <osl/module.hxx>
#include <rtl/ustrbuf.hxx>

using ::rtl::OUStringBuffer;

typedef ScAbstractDialogFactory* (__LOADONCALLAPI *ScFuncPtrCreateDialogFactory)();

extern "C" { static void SAL_CALL thisModule() {} }

ScAbstractDialogFactory* ScAbstractDialogFactory::Create()
{
    ScFuncPtrCreateDialogFactory fp = 0;
    static ::osl::Module aDialogLibrary;

    OUStringBuffer aStrBuf;
    aStrBuf.appendAscii( SVLIBRARY("scui") );

    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, aStrBuf.makeStringAndClear() ) )
        fp = ( ScAbstractDialogFactory* (__LOADONCALLAPI*)() )
            aDialogLibrary.getFunctionSymbol( ::rtl::OUString::createFromAscii("CreateDialogFactory") );
    if ( fp )
        return fp();
    return 0;
}
