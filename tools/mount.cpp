/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * This is a very tiny helper to allow overlay mounting.
 */

#include <config.h>

#include <common/CoolMount.hpp>
#include <common/security.h>

#include <cstdio>
#include <cstdlib>
#include <sysexits.h>

int main(int argc, char** argv)
{
    /*WARNING: PRIVILEGED CODE CHECKING START */
    /*WARNING*/ if (!hasCorrectUID(/* appName = */ "coolmount"))
    /*WARNING*/ {
    /*WARNING*/    fprintf(stderr, "Aborting.\n");
    /*WARNING*/    return EX_SOFTWARE;
    /*WARNING*/ }
    /*WARNING: PRIVILEGED CODE CHECKING END */

    return domount(argc, argv);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
