/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sallang.cxx,v $
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

#include <sallang.hxx>

// =======================================================================

// -----------------------------------------------------------------------
// Danish

static const wchar_t* aImplLangDanishTab[LSTR_COUNT] =
{
    L"Skift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Op",                                                   // LSTR_KEY_UP
    L"Ned",                                                  // LSTR_KEY_DOWN
    L"Venstre",                                              // LSTR_KEY_LEFT
    L"Hõjre",                                                // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Enter",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Tilbage",                                              // LSTR_KEY_BACKSPACE
    L"Mellemrum",                                            // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Dutch (Netherland/Belgian)

static const wchar_t* aImplLangDutchTab[LSTR_COUNT] =
{
    L"Shift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Boven",                                                // LSTR_KEY_UP
    L"Onder",                                                // LSTR_KEY_DOWN
    L"Links",                                                // LSTR_KEY_LEFT
    L"Links",                                                // LSTR_KEY_RIGHT
    L"Pos1",                                                 // LSTR_KEY_HOME
    L"Einde",                                                // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Return",                                               // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Backspace",                                            // LSTR_KEY_BACKSPACE
    L"Spatiebalk",                                           // LSTR_KEY_SPACE
    L"Ins",                                                  // LSTR_KEY_INSERT
    L"Verwijderen",                                          // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// English (US/UK/AUS/CAN/NZ/EIRE/SAFRICA/JAMAICA/CARRIBEAN)

static const wchar_t* aImplLangEnglishTab[LSTR_COUNT] =
{
    L"Shift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Up",                                                   // LSTR_KEY_UP
    L"Down",                                                 // LSTR_KEY_DOWN
    L"Left",                                                 // LSTR_KEY_LEFT
    L"Right",                                                // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Enter",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Backspace",                                            // LSTR_KEY_BACKSPACE
    L"Space",                                                // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Del",                                                  // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Finnish

static const wchar_t* aImplLangFinnishTab[LSTR_COUNT] =
{
    L"VaihtonÑppain",                                        // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"YlÑ",                                                  // LSTR_KEY_UP
    L"Ala",                                                  // LSTR_KEY_DOWN
    L"Vasen",                                                // LSTR_KEY_LEFT
    L"Oikea",                                                // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Enter",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Sarkain",                                              // LSTR_KEY_TAB
    L"Askelpalautin",                                        // LSTR_KEY_BACKSPACE
    L"VÑlinÑppÑin",                                          // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// French (French/Belgian/Canadian/Swiss/Luxenbourg)

static const wchar_t* aImplLangFrenchTab[LSTR_COUNT] =
{
    L"Maj",                                                  // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Haut",                                                 // LSTR_KEY_UP
    L"Bas",                                                  // LSTR_KEY_DOWN
    L"Gauche",                                               // LSTR_KEY_LEFT
    L"Droite",                                               // LSTR_KEY_RIGHT
    L"Origine",                                              // LSTR_KEY_HOME
    L"Fin",                                                  // LSTR_KEY_END
    L"Pg. PrÇc",                                             // LSTR_KEY_PAGEUP
    L"Pg. Suiv",                                             // LSTR_KEY_PAGEDOWN
    L"EntrÇe",                                               // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Ret. Arr",                                             // LSTR_KEY_BACKSPACE
    L"Espace",                                               // LSTR_KEY_SPACE
    L"InsÇrer",                                              // LSTR_KEY_INSERT
    L"Suppr",                                                // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// German (German/Swiss/Austrian/Luxembourg/Liechtenstein)

static const wchar_t* aImplLangGermanTab[LSTR_COUNT] =
{
    L"Umschalt",                                             // LSTR_KEY_SHIFT
    L"Strg",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Nach-Oben",                                            // LSTR_KEY_UP
    L"Nach-Unten",                                           // LSTR_KEY_DOWN
    L"Nach-Links",                                           // LSTR_KEY_LEFT
    L"Nach-Rechts",                                          // LSTR_KEY_RIGHT
    L"Pos1",                                                 // LSTR_KEY_HOME
    L"Ende",                                                 // LSTR_KEY_END
    L"Bild-Nach-Oben",                                       // LSTR_KEY_PAGEUP
    L"Bild-Nach-Unten",                                      // LSTR_KEY_PAGEDOWN
    L"Eingabe",                                              // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"RÅck",                                                 // LSTR_KEY_BACKSPACE
    L"Leer",                                                 // LSTR_KEY_SPACE
    L"Einfg",                                                // LSTR_KEY_INSERT
    L"Entf",                                                 // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Italian (Italian/Swiss)

static const wchar_t* aImplLangItalianTab[LSTR_COUNT] =
{
    L"Maiusc",                                               // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Sposta verso l'alto",                                  // LSTR_KEY_UP
    L"Sposta verso il basso",                                // LSTR_KEY_DOWN
    L"A sinistra",                                           // LSTR_KEY_LEFT
    L"A destra",                                             // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"Fine",                                                 // LSTR_KEY_END
    L"PgSu",                                                 // LSTR_KEY_PAGEUP
    L"PgGió",                                                // LSTR_KEY_PAGEDOWN
    L"Invio",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Backspace",                                            // LSTR_KEY_BACKSPACE
    L"Spaziatrice",                                          // LSTR_KEY_SPACE
    L"Ins",                                                  // LSTR_KEY_INSERT
    L"Canc",                                                 // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Norwegian (Bokmal)

static const wchar_t* aImplLangNorwegianTab[LSTR_COUNT] =
{
    L"Skift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Opp",                                                  // LSTR_KEY_UP
    L"Ned",                                                  // LSTR_KEY_DOWN
    L"Venstre",                                              // LSTR_KEY_LEFT
    L"Hõyre",                                                // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Enter",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Tilbake",                                              // LSTR_KEY_BACKSPACE
    L"Mellomrom",                                            // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Portuguse (Portuguse/Brazilian)

static const wchar_t* aImplLangPortugueseTab[LSTR_COUNT] =
{
    L"Shift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Acima",                                                // LSTR_KEY_UP
    L"Abaixo",                                               // LSTR_KEY_DOWN
    L"Esquerda",                                             // LSTR_KEY_LEFT
    L"Direita",                                              // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Enter",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Backspace",                                            // LSTR_KEY_BACKSPACE
    L"Space",                                                // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Delete",                                               // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Spanish (Spanish/Mexican/Modern/Guatemala/Costarica/Panama/Dominican/
//          Venezuela/Colombia/Peru/Argentina/Ecuador/Chile/Uruguay/
//          Paraguay/Bolivia)

static const wchar_t* aImplLangSpanishTab[LSTR_COUNT] =
{
    L"Mayós",                                                // LSTR_KEY_SHIFT
    L"Control",                                              // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Hacia arriba",                                         // LSTR_KEY_UP
    L"Hacia abajo",                                          // LSTR_KEY_DOWN
    L"Hacia la izquierda",                                   // LSTR_KEY_LEFT
    L"Hacia la derecha",                                     // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"Fin",                                                  // LSTR_KEY_END
    L"ReP†g",                                                // LSTR_KEY_PAGEUP
    L"AvP†g",                                                // LSTR_KEY_PAGEDOWN
    L"Entrada",                                              // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Ret",                                                  // LSTR_KEY_BACKSPACE
    L"Espacio",                                              // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Supr",                                                 // LSTR_KEY_DELETE
};

// -----------------------------------------------------------------------
// Swedish

static const wchar_t* aImplLangSwedishTab[LSTR_COUNT] =
{
    L"Skift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Up",                                                   // LSTR_KEY_UP
    L"Ned",                                                  // LSTR_KEY_DOWN
    L"VÑnster",                                              // LSTR_KEY_LEFT
    L"Hîger",                                                // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Retur",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Backsteg",                                             // LSTR_KEY_BACKSPACE
    L"Blank",                                                // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Delete",                                               // LSTR_KEY_DELETE
};

// =======================================================================

const sal_Unicode** ImplGetLangTab( LanguageType eLang )
{
    // Sprachtabelle ermitteln
    const wchar_t** pLangTab;
    //switch ( International::GetNeutralLanguage( eLang ) )
    switch ( eLang )
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

    return (const sal_Unicode**)pLangTab;
}
