/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sallang.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SALLANG_HXX
#define _SALLANG_HXX

//#ifndef _TOOLS_LANG_HXX
//#include <tools/lang.hxx>
//#endif
#include <i18npool/mslangid.hxx>

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
