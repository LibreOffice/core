#!/usr/bin/python
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
from optparse import OptionParser

# foo_component_getFactory functions are split into groups, so that you could
# choose e.g. 'core' and 'writer' functionality and through factory_map,
# relevant function sections will be referenced in lo_get_factory_map().
# That prevents garbage collector to ignore them as unused.

# The same groups are used for constructor based implementations
# referenced in lo_get_constructor_map().

core_factory_list = [
    ("libembobj.a", "embobj_component_getFactory"),
    ("libreflectionlo.a", "reflection_component_getFactory"),
    ("libstocserviceslo.a", "stocservices_component_getFactory"),
    ("libchartcontrollerlo.a", "chartcontroller_component_getFactory"),
    ("libchartcorelo.a", "chartcore_component_getFactory"),
    ("libcomphelper.a", "comphelp_component_getFactory"),
    ("libconfigmgrlo.a", "configmgr_component_getFactory"),
    ("libdrawinglayerlo.a", "drawinglayer_component_getFactory"),
    ("libfilterconfiglo.a", "filterconfig1_component_getFactory"),
    ("libfrmlo.a", "frm_component_getFactory"),
    ("libfsstoragelo.a", "fsstorage_component_getFactory"),
    ("libhyphenlo.a", "hyphen_component_getFactory"),
    ("libi18npoollo.a", "i18npool_component_getFactory"),
    ("liblnglo.a", "lng_component_getFactory"),
    ("liblnthlo.a", "lnth_component_getFactory"),
    ("liblocalebe1lo.a", "localebe1_component_getFactory"),
    ("libooxlo.a", "oox_component_getFactory"),
    ("libpackage2.a", "package2_component_getFactory"),
    ("libsmlo.a", "sm_component_getFactory"),
    ("libsotlo.a", "sot_component_getFactory"),
    ("libspelllo.a", "spell_component_getFactory"),
    ("libsrtrs1.a", "srtrs1_component_getFactory"),
    ("libucb1.a", "ucb_component_getFactory"),
    ("libucpfile1.a", "ucpfile_component_getFactory"),
    ("libucphier1.a", "ucphier1_component_getFactory"),
    ("libunordflo.a", "unordf_component_getFactory"),
    ("libunoxmllo.a", "unoxml_component_getFactory"),
    ("libutllo.a", "utl_component_getFactory"),
    ("libxmlsecurity.a", "xmlsecurity_component_getFactory"),
    ("libxolo.a", "xo_component_getFactory"),
    ("libxstor.a", "xstor_component_getFactory"),
    ]

core_constructor_list = [
# framework/util/fwk.component
    "com_sun_star_comp_framework_AutoRecovery_get_implementation",
    "com_sun_star_comp_framework_Desktop_get_implementation",
    "com_sun_star_comp_framework_Frame_get_implementation",
    "com_sun_star_comp_framework_JobExecutor_get_implementation",
    "com_sun_star_comp_framework_LayoutManager_get_implementation",
    "com_sun_star_comp_framework_ModuleManager_get_implementation",
    "com_sun_star_comp_framework_ModuleUIConfigurationManager_get_implementation",
    "com_sun_star_comp_framework_ModuleUIConfigurationManagerSupplier_get_implementation",
    "com_sun_star_comp_framework_PathSettings_get_implementation",
    "com_sun_star_comp_framework_PathSubstitution_get_implementation",
    "com_sun_star_comp_framework_StatusIndicatorFactory_get_implementation",
    "com_sun_star_comp_framework_TaskCreator_get_implementation",
    "com_sun_star_comp_framework_ToolBarControllerFactory_get_implementation",
    "com_sun_star_comp_framework_UIConfigurationManager_get_implementation",
    "com_sun_star_comp_framework_UIElementFactoryManager_get_implementation",
    "com_sun_star_comp_framework_URLTransformer_get_implementation",
    "com_sun_star_comp_framework_WindowStateConfiguration_get_implementation",
    "org_apache_openoffice_comp_framework_ContextChangeEventMultiplexer_get_implementation",
# i18npool/util/i18npool.component
    "com_sun_star_i18n_BreakIterator_get_implementation",
    "com_sun_star_i18n_BreakIterator_Unicode_get_implementation",
    "com_sun_star_i18n_CharacterClassification_get_implementation",
    "com_sun_star_i18n_CharacterClassification_Unicode_get_implementation",
    "com_sun_star_i18n_LocaleDataImpl_get_implementation",
    "com_sun_star_i18n_NumberFormatCodeMapper_get_implementation",
    "com_sun_star_i18n_Transliteration_get_implementation",
    "com_sun_star_i18n_Transliteration_IGNORE_CASE_get_implementation",
    "com_sun_star_i18n_Transliteration_IGNORE_KANA_get_implementation",
    "com_sun_star_i18n_Transliteration_IGNORE_WIDTH_get_implementation",
    "com_sun_star_text_DefaultNumberingProvider_get_implementation",
# sax/source/expatwrap/expwrap.component
    "com_sun_star_comp_extensions_xml_sax_FastParser_get_implementation",
    "com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation",
    "com_sun_star_extensions_xml_sax_Writer_get_implementation",
# sfx2/util/sfx.component
    "SfxDocumentMetaData_get_implementation",
    "com_sun_star_comp_office_FrameLoader_get_implementation",
    "com_sun_star_comp_sfx2_DocumentTemplates_get_implementation",
    "com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation",
# svtools/util/svt.component
    "com_sun_star_graphic_GraphicObject_get_implementation",
    "com_sun_star_comp_graphic_GraphicProvider_get_implementation",
# svx/util/svx.component
    "com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation",
# svx/util/svxcore.component
    "com_sun_star_comp_graphic_PrimitiveFactory2D_get_implementation",
    "com_sun_star_comp_Draw_GraphicExporter_get_implementation",
    "com_sun_star_comp_Svx_GraphicExportHelper_get_implementation",
    "com_sun_star_comp_Svx_GraphicImportHelper_get_implementation",
# stoc/source/inspect/introspection.component
    "com_sun_star_comp_stoc_Introspection_get_implementation",
# toolkit/util/tk.component
    "stardiv_Toolkit_UnoControlContainer_get_implementation",
    "stardiv_Toolkit_UnoControlContainerModel_get_implementation",
    "stardiv_Toolkit_VCLXToolkit_get_implementation",
# uui/util/uui.component
    "com_sun_star_comp_uui_UUIInteractionHandler_get_implementation",
# xmloff/util/xo.component
    "XMLVersionListPersistence_get_implementation",
    ]

