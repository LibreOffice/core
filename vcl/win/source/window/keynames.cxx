/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string.h>
#include <rtl/ustring.hxx>
#include <sal/macros.h>

#include <windows.h>

// Use unique ;) names to avoid clashes with the KEY_* (especially
// KEY_SHIFT) from <rsc/rsc-vcl-shared-types.hxx>

#define PAPUGA_KEY_ESC         0x10000
#define PAPUGA_KEY_BACK        0xE0000
#define PAPUGA_KEY_ENTER       0x1C0000
#define PAPUGA_KEY_SPACEBAR    0x390000
#define PAPUGA_KEY_HOME        0x1470000
#define PAPUGA_KEY_UP          0x1480000
#define PAPUGA_KEY_PAGEUP      0x1490000
#define PAPUGA_KEY_LEFT        0x14B0000
#define PAPUGA_KEY_RIGHT       0x14D0000
#define PAPUGA_KEY_END         0x14F0000
#define PAPUGA_KEY_DOWN        0x1500000
#define PAPUGA_KEY_PAGEDOWN    0x1510000
#define PAPUGA_KEY_INSERT      0x1520000
#define PAPUGA_KEY_DELETE      0x1530000
#define PAPUGA_KEY_CONTROL     0x21D0000
#define PAPUGA_KEY_SHIFT       0x22A0000
#define PAPUGA_KEY_ALT         0x2380000


namespace vcl_sal {

    struct KeysNameReplacement
    {
        LONG            aSymbol;
        const char*     pName;
    };

    struct KeyboardReplacements
    {
        const char*                     pLangName;
        const KeysNameReplacement*      pReplacements;
        int                             nReplacements;
    };

    // ====================================================================
    //
    // CAUTION CAUTION CAUTION
    // Every string value in the replacements tables must be in UTF-8
    // but with the UTF-8 bytes encoded, not as such! Be careful!
    //
    // ====================================================================

    static const struct KeysNameReplacement aImplReplacements_Asturian[] =
    {
        { PAPUGA_KEY_BACK, "Retrocesu" },
        { PAPUGA_KEY_ENTER, "Intro" },
        { PAPUGA_KEY_SPACEBAR, "Espaciu" },
        { PAPUGA_KEY_HOME, "Aniciu" },
        { PAPUGA_KEY_UP, "Arriba" },
        { PAPUGA_KEY_PAGEUP, "Re P\xc3\xa1" "x" },
        { PAPUGA_KEY_LEFT, "Izquierda" },
        { PAPUGA_KEY_RIGHT, "Drecha" },
        { PAPUGA_KEY_END, "Fin" },
        { PAPUGA_KEY_DOWN, "Abaxo" },
        { PAPUGA_KEY_PAGEDOWN, "Av P\xc3\xa1" "x" },
        { PAPUGA_KEY_INSERT, "Ins" },
        { PAPUGA_KEY_DELETE, "Supr" },
        { PAPUGA_KEY_SHIFT, "May\xc3\xba" "s" },
    };

    static const struct KeysNameReplacement aImplReplacements_Catalan[] =
    {
        { PAPUGA_KEY_BACK, "Retroc\xc3\xa9" "s" },
        { PAPUGA_KEY_ENTER, "Retorn" },
        { PAPUGA_KEY_SPACEBAR, "Espai" },
        { PAPUGA_KEY_HOME, "Inici" },
        { PAPUGA_KEY_UP, "Amunt" },
        { PAPUGA_KEY_PAGEUP, "Re P\xc3\xa0" "g" },
        { PAPUGA_KEY_LEFT, "Esquerra" },
        { PAPUGA_KEY_RIGHT, "Dreta" },
        { PAPUGA_KEY_END, "Fi" },
        { PAPUGA_KEY_DOWN, "Avall" },
        { PAPUGA_KEY_PAGEDOWN, "Av P\xc3\xa0" "g" },
        { PAPUGA_KEY_INSERT, "Ins" },
        { PAPUGA_KEY_DELETE, "Supr" },
        { PAPUGA_KEY_SHIFT, "Maj" },
    };

    static const struct KeysNameReplacement aImplReplacements_Estonian[] =
    {
        { PAPUGA_KEY_RIGHT, "Nool paremale" },
        { PAPUGA_KEY_LEFT, "Nool vasakule" },
        { PAPUGA_KEY_UP, "Nool \xc3\xbc" "les" },
        { PAPUGA_KEY_DOWN, "Nool alla" },
        { PAPUGA_KEY_BACK, "Tagasil\xc3\xbc" "ke" },
        { PAPUGA_KEY_ENTER, "Enter" },
        { PAPUGA_KEY_SPACEBAR, "T\xc3\xbc" "hik" },
    };

