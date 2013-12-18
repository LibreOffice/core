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
} lib_to_factory_mapping;

typedef struct {
    const char *name;
    void * (*constructor_function)(void *, void *);
} lib_to_constructor_mapping;

const lib_to_factory_mapping *lo_get_factory_map(void);
const lib_to_constructor_mapping *lo_get_constructor_map(void);

#ifdef __cplusplus
}
#endif

#define NON_APP_SPECIFIC_FACTORY_MAP \
    { "libembobj.a", embobj_component_getFactory }, \
    { "libemboleobj.a", emboleobj_component_getFactory }, \
    { "libintrospectionlo.a", introspection_component_getFactory }, \
    { "libreflectionlo.a", reflection_component_getFactory }, \
    { "libstocserviceslo.a", stocservices_component_getFactory }, \
    { "libcomphelper.a", comphelp_component_getFactory }, \
    { "libconfigmgrlo.a", configmgr_component_getFactory }, \
    { "libdeployment.a", deployment_component_getFactory }, \
    { "libevtattlo.a", evtatt_component_getFactory }, \
    { "libfilterconfiglo.a", filterconfig1_component_getFactory }, \
    { "libfsstoragelo.a", fsstorage_component_getFactory }, \
    { "libfwklo.a", fwk_component_getFactory }, \
    { "libfwllo.a", fwl_component_getFactory }, \
    { "libhyphenlo.a", hyphen_component_getFactory }, \
    { "libi18npoollo.a", i18npool_component_getFactory }, \
    { "liblnglo.a", lng_component_getFactory }, \
    { "liblnthlo.a", lnth_component_getFactory }, \
    { "liblocalebe1lo.a", localebe1_component_getFactory }, \
    { "libooxlo.a", oox_component_getFactory }, \
    { "libpackage2.a", package2_component_getFactory }, \
    { "libsfxlo.a", sfx_component_getFactory }, \
    { "libsotlo.a", sot_component_getFactory }, \
    { "libspelllo.a", spell_component_getFactory }, \
    { "libsvllo.a", svl_component_getFactory }, \
    { "libsvtlo.a", svt_component_getFactory }, \
    { "libsvxlo.a", svx_component_getFactory }, \
    { "libtklo.a", tk_component_getFactory }, \
    { "libucb1.a", ucb_component_getFactory }, \
    { "libucpexpand1lo.a", ucpexpand1_component_getFactory }, \
    { "libucpfile1.a", ucpfile_component_getFactory }, \
    { "libunordflo.a", unordf_component_getFactory }, \
    { "libunoxmllo.a", unoxml_component_getFactory }, \
    { "libutllo.a", utl_component_getFactory }, \
    { "libvcllo.a", vcl_component_getFactory }, \
    { "libxmlsecurity.a", xmlsecurity_component_getFactory }, \
    { "libxolo.a", xo_component_getFactory }, \
    { "libxoflo.a", xof_component_getFactory }, \
    { "libxstor.a", xstor_component_getFactory }, \

#define NON_APP_SPECIFIC_CONSTRUCTOR_MAP \
    { "com_sun_star_comp_extensions_xml_sax_ParserExpat", com_sun_star_comp_extensions_xml_sax_ParserExpat }, \
    { "com_sun_star_comp_extensions_xml_sax_FastParser", com_sun_star_comp_extensions_xml_sax_FastParser }, \
    { "com_sun_star_comp_stoc_DLLComponentLoader", com_sun_star_comp_stoc_DLLComponentLoader }, \
    { "com_sun_star_comp_stoc_ImplementationRegistration", com_sun_star_comp_stoc_ImplementationRegistration }, \
    { "com_sun_star_comp_stoc_NestedRegistry", com_sun_star_comp_stoc_NestedRegistry }, \
    { "com_sun_star_comp_stoc_ORegistryServiceManager", com_sun_star_comp_stoc_ORegistryServiceManager }, \
    { "com_sun_star_comp_stoc_OServiceManager", com_sun_star_comp_stoc_OServiceManager }, \
    { "com_sun_star_comp_stoc_OServiceManagerWrapper", com_sun_star_comp_stoc_OServiceManagerWrapper }, \
    { "com_sun_star_comp_stoc_SimpleRegistry", com_sun_star_comp_stoc_SimpleRegistry }, \
    { "com_sun_star_extensions_xml_sax_Writer", com_sun_star_extensions_xml_sax_Writer }, \
    { "com_sun_star_security_comp_stoc_AccessController", com_sun_star_security_comp_stoc_AccessController }, \
    { "com_sun_star_security_comp_stoc_FilePolicy", com_sun_star_security_comp_stoc_FilePolicy }, \

#endif /* DISABLE_DYNLOADING */

#endif // INCLUDED_OSL_DETAIL_COMPONENT_MAPPING_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
