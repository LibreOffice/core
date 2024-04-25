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

   x0 regs
   x1 stack
   x2 stack count
   x3 function
*/

    NESTED_ENTRY callVirtualFunction_fake

        // for unwind information, Windows has to store fp and lr
        PROLOG_SAVE_REG_PAIR	x29, x30, #-32!

        ALTERNATE_ENTRY callVirtualFunction

        sub   sp, sp, #32
        stp   fp, lr, [sp]
        mov   fp, sp

        // Stack space for arguments >= 8 (16-byte aligned):
        lsl   x2, x2, #3
        sub   x9, sp, x2
        bfc   x9, #0, #4
        mov   sp, x9

        // Copy arguments >= 8:
        cbz   x2, done
loop
        sub   x2, x2, #8
        ldr   x9, [x1, x2]
        str   x9, [sp, x2]
        cbnz  x2, loop
done

        mov   x9, x3                  // function

        mov   x10, x0
        str   x10, [fp, #16]

        // load the core argument passing registers
        ldp   x0, x1, [x10, #0]
        ldp   x2, x3, [x10, #16]
        ldp   x4, x5, [x10, #32]
        ldp   x6, x7, [x10, #48]

        ldp   d0, d1, [x10, #64]
        ldp   d2, d3, [x10, #80]
        ldp   d4, d5, [x10, #96]
        ldp   d6, d7, [x10, #112]

        blr   x9                      // call

        ldr   x10, [fp, #16]

        // save the simple return values
        stp   x0, x1, [x10, #0]
        stp   d0, d1, [x10, #64]
        stp   d2, d3, [x10, #80]

        add   sp, fp, #32
        ldp   fp, lr, [sp, #-32]
        ret

        NESTED_END callVirtualFunction_fake

    END

/* vim:set shiftwidth=4 softtabstop=4 expandtab */
