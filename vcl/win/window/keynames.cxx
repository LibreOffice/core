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

#include <sal/config.h>

#include <span>

#include <rtl/ustring.hxx>

#include <win/salframe.h>

// Use unique ;) names to avoid clashes with the KEY_* (especially
// KEY_SHIFT) from <vcl/vclenum.hxx>

constexpr UINT PAPUGA_KEY_ESC      = 0x10000;
constexpr UINT PAPUGA_KEY_BACK     = 0xE0000;
constexpr UINT PAPUGA_KEY_ENTER    = 0x1C0000;
constexpr UINT PAPUGA_KEY_SPACEBAR = 0x390000;
constexpr UINT PAPUGA_KEY_HOME     = 0x1470000;
constexpr UINT PAPUGA_KEY_UP       = 0x1480000;
constexpr UINT PAPUGA_KEY_PAGEUP   = 0x1490000;
constexpr UINT PAPUGA_KEY_LEFT     = 0x14B0000;
constexpr UINT PAPUGA_KEY_RIGHT    = 0x14D0000;
constexpr UINT PAPUGA_KEY_END      = 0x14F0000;
constexpr UINT PAPUGA_KEY_DOWN     = 0x1500000;
constexpr UINT PAPUGA_KEY_PAGEDOWN = 0x1510000;
constexpr UINT PAPUGA_KEY_INSERT   = 0x1520000;
constexpr UINT PAPUGA_KEY_DELETE   = 0x1530000;
constexpr UINT PAPUGA_KEY_CONTROL  = 0x21D0000;
constexpr UINT PAPUGA_KEY_SHIFT    = 0x22A0000;
constexpr UINT PAPUGA_KEY_ALT      = 0x2380000;

namespace vcl_sal {

    namespace {

    struct KeysNameReplacement
    {
        UINT aSymbol;
        OUString pName;
    };

    struct KeyboardReplacements
    {
        std::u16string_view pLangName;
        std::span<const KeysNameReplacement> pReplacements;
    };

    }

