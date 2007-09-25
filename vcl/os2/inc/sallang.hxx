/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sallang.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 10:07:16 $
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

#ifndef _SALLANG_HXX
#define _SALLANG_HXX

//#ifndef _TOOLS_LANG_HXX
//#include <tools/lang.hxx>
//#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif

// --------------------
// - Language Strings -
// --------------------

// --- Key-Namen ---
#define LSTR_KEY_SHIFT                       0
#define LSTR_KEY_CTRL                        1
#define LSTR_KEY_ALT                         2
#define LSTR_KEY_UP                          3
#define LSTR_KEY_DOWN                        4
#define LSTR_KEY_LEFT                        5
#define LSTR_KEY_RIGHT                       6
#define LSTR_KEY_HOME                        7
#define LSTR_KEY_END                         8
#define LSTR_KEY_PAGEUP                      9
#define LSTR_KEY_PAGEDOWN                   10
#define LSTR_KEY_RETURN                     11
#define LSTR_KEY_ESC                        12
#define LSTR_KEY_TAB                        13
#define LSTR_KEY_BACKSPACE                  14
#define LSTR_KEY_SPACE                      15
#define LSTR_KEY_INSERT                     16
#define LSTR_KEY_DELETE                     17

// --- Anzahl der Texte ---

#define LSTR_COUNT                          18

// --------------------------------------------
// - Methoden zum Abfragen der Sprach-Strings -
// --------------------------------------------

const sal_Unicode** ImplGetLangTab( LanguageType eLang );

#endif // _SALLANG_HXX
