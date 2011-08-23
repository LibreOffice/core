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

#ifndef _SFXINIDEF_HXX
#define _SFXINIDEF_HXX

#ifndef _INIMGR_HXX
#include <iniman.hxx>
#endif

namespace binfilter
{

class SfxIniEntry;
class SfxIniDefaulterList;
class SfxIniDefaultManager;
}

#if _SOLAR__PRIVATE
#include <bf_svtools/svarray.hxx>

namespace binfilter
{
class SfxIniDefaulter;
SV_DECL_PTRARR( SfxIniDefaulterList, SfxIniDefaulter*, 0, 2 );
}
#endif

namespace binfilter
{

//========================================================================

class SfxIniDefaulter

/*	[Beschreibung]

    Instanzen dieser Klasse k"onnen (f"ur die Laufzeit eines Moduls, z.B.
    einer DLL-Nutzung oder einer Applikation) angelegt werden, um Defaults
    f"ur eine Anzahl von ini-Eintr"agen zu liefern. Diese Defaults werden
    erfragt, wenn ein <SfxIniManager> einen Eintrag nicht in den ini-Files
    finden kann.

    Defaults verschiedener <SfxIniDefaulter>-Instanzen d"urfen sich nicht
    unterscheiden, da die Reihenfolge der Abarbeitung nicht definiert ist.

    Die Instanzen brauchen nicht zerst"ort zu werden, au\ser wenn der
    dazugeh"orige Code entladen wird (load-on-demand DLLs). Sonst geschieht
    dies automatisch beim Zerst"oren des <SfxIniDefaultManager>.
*/

{
    SfxIniDefaultManager*	_pManager;

public:
                            SfxIniDefaulter( SfxIniDefaultManager *pManager );
                            ~SfxIniDefaulter();

    virtual BOOL			QueryDefault( String &aValue,
                                          const SfxIniEntry &rEntry ) = 0;
};

//========================================================================

class SfxIniDefaultManager

/*	[Beschreibung]

    Genau eine Instanz dieser Klasse mu\s in jeder Application-Subklasse,
    die diesen Mechanismus nutzen m"ochte, in deren Ctor angelegt werden
    und sollte in deren Dtor zerst"ort werden.
*/

{
    SfxIniDefaulterList*	_pList;

public:
                            SfxIniDefaultManager();
                            ~SfxIniDefaultManager();

#if _SOLAR__PRIVATE
    void					Insert( SfxIniDefaulter *pDefaulter )
                            { _pList->C40_INSERT( SfxIniDefaulter, pDefaulter, _pList->Count() ); }
    void					Remove( SfxIniDefaulter *pDefaulter )
                            { _pList->Remove( _pList->C40_GETPOS( SfxIniDefaulter, pDefaulter ) ); }
    BOOL					QueryDefault( String &aValue,
                                          const SfxIniEntry &rEntry );
#endif
};

//========================================================================

class SfxIniEntry

/*	[Beschreibung]

    Instanzen dieser Klasse beschreiben einen Eintrag eines Ini-Files,
    um mit der Klasse <SfxIniDefaulter> erfragt werden zu k"onnen, falls
    der <SfxIniManager> einen Eintrag nicht in den Ini-Files auffindet.
*/

{
friend class SfxIniManager;

    const String&   		_aGroup;
    const String&			_aKey;
    SfxIniGroup 			_eGroup;
    SfxIniKey				_eKey;
    USHORT					_nIndex;

private:
                            SfxIniEntry( const String&  aGroup,
                                         const String&	aKey,
                                         SfxIniGroup	eGroup,
                                         SfxIniKey		eKey,
                                         USHORT		 	nIndex );
                            SfxIniEntry( const SfxIniEntry & ); // n.i.
    SfxIniEntry&			operator=( const SfxIniEntry & ); // n.i.

public:
    const String&			GetGroupName() const { return _aGroup; }
    SfxIniGroup 			GetGroup() const { return _eGroup; }
    const String&			GetKeyName() const { return _aKey; }
    SfxIniKey				GetKey() const { return _eKey; }
    USHORT 					GetIndex() const { return _nIndex; }
};

}

#endif

