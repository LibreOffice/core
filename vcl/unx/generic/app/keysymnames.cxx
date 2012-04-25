/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#if !defined(SOLARIS) && !defined(AIX)
#include <tools/prex.h>
#include <X11/XKBlib.h>
#include <tools/postx.h>
#endif

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

#ifdef SOLARIS
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/kbio.h>
#include <sys/kbd.h>
#include <stdio.h>
#include <fcntl.h>
#include <deflt.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#include <string.h>

namespace vcl_sal {

    struct KeysymNameReplacement
    {
        KeySym          aSymbol;
        const char*     pName;
    };

    struct KeyboardReplacements
    {
        const char*                     pKeyboardName;
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
        { XK_Right, "Hyre" },
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

    static const struct KeyboardReplacements aKeyboards[] =
    {
#ifdef SOLARIS
        { "Germany5", aImplReplacements_German, SAL_N_ELEMENTS(aImplReplacements_German) },
        { "Germany4", aImplReplacements_German, SAL_N_ELEMENTS(aImplReplacements_German) },
        { "France5",  aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
        { "France6",  aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
        { "France_x86", aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
        { "Italy5", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        { "Italy5-Hobo", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        { "Italy4", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        { "Italy6", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        { "Italy_x86", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        { "Netherland4", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "Netherland5", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "Netherland5-Hobo", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "Netherland6", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "Netherland_x86", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "Norway5", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        { "Norway5-Hobo", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        { "Norway4", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        { "Norway6", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        { "Norway_x86", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        { "Portugal5", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "Portugal5-Hobo", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "Portugal4", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "Portugal6", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "Portugal_x86", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "Spain5", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "Spain5-Hobo", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "Spain4", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "Spain6", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "Spain_x86", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "Sweden5", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
        { "Sweden5-Hobo", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
        { "Sweden4", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
        { "Sweden6", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
        { "Sweden_x86", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
#endif
        { "U.S. English", aImplReplacements_English, SAL_N_ELEMENTS(aImplReplacements_English) },
        { "United Kingdom", aImplReplacements_English, SAL_N_ELEMENTS(aImplReplacements_English) },
        // Germany, German
        { "German", aImplReplacements_German, SAL_N_ELEMENTS(aImplReplacements_German) },
        { "France", aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
        { "French", aImplReplacements_French, SAL_N_ELEMENTS(aImplReplacements_French) },
        // Italy, Italian
        { "Ital", aImplReplacements_Italian, SAL_N_ELEMENTS(aImplReplacements_Italian) },
        // Norway, Norwegian
        { "Norw", aImplReplacements_Norwegian, SAL_N_ELEMENTS(aImplReplacements_Norwegian) },
        // Portugal, Portuguese
        { "Portu", aImplReplacements_Portuguese, SAL_N_ELEMENTS(aImplReplacements_Portuguese) },
        { "Spain", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        { "Spanish", aImplReplacements_Spanish, SAL_N_ELEMENTS(aImplReplacements_Spanish) },
        // Sweden, Swedish
        { "Swed", aImplReplacements_Swedish, SAL_N_ELEMENTS(aImplReplacements_Swedish) },
        { "Netherland", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        { "Dutch", aImplReplacements_Dutch, SAL_N_ELEMENTS(aImplReplacements_Dutch) },
        // Turkish, Turkey
        { "Turk", aImplReplacements_Turkish, SAL_N_ELEMENTS(aImplReplacements_Turkish) },
        // Russian, Russia
        { "Russia", aImplReplacements_Russian, SAL_N_ELEMENTS(aImplReplacements_Russian) },
        { "English", aImplReplacements_English, SAL_N_ELEMENTS(aImplReplacements_English) }
    };

    rtl::OUString getKeysymReplacementName( const char* pKeyboard, KeySym nSymbol )
    {
        for( unsigned int n = 0; n < SAL_N_ELEMENTS(aKeyboards); n++ )
        {
            if( ! strncasecmp( pKeyboard, aKeyboards[n].pKeyboardName, strlen( aKeyboards[n].pKeyboardName ) ) )
            {
                const struct KeysymNameReplacement* pRepl = aKeyboards[n].pReplacements;
                for( int m = aKeyboards[n].nReplacements ; m ; )
                {
                    if( nSymbol == pRepl[--m].aSymbol )
                        return String( pRepl[m].pName, RTL_TEXTENCODING_UTF8 );
                }
            }
        }
        // try english fallbacks
        const struct KeysymNameReplacement* pRepl = aImplReplacements_English;
        for( int m = SAL_N_ELEMENTS(aImplReplacements_English); m ; )
        {
            if( nSymbol == pRepl[--m].aSymbol )
                return String( pRepl[m].pName, RTL_TEXTENCODING_UTF8 );
        }
        return rtl::OUString();
    }

}

#ifdef SOLARIS
typedef struct {
    int         n_layout;
    const char* p_description;
} keyboard_layout;

static const keyboard_layout type0_layout[] =
{
    { 0, "US4" },
    { -1, NULL }
};

static const keyboard_layout type3_layout[] =
{
    { 0, "US3" },
    { -1, NULL }
};

static const keyboard_layout type4_layout[] =
{
    { 0,  "US4" },
    { 1,  "US4" },
    { 2,  "FranceBelg4" },
    { 3,  "Canada4" },
    { 4,  "Denmark4" },
    { 5,  "Germany4" },
    { 6,  "Italy4" },
    { 7,  "Netherland4" },
    { 8,  "Norway4" },
    { 9,  "Portugal4" },
    { 10, "SpainLatAm4" },
    { 11, "SwedenFin4" },
    { 12, "Switzer_Fr4" },
    { 13, "Switzer_Ge4" },
    { 14, "UK4" },
    { 16, "Korea4" },
    { 17, "Taiwan4" },
    { 19, "US101A_PC" },
    { 19, "US101A_Sun" },
    { 32, "Japan4" },
    { 33, "US5" },
    { 34, "US_UNIX5" },
    { 35, "France5" },
    { 36, "Denmark5" },
    { 37, "Germany5" },
    { 38, "Italy5" },
    { 39, "Netherland5" },
    { 40, "Norway5" },
    { 41, "Portugal5" },
    { 42, "Spain5" },
    { 43, "Sweden5" },
    { 44, "Switzer_Fr5" },
    { 45, "Switzer_Ge5" },
    { 46, "UK5" },
    { 47, "Korea5" },
    { 48, "Taiwan5" },
    { 49, "Japan5" },
    { 50, "Canada_Fr5" },
    { 51, "Hungary5" },
    { 52, "Poland5" },
    { 53, "Czech5" },
    { 54, "Russia5" },
    { 55, "Latvia5" },
    { 56, "Turkey5" },
    { 57, "Greece5" },
    { 58, "Estonia5" },
    { 59, "Lithuania5" },
    { 63, "Canada_Fr5_TBITS5" },
    { 80, "US5_Hobo" },
    { 81, "US_UNIX5_Hobo" },
    { 82, "France5_Hobo" },
    { 83, "Denmark5_Hobo" },
    { 84, "Germany5_Hobo" },
    { 85, "Italy5_Hobo" },
    { 86, "Netherland5_Hobo" },
    { 87, "Norway5_Hobo" },
    { 88, "Portugal5_Hobo" },
    { 89, "Spain5_Hobo" },
    { 90, "Sweden5_Hobo" },
    { 91, "Switzer_Fr5_Hobo" },
    { 92, "Switzer_Ge5_Hobo" },
    { 93, "UK5_Hobo" },
    { 94, "Korea5_Hobo" },
    { 95, "Taiwan5_Hobo" },
    { 96, "Japan5_Hobo" },
    { 97, "Canada_Fr5_Hobo" },
    { -1, NULL }
};

static const keyboard_layout type101_layout[] =
{
    {  0, "US101A_x86" },
    {  1, "US101A_x86" },
    { 34, "J3100_x86" },
    { 35, "France_x86" },
    { 36, "Denmark_x86" },
    { 37, "Germany_x86" },
    { 38, "Italy_x86" },
    { 39, "Netherland_x86" },
    { 40, "Norway_x86" },
    { 41, "Portugal_x86" },
    { 42, "Spain_x86" },
    { 43, "Sweden_x86" },
    { 44, "Switzer_Fr_x86" },
    { 45, "Switzer_Ge_x86" },
    { 46, "UK_x86" },
    { 47, "Korea_x86" },
    { 48, "Taiwan_x86" },
    { 49, "Japan_x86" },
    { 50, "Canada_Fr2_x86" },
    { 51, "Hungary_x86" },
    { 52, "Poland_x86" },
    { 53, "Czech_x86" },
    { 54, "Russia_x86" },
    { 55, "Latvia_x86" },
    { 56, "Turkey_x86" },
    { 57, "Greece_x86" },
    { 59, "Lithuania_x86" },
    { 1001, "MS_US101A_x86" },
    { -1, NULL }
};

static const keyboard_layout type6_layout[] =
{
    { 0,  "US6" },
    { 6,  "Denmark6" },
    { 7,  "Finnish6" },
    { 8,  "France6" },
    { 9,  "Germany6" },
    { 14, "Italy6" },
    { 15, "Japan6" },
    { 16, "Korea6" },
    { 18, "Netherland6" },
    { 19, "Norway6" },
    { 22, "Portugal6" },
    { 25, "Spain6" },
    { 26, "Sweden6" },
    { 27, "Switzer_Fr6" },
    { 28, "Switzer_Ge6" },
    { 30, "Taiwan6" },
    { 32, "UK6" },
    { 33, "US6" },
    { -1, NULL }
};
#endif


#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
