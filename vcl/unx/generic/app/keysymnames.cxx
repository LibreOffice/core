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

#include <unx/saldisp.hxx>
#include <X11/keysym.h>
#include <sal/macros.h>

#if !defined (SunXK_Undo)
#define SunXK_Undo      0x0000FF65  // XK_Undo
#define SunXK_Again     0x0000FF66  // XK_Redo
#define SunXK_Find      0x0000FF68  // XK_Find
#define SunXK_Stop      0x0000FF69  // XK_Cancel
#define SunXK_Props     0x1005FF70
#define SunXK_Front     0x1005FF71
#define SunXK_Copy      0x1005FF72
#define SunXK_Open      0x1005FF73
#define SunXK_Paste     0x1005FF74
#define SunXK_Cut       0x1005FF75
#endif

#include <string.h>
#include <rtl/ustring.hxx>

namespace vcl_sal {

    struct KeysymNameReplacement
    {
        KeySym          aSymbol;
        const char*     pName;
    };

    struct KeyboardReplacements
    {
        const char*                     pLangName;
        const KeysymNameReplacement*    pReplacements;
        int                             nReplacements;
    };

    // ====================================================================
    //
    // CAUTION CAUTION CAUTION
    // every string value in the replacements tables must be in UTF8
    // be careful with your editor !
    //
    // ====================================================================

