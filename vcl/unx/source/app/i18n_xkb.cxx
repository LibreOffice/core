/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18n_xkb.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:33:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"


#include <stdio.h>

#if defined(LINUX) || defined(FREEBSD) || defined(MACOSX) // should really check for xfree86 or for X11R6.1 and higher
#define __XKeyboardExtension__ 1
#else
#define __XKeyboardExtension__ 0
#endif

#include <prex.h>
#include <X11/X.h>
#if __XKeyboardExtension__
#include <X11/XKBlib.h>
#endif
#include <postx.h>

#ifndef _SAL_I18N_XKBDEXTENSION_HXX
#include "i18n_xkb.hxx"
#endif

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
                nXKBType, nXKBType );
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