# edit group for apps, where you can edit documents
edit_factory_list = [
    ]

edit_constructor_list = [
    "com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation",
    "com_sun_star_i18n_InputSequenceChecker_get_implementation",
    "com_sun_star_i18n_OrdinalSuffix_get_implementation",
    ]

calc_factory_list = [
    ("libscdlo.a", "scd_component_getFactory"),
    ("libscfiltlo.a", "scfilt_component_getFactory"),
    ("libsclo.a", "sc_component_getFactory"),
    ]

calc_constructor_list = [
    ]

draw_factory_list = [
    ("libsddlo.a", "sdd_component_getFactory"),
    ("libsdlo.a", "sd_component_getFactory"),
    ("libsvgfilterlo.a", "svgfilter_component_getFactory"),
    ("libwpftdrawlo.a", "wpftdraw_component_getFactory"),
    ]

draw_constructor_list = [
    ]

writer_factory_list = [
    ("libsblo.a", "sb_component_getFactory"),
    ("libswdlo.a", "swd_component_getFactory"),
    ("libswlo.a", "sw_component_getFactory"),
    ("libwriterfilterlo.a", "writerfilter_component_getFactory"),
    ]

writer_constructor_list = [
    ]

factory_map = {
    'core' : core_factory_list,
    'edit' : edit_factory_list,
    'calc' : calc_factory_list,
    'draw' : draw_factory_list,
    'writer' : writer_factory_list,
    }

constructor_map = {
    'core' : core_constructor_list,
    'edit' : edit_constructor_list,
    'calc' : calc_constructor_list,
    'draw' : draw_constructor_list,
    'writer' : writer_constructor_list,
    }

opts = OptionParser()
opts.add_option("-j", "--java-guard", action="store_true", help="include external java functions", dest="java", default=False)
opts.add_option("-g", "--group", action="append", help="group of implementations to make available in application", dest="groups")

(options, args) = opts.parse_args()

print ("""/*
 * This is a generated file. Do not edit.
 *   file generated by solenv/bin/native-code.py
 */

#include <osl/detail/component-mapping.h>

extern "C" {
""")

if options.groups:
    for factory_group in options.groups:
        for (factory_name,factory_function) in factory_map[factory_group]:
            if factory_function == 'sb_component_getFactory':
                print ('#ifdef ANDROID')
            print ('void * '+factory_function+'( const char* , void* , void* );')
            if factory_function == 'sb_component_getFactory':
                print ('#endif')

print ('')
if options.groups:
    for constructor_group in options.groups:
        for constructor in constructor_map[constructor_group]:
            print ('void * '+constructor+'( void *, void * );')

print ("""
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {""")

if options.groups:
    for factory_group in options.groups:
        for (factory_name,factory_function) in factory_map[factory_group]:
            if factory_function == 'sb_component_getFactory':
                print ('#ifdef ANDROID')
            print ('        { "'+factory_name+'", '+factory_function+' },')
            if factory_function == 'sb_component_getFactory':
                print ('#endif')

print ("""
        { 0, 0 }
    };""")

if options.java:
    print ("""
    // Guard against possible function-level link-time pruning of
    // "unused" code. We need to pull these in, too, as they aren't in
    // any of the libs we link with -Wl,--whole-archive. Is this necessary?
    extern void Java_org_libreoffice_android_AppSupport_runMain();
    volatile void *p = (void *) Java_org_libreoffice_android_AppSupport_runMain;

    extern void Java_org_libreoffice_android_AppSupport_renderVCL();
    p = (void *) Java_org_libreoffice_android_AppSupport_renderVCL;""")

print ("""
    return map;
}

const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {""")

if options.groups:
    for constructor_group in options.groups:
        for constructor in constructor_map[constructor_group]:
            print ('        { "' +constructor+ '", ' +constructor+ ' },')

print ("""
        { 0, 0 }
    };

    return map;
}

}""")
