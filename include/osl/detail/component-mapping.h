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
    const char *name;
    void * (*component_getFactory_function)(const char *, void *, void *);
} lib_to_component_mapping;

const lib_to_component_mapping *lo_get_library_map(void);
const lib_to_component_mapping *lo_get_implementation_map(void);

#ifdef __cplusplus
}
#endif

#define NON_APP_SPECIFIC_COMPONENT_MAP \
    { "libintrospectionlo.a", introspection_component_getFactory }, \
    { "libreflectionlo.a", reflection_component_getFactory }, \
    { "libstocserviceslo.a", stocservices_component_getFactory }, \
    { "libcomphelper.a", comphelp_component_getFactory }, \
    { "libconfigmgrlo.a", configmgr_component_getFactory }, \
    { "libdeployment.a", deployment_component_getFactory }, \
    { "libfilterconfiglo.a", filterconfig1_component_getFactory }, \
    { "libfwklo.a", fwk_component_getFactory }, \
    { "libi18npoollo.a", i18npool_component_getFactory }, \
    { "liblocalebe1lo.a", localebe1_component_getFactory }, \
    { "libpackage2.a", package2_component_getFactory }, \
    { "libsfxlo.a", sfx_component_getFactory }, \
    { "libsvllo.a", svl_component_getFactory }, \
    { "libtklo.a", tk_component_getFactory }, \
    { "libucb1.a", ucb_component_getFactory }, \
    { "libucpexpand1lo.a", ucpexpand1_component_getFactory }, \
    { "libucpfile1.a", ucpfile_component_getFactory }, \
    { "libutllo.a", utl_component_getFactory }, \
    { "libvcllo.a", vcl_component_getFactory }, \
    { "libxstor.a", xstor_component_getFactory }, \

#define NON_APP_SPECIFIC_DIRECT_COMPONENT_MAP \
    { "com.sun.star.comp.extensions.xml.sax.ParserExpat", com_sun_star_comp_extensions_xml_sax_ParserExpat_component_getFactory }, \
    { "com.sun.star.comp.extensions.xml.sax.FastParser", com_sun_star_comp_extensions_xml_sax_FastParser_component_getFactory }, \
    { "com.sun.star.comp.stoc.DLLComponentLoader.component.getFactory", com_sun_star_comp_stoc_DLLComponentLoader_component_getFactory }, \
    { "com.sun.star.comp.stoc.ImplementationRegistration.component.getFactory", com_sun_star_comp_stoc_ImplementationRegistration_component_getFactory }, \
    { "com.sun.star.comp.stoc.NestedRegistry.component.getFactory", com_sun_star_comp_stoc_NestedRegistry_component_getFactory }, \
    { "com.sun.star.comp.stoc.ORegistryServiceManager.component.getFactory", com_sun_star_comp_stoc_ORegistryServiceManager_component_getFactory }, \
    { "com.sun.star.comp.stoc.OServiceManager.component.getFactory", com_sun_star_comp_stoc_OServiceManager_component_getFactory }, \
    { "com.sun.star.comp.stoc.OServiceManagerWrapper.component.getFactory", com_sun_star_comp_stoc_OServiceManagerWrapper_component_getFactory }, \
    { "com.sun.star.comp.stoc.SimpleRegistry.component.getFactory", com_sun_star_comp_stoc_SimpleRegistry_component_getFactory }, \
    { "com.sun.star.extensions.xml.sax.Writer", com_sun_star_extensions_xml_sax_Writer_component_getFactory }, \
    { "com.sun.star.security.comp.stoc.AccessController.component.getFactory", com_sun_star_security_comp_stoc_AccessController_component_getFactory }, \
    { "com.sun.star.security.comp.stoc.FilePolicy.component.getFactory", com_sun_star_security_comp_stoc_FilePolicy_component_getFactory }, \

#endif /* DISABLE_DYNLOADING */

#endif // INCLUDED_OSL_DETAIL_COMPONENT_MAPPING_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