    static const struct KeysNameReplacement aImplReplacements_Lithuanian[] =
    {
        { PAPUGA_KEY_ESC, "Gr" },
        { PAPUGA_KEY_BACK, "Naikinti" },
        { PAPUGA_KEY_ENTER, "\xc4\xae" "vesti" },
        { PAPUGA_KEY_SPACEBAR, "Tarpas" },
        { PAPUGA_KEY_HOME, "Prad" },
        { PAPUGA_KEY_UP, "Auk\xc5\xa1" "tyn" },
        { PAPUGA_KEY_PAGEUP, "Psl\xe2\x86\x91" },
        { PAPUGA_KEY_LEFT, "Kair\xc4\x97" "n" },
        { PAPUGA_KEY_RIGHT, "De\xc5\xa1" "in\xc4\x97" "n" },
        { PAPUGA_KEY_END, "Pab" },
        { PAPUGA_KEY_DOWN, "\xc5\xbd" "emyn" },
        { PAPUGA_KEY_PAGEDOWN, "Psl\xe2\x86\x93" },
        { PAPUGA_KEY_INSERT, "\xc4\xae" "terpti" },
        { PAPUGA_KEY_DELETE, "\xc5\xa0" "al" },
        { PAPUGA_KEY_CONTROL, "Vald" },
        { PAPUGA_KEY_SHIFT, "Lyg2" },
        { PAPUGA_KEY_ALT, "Alt" },
    };

    static const struct KeysNameReplacement aImplReplacements_Slovenian[] =
    {
        { PAPUGA_KEY_ESC, "Ube\xc5\xbe" "nica" },
        { PAPUGA_KEY_BACK, "Vra\xc4\x8d" "alka" },
        { PAPUGA_KEY_ENTER, "Vna\xc5\xa1" "alka" },
        { PAPUGA_KEY_SPACEBAR, "Preslednica" },
        { PAPUGA_KEY_HOME, "Za\xc4\x8d" "etek" },
        { PAPUGA_KEY_UP, "Navzgor" },
        { PAPUGA_KEY_PAGEUP, "Prej\xc5\xa1" "nja stran" },
        { PAPUGA_KEY_LEFT, "Levo" },
        { PAPUGA_KEY_RIGHT, "Desno" },
        { PAPUGA_KEY_END, "Konec" },
        { PAPUGA_KEY_DOWN, "Navzdol" },
        { PAPUGA_KEY_PAGEDOWN, "Naslednja stran" },
        { PAPUGA_KEY_INSERT, "Vrivalka" },
        { PAPUGA_KEY_DELETE, "Brisalka" },
        { PAPUGA_KEY_CONTROL, "Krmilka" },
        { PAPUGA_KEY_SHIFT, "Dvigalka" },
        { PAPUGA_KEY_ALT, "Izmenjalka" },
    };

    static const struct KeysNameReplacement aImplReplacements_Spanish[] =
    {
        { PAPUGA_KEY_BACK, "Retroceso" },
        { PAPUGA_KEY_ENTER, "Intro" },
        { PAPUGA_KEY_SPACEBAR, "Espacio" },
        { PAPUGA_KEY_HOME, "Inicio" },
        { PAPUGA_KEY_UP, "Arriba" },
        { PAPUGA_KEY_PAGEUP, "Re P\xc3\xa1" "g" },
        { PAPUGA_KEY_LEFT, "Izquierda" },
        { PAPUGA_KEY_RIGHT, "Derecha" },
        { PAPUGA_KEY_END, "Fin" },
        { PAPUGA_KEY_DOWN, "Abajo" },
        { PAPUGA_KEY_PAGEDOWN, "Av P\xc3\xa1" "g" },
        { PAPUGA_KEY_INSERT, "Ins" },
        { PAPUGA_KEY_DELETE, "Supr" },
        { PAPUGA_KEY_SHIFT, "May\xc3\xba" "s" },
    };

    static const struct KeysNameReplacement aImplReplacements_Hungarian[] =
    {
        { PAPUGA_KEY_RIGHT, "Jobbra" },
        { PAPUGA_KEY_LEFT, "Balra" },
        { PAPUGA_KEY_UP, "Fel" },
        { PAPUGA_KEY_DOWN, "Le" },
        { PAPUGA_KEY_ENTER, "Enter" },
        { PAPUGA_KEY_SPACEBAR, "Sz\xc3\xb3" "k\xc3\xb6" "z" },
    };

    static const struct KeyboardReplacements aKeyboards[] =
    {
        { "ast",aImplReplacements_Asturian, SAL_N_ELEMENTS(aImplReplacements_Asturian) },
        { "ca", aImplReplacements_Catalan, SAL_N_ELEMENTS(aImplReplacements_Catalan) },
        { "et", aImplReplacements_Estonian, SAL_N_ELEMENTS(aImplReplacements_Estonian) },
        { "hu", aImplReplacements_Hungarian, SAL_N_ELEMENTS(aImplReplacements_Hungarian) },
        { "lt", aImplReplacements_Lithuanian, SAL_N_ELEMENTS(aImplReplacements_Lithuanian) },
        { "sl", aImplReplacements_Slovenian, SAL_N_ELEMENTS(aImplReplacements_Slovenian) },
        { "es", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
    };

    // translate keycodes, used within the displayed menu shortcuts
    OUString getKeysReplacementName( OUString pLang, LONG nSymbol )
    {
        for( unsigned int n = 0; n < SAL_N_ELEMENTS(aKeyboards); n++ )
        {
            if( pLang.equalsAscii( aKeyboards[n].pLangName ) )
            {
                const struct KeysNameReplacement* pRepl = aKeyboards[n].pReplacements;
                for( int m = aKeyboards[n].nReplacements ; m ; )
                {
                    if( nSymbol == pRepl[--m].aSymbol )
                        return OUString( pRepl[m].pName, strlen(pRepl[m].pName), RTL_TEXTENCODING_UTF8 );
                }
            }
        }

        return OUString();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
