/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http: *www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
 * Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef COMPONENT_MAPPING_H
#define COMPONENT_MAPPING_H

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

#endif /* COMPONENT_MAPPING_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
