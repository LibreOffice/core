/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#ifdef DISABLE_DYNLOADING

#ifdef __cplusplus
extern "C" {
#endif

/* On iOS and perhaps Android static linking of the LO code into one
 * executable (on Android, into one shared library) is used. In order to get
 * the needed UNO component linked in, the "main" code for an app needs to
 * implement the lo_get_libmap() function to map UNO component library names
 * as produced in a build for iOS (like configmgr.uno.a or libsclo.a) to the
 * corresponding component_getFactory functions.
 */

typedef struct {
    const char *name;
    void * (*component_getFactory_function)(const char *, void *, void *);
} lib_to_factory_mapping;

typedef struct {
    const char *name;
    void (*constructor_function)(void);
} lib_to_constructor_mapping;

const lib_to_factory_mapping *lo_get_factory_map(void);
const lib_to_constructor_mapping *lo_get_constructor_map(void);

#ifdef __cplusplus
}
#endif





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