    constexpr KeysNameReplacement aImplReplacements_Asturian[] =
    {
        { PAPUGA_KEY_BACK, u"Retrocesu"_ustr },
        { PAPUGA_KEY_ENTER, u"Intro"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Espaciu"_ustr },
        { PAPUGA_KEY_HOME, u"Aniciu"_ustr },
        { PAPUGA_KEY_UP, u"Arriba"_ustr },
        { PAPUGA_KEY_PAGEUP, u"Re Páx"_ustr },
        { PAPUGA_KEY_LEFT, u"Izquierda"_ustr },
        { PAPUGA_KEY_RIGHT, u"Drecha"_ustr },
        { PAPUGA_KEY_END, u"Fin"_ustr },
        { PAPUGA_KEY_DOWN, u"Abaxo"_ustr },
        { PAPUGA_KEY_PAGEDOWN, u"Av Páx"_ustr },
        { PAPUGA_KEY_INSERT, u"Ins"_ustr },
        { PAPUGA_KEY_DELETE, u"Supr"_ustr },
        { PAPUGA_KEY_SHIFT, u"Mayús"_ustr },
    };

    constexpr KeysNameReplacement aImplReplacements_Catalan[] =
    {
        { PAPUGA_KEY_BACK, u"Retrocés"_ustr },
        { PAPUGA_KEY_ENTER, u"Retorn"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Espai"_ustr },
        { PAPUGA_KEY_HOME, u"Inici"_ustr },
        { PAPUGA_KEY_UP, u"Amunt"_ustr },
        { PAPUGA_KEY_PAGEUP, u"Re Pàg"_ustr },
        { PAPUGA_KEY_LEFT, u"Esquerra"_ustr },
        { PAPUGA_KEY_RIGHT, u"Dreta"_ustr },
        { PAPUGA_KEY_END, u"Fi"_ustr },
        { PAPUGA_KEY_DOWN, u"Avall"_ustr },
        { PAPUGA_KEY_PAGEDOWN, u"Av Pàg"_ustr },
        { PAPUGA_KEY_INSERT, u"Ins"_ustr },
        { PAPUGA_KEY_DELETE, u"Supr"_ustr },
        { PAPUGA_KEY_SHIFT, u"Maj"_ustr },
    };

    constexpr KeysNameReplacement aImplReplacements_Estonian[] =
    {
        { PAPUGA_KEY_RIGHT, u"Nool paremale"_ustr },
        { PAPUGA_KEY_LEFT, u"Nool vasakule"_ustr },
        { PAPUGA_KEY_UP, u"Nool üles"_ustr },
        { PAPUGA_KEY_DOWN, u"Nool alla"_ustr },
        { PAPUGA_KEY_BACK, u"Tagasilüke"_ustr },
        { PAPUGA_KEY_ENTER, u"Enter"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Tühik"_ustr },
    };

    constexpr KeysNameReplacement aImplReplacements_Lithuanian[] =
    {
        { PAPUGA_KEY_ESC, u"Gr"_ustr },
        { PAPUGA_KEY_BACK, u"Naikinti"_ustr },
        { PAPUGA_KEY_ENTER, u"Įvesti"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Tarpas"_ustr },
        { PAPUGA_KEY_HOME, u"Prad"_ustr },
        { PAPUGA_KEY_UP, u"Aukštyn"_ustr },
        { PAPUGA_KEY_PAGEUP, u"Psl↑"_ustr },
        { PAPUGA_KEY_LEFT, u"Kairėn"_ustr },
        { PAPUGA_KEY_RIGHT, u"Dešinėn"_ustr },
        { PAPUGA_KEY_END, u"Pab"_ustr },
        { PAPUGA_KEY_DOWN, u"Žemyn"_ustr },
        { PAPUGA_KEY_PAGEDOWN, u"Psl↓"_ustr },
        { PAPUGA_KEY_INSERT, u"Įterpti"_ustr },
        { PAPUGA_KEY_DELETE, u"Šal"_ustr },
        { PAPUGA_KEY_CONTROL, u"Vald"_ustr },
        { PAPUGA_KEY_SHIFT, u"Lyg2"_ustr },
        { PAPUGA_KEY_ALT, u"Alt"_ustr },
    };

    constexpr KeysNameReplacement aImplReplacements_Slovenian[] =
    {
        { PAPUGA_KEY_ESC, u"Ubežnica"_ustr },
        { PAPUGA_KEY_BACK, u"Vračalka"_ustr },
        { PAPUGA_KEY_ENTER, u"Vnašalka"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Preslednica"_ustr },
        { PAPUGA_KEY_HOME, u"Začetek"_ustr },
        { PAPUGA_KEY_UP, u"Navzgor"_ustr },
        { PAPUGA_KEY_PAGEUP, u"Prejšnja stran"_ustr },
        { PAPUGA_KEY_LEFT, u"Levo"_ustr },
        { PAPUGA_KEY_RIGHT, u"Desno"_ustr },
        { PAPUGA_KEY_END, u"Konec"_ustr },
        { PAPUGA_KEY_DOWN, u"Navzdol"_ustr },
        { PAPUGA_KEY_PAGEDOWN, u"Naslednja stran"_ustr },
        { PAPUGA_KEY_INSERT, u"Vrivalka"_ustr },
        { PAPUGA_KEY_DELETE, u"Brisalka"_ustr },
        { PAPUGA_KEY_CONTROL, u"Krmilka"_ustr },
        { PAPUGA_KEY_SHIFT, u"Dvigalka"_ustr },
        { PAPUGA_KEY_ALT, u"Izmenjalka"_ustr },
    };

    constexpr KeysNameReplacement aImplReplacements_Spanish[] =
    {
        { PAPUGA_KEY_BACK, u"Retroceso"_ustr },
        { PAPUGA_KEY_ENTER, u"Intro"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Espacio"_ustr },
        { PAPUGA_KEY_HOME, u"Inicio"_ustr },
        { PAPUGA_KEY_UP, u"Arriba"_ustr },
        { PAPUGA_KEY_PAGEUP, u"Re Pág"_ustr },
        { PAPUGA_KEY_LEFT, u"Izquierda"_ustr },
        { PAPUGA_KEY_RIGHT, u"Derecha"_ustr },
        { PAPUGA_KEY_END, u"Fin"_ustr },
        { PAPUGA_KEY_DOWN, u"Abajo"_ustr },
        { PAPUGA_KEY_PAGEDOWN, u"Av Pág"_ustr },
        { PAPUGA_KEY_INSERT, u"Ins"_ustr },
        { PAPUGA_KEY_DELETE, u"Supr"_ustr },
        { PAPUGA_KEY_SHIFT, u"Mayús"_ustr },
    };

    constexpr KeysNameReplacement aImplReplacements_Hungarian[] =
    {
        { PAPUGA_KEY_RIGHT, u"Jobbra"_ustr },
        { PAPUGA_KEY_LEFT, u"Balra"_ustr },
        { PAPUGA_KEY_UP, u"Fel"_ustr },
        { PAPUGA_KEY_DOWN, u"Le"_ustr },
        { PAPUGA_KEY_ENTER, u"Enter"_ustr },
        { PAPUGA_KEY_SPACEBAR, u"Szóköz"_ustr },
    };

    constexpr KeyboardReplacements aKeyboards[] =
    {
        { u"ast", aImplReplacements_Asturian   },
        { u"ca",  aImplReplacements_Catalan    },
        { u"et",  aImplReplacements_Estonian   },
        { u"hu",  aImplReplacements_Hungarian  },
        { u"lt",  aImplReplacements_Lithuanian },
        { u"sl",  aImplReplacements_Slovenian  },
        { u"es",  aImplReplacements_Spanish    },
    };

    // translate keycodes, used within the displayed menu shortcuts
    OUString getKeysReplacementName(std::u16string_view pLang, UINT nSymbol)
    {
        for( const auto& rKeyboard : aKeyboards )
        {
            if (pLang == rKeyboard.pLangName)
            {
                for (const auto& rRepl : rKeyboard.pReplacements)
                {
                    if (nSymbol == rRepl.aSymbol)
                        return rRepl.pName;
                }
            }
        }

        return OUString();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
