/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OSL_DETAIL_COMPONENT_MAPPING_H
#define INCLUDED_OSL_DETAIL_COMPONENT_MAPPING_H

#include <osl/detail/component-declarations.h>

#ifdef DISABLE_DYNLOADING

#ifdef __cplusplus
extern "C" {
#endif

/* On iOS and perhaps Android static linking of the LO code into one
 * executable (on Android, into one shared library) is used. In order to get
 * the needed UNO coponent linked in, the "main" code for an app needs to
 * implement the lo_get_libmap() function to map UNO component library names
 * as produced in a build for iOS (like configmgr.uno.a or libsclo.a) to the
 * corresponding component_getFactory functions.
 */

typedef struct {
    const char *lib;
    void * (*component_getFactory_function)(const char *, void *, void *);
} lib_to_component_mapping;

const lib_to_component_mapping *lo_get_libmap(void);

#ifdef __cplusplus
}
#endif

#endif /* DISABLE_DYNLOADING */

#endif // INCLUDED_OSL_DETAIL_COMPONENT_MAPPING_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
