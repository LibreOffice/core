#!/usr/bin/python
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
from optparse import OptionParser

import re
import sys
import xml.etree.ElementTree as ET

# foo_component_getFactory functions are split into groups, so that you could
# choose e.g. 'core' and 'writer' functionality and through factory_map,
# relevant function sections will be referenced in lo_get_factory_map().
# That prevents garbage collector to ignore them as unused.

# The same groups are used for constructor based implementations
# referenced in lo_get_constructor_map().

core_factory_list = [
    ("libembobj.a", "embobj_component_getFactory"),
    ("libevtattlo.a", "evtatt_component_getFactory"),
    ("libreflectionlo.a", "reflection_component_getFactory"),
    ("libcomphelper.a", "comphelp_component_getFactory"),
    ("libconfigmgrlo.a", "configmgr_component_getFactory"),
    ("libdrawinglayerlo.a", "drawinglayer_component_getFactory"),
    ("libfilterconfiglo.a", "filterconfig1_component_getFactory"),
    ("libfsstoragelo.a", "fsstorage_component_getFactory"),
    ("libhyphenlo.a", "hyphen_component_getFactory"),
    ("libi18npoollo.a", "i18npool_component_getFactory"),
    ("libi18nsearchlo.a", "i18nsearch_component_getFactory"),
    ("libinvocadaptlo.a", "invocadapt_component_getFactory"),
    ("liblnglo.a", "lng_component_getFactory"),
    ("liblnthlo.a", "lnth_component_getFactory"),
    ("liblocalebe1lo.a", "localebe1_component_getFactory"),
    ("libooxlo.a", "oox_component_getFactory"),
    ("libpackage2.a", "package2_component_getFactory"),
    ("libslideshowlo.a", "slideshow_component_getFactory"),
    ("libsmlo.a", "sm_component_getFactory"),
    ("libsotlo.a", "sot_component_getFactory"),
    ("libspelllo.a", "spell_component_getFactory"),
    ("libsrtrs1.a", "srtrs1_component_getFactory"),
    ("libstoragefdlo.a", "storagefd_component_getFactory"),
    ("libucb1.a", "ucb_component_getFactory"),
    ("libucpfile1.a", "ucpfile_component_getFactory"),
    ("libucphier1.a", "ucphier1_component_getFactory"),
    ("libucptdoc1lo.a", "ucptdoc1_component_getFactory"),
    ("libunordflo.a", "unordf_component_getFactory"),
    ("libunoxmllo.a", "unoxml_component_getFactory"),
    ("libutllo.a", "utl_component_getFactory"),
    ("libxmlsecurity.a", "xmlsecurity_component_getFactory"),
    ("libxoflo.a", "xof_component_getFactory"),
    ("libxolo.a", "xo_component_getFactory"),
    ("libxsec_xmlsec.a", "xsec_xmlsec_component_getFactory", "#ifndef ANDROID"),
    ("libxstor.a", "xstor_component_getFactory"),
    ("libcanvasfactorylo.a", "canvasfactory_component_getFactory"),
    ("libvbaeventslo.a", "vbaevents_component_getFactory", "#if HAVE_FEATURE_SCRIPTING"),
    ("libvclcanvaslo.a", "vclcanvas_component_getFactory"),
    ("libmtfrendererlo.a", "mtfrenderer_component_getFactory"),
    ]

