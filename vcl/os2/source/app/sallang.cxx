/*************************************************************************
 *
 *  $RCSfile: sallang.cxx,v $
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

#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif

#ifndef _SALLANG_HXX
#include <sallang.hxx>
#endif

// =======================================================================

// -----------------------------------------------------------------------
// Danish

static const char* aImplLangDanishTab[LSTR_COUNT] =
{
    "Skift",                                                // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Op",                                                   // LSTR_KEY_UP
    "Ned",                                                  // LSTR_KEY_DOWN
    "Venstre",                                              // LSTR_KEY_LEFT
    "Hõjre",                                                // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "End",                                                  // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Enter",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Tilbage",                                              // LSTR_KEY_BACKSPACE
    "Mellemrum",                                            // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Dutch (Netherland/Belgian)

static const char* aImplLangDutchTab[LSTR_COUNT] =
{
    "Shift",                                                // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Boven",                                                // LSTR_KEY_UP
    "Onder",                                                // LSTR_KEY_DOWN
    "Links",                                                // LSTR_KEY_LEFT
    "Links",                                                // LSTR_KEY_RIGHT
    "Pos1",                                                 // LSTR_KEY_HOME
    "Einde",                                                // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Return",                                               // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Backspace",                                            // LSTR_KEY_BACKSPACE
    "Spatiebalk",                                           // LSTR_KEY_SPACE
    "Ins",                                                  // LSTR_KEY_INSERT
    "Verwijderen",                                          // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// English (US/UK/AUS/CAN/NZ/EIRE/SAFRICA/JAMAICA/CARRIBEAN)

static const char* aImplLangEnglishTab[LSTR_COUNT] =
{
    "Shift",                                                // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Up",                                                   // LSTR_KEY_UP
    "Down",                                                 // LSTR_KEY_DOWN
    "Left",                                                 // LSTR_KEY_LEFT
    "Right",                                                // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "End",                                                  // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Enter",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Backspace",                                            // LSTR_KEY_BACKSPACE
    "Space",                                                // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Del",                                                  // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Finnish

static const char* aImplLangFinnishTab[LSTR_COUNT] =
{
    "VaihtonÑppain",                                        // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "YlÑ",                                                  // LSTR_KEY_UP
    "Ala",                                                  // LSTR_KEY_DOWN
    "Vasen",                                                // LSTR_KEY_LEFT
    "Oikea",                                                // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "End",                                                  // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Enter",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Sarkain",                                              // LSTR_KEY_TAB
    "Askelpalautin",                                        // LSTR_KEY_BACKSPACE
    "VÑlinÑppÑin",                                          // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// French (French/Belgian/Canadian/Swiss/Luxenbourg)

static const char* aImplLangFrenchTab[LSTR_COUNT] =
{
    "Maj",                                                  // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Haut",                                                 // LSTR_KEY_UP
    "Bas",                                                  // LSTR_KEY_DOWN
    "Gauche",                                               // LSTR_KEY_LEFT
    "Droite",                                               // LSTR_KEY_RIGHT
    "Origine",                                              // LSTR_KEY_HOME
    "Fin",                                                  // LSTR_KEY_END
    "Pg. PrÇc",                                             // LSTR_KEY_PAGEUP
    "Pg. Suiv",                                             // LSTR_KEY_PAGEDOWN
    "EntrÇe",                                               // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Ret. Arr",                                             // LSTR_KEY_BACKSPACE
    "Espace",                                               // LSTR_KEY_SPACE
    "InsÇrer",                                              // LSTR_KEY_INSERT
    "Suppr",                                                // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// German (German/Swiss/Austrian/Luxembourg/Liechtenstein)

static const char* aImplLangGermanTab[LSTR_COUNT] =
{
    "Umschalt",                                             // LSTR_KEY_SHIFT
    "Strg",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Nach-Oben",                                            // LSTR_KEY_UP
    "Nach-Unten",                                           // LSTR_KEY_DOWN
    "Nach-Links",                                           // LSTR_KEY_LEFT
    "Nach-Rechts",                                          // LSTR_KEY_RIGHT
    "Pos1",                                                 // LSTR_KEY_HOME
    "Ende",                                                 // LSTR_KEY_END
    "Bild-Nach-Oben",                                       // LSTR_KEY_PAGEUP
    "Bild-Nach-Unten",                                      // LSTR_KEY_PAGEDOWN
    "Eingabe",                                              // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "RÅck",                                                 // LSTR_KEY_BACKSPACE
    "Leer",                                                 // LSTR_KEY_SPACE
    "Einfg",                                                // LSTR_KEY_INSERT
    "Entf",                                                 // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Italian (Italian/Swiss)

static const char* aImplLangItalianTab[LSTR_COUNT] =
{
    "Maiusc",                                               // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Sposta verso l'alto",                                  // LSTR_KEY_UP
    "Sposta verso il basso",                                // LSTR_KEY_DOWN
    "A sinistra",                                           // LSTR_KEY_LEFT
    "A destra",                                             // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "Fine",                                                 // LSTR_KEY_END
    "PgSu",                                                 // LSTR_KEY_PAGEUP
    "PgGió",                                                // LSTR_KEY_PAGEDOWN
    "Invio",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Backspace",                                            // LSTR_KEY_BACKSPACE
    "Spaziatrice",                                          // LSTR_KEY_SPACE
    "Ins",                                                  // LSTR_KEY_INSERT
    "Canc",                                                 // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Norwegian (Bokmal)

static const char* aImplLangNorwegianTab[LSTR_COUNT] =
{
    "Skift",                                                // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Opp",                                                  // LSTR_KEY_UP
    "Ned",                                                  // LSTR_KEY_DOWN
    "Venstre",                                              // LSTR_KEY_LEFT
    "Hõyre",                                                // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "End",                                                  // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Enter",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Tilbake",                                              // LSTR_KEY_BACKSPACE
    "Mellomrom",                                            // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Portuguse (Portuguse/Brazilian)

static const char* aImplLangPortugueseTab[LSTR_COUNT] =
{
    "Shift",                                                // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Acima",                                                // LSTR_KEY_UP
    "Abaixo",                                               // LSTR_KEY_DOWN
    "Esquerda",                                             // LSTR_KEY_LEFT
    "Direita",                                              // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "End",                                                  // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Enter",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Backspace",                                            // LSTR_KEY_BACKSPACE
    "Space",                                                // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Spanish (Spanish/Mexican/Modern/Guatemala/Costarica/Panama/Dominican/
//          Venezuela/Colombia/Peru/Argentina/Ecuador/Chile/Uruguay/
//          Paraguay/Bolivia)

static const char* aImplLangSpanishTab[LSTR_COUNT] =
{
    "Mayós",                                                // LSTR_KEY_SHIFT
    "Control",                                              // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Hacia arriba",                                         // LSTR_KEY_UP
    "Hacia abajo",                                          // LSTR_KEY_DOWN
    "Hacia la izquierda",                                   // LSTR_KEY_LEFT
    "Hacia la derecha",                                     // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "Fin",                                                  // LSTR_KEY_END
    "ReP†g",                                                // LSTR_KEY_PAGEUP
    "AvP†g",                                                // LSTR_KEY_PAGEDOWN
    "Entrada",                                              // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Ret",                                                  // LSTR_KEY_BACKSPACE
    "Espacio",                                              // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Supr",                                                 // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Swedish

static const char* aImplLangSwedishTab[LSTR_COUNT] =
{
    "Skift",                                                // LSTR_KEY_SHIFT
    "Ctrl",                                                 // LSTR_KEY_CTRL
    "Alt",                                                  // LSTR_KEY_ALT
    "Up",                                                   // LSTR_KEY_UP
    "Ned",                                                  // LSTR_KEY_DOWN
    "VÑnster",                                              // LSTR_KEY_LEFT
    "Hîger",                                                // LSTR_KEY_RIGHT
    "Home",                                                 // LSTR_KEY_HOME
    "End",                                                  // LSTR_KEY_END
    "PageUp",                                               // LSTR_KEY_PAGEUP
    "PageDown",                                             // LSTR_KEY_PAGEDOWN
    "Retur",                                                // LSTR_KEY_RETURN
    "Esc",                                                  // LSTR_KEY_ESC
    "Tab",                                                  // LSTR_KEY_TAB
    "Backsteg",                                             // LSTR_KEY_BACKSPACE
    "Blank",                                                // LSTR_KEY_SPACE
    "Insert",                                               // LSTR_KEY_INSERT
    "Delete",                                               // LSTR_KEY_DELETE
};

// =======================================================================

const char** ImplGetLangTab( LanguageType eLang )
{
    // Sprachtabelle ermitteln
    const char** pLangTab;
    switch ( International::GetNeutralLanguage( eLang ) )
    {
        case LANGUAGE_DANISH:
            pLangTab = aImplLangDanishTab;
            break;

        case LANGUAGE_DUTCH:
        case LANGUAGE_DUTCH_BELGIAN:
            pLangTab = aImplLangDutchTab;
            break;

        case LANGUAGE_FINNISH:
            pLangTab = aImplLangFinnishTab;
            break;

        case LANGUAGE_FRENCH:
            pLangTab = aImplLangFrenchTab;
            break;

        case LANGUAGE_GERMAN:
            pLangTab = aImplLangGermanTab;
            break;

        case LANGUAGE_ITALIAN:
            pLangTab = aImplLangItalianTab;
            break;

        case LANGUAGE_NORWEGIAN:
        case LANGUAGE_NORWEGIAN_BOKMAL:
            pLangTab = aImplLangNorwegianTab;
            break;

        case LANGUAGE_PORTUGUESE:
        case LANGUAGE_PORTUGUESE_BRAZILIAN:
            pLangTab = aImplLangPortugueseTab;
            break;

        case LANGUAGE_SPANISH:
            pLangTab = aImplLangSpanishTab;
            break;

        case LANGUAGE_SWEDISH:
            pLangTab = aImplLangSwedishTab;
            break;

        default:
            pLangTab = aImplLangEnglishTab;
            break;
    }

    return pLangTab;
}
