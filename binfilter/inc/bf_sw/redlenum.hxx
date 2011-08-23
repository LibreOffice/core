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
#ifndef _REDLENUM_HXX
#define _REDLENUM_HXX
namespace binfilter {


// Redline Typen
enum SwRedlineType
{
    // die RedlineTypen gehen von 0 bis 127
    REDLINE_INSERT, 			// Inhalt wurde eingefuegt
    REDLINE_DELETE, 			// Inhalt wurde geloescht
    REDLINE_FORMAT,				// Attributierung wurde angewendet
    REDLINE_TABLE,				// TabellenStruktur wurde veraendert
    REDLINE_FMTCOLL,			// FormatVorlage wurde veraendert (Autoformat!)

    // ab 128 koennen Flags hineinverodert werden
    REDLINE_NO_FLAG_MASK	= 0x007f,
    REDLINE_FLAG_MASK		= 0xff80,
    REDLINE_FORM_AUTOFMT	= 0x0080	// kann als Flag im RedlineType stehen
};


// Redline Modus vom Doc
enum SwRedlineMode
{
    REDLINE_NONE,						// kein RedlineMode
    REDLINE_ON		= 0x01,				// RedlineMode on
    REDLINE_IGNORE	= 0x02,				// auf Redlines nicht reagieren

    REDLINE_SHOW_INSERT = 0x10,		// alle Einfuegungen anzeigen
    REDLINE_SHOW_DELETE = 0x20,		// alle Loeschungen anzeigen

    REDLINE_SHOW_MASK = REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE,

    // fuer die interne Verwaltung:
    //	die originalen Redlines inclusive des Contents entfernen
    //	(ClipBoard/Textbausteine)
    REDLINE_DELETE_REDLINES = 0x100,
    //	beim Loeschen innerhalb ein RedlineObjectes, waehrend des Appends,
    // das DeleteRedline ignorieren
    REDLINE_IGNOREDELETE_REDLINES = 0x200,
    // don't combine any readlines. This flags is may only used in the Undo.
    REDLINE_DONTCOMBINE_REDLINES = 0x400
};

inline int IsShowChanges( const USHORT eM )
{
    return (REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) ==
                    (eM & REDLINE_SHOW_MASK);
}
inline int IsHideChanges( const USHORT eM )
{
    return REDLINE_SHOW_INSERT == (eM & REDLINE_SHOW_MASK);
}
inline int IsShowOriginal( const USHORT eM )
{
    return REDLINE_SHOW_DELETE == (eM & REDLINE_SHOW_MASK);
}
inline int IsRedlineOn( const USHORT eM )
{
    return REDLINE_ON == (eM & (REDLINE_ON | REDLINE_IGNORE ));
}



} //namespace binfilter
#endif