core_constructor_list = [
# chart2/source/chartcore.component
    "com_sun_star_comp_chart_AreaChartType_get_implementation",
    "com_sun_star_comp_chart_BarChartType_get_implementation",
    "com_sun_star_comp_chart_BubbleChartType_get_implementation",
    "com_sun_star_comp_chart_CandleStickChartType_get_implementation",
    "com_sun_star_comp_chart_ChartTypeManager_get_implementation",
    "com_sun_star_comp_chart_ColumnChartType_get_implementation",
    "com_sun_star_comp_chart_DataSeries_get_implementation",
    "com_sun_star_comp_chart_FilledNetChartType_get_implementation",
    "com_sun_star_comp_chart_GL3DBarChartType_get_implementation",
    "com_sun_star_comp_chart_FormattedString_get_implementation",
    "com_sun_star_comp_chart_LineChartType_get_implementation",
    "com_sun_star_comp_chart_NetChartType_get_implementation",
    "com_sun_star_comp_chart_PieChartType_get_implementation",
    "com_sun_star_comp_chart_ScatterChartType_get_implementation",
    "com_sun_star_comp_chart2_Axis_get_implementation",
    "com_sun_star_comp_chart2_ChartModel_get_implementation",
    "com_sun_star_comp_chart2_Diagram_get_implementation",
    "com_sun_star_comp_chart2_LabeledDataSequence_get_implementation",
    "com_sun_star_comp_chart2_Legend_get_implementation",
    "com_sun_star_comp_chart2_LinearRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_Title_get_implementation",
    "com_sun_star_comp_chart2_XMLFilter_get_implementation",
# chart2/source/controller/chartcontroller.component
    "com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation",
# dbaccess/util/dba.component
    "com_sun_star_comp_dba_ORowSet_get_implementation",
# forms/util/frm.component
    "com_sun_star_comp_forms_FormOperations_get_implementation",
    "com_sun_star_comp_forms_ODatabaseForm_get_implementation",
    "com_sun_star_comp_forms_OFormattedFieldWrapper_ForcedFormatted_get_implementation",
    "com_sun_star_comp_forms_ORichTextModel_get_implementation",
    "com_sun_star_form_OCheckBoxControl_get_implementation",
    "com_sun_star_form_OCheckBoxModel_get_implementation",
    "com_sun_star_form_OComboBoxControl_get_implementation",
    "com_sun_star_form_OComboBoxModel_get_implementation",
    "com_sun_star_form_ODateControl_get_implementation",
    "com_sun_star_form_ODateModel_get_implementation",
    "com_sun_star_form_OEditControl_get_implementation",
    "com_sun_star_form_OEditModel_get_implementation",
    "com_sun_star_form_OFormsCollection_get_implementation",
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
    "com_sun_star_comp_framework_ModuleAcceleratorConfiguration_get_implementation",
    "org_apache_openoffice_comp_framework_ContextChangeEventMultiplexer_get_implementation",
# i18npool/util/i18npool.component
    "com_sun_star_i18n_BreakIterator_get_implementation",
    "com_sun_star_i18n_BreakIterator_Unicode_get_implementation",
    "com_sun_star_i18n_CharacterClassification_get_implementation",
    "com_sun_star_i18n_CharacterClassification_Unicode_get_implementation",
    "com_sun_star_i18n_Collator_get_implementation",
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
    "com_sun_star_comp_sfx2_AppDispatchProvider_get_implementation",
    "com_sun_star_comp_sfx2_DocumentTemplates_get_implementation",
    "com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation",
# stoc/source/inspect/introspection.component
    "com_sun_star_comp_stoc_Introspection_get_implementation",
# stoc/util/stocservices.component
    "com_sun_star_comp_stoc_TypeConverter_get_implementation",
    "com_sun_star_comp_uri_ExternalUriReferenceTranslator_get_implementation",
    "com_sun_star_comp_uri_UriReferenceFactory_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTexpand_get_implementation",
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
# toolkit/util/tk.component
    "stardiv_Toolkit_StdTabController_get_implementation",
    "stardiv_Toolkit_UnoCheckBoxControl_get_implementation",
    "stardiv_Toolkit_UnoComboBoxControl_get_implementation",
    "stardiv_Toolkit_UnoControlCheckBoxModel_get_implementation",
    "stardiv_Toolkit_UnoControlComboBoxModel_get_implementation",
    "stardiv_Toolkit_UnoControlContainer_get_implementation",
    "stardiv_Toolkit_UnoControlContainerModel_get_implementation",
    "stardiv_Toolkit_UnoDateFieldControl_get_implementation",
    "stardiv_Toolkit_UnoControlDateFieldModel_get_implementation",
    "stardiv_Toolkit_VCLXPointer_get_implementation",
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
    ("libavmedialo.a", "avmedia_component_getFactory"),
    ("libanalysislo.a", "analysis_component_getFactory"),
    ("libdatelo.a", "date_component_getFactory"),
    ("libpricinglo.a", "pricing_component_getFactory"),
    ("libsvllo.a", "svl_component_getFactory"),
    ]

calc_constructor_list = [
    ]

draw_factory_list = [
    ("libsddlo.a", "sdd_component_getFactory"),
    ("libsdlo.a", "sd_component_getFactory"),
    ("libsvgfilterlo.a", "svgfilter_component_getFactory"),
    ("libdeployment.a", "deployment_component_getFactory"),
    ("libemboleobj.a", "emboleobj_component_getFactory"),
    ("libanimcorelo.a", "animcore_component_getFactory"),
    ]

draw_constructor_list = [
    ]

writer_factory_list = [
    ("libswlo.a", "sw_component_getFactory"),
    ]

