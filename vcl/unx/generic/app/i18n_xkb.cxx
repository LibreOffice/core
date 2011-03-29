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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"


#include <stdio.h>

#include "unx/saldisp.hxx"
#include "unx/saldata.hxx"
#include "unx/i18n_xkb.hxx"

SalI18N_KeyboardExtension::SalI18N_KeyboardExtension( Display*
#if __XKeyboardExtension__
pDisplay
#endif
)
    : mbUseExtension( (sal_Bool)__XKeyboardExtension__ ),
      mnDefaultGroup( 0 )
{
    #if __XKeyboardExtension__

    mpDisplay = pDisplay;

    // allow user to set the default keyboard group idx or to disable the usage
    // of x keyboard extension at all:
    //      setenv SAL_XKEYBOARDGROUP       disables keyboard extension
    //      setenv SAL_XKEYBOARDGROUP 2     sets the keyboard group index to 2
    // keyboard group index must be in [1,4], may be specified in hex or decimal
    static char *pUseKeyboardExtension = getenv( "SAL_XKEYBOARDGROUP" );
    if ( pUseKeyboardExtension != NULL )
    {
        mbUseExtension = pUseKeyboardExtension[0] != '\0' ;
        if ( mbUseExtension )
            mnDefaultGroup = strtol( pUseKeyboardExtension, NULL, 0 );
        if ( mnDefaultGroup > XkbMaxKbdGroup )
            mnDefaultGroup = 0;
    }

    // query XServer support for XKB Extension,
    // do not call XQueryExtension() / XInitExtension() due to possible version
    // clashes !
    if ( mbUseExtension )
    {
        int nMajorExtOpcode;
        int nExtMajorVersion = XkbMajorVersion;
        int nExtMinorVersion = XkbMinorVersion;

        mbUseExtension = (sal_Bool)XkbQueryExtension( mpDisplay,
            &nMajorExtOpcode, (int*)&mnEventBase, (int*)&mnErrorBase,
            &nExtMajorVersion, &nExtMinorVersion );
    }

    // query notification for changes of the keyboard group
    if ( mbUseExtension )
    {
        #define XkbGroupMask (  XkbGroupStateMask | XkbGroupBaseMask \
                                | XkbGroupLatchMask | XkbGroupLockMask )

        mbUseExtension = XkbSelectEventDetails( mpDisplay,
            XkbUseCoreKbd, XkbStateNotify, XkbGroupMask, XkbGroupMask );
    }

    // query initial keyboard group
    if ( mbUseExtension )
    {
        XkbStateRec aStateRecord;
        XkbGetState( mpDisplay, XkbUseCoreKbd, &aStateRecord );
        mnGroup = aStateRecord.group;
    }

    #endif // __XKeyboardExtension__
}

void
SalI18N_KeyboardExtension::Dispatch( XEvent*
#if __XKeyboardExtension__
pEvent
#endif
)
{
    #if __XKeyboardExtension__

    // must the event be handled?
    if (   !mbUseExtension
        || (pEvent->type != mnEventBase) )
        return;

    // only handle state notify events for now, and only interested
    // in group details
    sal_uInt32 nXKBType = ((XkbAnyEvent*)pEvent)->xkb_type;
    switch ( nXKBType )
    {
        case XkbStateNotify:

            mnGroup = ((XkbStateNotifyEvent*)pEvent)->group;
            break;

        default:

            #if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "Got unrequested XkbAnyEvent %#x/%i\n",
                    static_cast<unsigned int>(nXKBType), static_cast<int>(nXKBType) );
            #endif
            break;
    }
    #endif // __XKeyboardExtension__
}

#if __XKeyboardExtension__
sal_uInt32
SalI18N_KeyboardExtension::LookupKeysymInGroup( sal_uInt32 nKeyCode,
                                                 sal_uInt32 nShiftState,
                                                   sal_uInt32 nGroup ) const
#else
sal_uInt32
SalI18N_KeyboardExtension::LookupKeysymInGroup( sal_uInt32,sal_uInt32,sal_uInt32 ) const
#endif
{
    #if __XKeyboardExtension__

    if ( !mbUseExtension )
        return NoSymbol;

    nShiftState &= ShiftMask;

    KeySym      nKeySymbol;
    nKeySymbol = XkbKeycodeToKeysym( mpDisplay, nKeyCode, nGroup, nShiftState );
    return nKeySymbol;

    #else

    return NoSymbol;

    #endif // __XKeyboardExtension__
}


