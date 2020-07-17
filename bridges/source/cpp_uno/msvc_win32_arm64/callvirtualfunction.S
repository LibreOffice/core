/* -*- tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

    OPT 2   // disable listing
// macros to add unwind information
#include "ksarm64.h"
    OPT 1   // re-enable listing

    EXPORT  callVirtualFunction

    TEXTAREA, ALIGN=8

/*
   extern void callVirtualFunction

   x0 stack
   x1 frame
   x2 function
   x3 return
*/

    NESTED_ENTRY callVirtualFunction_fake

        // for unwind information, Windows has to store fp and lr
        PROLOG_SAVE_REG_PAIR	x29, x30, #-32!

        ALTERNATE_ENTRY callVirtualFunction

        // use a stack frame allocated by our caller
        stp   x29, x30, [x1]
        mov   x29, x1
        mov   sp, x0

        mov   x9, x2                  // function
        mov   x8, x3                  // complex return
        str   x3, [x29, #16]          // save rvalue

        // load the core argument passing registers
        ldp   x0, x1, [sp, #0]
        ldp   x2, x3, [sp, #16]
        ldp   x4, x5, [sp, #32]
        ldp   x6, x7, [sp, #48]

        ldp   d0, d1, [sp, #64]
        ldp   d2, d3, [sp, #80]
        ldp   d4, d5, [sp, #96]
        ldp   d6, d7, [sp, #112]

        blr   x9                      // call

        ldr   x3, [x29, #16]          // reload rvalue

        // partially deconstruct the stack frame
        mov   sp, x29
        ldp   x29, x30, [x29]

        // save the simple return values
        stp   x0, x1, [sp, #0]
        stp   d0, d1, [sp, #64]
        stp   d2, d3, [sp, #80]

        NESTED_END callVirtualFunction_fake

    END

/* vim:set shiftwidth=4 softtabstop=4 expandtab */