writer_constructor_list = [
# basic/util/sb.component
    ("com_sun_star_comp_sfx2_ScriptLibraryContainer_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# filter/source/textfilterdetect/textfd.component 
    "com_sun_star_comp_filters_PlainTextFilterDetect_get_implementation",
# sw/util/sw.component
    "com_sun_star_comp_Writer_XMLOasisContentImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisMetaImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisSettingsImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisStylesImporter_get_implementation",
# sw/util/swd.component
    "com_sun_star_comp_writer_FormatDetector_get_implementation",
# writerfilter/util/writerfilter.component
    "com_sun_star_comp_Writer_RtfFilter_get_implementation",
    "com_sun_star_comp_Writer_WriterFilter_get_implementation",
    "com_sun_star_comp_Writer_WriterFilterDetector_get_implementation",
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

def get_constructor_guard(constructor):
    if type(full_constructor_map[constructor]) is bool:
        return None
    else:
        return full_constructor_map[constructor]

# instead of outputting native-code.cxx, reduce the services.rdb according to
# the constraints, so that we can easily emulate what services do we need to
# add for a fully functional file loading / saving / ...
def limit_rdb(services_rdb, full_factory_map, full_constructor_map):
    ET.register_namespace('','http://openoffice.org/2010/uno-components')
    tree = ET.parse(services_rdb[0])
    root = tree.getroot()

    for component in root.findall('{http://openoffice.org/2010/uno-components}component'):
        # direct
        uri = component.get('uri')
        component_name = None
        if uri != None:
            component_name = re.sub('^vnd.sun.star.expand:\$LO_LIB_DIR/([^.]*).so$', '\\1.a', uri)
        if component_name in full_factory_map:
            continue

        # via a constructor - limit only to those we have
        has_constructor = False
        for implementation in component.findall('{http://openoffice.org/2010/uno-components}implementation'):
            constructor = implementation.get('constructor')
            if constructor in full_constructor_map:
                has_constructor = True
            else:
                component.remove(implementation)

        if not has_constructor:
            root.remove(component)

    tree.write(services_rdb[0] + '.out', xml_declaration = True, method = 'xml')


# do the actual work
opts = OptionParser()
opts.add_option("-j", "--java-guard", action="store_true", help="include external java functions", dest="java", default=False)
opts.add_option("-g", "--group", action="append", help="group of implementations to make available in application", dest="groups")
opts.add_option("-r", "--limit-rdb", action="append", help="instead of outputting native-code.cxx, limit the services.rdb only to the services defined by the groups", dest="services")

(options, args) = opts.parse_args()

# dict of all the contructors that we need according to -g's
full_constructor_map = {}
if options.groups:
    for constructor_group in options.groups:
        for constructor in constructor_map[constructor_group]:
            if type(constructor) is tuple:
                full_constructor_map[constructor[0]] = constructor[1]
            else:
                full_constructor_map[constructor] = True

# dict of all the factories that we need according to -g's
full_factory_map = {}
if options.groups:
    for factory_group in options.groups:
        for entry in factory_map[factory_group]:
            factory_guard = None
            if len(entry) > 2:
                factory_guard = entry[2]
            map_entry = { 'function': entry[1], 'guard': factory_guard }
            full_factory_map[entry[0]] = map_entry

# are we only shuffling the services.rdb?
if options.services:
    limit_rdb(options.services, full_factory_map, full_constructor_map)
    exit(0)

print ("""/*
 * This is a generated file. Do not edit.
 *   file generated by solenv/bin/native-code.py
 */

#include <osl/detail/component-mapping.h>

extern "C" {
""")

for entry in sorted(full_factory_map.keys()):
    factory_function = full_factory_map[entry]['function']
    factory_guard = full_factory_map[entry]['guard']
    if factory_guard:
        print (factory_guard)
    print('void * ' + factory_function + '( const char* , void* , void* );')
    if factory_guard:
        print ('#endif')

print ('')
for constructor in sorted(full_constructor_map.keys()):
    constructor_guard = get_constructor_guard(constructor)
    if constructor_guard:
        print (constructor_guard)
    print ('void * '+constructor+'( void *, void * );')
    if constructor_guard:
        print ('#endif')

print ("""
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {""")

for entry in sorted(full_factory_map.keys()):
    factory_function = full_factory_map[entry]['function']
    factory_guard = full_factory_map[entry]['guard']
    if factory_guard:
        print (factory_guard)
    print('        { "' + entry + '", ' + factory_function + ' },')
    if factory_guard:
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
    p = (void *) Java_org_libreoffice_android_AppSupport_renderVCL;

    extern void Java_org_libreoffice_kit_LibreOfficeKit_initializeNative();
    p = (void *) Java_org_libreoffice_kit_LibreOfficeKit_initializeNative;

    extern void Java_org_libreoffice_kit_Office_getError();
    p = (void *) Java_org_libreoffice_kit_Office_getError;

    """)

print ("""
    return map;
}

const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {""")

for constructor in sorted(full_constructor_map.keys()):
    constructor_guard = get_constructor_guard(constructor)
    if constructor_guard:
        print (constructor_guard)
    print ('        { "' +constructor+ '", ' +constructor+ ' },')
    if constructor_guard:
        print ('#endif')

print ("""
        { 0, 0 }
    };

    return map;
}

}""")

# vim:set shiftwidth=4 softtabstop=4 expandtab:
