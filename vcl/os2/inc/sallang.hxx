/*************************************************************************
 *
 *  $RCSfile: sallang.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SALLANG_HXX
#define _SALLANG_HXX

#ifndef _TOOLS_LANG_HXX
#include <tools/lang.hxx>
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

const char** ImplGetLangTab( LanguageType eLang );

#endif // _SALLANG_HXX
