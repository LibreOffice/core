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

#ifndef _SBXBASE_HXX
#define _SBXBASE_HXX

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _LIST_HXX
#include "tools/list.hxx"
#endif

#ifndef _SVARRAY_HXX
#include "bf_svtools/svarray.hxx"
#endif

#ifndef _SBXDEF_HXX
#include "sbxdef.hxx"
#endif

namespace binfilter {

class SbxFactory;
class SbxVariable;
class SbxBasicFormater;

SV_DECL_PTRARR_DEL(SbxFacs,SbxFactory*,5,5)
DECLARE_LIST(SbxVarList_Impl, SbxVariable*)

// AppData-Struktur fuer SBX:
struct SbxAppData
{
    SbxError			eSbxError;	// Fehlercode
    SbxFacs 			aFacs;		// Factories
    SbxVarList_Impl		aVars;		// fuer Dump
                                            // Format()-Befehl
    LanguageType        eBasicFormaterLangType;
        // Bem.: es ist sinnvoll diese Klasse 'global' zu speichern, da
        // 		 einige Resourcen (Strings) in der Klasse gehalten werden.

    SbxAppData() : eSbxError( SbxERR_OK ), aFacs() {}
};

SbxAppData* GetSbxData_Impl();

}

#endif
