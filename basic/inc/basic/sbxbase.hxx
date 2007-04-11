/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:53:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include "svtools/svarray.hxx"
#endif

#ifndef _SBXDEF_HXX
#include <basic/sbxdef.hxx>
#endif

class SbxFactory;
class SbxVariable;
class SbxBasicFormater;

SV_DECL_PTRARR_DEL(SbxFacs,SbxFactory*,5,5)
DECLARE_LIST(SbxVarList_Impl, SbxVariable*)

// AppData-Struktur fuer SBX:
struct SbxAppData
{
    SbxError            eSbxError;  // Fehlercode
    SbxFacs             aFacs;      // Factories
    SbxVarList_Impl     aVars;      // fuer Dump
    SbxBasicFormater    *pBasicFormater;    // Zeiger auf Hilfsklasse f"ur den
                                            // Format()-Befehl
    LanguageType        eBasicFormaterLangType;
        // Bem.: es ist sinnvoll diese Klasse 'global' zu speichern, da
        //       einige Resourcen (Strings) in der Klasse gehalten werden.

    SbxAppData() : eSbxError( SbxERR_OK ), aFacs(), pBasicFormater( NULL ) {}
    ~SbxAppData();
};

SbxAppData* GetSbxData_Impl();

#endif
