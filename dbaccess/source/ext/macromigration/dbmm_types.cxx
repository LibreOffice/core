/*************************************************************************
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
#include "precompiled_dbaccess.hxx"

#include "dbmm_types.hxx"

#include "dbmm_global.hrc"
#include "dbmm_module.hxx"

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= helper
    //====================================================================
    //--------------------------------------------------------------------
    String getScriptTypeDisplayName( const ScriptType _eType )
    {
        sal_uInt16 nResId( 0 );

        switch ( _eType )
        {
        case eBasic:        nResId = STR_OOO_BASIC;     break;
        case eBeanShell:    nResId = STR_BEAN_SHELL;    break;
        case eJavaScript:   nResId = STR_JAVA_SCRIPT;   break;
        case ePython:       nResId = STR_PYTHON;        break;
        case eJava:         nResId = STR_JAVA;          break;
        case eDialog:       nResId = STR_DIALOG;        break;
        }
        OSL_ENSURE( nResId != 0, "getScriptTypeDisplayName: illegal script type!" );
        return nResId ? String( MacroMigrationResId( nResId ) ) : String();
    }

//........................................................................
} // namespace dbmm
//........................................................................
