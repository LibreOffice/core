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
        { XK_Right, "Sağ" },
        { XK_Left, "Sol" },
        { XK_Up, "Yukarı" },
        { XK_Down, "Aşağı" },
        { XK_space, "Boşluk" }
    };

    static const struct KeysymNameReplacement aImplReplacements_Russian[] =
    {
        { XK_Right, "Вправо" },
        { XK_Left, "Влево" },
        { XK_Up, "Вверх" },
        { XK_Down, "Вниз" },
        { XK_space, "Пробел" }
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
        { XK_BackSpace, "Rückschritt" },
        { XK_Return, "Eingabe" },
        { XK_slash, "Schrägstrich" },
        { XK_space, "Leertaste" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Wiederholen" },
        { SunXK_Props, "Eigenschaften" },
        { SunXK_Undo,  "Zurücknehmen" },
        { SunXK_Front, "Vordergrund" },
        { SunXK_Copy,  "Kopieren" },
        { SunXK_Open,  "Öffnen" },
        { SunXK_Paste, "Einsetzen" },
        { SunXK_Find,  "Suchen" },
        { SunXK_Cut,   "Ausschneiden" },
    };

    static const struct KeysymNameReplacement aImplReplacements_French[] =
    {
        { XK_Shift_L, "Maj" },
        { XK_Shift_R, "Maj" },
        { XK_Page_Up, "Pg. Préc" },
        { XK_Page_Down, "Pg. Suiv" },
        { XK_End, "Fin" },
        { XK_Home, "Origine" },
        { XK_Insert, "Insérer" },
        { XK_Delete, "Suppr" },
        { XK_Escape, "Esc" },
        { XK_Right, "Droite" },
        { XK_Left, "Gauche" },
        { XK_Up, "Haut" },
        { XK_Down, "Bas" },
        { XK_BackSpace, "Ret. Arr" },
        { XK_Return, "Retour" },
        { XK_KP_Enter, "Entrée" },
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
        { SunXK_Props, "Proprietà" },
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
        { XK_Right, "Høyre" },
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
        { SunXK_Open,  "Åpne" },
        { SunXK_Paste, "Lim" },
        { SunXK_Find,  "Søk" },
        { SunXK_Cut,   "Klipp" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Swedish[] =
    {
        { XK_Shift_L, "Skift" },
        { XK_Shift_R, "Skift" },
        { XK_Page_Up, "PageUp" },
        { XK_Page_Down, "PageDown" },
        { XK_Escape, "Esc" },
        { XK_Right, "Höger" },
        { XK_Left, "Vänster" },
        { XK_Up, "Up" },
        { XK_Down, "Ned" },
        { XK_BackSpace, "Backsteg" },
        { XK_Return, "Retur" },
        { XK_space, "Blank" },
        { SunXK_Stop,  "Avbryt" },
        { SunXK_Again, "Upprepa" },
        { SunXK_Props, "Egenskaper" },
        { SunXK_Undo,  "Ångra" },
        { SunXK_Front, "Fram" },
        { SunXK_Copy,  "Kopiera" },
        { SunXK_Open,  "Öppna" },
        { SunXK_Paste, "Klistra in" },
        { SunXK_Find,  "Sök" },
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
        { XK_Page_Up, "Prejšnja stranf" },
        { XK_Page_Down, "Naslednja stran" },
        { XK_End, "Konec" },
        { XK_Home, "Začetek" },
        { XK_Insert, "Vstavljalka" },
        { XK_Delete, "Brisalka" },
        { XK_Escape, "Ubežnica" },
        { XK_Right, "Desno" },
        { XK_Left, "Levo" },
        { XK_Up, "Navzgor" },
        { XK_Down, "Navzdol" },
        { XK_BackSpace, "Vračalka" },
        { XK_Return, "Vnašalka" },
        { XK_slash, "Poševnica" },
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
        { SunXK_Cut,   "Izreži" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Spanish[] =
    {
        { XK_Shift_L, "Mayús" },
        { XK_Shift_R, "Mayús" },
        { XK_Page_Up, "RePág" },
        { XK_Page_Down, "AvPág" },
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
        { XK_Up, "Nool üles" },
        { XK_Down, "Nool alla" },
        { XK_BackSpace, "Tagasilüke" },
        { XK_Return, "Enter" },
        { XK_slash, "Kaldkriips" },
        { XK_space, "Tühik" },
        { XK_asterisk, "Tärn" },
        { SunXK_Stop,  "Peata" },
        { SunXK_Again, "Korda" },
        { SunXK_Props, "Omadused" },
        { SunXK_Undo,  "Võta tagasi" },
        { SunXK_Front, "Esiplaanile" },
        { SunXK_Copy,  "Kopeeri" },
        { SunXK_Open,  "Ava" },
        { SunXK_Paste, "Aseta" },
        { SunXK_Find,  "Otsi" },
        { SunXK_Cut,   "Lõika" },
    };

    static const struct KeysymNameReplacement aImplReplacements_Catalan[] =
    {
        { XK_Shift_L, "Maj" },
        { XK_Shift_R, "Maj" },
        { XK_Page_Up, "Re Pàg" },
        { XK_Page_Down, "Av Pàg" },
        { XK_End, "Fi" },
        { XK_Home, "Inici" },
        { XK_Delete, "Supr" },
        { XK_Escape, "Esc" },
        { XK_Right, "Dreta" },
        { XK_Left, "Esquerra" },
        { XK_Up, "Amunt" },
        { XK_Down, "Avall" },
        { XK_BackSpace, "Retrocés" },
        { XK_Return, "Retorn" },
        { XK_space, "Espai" },
        { XK_KP_Enter, "Retorn" },
        { SunXK_Stop,  "Atura" },
        { SunXK_Again, "Repeteix" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Desfés" },
        { SunXK_Front, "Davant" },
        { SunXK_Copy,  "Còpia" },
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
        { XK_Page_Up, "Psl↑" },
        { XK_Page_Down, "Psl↓" },
        { XK_End, "Pab" },
        { XK_Home, "Prad" },
        { XK_Insert, "Įterpti" },
        { XK_Delete, "Šal" },
        { XK_Escape, "Gr" },
        { XK_Right, "Dešinėn" },
        { XK_Left, "Kairėn" },
        { XK_Up, "Aukštyn" },
        { XK_Down, "Žemyn" },
        { XK_BackSpace, "Naikinti" },
        { XK_Return, "Įvesti" },
        { XK_asterisk, "Žvaigždutė" },
        { XK_slash, "Dešininis brūkšnys" },
        { XK_space, "Tarpas" },
        { SunXK_Stop,  "Stabdyti" },
        { SunXK_Again, "Kartoti" },
        { SunXK_Props, "Savybės" },
        { SunXK_Undo,  "Atšaukti" },
        { SunXK_Front, "Priekinis planas" },
        { SunXK_Copy,  "Kopijuoti" },
        { SunXK_Open,  "Atverti" },
        { SunXK_Paste, "Įdėti" },
        { SunXK_Find,  "Ieškoti" },
        { SunXK_Cut,   "Iškirpti" },
    };

    static const struct KeyboardReplacements aKeyboards[] =
    {
        { "ca", aImplReplacements_Catalan, SAL_N_ELEMENTS(aImplReplacements_Catalan) },
        { "de", aImplReplacements_German, SAL_N_ELEMENTS(aImplReplacements_German) },
        { "sl", aImplReplacements_Slovenian, SAL_N_ELEMENTS(aImplReplacements_Slovenian) },
        { "es", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "et", aImplReplacements_Estonian, SAL_N_ELEMENTS(aImplReplacements_Estonian) },
        { "fr", aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
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
