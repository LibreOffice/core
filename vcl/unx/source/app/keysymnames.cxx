/*************************************************************************
 *
 *  $RCSfile: keysymnames.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2000-11-22 10:01:12 $
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

#ifndef SOLARIS
#include <prex.h>
#include <X11/XKBlib.h>
#include <postx.h>
#endif

#include <saldisp.hxx>
#include <X11/keysym.h>

#ifdef SOLARIS
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/kbio.h>
#include <sys/kbd.h>
#include <stdio.h>
#include <fcntl.h>
#include <deflt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#ifndef KB_USB // compile with too old headers
#define KB_USB 6
#endif
#endif

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
        rtl_TextEncoding                nEncoding;
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
        { XK_space, "Leertaste" }
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
        { XK_KP_Enter, "Entrée" }
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
        { XK_space, "Spaziatrice" }
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
        { XK_space, "Spatiebalk" }
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
        { XK_Return, "Enter" }
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
        { XK_space, "Blank" }
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
        { XK_BackSpace, "Rückschritt" },
        { XK_Return, "Eingabe" },
        { XK_slash, "Schrägstrich" }
    };

    static const struct KeysymNameReplacement aImplReplacements_Spanish[] =
    {
        { XK_Shift_L, "Mayos" },
        { XK_Shift_R, "Mayos" },
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
        { XK_KP_Enter, "Intro" }
    };

    static const struct KeyboardReplacements aKeyboards[] =
    {
#ifdef SOLARIS
        { "Germany5", aImplReplacements_German, sizeof(aImplReplacements_German)/sizeof(aImplReplacements_German[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Germany4", aImplReplacements_German, sizeof(aImplReplacements_German)/sizeof(aImplReplacements_German[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "France5", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "France6", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "France_x86", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Italy5", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Italy5-Hobo", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Italy4", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Italy6", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Italy_x86", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Netherland4", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Netherland5", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Netherland5-Hobo", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Netherland6", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Netherland_x86", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Norway5", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Norway5-Hobo", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Norway4", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Norway6", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Norway_x86", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Portugal5", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Portugal5-Hobo", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Portugal4", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Portugal6", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Portugal_x86", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Spain5", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Spain5-Hobo", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Spain4", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Spain6", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Spain_x86", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Sweden5", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Sweden5-Hobo", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Sweden4", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Sweden6", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Sweden_x86", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]), RTL_TEXTENCODING_ISO_8859_1 },
#endif
        { "German", aImplReplacements_German, sizeof(aImplReplacements_German)/sizeof(aImplReplacements_German[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "French", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Norwegian", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Swedish", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Portuguese", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Spanish", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]), RTL_TEXTENCODING_ISO_8859_1 },
        { "Italian", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]), RTL_TEXTENCODING_ISO_8859_1 }
    };

    String getKeysymReplacementName( const char* pKeyboard, KeySym nSymbol )
    {
        for( int n = 0; n < sizeof(aKeyboards)/sizeof(aKeyboards[0]); n++ )
        {
            if( ! strcasecmp( pKeyboard, aKeyboards[n].pKeyboardName ) )
            {
                const struct KeysymNameReplacement* pRepl = aKeyboards[n].pReplacements;
                for( int m = aKeyboards[n].nReplacements ; m ; )
                {
                    if( nSymbol == pRepl[--m].aSymbol )
                        return String( pRepl[m].pName, aKeyboards[n].nEncoding );
                }
            }
        }
        return String();
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


#ifdef DEBUG
#include <stdio.h>
#endif

const char* SalDisplay::GetKeyboardName( BOOL bRefresh )
{
    if( bRefresh || ! m_aKeyboardName.Len() )
    {
#ifdef SOLARIS
        if( IsLocal() )
        {
            int kbd = open( "/dev/kbd", O_RDONLY );
            if( kbd >= 0 )
            {
                int kbd_type = 0;
                if( ! ioctl( kbd, KIOCTYPE, &kbd_type ) )
                {
                    int kbd_layout = 0;
                    if( ! ioctl( kbd, KIOCLAYOUT, &kbd_layout ) )
                    {
                        const keyboard_layout *p_layout = NULL;
                        switch( kbd_type )
                        {
                            case KB_KLUNK: p_layout = type0_layout;   break;
                            case KB_SUN3:  p_layout = type3_layout;   break;
                            case KB_SUN4:  p_layout = type4_layout;   break;
                            case KB_USB:   p_layout = type6_layout;   break;
                            case KB_PC:    p_layout = type101_layout; break;
                        }

                        if( p_layout )
                        {
                            while( p_layout->n_layout != -1 )
                            {
                                if ( p_layout->n_layout == kbd_layout )
                                {
                                    m_aKeyboardName = p_layout->p_description;
                                    break;
                                }
                                p_layout++;
                            }
                        }
                    }
                }
            }
        }
#else
        int opcode, event, error;
        int major = XkbMajorVersion, minor = XkbMinorVersion;
        if( ! m_aKeyboardName.Len() &&
            #ifndef USE_PSPRINT
            XSalIsDisplay( GetDisplay() ) &&
            #endif
            XkbQueryExtension( GetDisplay(), &opcode, &event,&error, &major, &minor ) )
        {
            XkbDescPtr pXkbDesc = NULL;
            // try X keyboard extension
            if( pXkbDesc = XkbGetKeyboard( GetDisplay(), XkbAllComponentsMask, XkbUseCoreKbd ) )
            {
                const char* pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->groups[0] );
                m_aKeyboardName = pAtom;
                XFree( (void*)pAtom );

#ifdef DEBUG
#define PRINT_ATOM( x ) { if( pXkbDesc->names->x ) { pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->x ); fprintf( stderr, "%s: %s\n", #x, pAtom ); XFree( (void*)pAtom ); } else fprintf( stderr, "%s: <nil>\n", #x ); }

                PRINT_ATOM( keycodes );
                PRINT_ATOM( geometry );
                PRINT_ATOM( symbols );
                PRINT_ATOM( types );
                PRINT_ATOM( compat );
                PRINT_ATOM( phys_symbols );

                int i;
                for( i = 0; i < XkbNumVirtualMods; i++ )
                    PRINT_ATOM( vmods[i] );
                for( i = 0; i < XkbNumIndicators; i++ )
                    PRINT_ATOM( indicators[i] );
                for( i = 0; i < XkbNumKbdGroups; i++ )
                    PRINT_ATOM( groups[i] );
#endif
                XkbFreeKeyboard( pXkbDesc, 0, True );
            }
        }
#endif
    }
    return m_aKeyboardName.GetBuffer();
}