    static const struct KeysymNameReplacement aImplReplacements_English[] =
    {
        { XK_Control_L, "Ctrl" },
        { XK_Control_R, "Ctrl" },
        { XK_Escape, "Esc" },
        { XK_space, "Space" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Turkish[] =
    {
        { XK_Control_L, "Ctrl" },
        { XK_Control_R, "Ctrl" },
        { XK_Right, "Sa\304\237" },
        { XK_Left, "Sol" },
        { XK_Up, "Yukar\304\261" },
        { XK_Down, "A\305\237a\304\237\304\261" },
        { XK_space, "Bo\305\237luk" }
    };

    static const struct KeysymNameReplacement aImplReplacements_Russian[] =
    {
        { XK_Right, "\320\222\320\277\321\200\320\260\320\262\320\276" },
        { XK_Left, "\320\222\320\273\320\265\320\262\320\276" },
        { XK_Up, "\320\222\320\262\320\265\321\200\321\205" },
        { XK_Down, "\320\222\320\275\320\270\320\267" },
        { XK_space, "\320\237\321\200\320\276\320\261\320\265\320\273" }
    };

    static const struct KeysymNameReplacement aImplReplacements_German[] =
    {
        { XK_Control_L, "Strg" },
        { XK_Control_R, "Strg" },
        { XK_Shift_L, "Umschalt" },
        { XK_Shift_R, "Umschalt" },
        { XK_Alt_L, "Alt" },
        { XK_Alt_R, "Alt Gr" },
        { XK_Page_Up, "Bild auf" },
        { XK_Page_Down, "Bild ab" },
        { XK_End, "Ende" },
        { XK_Home, "Pos 1" },
        { XK_Insert, "Einfg" },
        { XK_Delete, "Entf" },
        { XK_Escape, "Esc" },
        { XK_Right, "Rechts" },
        { XK_Left, "Links" },
        { XK_Up, "Oben" },
        { XK_Down, "Unten" },
        { XK_BackSpace, "R\303\274ckschritt" },
        { XK_Return, "Eingabe" },
        { XK_slash, "Schr\303\244gstrich" },
        { XK_space, "Leertaste" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Wiederholen" },
        { SunXK_Props, "Eigenschaften" },
        { SunXK_Undo,  "Zur\303\274cknehmen" },
        { SunXK_Front, "Vordergrund" },
        { SunXK_Copy,  "Kopieren" },
        { SunXK_Open,  "\303\226ffnen" },
        { SunXK_Paste, "Einsetzen" },
        { SunXK_Find,  "Suchen" },
        { SunXK_Cut,   "Ausschneiden" },
    };

    static const struct KeysymNameReplacement aImplReplacements_French[] =
    {
        { XK_Shift_L, "Maj" },
        { XK_Shift_R, "Maj" },
        { XK_Page_Up, "Pg. Pr\303\251c" },
        { XK_Page_Down, "Pg. Suiv" },
        { XK_End, "Fin" },
        { XK_Home, "Origine" },
        { XK_Insert, "Ins\303\251rer" },
        { XK_Delete, "Suppr" },
        { XK_Escape, "Esc" },
        { XK_Right, "Droite" },
        { XK_Left, "Gauche" },
        { XK_Up, "Haut" },
        { XK_Down, "Bas" },
        { XK_BackSpace, "Ret. Arr" },
        { XK_Return, "Retour" },
        { XK_KP_Enter, "Entr\303\251e" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Encore" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Annuler" },
        { SunXK_Front, "Devant" },
        { SunXK_Copy,  "Copy" },
        { SunXK_Open,  "Ouvrir" },
        { SunXK_Paste, "Coller" },
        { SunXK_Find,  "Cher." },
        { SunXK_Cut,   "Couper" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Italian[] =
    {
        { XK_Shift_L, "Maiusc" },
        { XK_Shift_R, "Maiusc" },
        { XK_Page_Up, "PgSu" },
        { XK_Page_Down, "PgGiu" },
        { XK_End, "Fine" },
        { XK_Insert, "Ins" },
        { XK_Delete, "Canc" },
        { XK_Escape, "Esc" },
        { XK_Right, "A destra" },
        { XK_Left, "A sinistra" },
        { XK_Up, "Sposta verso l'alto" },
        { XK_Down, "Sposta verso il basso" },
        { XK_BackSpace, "Backspace" },
        { XK_Return, "Invio" },
        { XK_space, "Spazio" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Ancora" },
        { SunXK_Props, "Propriet\303\240" },
        { SunXK_Undo,  "Annulla" },
        { SunXK_Front, "Davanti" },
        { SunXK_Copy,  "Copia" },
        { SunXK_Open,  "Apri" },
        { SunXK_Paste, "Incolla" },
        { SunXK_Find,  "Trova" },
        { SunXK_Cut,   "Taglia" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Dutch[] =
    {
        { XK_Page_Up, "PageUp" },
        { XK_Page_Down, "PageDown" },
        { XK_Escape, "Esc" },
        { XK_Right, "Rechts" },
        { XK_Left, "Links" },
        { XK_Up, "Boven" },
        { XK_Down, "Onder" },
        { XK_BackSpace, "Backspace" },
        { XK_Return, "Return" },
        { XK_space, "Spatiebalk" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Again" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Undo" },
        { SunXK_Front, "Front" },
        { SunXK_Copy,  "Copy" },
        { SunXK_Open,  "Open" },
        { SunXK_Paste, "Paste" },
        { SunXK_Find,  "Find" },
        { SunXK_Cut,   "Cut" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Norwegian[] =
    {
        { XK_Shift_L, "Skift" },
        { XK_Shift_R, "Skift" },
        { XK_Page_Up, "PageUp" },
        { XK_Page_Down, "PageDown" },
        { XK_Escape, "Esc" },
        { XK_Right, "H\303\270yre" },
        { XK_Left, "Venstre" },
        { XK_Up, "Opp" },
        { XK_Down, "Ned" },
        { XK_BackSpace, "Tilbake" },
        { XK_Return, "Enter" },
        { SunXK_Stop,  "Avbryt" },
        { SunXK_Again, "Gjenta" },
        { SunXK_Props, "Egenskaper" },
        { SunXK_Undo,  "Angre" },
        { SunXK_Front, "Front" },
        { SunXK_Copy,  "Kopi" },
        { SunXK_Open,  "\303\205pne" },
        { SunXK_Paste, "Lim" },
        { SunXK_Find,  "S\303\270k" },
        { SunXK_Cut,   "Klipp" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Swedish[] =
    {
        { XK_Shift_L, "Skift" },
        { XK_Shift_R, "Skift" },
        { XK_Page_Up, "PageUp" },
        { XK_Page_Down, "PageDown" },
        { XK_Escape, "Esc" },
        { XK_Right, "H\303\266ger" },
        { XK_Left, "V\303\244nster" },
        { XK_Up, "Up" },
        { XK_Down, "Ned" },
        { XK_BackSpace, "Backsteg" },
        { XK_Return, "Retur" },
        { XK_space, "Blank" },
        { SunXK_Stop,  "Avbryt" },
        { SunXK_Again, "Upprepa" },
        { SunXK_Props, "Egenskaper" },
        { SunXK_Undo,  "\303\205ngra" },
        { SunXK_Front, "Fram" },
        { SunXK_Copy,  "Kopiera" },
        { SunXK_Open,  "\303\226ppna" },
        { SunXK_Paste, "Klistra in" },
        { SunXK_Find,  "S\303\266k" },
        { SunXK_Cut,   "Klipp ut" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Portuguese[] =
    {
        { XK_Page_Up, "PageUp" },
        { XK_Page_Down, "PageDown" },
        { XK_Escape, "Esc" },
        { XK_Right, "Direita" },
        { XK_Left, "Esquerda" },
        { XK_Up, "Acima" },
        { XK_Down, "Abaixo" },
        { XK_BackSpace, "Backspace" },
        { XK_Return, "Enter" },
        { XK_slash, "Barra" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Again" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Undo" },
        { SunXK_Front, "Front" },
        { SunXK_Copy,  "Copy" },
        { SunXK_Open,  "Open" },
        { SunXK_Paste, "Paste" },
        { SunXK_Find,  "Find" },
        { SunXK_Cut,   "Cut" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Slovenian[] =
    {
        { XK_Control_L, "Krmilka" },
        { XK_Control_R, "Krmilka" },
        { XK_Shift_L, "Dvigalka" },
        { XK_Shift_R, "Dvigalka" },
        { XK_Alt_L, "Izmenjalka" },
        { XK_Alt_R, "Desna izmenjalka" },
        { XK_Page_Up, "Prej\305\241nja stranf" },
        { XK_Page_Down, "Naslednja stran" },
        { XK_End, "Konec" },
        { XK_Home, "Za\304\215etek" },
        { XK_Insert, "Vstavljalka" },
        { XK_Delete, "Brisalka" },
        { XK_Escape, "Ube\305\276nica" },
        { XK_Right, "Desno" },
        { XK_Left, "Levo" },
        { XK_Up, "Navzgor" },
        { XK_Down, "Navzdol" },
        { XK_BackSpace, "Vra\304\215alka" },
        { XK_Return, "Vna\305\241alka" },
        { XK_slash, "Po\305\241evnica" },
        { XK_space, "Preslednica" },
        { SunXK_Stop,  "Ustavi" },
        { SunXK_Again, "Ponovi" },
        { SunXK_Props, "Lastnosti" },
        { SunXK_Undo,  "Razveljavi" },
        { SunXK_Front, "Ospredje" },
        { SunXK_Copy,  "Kopiraj" },
        { SunXK_Open,  "Odpri" },
        { SunXK_Paste, "Prilepi" },
        { SunXK_Find,  "Najdi" },
        { SunXK_Cut,   "Izre\305\276i" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Spanish[] =
    {
        { XK_Shift_L, "May\303\272s" },
        { XK_Shift_R, "May\303\272s" },
        { XK_Page_Up, "ReP\303\241g" },
        { XK_Page_Down, "AvP\303\241g" },
        { XK_End, "Fin" },
        { XK_Home, "Inicio" },
        { XK_Delete, "Supr" },
        { XK_Escape, "Esc" },
        { XK_Right, "Hacia la derecha" },
        { XK_Left, "Hacia la izquierda" },
        { XK_Up, "Hacia arriba" },
        { XK_Down, "Hacia abajo" },
        { XK_BackSpace, "Ret" },
        { XK_Return, "Entrada" },
        { XK_space, "Espacio" },
        { XK_KP_Enter, "Intro" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Repetir" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Anular" },
        { SunXK_Front, "Delante" },
        { SunXK_Copy,  "Copiar" },
        { SunXK_Open,  "Abrir" },
        { SunXK_Paste, "Pegar" },
        { SunXK_Find,  "Buscar" },
        { SunXK_Cut,   "Cortar" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Estonian[] =
    {
        { XK_Page_Up, "PgUp" },
        { XK_Page_Down, "PgDown" },
        { XK_End, "End" },
        { XK_Home, "Home" },
        { XK_Insert, "Ins" },
        { XK_Delete, "Del" },
        { XK_Escape, "Esc" },
        { XK_Right, "Nool paremale" },
        { XK_Left, "Nool vasakule" },
        { XK_Up, "Nool \303\274les" },
        { XK_Down, "Nool alla" },
        { XK_BackSpace, "Tagasil\303\274ke" },
        { XK_Return, "Enter" },
        { XK_slash, "Kaldkriips" },
        { XK_space, "T\303\274hik" },
        { XK_asterisk, "T\303\244rn" },
        { SunXK_Stop,  "Peata" },
        { SunXK_Again, "Korda" },
        { SunXK_Props, "Omadused" },
        { SunXK_Undo,  "V\303\265ta tagasi" },
        { SunXK_Front, "Esiplaanile" },
        { SunXK_Copy,  "Kopeeri" },
        { SunXK_Open,  "Ava" },
        { SunXK_Paste, "Aseta" },
        { SunXK_Find,  "Otsi" },
        { SunXK_Cut,   "L\303\265ika" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Catalan[] =
    {
        { XK_Shift_L, "Maj" },
        { XK_Shift_R, "Maj" },
        { XK_Page_Up, "Re P\303\240g" },
        { XK_Page_Down, "Av P\303\240g" },
        { XK_End, "Fi" },
        { XK_Home, "Inici" },
        { XK_Delete, "Supr" },
        { XK_Escape, "Esc" },
        { XK_Right, "Dreta" },
        { XK_Left, "Esquerra" },
        { XK_Up, "Amunt" },
        { XK_Down, "Avall" },
        { XK_BackSpace, "Retroc\303\251s" },
        { XK_Return, "Retorn" },
        { XK_space, "Espai" },
        { XK_KP_Enter, "Retorn" },
        { SunXK_Stop,  "Atura" },
        { SunXK_Again, "Repeteix" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Desf\303\251s" },
        { SunXK_Front, "Davant" },
        { SunXK_Copy,  "C\303\262pia" },
        { SunXK_Open,  "Obre" },
        { SunXK_Paste, "Enganxa" },
        { SunXK_Find,  "Cerca" },
        { SunXK_Cut,   "Retalla" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Lithuanian[] =
    {
        { XK_Control_L, "Vald" },
        { XK_Control_R, "Vald" },
        { XK_Shift_L, "Lyg2" },
        { XK_Shift_R, "Lyg2" },
        { XK_Alt_L, "Alt" },
        { XK_Alt_R, "Lyg3" },
        { XK_Page_Up, "Psl\342\206\221" },
        { XK_Page_Down, "Psl\342\206\223" },
        { XK_End, "Pab" },
        { XK_Home, "Prad" },
        { XK_Insert, "\304\256terpti" },
        { XK_Delete, "\305\240al" },
        { XK_Escape, "Gr" },
        { XK_Right, "De\305\241in\304\227n" },
        { XK_Left, "Kair\304\227n" },
        { XK_Up, "Auk\305\241tyn" },
        { XK_Down, "\305\275emyn" },
        { XK_BackSpace, "Naikinti" },
        { XK_Return, "\304\256vesti" },
        { XK_asterisk, "\305\275vaig\305\276dut\304\227" },
        { XK_slash, "De\305\241ininis br\305\253k\305\241nys" },
        { XK_space, "Tarpas" },
        { SunXK_Stop,  "Stabdyti" },
        { SunXK_Again, "Kartoti" },
        { SunXK_Props, "Savyb\304\227s" },
        { SunXK_Undo,  "At\305\241aukti" },
        { SunXK_Front, "Priekinis planas" },
        { SunXK_Copy,  "Kopijuoti" },
        { SunXK_Open,  "Atverti" },
        { SunXK_Paste, "\304\256d\304\227ti" },
        { SunXK_Find,  "Ie\305\241koti" },
        { SunXK_Cut,   "I\305\241kirpti" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Hungarian[] =
    {
        { XK_Right, "Jobbra" },
        { XK_Left, "Balra" },
        { XK_Up, "Fel" },
        { XK_Down, "Le" },
        { XK_Return, "Enter" },
        { XK_space, "Sz\303\263k\303\266z" },
        { XK_asterisk, "Csillag" },
        { XK_slash, "Oszt\303\241sjel" },
    };

    static const struct KeyboardReplacements aKeyboards[] =
    {
        { "ca", aImplReplacements_Catalan, SAL_N_ELEMENTS(aImplReplacements_Catalan) },
        { "de", aImplReplacements_German, SAL_N_ELEMENTS(aImplReplacements_German) },
        { "sl", aImplReplacements_Slovenian, SAL_N_ELEMENTS(aImplReplacements_Slovenian) },
        { "es", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "et", aImplReplacements_Estonian, SAL_N_ELEMENTS(aImplReplacements_Estonian) },
        { "fr", aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
        { "hu", aImplReplacements_Hungarian, SAL_N_ELEMENTS(aImplReplacements_Hungarian) },
        { "it", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        { "lt", aImplReplacements_Lithuanian, SAL_N_ELEMENTS(aImplReplacements_Lithuanian) },
        { "nl", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "no", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        { "pt", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "ru", aImplReplacements_Russian, SAL_N_ELEMENTS(aImplReplacements_Russian) },
        { "sv", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
        { "tr", aImplReplacements_Turkish, SAL_N_ELEMENTS(aImplReplacements_Turkish) },
    };

    // translate keycodes, used within the displayed menu shortcuts
    OUString getKeysymReplacementName( OUString pLang, KeySym nSymbol )
    {
        for( unsigned int n = 0; n < SAL_N_ELEMENTS(aKeyboards); n++ )
        {
            if( pLang.equalsAscii( aKeyboards[n].pLangName ) )
            {
                const struct KeysymNameReplacement* pRepl = aKeyboards[n].pReplacements;
                for( int m = aKeyboards[n].nReplacements ; m ; )
                {
                    if( nSymbol == pRepl[--m].aSymbol )
                        return OUString( pRepl[m].pName, strlen(pRepl[m].pName), RTL_TEXTENCODING_UTF8 );
                }
            }
        }

        // try english fallbacks
        const struct KeysymNameReplacement* pRepl = aImplReplacements_English;
        for( int m = SAL_N_ELEMENTS(aImplReplacements_English); m ; )
        {
            if( nSymbol == pRepl[--m].aSymbol )
                return OUString( pRepl[m].pName, strlen(pRepl[m].pName), RTL_TEXTENCODING_UTF8 );
        }

        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
