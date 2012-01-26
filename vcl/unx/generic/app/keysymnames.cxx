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

const char* SalDisplay::GetKeyboardName( bool bRefresh )
{
    if (bRefresh || m_aKeyboardName.isEmpty())
    {
#if defined(SOLARIS)
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
                close(kbd);
            }
        }
#elif !defined(AIX)
        int opcode, event, error;
        int major = XkbMajorVersion, minor = XkbMinorVersion;
        if( XkbQueryExtension( GetDisplay(), &opcode, &event,&error, &major, &minor ) )
        {
            XkbDescPtr pXkbDesc = NULL;
            // try X keyboard extension
            if( (pXkbDesc = XkbGetKeyboard( GetDisplay(), XkbAllComponentsMask, XkbUseCoreKbd )) )
            {
                const char* pAtom = NULL;
                if( pXkbDesc->names->groups[0] )
                {
                    pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->groups[0] );
                    m_aKeyboardName = pAtom;
                    XFree( (void*)pAtom );
                }
                else
                    m_aKeyboardName = "<unknown keyboard>";
#if OSL_DEBUG_LEVEL > 1
#define PRINT_ATOM( x ) { if( pXkbDesc->names->x ) { pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->x ); fprintf( stderr, "%s: %s\n", #x, pAtom ); XFree( (void*)pAtom ); } else fprintf( stderr, "%s: <nil>\n", #x ); }

                PRINT_ATOM( keycodes );
                PRINT_ATOM( geometry );
                PRINT_ATOM( symbols );
                PRINT_ATOM( types );
                PRINT_ATOM( compat );
                PRINT_ATOM( phys_symbols );

#define PRINT_ATOM_2( x ) { if( pXkbDesc->names->x[i] ) { pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->x[i] ); fprintf( stderr, "%s[%d]: %s\n", #x, i, pAtom ); XFree( (void*)pAtom ); } else fprintf( stderr, "%s[%d]: <nil>\n", #x, i ); }
                int i;
                for( i = 0; i < XkbNumVirtualMods; i++ )
                    PRINT_ATOM_2( vmods );
                for( i = 0; i < XkbNumIndicators; i++ )
                    PRINT_ATOM_2( indicators );
                for( i = 0; i < XkbNumKbdGroups; i++ )
                    PRINT_ATOM_2( groups );
#endif
                XkbFreeKeyboard( pXkbDesc, XkbAllComponentsMask, True );
            }
        }
#endif
        if (m_aKeyboardName.isEmpty())
            m_aKeyboardName = "<unknown keyboard>";
    }
    return m_aKeyboardName.getStr();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
