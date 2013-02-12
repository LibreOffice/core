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

#define KEY_ESC         0x10000
#define KEY_BACK        0xE0000
#define KEY_ENTER       0x1C0000
#define KEY_SPACEBAR    0x390000
#define KEY_HOME        0x1470000
#define KEY_UP          0x1480000
#define KEY_PAGEUP      0x1490000
#define KEY_LEFT        0x14B0000
#define KEY_RIGHT       0x14D0000
#define KEY_END         0x14F0000
#define KEY_DOWN        0x1500000
#define KEY_PAGEDOWN    0x1510000
#define KEY_INSERT      0x1520000
#define KEY_DELETE      0x1530000
#define KEY_CONTROL     0x21D0000
#define KEY_SHIFT       0x22A0000
#define KEY_ALT         0x2380000


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
    // every string value in the replacements tables must be in UTF8
    // be careful with your editor !
    //
    // ====================================================================

    static const struct KeysNameReplacement aImplReplacements_Asturian[] =
    {
        { KEY_BACK, "Retrocesu" },
        { KEY_ENTER, "Intro" },
        { KEY_SPACEBAR, "Espaciu" },
        { KEY_HOME, "Aniciu" },
        { KEY_UP, "Arriba" },
        { KEY_PAGEUP, "Re Páx" },
        { KEY_LEFT, "Izquierda" },
        { KEY_RIGHT, "Drecha" },
        { KEY_END, "Fin" },
        { KEY_DOWN, "Abaxo" },
        { KEY_PAGEDOWN, "Av Páx" },
        { KEY_INSERT, "Ins" },
        { KEY_DELETE, "Supr" },
        { KEY_SHIFT, "Mayús" },
    };

    static const struct KeysNameReplacement aImplReplacements_Catalan[] =
    {
        { KEY_BACK, "Retrocés" },
        { KEY_ENTER, "Retorn" },
        { KEY_SPACEBAR, "Espai" },
        { KEY_HOME, "Inici" },
        { KEY_UP, "Amunt" },
        { KEY_PAGEUP, "Re Pàg" },
        { KEY_LEFT, "Esquerra" },
        { KEY_RIGHT, "Dreta" },
        { KEY_END, "Fi" },
        { KEY_DOWN, "Avall" },
        { KEY_PAGEDOWN, "Av Pàg" },
        { KEY_INSERT, "Ins" },
        { KEY_DELETE, "Supr" },
        { KEY_SHIFT, "Maj" },
    };

    static const struct KeysNameReplacement aImplReplacements_Estonian[] =
    {
        { KEY_RIGHT, "Nool paremale" },
        { KEY_LEFT, "Nool vasakule" },
        { KEY_UP, "Nool üles" },
        { KEY_DOWN, "Nool alla" },
        { KEY_BACK, "Tagasilüke" },
        { KEY_ENTER, "Enter" },
        { KEY_SPACEBAR, "Tühik" },
    };

    static const struct KeysNameReplacement aImplReplacements_Lithuanian[] =
    {
        { KEY_ESC, "Gr" },
        { KEY_BACK, "Naikinti" },
        { KEY_ENTER, "Įvesti" },
        { KEY_SPACEBAR, "Tarpas" },
        { KEY_HOME, "Prad" },
        { KEY_UP, "Aukštyn" },
        { KEY_PAGEUP, "Psl↑" },
        { KEY_LEFT, "Kairėn" },
        { KEY_RIGHT, "Dešinėn" },
        { KEY_END, "Pab" },
        { KEY_DOWN, "Žemyn" },
        { KEY_PAGEDOWN, "Psl↓" },
        { KEY_INSERT, "Įterpti" },
        { KEY_DELETE, "Šal" },
        { KEY_CONTROL, "Vald" },
        { KEY_SHIFT, "Lyg2" },
        { KEY_ALT, "Alt" },
    };

    static const struct KeysNameReplacement aImplReplacements_Slovenian[] =
    {
        { KEY_ESC, "Ubežnica" },
        { KEY_BACK, "Vračalka" },
        { KEY_ENTER, "Vnašalka" },
        { KEY_SPACEBAR, "Preslednica" },
        { KEY_HOME, "Začetek" },
        { KEY_UP, "Navzgor" },
        { KEY_PAGEUP, "Prejšnja stran" },
        { KEY_LEFT, "Levo" },
        { KEY_RIGHT, "Desno" },
        { KEY_END, "Konec" },
        { KEY_DOWN, "Navzdol" },
        { KEY_PAGEDOWN, "Naslednja stran" },
        { KEY_INSERT, "Vrivalka" },
        { KEY_DELETE, "Brisalka" },
        { KEY_CONTROL, "Krmilka" },
        { KEY_SHIFT, "Dvigalka" },
        { KEY_ALT, "Izmenjalka" },
    };

    static const struct KeysNameReplacement aImplReplacements_Spanish[] =
    {
        { KEY_BACK, "Retroceso" },
        { KEY_ENTER, "Intro" },
        { KEY_SPACEBAR, "Espacio" },
        { KEY_HOME, "Inicio" },
        { KEY_UP, "Arriba" },
        { KEY_PAGEUP, "Re Pág" },
        { KEY_LEFT, "Izquierda" },
        { KEY_RIGHT, "Derecha" },
        { KEY_END, "Fin" },
        { KEY_DOWN, "Abajo" },
        { KEY_PAGEDOWN, "Av Pág" },
        { KEY_INSERT, "Ins" },
        { KEY_DELETE, "Supr" },
        { KEY_SHIFT, "Mayús" },
    };

    static const struct KeyboardReplacements aKeyboards[] =
    {
        { "ast",aImplReplacements_Asturian, SAL_N_ELEMENTS(aImplReplacements_Asturian) },
        { "ca", aImplReplacements_Catalan, SAL_N_ELEMENTS(aImplReplacements_Catalan) },
        { "et", aImplReplacements_Estonian, SAL_N_ELEMENTS(aImplReplacements_Estonian) },
        { "lt", aImplReplacements_Lithuanian, SAL_N_ELEMENTS(aImplReplacements_Lithuanian) },
        { "sl", aImplReplacements_Slovenian, SAL_N_ELEMENTS(aImplReplacements_Slovenian) },
        { "es", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
    };

    // translate keycodes, used within the displayed menu shortcuts
    rtl::OUString getKeysReplacementName( rtl::OUString pLang, LONG nSymbol )
    {
        for( unsigned int n = 0; n < SAL_N_ELEMENTS(aKeyboards); n++ )
        {
            if( pLang.equalsAscii( aKeyboards[n].pLangName ) )
            {
                const struct KeysNameReplacement* pRepl = aKeyboards[n].pReplacements;
                for( int m = aKeyboards[n].nReplacements ; m ; )
                {
                    if( nSymbol == pRepl[--m].aSymbol )
                        return rtl::OUString( pRepl[m].pName, strlen(pRepl[m].pName), RTL_TEXTENCODING_UTF8 );
                }
            }
        }

        return rtl::OUString();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
