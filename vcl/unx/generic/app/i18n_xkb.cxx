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

#include <stdio.h>

#include "unx/saldisp.hxx"
#include "unx/saldata.hxx"
#include "unx/i18n_xkb.hxx"

SalI18N_KeyboardExtension::SalI18N_KeyboardExtension( Display* pDisplay )
    : mbUseExtension(true)
    , mnDefaultGroup(0)
    , mnGroup(0)
    , mnEventBase(0)
    , mnErrorBase(0)
    , mpDisplay(pDisplay)
{

    // allow user to set the default keyboard group idx or to disable the usage
    // of x keyboard extension at all:
    //      setenv SAL_XKEYBOARDGROUP       disables keyboard extension
    //      setenv SAL_XKEYBOARDGROUP 2     sets the keyboard group index to 2
    // keyboard group index must be in [1,4], may be specified in hex or decimal
    static char *pUseKeyboardExtension = getenv( "SAL_XKEYBOARDGROUP" );
    if ( pUseKeyboardExtension != nullptr )
    {
        mbUseExtension = pUseKeyboardExtension[0] != '\0' ;
        if ( mbUseExtension )
            mnDefaultGroup = strtol( pUseKeyboardExtension, nullptr, 0 );
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

        mbUseExtension = XkbQueryExtension( mpDisplay,
            &nMajorExtOpcode, &mnEventBase, &mnErrorBase,
            &nExtMajorVersion, &nExtMinorVersion ) != 0;
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
}

void
SalI18N_KeyboardExtension::Dispatch( XEvent* pEvent )
{
    // must the event be handled?
    if (   !mbUseExtension
        || (pEvent->type != mnEventBase) )
        return;

    // only handle state notify events for now, and only interested
    // in group details
    sal_uInt32 nXKBType = reinterpret_cast<XkbAnyEvent*>(pEvent)->xkb_type;
    switch ( nXKBType )
    {
        case XkbStateNotify:

            mnGroup = reinterpret_cast<XkbStateNotifyEvent*>(pEvent)->group;
            break;

        default:

            #if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "Got unrequested XkbAnyEvent %#x/%i\n",
                    static_cast<unsigned int>(nXKBType), static_cast<int>(nXKBType) );
            #endif
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
