#!/usr/bin/env python3
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    ("libi18npoollo.a", "i18npool_component_getFactory"),
    ("libvcllo.a", "vcl_component_getFactory"),
    ("libsvtlo.a", "svt_component_getFactory"),
    ]

core_constructor_list = [
# animations/source/animcore/animcore.component
    "com_sun_star_animations_AnimatePhysics_get_implementation",
# basic/util/sb.component
    ("com_sun_star_comp_sfx2_DialogLibraryContainer_get_implementation","#if HAVE_FEATURE_SCRIPTING"),
    ("com_sun_star_comp_sfx2_ScriptLibraryContainer_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# UnoControls/util/ctl.component
    "stardiv_UnoControls_FrameControl_get_implementation",
    "stardiv_UnoControls_ProgressBar_get_implementation",
    "stardiv_UnoControls_ProgressMonitor_get_implementation",
    "stardiv_UnoControls_StatusIndicator_get_implementation",
# canvas/source/factory/canvasfactory.component
    ("com_sun_star_comp_rendering_CanvasFactory_get_implementation", "#ifndef ENABLE_WASM_STRIP_CANVAS"),
# canvas/source/vcl/vclcanvas.component
    ("com_sun_star_comp_rendering_Canvas_VCL_get_implementation", "#ifndef ENABLE_WASM_STRIP_CANVAS"),
    ("com_sun_star_comp_rendering_SpriteCanvas_VCL_get_implementation", "#ifndef ENABLE_WASM_STRIP_CANVAS"),
# chart2/source/chartcore.component
    ("com_sun_star_chart2_ExponentialScaling_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_chart2_LinearScaling_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_chart2_LogarithmicScaling_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_chart2_PowerScaling_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_AreaChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_BarChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_BubbleChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_CachedDataSequence_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_CandleStickChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_ChartTypeManager_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_ColumnChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_DataSeries_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_DataSource_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_FilledNetChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_FormattedString_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_InternalDataProvider_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_LineChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_NetChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_PieChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart_ScatterChartType_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_Axis_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_CartesianCoordinateSystem2d_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_CartesianCoordinateSystem3d_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ChartController_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ChartModel_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ChartView_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ConfigDefaultColorScheme_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_Diagram_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ErrorBar_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ExponentialRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_GridProperties_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_LabeledDataSequence_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_Legend_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_LinearRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_LogarithmicRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_MeanValueRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_MovingAverageRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_PageBackground_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_PolarCoordinateSystem2d_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_PolarCoordinateSystem3d_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_PolynomialRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_PotentialRegressionCurve_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_RegressionEquation_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_Title_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_XMLFilter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
# chart2/source/controller/chartcontroller.component
    ("com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_ChartFrameLoader_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_chart2_WizardDialog_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("org_libreoffice_chart2_Chart2ToolboxController", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("org_libreoffice_comp_chart2_sidebar_ChartPanelFactory", "#ifndef ENABLE_WASM_STRIP_CHART"),
# comphelper/util/comphelp.component
    "com_sun_star_comp_MemoryStream",
    "com_sun_star_comp_task_OfficeRestartManager",
    "AnyCompareFactory_get_implementation",
    "IndexedPropertyValuesContainer_get_implementation",
    "NamedPropertyValuesContainer_get_implementation",
    "com_sun_star_comp_comphelper_OPropertyBag",
    "com_sun_star_comp_SequenceInputStreamService",
    "com_sun_star_comp_SequenceOutputStreamService",
    "com_sun_star_comp_util_OfficeInstallationDirectories",
    "org_openoffice_comp_comphelper_EnumerableMap",
# configmgr/source/configmgr.component
    "com_sun_star_comp_configuration_ConfigurationProvider_get_implementation",
    "com_sun_star_comp_configuration_ConfigurationRegistry_get_implementation",
    "com_sun_star_comp_configuration_DefaultProvider_get_implementation",
    "com_sun_star_comp_configuration_ReadOnlyAccess_get_implementation",
    "com_sun_star_comp_configuration_ReadWriteAccess_get_implementation",
    "com_sun_star_comp_configuration_Update_get_implementation",
# connectivity/source/manager/sdbc2.component
    ("connectivity_OSDBCDriverManager_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
# connectivity/source/drivers/dbase/dbase.component
    ("connectivity_dbase_ODriver", "#if HAVE_FEATURE_DBCONNECTIVITY"),
# cppcanvas/source/uno/mtfrenderer.component
    ("com_sun_star_comp_rendering_MtfRenderer_get_implementation", "#ifndef ENABLE_WASM_STRIP_CANVAS"),
# cui/util/cui.component
    "com_sun_star_cui_ColorPicker_get_implementation",
# dbaccess/util/dba.component
    ("com_sun_star_comp_dba_DataAccessDescriptorFactory", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
    ("com_sun_star_comp_dba_OCommandDefinition", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
    ("com_sun_star_comp_dba_OComponentDefinition", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
    ("com_sun_star_comp_dba_ODatabaseContext_get_implementation", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
    ("com_sun_star_comp_dba_ODatabaseDocument", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
    ("com_sun_star_comp_dba_ODatabaseSource", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
    ("com_sun_star_comp_dba_ORowSet_get_implementation", "#ifndef ENABLE_WASM_STRIP_DBACCESS"),
# drawinglayer/drawinglayer.component
    "drawinglayer_XPrimitive2DRenderer",
# embeddedobj/util/embobj.component
    "embeddedobj_UNOEmbeddedObjectCreator_get_implementation",
    "embeddedobj_OOoEmbeddedObjectFactory_get_implementation",
    "embeddedobj_OOoSpecialEmbeddedObjectFactory_get_implementation",
# emfio/emfio.component
    "emfio_emfreader_XEmfParser_get_implementation",
# eventattacher/source/evtatt.component
    "eventattacher_EventAttacher",
# extensions/source/logging/log.component
    ("com_sun_star_comp_extensions_FileHandler", "#ifdef ANDROID"),
    ("com_sun_star_comp_extensions_LoggerPool", "#ifdef ANDROID"),
    ("com_sun_star_comp_extensions_PlainTextFormatter", "#ifdef ANDROID"),
    ("com_sun_star_comp_extensions_SimpleTextFormatter", "#ifdef ANDROID"),
# extensions/source/bibliography/bib.component
    "extensions_BibliographyLoader_get_implementation",
# filter/source/config/cache/filterconfig1.component
    "filter_ConfigFlush_get_implementation",
    "filter_TypeDetection_get_implementation",
    "filter_FrameLoaderFactory_get_implementation",
    "filter_FilterFactory_get_implementation",
    "filter_ContentHandlerFactory_get_implementation",
# filter/source/odfflatxml/odfflatxml.component
    "filter_OdfFlatXml_get_implementation",
# filter/source/pdf/pdffilter.component
    "filter_PdfDecomposer_get_implementation",
    "filter_PDFExportInteractionHandler_get_implementation",
    "filter_PDFFilter_get_implementation",
    "filter_PDFDialog_get_implementation",
# filter/source/xmlfilterdetect/xmlfd.component
    "filter_XMLFilterDetect_get_implementation",
# filter/source/xmlfilteradaptor/xmlfa.component
    "filter_XmlFilterAdaptor_get_implementation",
# filter/source/storagefilterdetect/storagefd.component
    "filter_StorageFilterDetect_get_implementation",
# forms/util/frm.component
    ("com_sun_star_comp_forms_FormOperations_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_comp_forms_ODatabaseForm_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_comp_forms_OFormattedFieldWrapper_ForcedFormatted_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_comp_form_ORichTextControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_comp_forms_ORichTextModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_comp_forms_OScrollBarModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_comp_forms_OSpinButtonModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_Model_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OButtonControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OButtonModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OCheckBoxControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OCheckBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OComboBoxControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OComboBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_ODateControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_ODateModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OEditControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OEditModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OFixedTextModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OFormsCollection_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OGridControlModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OGroupBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OListBoxControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_OListBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_ONumericModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_ORadioButtonControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_ORadioButtonModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
    ("com_sun_star_form_XForms_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS"),
# framework/util/fwk.component
    ("com_sun_star_comp_framework_AutoRecovery_get_implementation","#ifndef ENABLE_WASM_STRIP_RECOVERYUI"),
    "com_sun_star_comp_framework_Desktop_get_implementation",
    "com_sun_star_comp_framework_DocumentAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_Frame_get_implementation",
    "com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_JobExecutor_get_implementation",
    "com_sun_star_comp_framework_jobs_JobDispatch_get_implementation",
    "com_sun_star_comp_framework_LayoutManager_get_implementation",
    "com_sun_star_comp_framework_ModuleManager_get_implementation",
    "com_sun_star_comp_framework_ModuleUIConfigurationManager_get_implementation",
    "com_sun_star_comp_framework_ModuleUIConfigurationManagerSupplier_get_implementation",
    "com_sun_star_comp_framework_PathSettings_get_implementation",
    "com_sun_star_comp_framework_PathSubstitution_get_implementation",
    "com_sun_star_comp_framework_ObjectMenuController_get_implementation",
    "com_sun_star_comp_framework_PopupMenuControllerFactory_get_implementation",
    "com_sun_star_comp_framework_ControlMenuController_get_implementation",
    "com_sun_star_comp_framework_ThesaurusMenuController_get_implementation",
    "com_sun_star_comp_framework_ToolbarAsMenuController_get_implementation",
    "com_sun_star_comp_framework_ResourceMenuController_get_implementation",
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
    "com_sun_star_i18n_NativeNumberSupplier_get_implementation",
    "com_sun_star_i18n_NumberFormatCodeMapper_get_implementation",
    "com_sun_star_i18n_Transliteration_get_implementation",
    "com_sun_star_i18n_Transliteration_IGNORE_CASE_get_implementation",
    "com_sun_star_i18n_Transliteration_IGNORE_KANA_get_implementation",
    "com_sun_star_i18n_Transliteration_IGNORE_WIDTH_get_implementation",
    "com_sun_star_text_DefaultNumberingProvider_get_implementation",
    ("i18npool_BreakIterator_ja_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_BreakIterator_ko_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ko"),
    ("i18npool_BreakIterator_th_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_th"),
    ("i18npool_BreakIterator_zh_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    ("i18npool_BreakIterator_zh_TW_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    "i18npool_CalendarImpl_get_implementation",
    "i18npool_Calendar_ROC_get_implementation",
    "i18npool_Calendar_dangi_get_implementation",
    "i18npool_Calendar_buddhist_get_implementation",
    "i18npool_Calendar_gengou_get_implementation",
    "i18npool_Calendar_gregorian_get_implementation",
    "i18npool_Calendar_hanja_get_implementation",
    "i18npool_Calendar_hanja_yoil_get_implementation",
    "i18npool_Calendar_hijri_get_implementation",
    "i18npool_Calendar_jewish_get_implementation",
    "i18npool_ChapterCollator_get_implementation",
    "i18npool_Collator_Unicode_get_implementation",
    "i18npool_IndexEntrySupplier_get_implementation",
    "i18npool_IndexEntrySupplier_Unicode_get_implementation",
    "i18npool_IndexEntrySupplier_asian_get_implementation",
    ("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_InputSequenceChecker_hi_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_hi"),
    ("i18npool_InputSequenceChecker_th_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_th"),
    ("i18npool_TextConversion_ko_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ko"),
    ("i18npool_TextConversion_zh_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    "i18npool_CharToNumEastIndic_ar_get_implementation",
    ("i18npool_CharToNumFullwidth_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_CharToNumHangul_ko_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ko"),
    "i18npool_CharToNumIndic_ar_get_implementation",
    ("i18npool_CharToNumIndic_hi_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_hi"),
    ("i18npool_CharToNumKanjiShort_ja_JP_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_CharToNumKanjiTraditional_ja_JP_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ja"),
    ("i18npool_CharToNumLower_ko_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ko"),
    ("i18npool_CharToNumUpper_ko_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_ko"),
    ("i18npool_CharToNum_th_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_th"),
    ("i18npool_NumToTextFullwidth_zh_CN_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    ("i18npool_NumToTextFullwidth_zh_TW_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    ("i18npool_NumToTextLower_zh_CN_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    ("i18npool_NumToTextLower_zh_TW_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    ("i18npool_NumToTextUpper_zh_CN_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
    ("i18npool_NumToTextUpper_zh_TW_get_implementation", "#if WITH_LOCALE_ALL || WITH_LOCALE_zh"),
# i18nsearch/sourceh/search/i18nsearch.component
    "i18npool_TextSearch_get_implementation",
# io/source/io.component
    "io_Pump_get_implementation",
    "io_ODataInputStream_get_implementation",
    "io_ODataOutputStream_get_implementation",
    "io_OMarkableInputStream_get_implementation",
    "io_OMarkableOutputStream_get_implementation",
    "io_OObjectInputStream_get_implementation",
    "io_OObjectOutputStream_get_implementation",
    "io_OPipeImpl_get_implementation",
    "io_OAcceptor_get_implementation",
    "io_OConnector_get_implementation",
    "io_OTextInputStream_get_implementation",
    "io_OTextOutputStream_get_implementation",
# linguistic/source/lng.component
    "linguistic_ConvDicList_get_implementation",
    "linguistic_DicList_get_implementation",
    "linguistic_LinguProps_get_implementation",
    "linguistic_LngSvcMgr_get_implementation",
    "linguistic_GrammarCheckingIterator_get_implementation",
# linguistic/source/spellcheck/MacOSXSpellMacOSXSpell.component
    ("lingucomponent_MacSpellChecker_get_implementation", "#ifdef IOS"),
# lingucomponent/source/thesaurus/libnth/lnth.component
    ("lingucomponent_Thesaurus_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    ("lingucomponent_SpellChecker_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    ("lingucomponent_LangGuess_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    ("lingucomponent_Hyphenator_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
# package/source/xstor/xstor.component
    "package_OStorageFactory_get_implementation",
# package/util/package2.component
    "package_OZipFileAccess_get_implementation",
    "package_ZipPackage_get_implementation",
    "package_ManifestReader_get_implementation",
    "package_ManifestWriter_get_implementation",
# sax/source/expatwrap/expwrap.component
    "com_sun_star_comp_extensions_xml_sax_FastParser_get_implementation",
    "com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation",
    "com_sun_star_extensions_xml_sax_Writer_get_implementation",
# scripting/util/scriptframe.component
    ("scripting_BrowseNodeFactoryImpl_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("scripting_MasterScriptProvider_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("scripting_MasterScriptProviderFactory_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("scripting_ScriptingFrameworkURIHelper_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# scripting/source/basprov/basprov.component
    ("scripting_BasicProviderImpl_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# sfx2/util/sfx.component
    "SfxDocumentMetaData_get_implementation",
    "com_sun_star_comp_office_FrameLoader_get_implementation",
    "com_sun_star_comp_sfx2_AppDispatchProvider_get_implementation",
    "com_sun_star_comp_sfx2_DocumentTemplates_get_implementation",
    "com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation",
# shell/source/backends/localebe/localebe1.component
    "shell_LocaleBackend_get_implementation",
# sot/util/sot.component
    "com_sun_star_comp_embed_OLESimpleStorage",
# stoc/source/inspect/introspection.component
    "com_sun_star_comp_stoc_Introspection_get_implementation",
# stoc/source/invocation_adapter/invocadapt.component
    "stoc_invocation_adapter_get_implementation",
# stoc/source/corereflection/reflection.component
    "com_sun_star_comp_stoc_CoreReflection_get_implementation",
# stoc/source/proxy_factory/proxyfac.component
    "stoc_FactoryImpl_get_implementation",
# stoc/util/stocservices.component
    "com_sun_star_comp_stoc_OServiceManagerWrapper_get_implementation",
    "com_sun_star_comp_stoc_TypeConverter_get_implementation",
    "com_sun_star_comp_uri_ExternalUriReferenceTranslator_get_implementation",
    "com_sun_star_comp_uri_UriReferenceFactory_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTexpand_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTscript_get_implementation",
# starmath/util/sm.component
    "Math_FormulaDocument_get_implementation",
    "Math_XMLContentExporter_get_implementation",
    "Math_XMLExporter_get_implementation",
    "Math_XMLImporter_get_implementation",
    "Math_XMLMetaExporter_get_implementation",
    "Math_XMLOasisMetaExporter_get_implementation",
    "Math_XMLOasisMetaImporter_get_implementation",
    "Math_XMLOasisSettingsExporter_get_implementation",
    "Math_XMLOasisSettingsImporter_get_implementation",
    "Math_XMLSettingsExporter_get_implementation",
    "com_sun_star_comp_Math_MathTypeFilter_get_implementation",
# svl/source/fsstor/fsstorage.component
    "svl_FSStorageFactory_get_implementation",
# vcl/vcl.android.component
    "com_sun_star_graphic_GraphicObject_get_implementation",
    "com_sun_star_comp_graphic_GraphicMapper_get_implementation",
    "com_sun_star_comp_graphic_GraphicProvider_get_implementation",
    "com_sun_star_frame_VCLSessionManagerClient_get_implementation",
    "vcl_FontIdentificator_get_implementation",
# svgio/svgio.component
    "svgio_XSvgParser_get_implementation",
# svx/util/svx.component
    "com_sun_star_comp_svx_CharacterSpacingToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_CTLToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_LineSpacingToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_SmartTagMenuController_get_implementation",
    "com_sun_star_comp_svx_UnderlineToolBoxControl_get_implementation",
    "com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation",
    "com_sun_star_drawing_SvxShapeCollection_get_implementation",
    "com_sun_star_svx_FontHeightToolBoxController_get_implementation",
    "org_apache_openoffice_comp_svx_sidebar_PanelFactory_get_implementation",
# svx/util/svxcore.component
    "com_sun_star_comp_graphic_PrimitiveFactory2D_get_implementation",
    "com_sun_star_comp_Draw_GraphicExporter_get_implementation",
    "com_sun_star_comp_svx_ColorToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_FontNameToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_FrameToolBoxControl_get_implementation",
    "com_sun_star_comp_Svx_GraphicExportHelper_get_implementation",
    "com_sun_star_comp_Svx_GraphicImportHelper_get_implementation",
    "com_sun_star_comp_svx_LineEndToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_LineStyleToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_StyleToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_StylesPreviewToolBoxControl_get_implementation",
# toolkit/util/tk.component
    "com_sun_star_comp_embed_HatchWindowFactory_get_implementation",
    "stardiv_Toolkit_StdTabController_get_implementation",
    "stardiv_Toolkit_UnoButtonControl_get_implementation",
    "stardiv_Toolkit_UnoCheckBoxControl_get_implementation",
    "stardiv_Toolkit_UnoComboBoxControl_get_implementation",
    "stardiv_Toolkit_UnoControlButtonModel_get_implementation",
    "stardiv_Toolkit_UnoControlCheckBoxModel_get_implementation",
    "stardiv_Toolkit_UnoControlComboBoxModel_get_implementation",
    "stardiv_Toolkit_UnoControlContainer_get_implementation",
    "stardiv_Toolkit_UnoControlContainerModel_get_implementation",
    "stardiv_Toolkit_UnoControlDateFieldModel_get_implementation",
    "stardiv_Toolkit_UnoControlDialogModel_get_implementation",
    "stardiv_Toolkit_UnoControlFixedTextModel_get_implementation",
    "stardiv_Toolkit_UnoControlFormattedFieldModel_get_implementation",
    "stardiv_Toolkit_UnoControlGroupBoxModel_get_implementation",
    "stardiv_Toolkit_UnoControlListBoxModel_get_implementation",
    "stardiv_Toolkit_UnoControlNumericFieldModel_get_implementation",
    "stardiv_Toolkit_UnoControlRadioButtonModel_get_implementation",
    "stardiv_Toolkit_UnoControlScrollBarModel_get_implementation",
    "stardiv_Toolkit_UnoDateFieldControl_get_implementation",
    "stardiv_Toolkit_UnoGroupBoxControl_get_implementation",
    "stardiv_Toolkit_UnoListBoxControl_get_implementation",
    "stardiv_Toolkit_UnoRadioButtonControl_get_implementation",
    "stardiv_Toolkit_UnoSpinButtonModel_get_implementation",
    "stardiv_Toolkit_VCLXPointer_get_implementation",
    "stardiv_Toolkit_VCLXPopupMenu_get_implementation",
    "stardiv_Toolkit_VCLXToolkit_get_implementation",
# ucb/source/core/ucb1.component
    "ucb_UcbCommandEnvironment_get_implementation",
    "ucb_UcbContentProviderProxyFactory_get_implementation",
    "ucb_UcbPropertiesManager_get_implementation",
    "ucb_UcbStore_get_implementation",
    "ucb_UniversalContentBroker_get_implementation",
    "ucb_OFileAccess_get_implementation",
# ucb/source/ucp/file/ucpfile1.component
    "ucb_file_FileProvider_get_implementation",
# ucb/source/ucp/expand/ucpexpand1.component
    ("ucb_expand_ExpandContentProviderImpl_get_implementation", "#ifdef ANDROID"),
# ucb/source/sorter/srtrs1.component
    "ucb_SortedDynamicResultSetFactory_get_implementation",
# ucb/source/tdoc/ucptdoc1.component
    "ucb_tdoc_ContentProvider_get_implementation",
    "ucb_tdoc_DocumentContentFactory_get_implementation",
# ucb/source/ucp/ucphier1.component
    "ucb_HierarchyContentProvider_get_implementation",
    "ucb_HierarchyDataSource_get_implementation",
# ucb/source/ucp/package/ucppkg1
    "ucb_package_ContentProvider_get_implementation",
# unotools/util/utl.component
    "unotools_ServiceDocument_get_implementation",
    "unotools_OTempFileService_get_implementation",
# unoxml/source/rdf/unordf.component
    "unoxml_rdfRepository_get_implementation",
    "unoxml_CURI_get_implementation",
    "unoxml_CLiteral_get_implementation",
    "unoxml_CBlankNode_get_implementation",
# unoxml/source/service/unoxml.component
    "unoxml_CXPathAPI_get_implementation",
    "unoxml_CSAXDocumentBuilder_get_implementation",
    "unoxml_CDocumentBuilder_get_implementation",
# uui/util/uui.component
    "com_sun_star_comp_uui_UUIInteractionHandler_get_implementation",
    "com_sun_star_comp_uui_UUIInteractionRequestStringResolver_get_implementation",
# vcl/*.component
    "dtrans_CMimeContentTypeFactory_get_implementation",
    "vcl_SystemClipboard_get_implementation",
# xmloff/source/transform/xof.component
    ("xmloff_XMLCalcContentImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLCalcImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLCalcMetaImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLCalcSettingsImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLCalcStylesImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLChartContentImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLChartImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLChartStylesImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLDrawContentImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLDrawImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLDrawMetaImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLDrawSettingsImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLDrawStylesImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLImpressContentImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLImpressImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLImpressMetaImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLImpressSettingsImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLImpressStylesImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLMathMetaImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLMathSettingsImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_OOo2OasisTransformer_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_Oasis2OOoTransformer_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLAutoTextEventImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLWriterContentImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLWriterImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLWriterMetaImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLWriterSettingsImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLWriterStylesImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("xmloff_XMLMetaImportOOO_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
# xmloff/util/xo.component
    "XMLMetaExportComponent_get_implementation",
    "XMLMetaExportOOo_get_implementation",
    "XMLMetaImportComponent_get_implementation",
    "XMLVersionListPersistence_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisImporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisStylesExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisContentExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisMetaExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisMetaImporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisContentImporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisStylesImporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisSettingsExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisSettingsImporter_get_implementation",
    "com_sun_star_comp_Impress_XMLExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisSettingsExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisMetaExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisContentExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisStylesExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisExporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisImporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisStylesImporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisContentImporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisMetaImporter_get_implementation",
    "com_sun_star_comp_Draw_XMLOasisSettingsImporter_get_implementation",
    "com_sun_star_comp_Xmloff_AnimationsImport",
    "com_sun_star_comp_DrawingLayer_XMLExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLClipboardExporter_get_implementation",
    ("com_sun_star_comp_Chart_XMLOasisImporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisMetaImporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisMetaExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisStylesExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisContentExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLStylesExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLContentExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisStylesImporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisContentImporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    ("com_sun_star_comp_Chart_XMLOasisExporter_get_implementation", "#ifndef ENABLE_WASM_STRIP_CHART"),
    "com_sun_star_comp_Writer_XMLOasisAutotextEventsExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLAutotextEventsExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisAutotextEventsImporter_get_implementation",
# xmlscript/util/xmlscript.component
    "com_sun_star_comp_xml_input_SaxDocumentHandler_get_implementation",
    "com_sun_star_comp_xmlscript_XMLBasicExporter",
    "com_sun_star_comp_xmlscript_XMLOasisBasicExporter",
# xmlsecurity/util/xmlsecurity.component
    ("com_sun_star_security_CertificateContainer_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_security_DocumentDigitalSignatures_get_implementation", "#if HAVE_FEATURE_NSS"),
# xmlsecurity/util/xsec_xmlsec.component
    ("com_sun_star_xml_crypto_NSSInitializer_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_xml_crypto_SEInitializer_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_xml_security_SEInitializer_Gpg_get_implementation", "#if HAVE_FEATURE_GPGME"),
    ("com_sun_star_xml_crypto_SecurityEnvironment_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_xml_wrapper_XMLDocumentWrapper_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_xml_wrapper_XMLElementWrapper_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_xml_crypto_XMLSecurityContext_get_implementation", "#if HAVE_FEATURE_NSS"),
    ("com_sun_star_xml_crypto_XMLSignature_get_implementation", "#if HAVE_FEATURE_NSS"),
# oox/util/oox.component
    "com_sun_star_comp_oox_core_FastTokenHandler_get_implementation",
    "com_sun_star_comp_oox_FormatDetector_get_implementation",
    "com_sun_star_comp_oox_docprop_DocumentPropertiesImporter_get_implementation",
    "com_sun_star_comp_oox_ppt_PowerPointImport_get_implementation",
    "com_sun_star_comp_oox_crypto_StrongEncryptionDataSpace_get_implementation",
    ]

# edit group for apps, where you can edit documents
edit_factory_list = [
    ]

edit_constructor_list = [
# framework/util/fwk.component
    "com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_UICommandDescription_get_implementation",
# i18npool/util/i18npool.component
    "com_sun_star_i18n_InputSequenceChecker_get_implementation",
    "com_sun_star_i18n_OrdinalSuffix_get_implementation",
# sc/util/sc.component
    "Calc_XMLOasisContentExporter_get_implementation",
    "Calc_XMLOasisExporter_get_implementation",
    "Calc_XMLOasisMetaExporter_get_implementation",
    "Calc_XMLOasisSettingsExporter_get_implementation",
    "Calc_XMLOasisStylesExporter_get_implementation",
    "Calc_FilterOptionsDialog_get_implementation",
# starmath/util/sm.component
    "Math_XMLContentExporter_get_implementation",
    "Math_XMLOasisMetaExporter_get_implementation",
    "Math_XMLOasisSettingsExporter_get_implementation",
    "Math_XMLImporter_get_implementation",
    "Math_XMLOasisMetaImporter_get_implementation",
    "Math_XMLOasisSettingsImporter_get_implementation",
# starmath/util/smd.component
    "math_FormatDetector_get_implementation",
# sw/util/sw.component
    "com_sun_star_comp_Writer_XMLOasisContentExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisMetaExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisSettingsExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisStylesExporter_get_implementation",
    "com_sun_star_comp_Writer_WriterModule_get_implementation",
    "org_apache_openoffice_comp_sw_sidebar_SwPanelFactory_get_implementation",
    ]

# math
math_factory_list = [
    ]

math_constructor_list = [
# starmath/util/sm.component
    "Math_XMLOasisMetaExporter_get_implementation",
    "Math_XMLOasisSettingsExporter_get_implementation",
    ]

calc_factory_list = [
    ]

calc_constructor_list = [
# avmedia/util/avmedia.component
    ("com_sun_star_comp_framework_SoundHandler_get_implementation", "#if HAVE_FEATURE_AVMEDIA"),
# sc/util/sc.component
    "ScPanelFactory_get_implementation",
    "Calc_SpreadsheetDocument_get_implementation",
    "Calc_XMLOasisContentImporter_get_implementation",
    "Calc_XMLOasisImporter_get_implementation",
    "Calc_XMLOasisMetaImporter_get_implementation",
    "Calc_XMLOasisSettingsImporter_get_implementation",
    "Calc_XMLOasisStylesImporter_get_implementation",
    "Calc_ScSpreadsheetSettings_get_implementation",
# sc/util/scd.component
    "com_sun_star_comp_calc_ExcelBiffFormatDetector_get_implementation",
    "com_sun_star_comp_calc_FormatDetector_get_implementation",
# sc/util/scfilt.component
    "com_sun_star_comp_oox_xls_ExcelFilter_get_implementation",
    "com_sun_star_comp_oox_xls_FormulaParser_get_implementation",
# sc/util/vbaobj.component
    ("Calc_ScVbaApplication_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("ScVbaEventsHelper_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("ScVbaGlobals_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("Calc_ScVbaHyperlink_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("ScVbaTextFrame_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("Calc_ScVbaWindow_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("Calc_ScVbaWorkbook_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("Calc_ScVbaWorksheet_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("Calc_ScVbaRange_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# scaddins/source/analysis/analysis.component
    "scaddins_AnalysisAddIn_get_implementation",
# scaddins/source/datefunc/date.component
    "scaddins_ScaDateAddIn_get_implementation",
# scaddins/source/pricing/pricing.component
    "scaddins_ScaPricingAddIn_get_implementation",
# scripting/source/vbaevents/vbaevents.component
    ("ooo_vba_VBAToOOEventDesc_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# svl/util/svl.component
    "com_sun_star_uno_util_numbers_SvNumberFormatsSupplierServiceObject_get_implementation",
    "com_sun_star_uno_util_numbers_SvNumberFormatterServiceObject_get_implementation",
# scripting/source/vbaevents/vbaevents.component
    ("ooo_vba_VBAToOOEventDesc_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("ooo_vba_EventListener_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ]

draw_factory_list = [
    ]

draw_constructor_list = [
# animations/source/animcore/animcore.component
    "com_sun_star_animations_AnimateColor_get_implementation",
    "com_sun_star_animations_AnimateMotion_get_implementation",
    "com_sun_star_animations_AnimateSet_get_implementation",
    "com_sun_star_animations_AnimateTransform_get_implementation",
    "com_sun_star_animations_Animate_get_implementation",
    "com_sun_star_animations_Audio_get_implementation",
    "com_sun_star_animations_Command_get_implementation",
    "com_sun_star_animations_IterateContainer_get_implementation",
    "com_sun_star_animations_ParallelTimeContainer_get_implementation",
    "com_sun_star_animations_SequenceTimeContainer_get_implementation",
    "com_sun_star_animations_TransitionFilter_get_implementation",
# desktop/source/deployment/deployment.component
    ("desktop/source/deployment/deployment.component", "#ifndef ENABLE_WASM_STRIP_CLUCENE"),
    "com_sun_star_comp_deployment_ExtensionManager_get_implementation",
    "com_sun_star_comp_deployment_PackageInformationProvider_get_implementation",
    "com_sun_star_comp_deployment_PackageManagerFactory_get_implementation",
    "com_sun_star_comp_deployment_ProgressLog_get_implementation",
    ("com_sun_star_comp_deployment_component_PackageRegistryBackend_get_implementation", "#ifndef ENABLE_WASM_STRIP_CLUCENE"),
    "com_sun_star_comp_deployment_configuration_PackageRegistryBackend_get_implementation",
    "com_sun_star_comp_deployment_executable_PackageRegistryBackend_get_implementation",
    ("com_sun_star_comp_deployment_help_PackageRegistryBackend_get_implementation", "#ifndef ENABLE_WASM_STRIP_CLUCENE"),
    "com_sun_star_comp_deployment_script_PackageRegistryBackend_get_implementation",
    "com_sun_star_comp_deployment_sfwk_PackageRegistryBackend_get_implementation",
# embedded/source/msole/emboleobj.windows.component
    "embeddedobj_OleEmbeddedObjectFactory_get_implementation",
# filter/source/svg/svgfilter.component
    "filter_SVGFilter_get_implementation",
    "filter_SVGWriter_get_implementation",
# sd/util/sd.component
    "sd_DrawingDocument_get_implementation",
    "com_sun_star_comp_Draw_DrawingModule_get_implementation",
    "sd_PresentationDocument_get_implementation",
    "com_sun_star_comp_Draw_PresenterHelper_get_implementation",
    "com_sun_star_comp_Draw_PresenterPreviewCache_get_implementation",
    "com_sun_star_comp_Draw_PresenterTextView_get_implementation",
    "com_sun_star_comp_Draw_SlideRenderer_get_implementation",
    "com_sun_star_comp_Draw_SlideSorter_get_implementation",
    "com_sun_star_comp_Draw_framework_configuration_Configuration_get_implementation",
    "com_sun_star_comp_draw_SdHtmlOptionsDialog_get_implementation",
    "com_sun_star_comp_sd_InsertSlideController_get_implementation",
    "com_sun_star_comp_sd_SlideLayoutController_get_implementation",
    "com_sun_star_comp_sd_DisplayModeController_get_implementation",
    "RandomAnimationNode_get_implementation",
    "com_sun_star_comp_Draw_framework_BasicPaneFactory_get_implementation",
    "com_sun_star_comp_Draw_framework_BasicToolBarFactory_get_implementation",
    "com_sun_star_comp_Draw_framework_BasicViewFactory_get_implementation",
    "com_sun_star_comp_Draw_framework_PresentationFactoryProvider_get_implementation",
    "com_sun_star_comp_Draw_framework_ResourceID_get_implementation",
    "com_sun_star_comp_Draw_framework_configuration_ConfigurationController_get_implementation",
    "com_sun_star_comp_Draw_framework_module_ModuleController_get_implementation",
    "org_openoffice_comp_Draw_framework_PanelFactory_get_implementation",
# sd/util/sdd.component
    "com_sun_star_comp_draw_FormatDetector_get_implementation",
# sd/util/sdfilt.component
    "css_comp_Impress_oox_PowerPointExport",
# writerperfect/source/draw/wpftdraw.component
    "com_sun_star_comp_Draw_VisioImportFilter_get_implementation",
# sdext/source/pdfimport/pdfimport.component
    ("sdext_PDFIHybridAdaptor_get_implementation", "#if HAVE_FEATURE_PDFIMPORT"),
    ("sdext_PDFIRawAdaptor_Writer_get_implementation", "#if HAVE_FEATURE_PDFIMPORT"),
    ("sdext_PDFIRawAdaptor_Draw_get_implementation", "#if HAVE_FEATURE_PDFIMPORT"),
    ("sdext_PDFIRawAdaptor_Impress_get_implementation", "#if HAVE_FEATURE_PDFIMPORT"),
    ("sdext_PDFDetector_get_implementation", "#if HAVE_FEATURE_PDFIMPORT"),
    ]

writer_factory_list = [
    ]

writer_constructor_list = [
# basic/util/sb.component
    ("com_sun_star_comp_sfx2_DialogLibraryContainer_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
    ("com_sun_star_comp_sfx2_ScriptLibraryContainer_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# filter/source/textfilterdetect/textfd.component
    "com_sun_star_comp_filters_PlainTextFilterDetect_get_implementation",
# sw/util/sw.component
    "Writer_SwTextDocument_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisContentImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisMetaImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisSettingsImporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisStylesImporter_get_implementation",
    "com_sun_star_util_comp_FinalThreadManager_get_implementation",
# sw/util/swd.component
    "com_sun_star_comp_writer_FormatDetector_get_implementation",
# sw/util/msword.component
    "com_sun_star_comp_Writer_RtfExport_get_implementation",
    "com_sun_star_comp_Writer_DocxExport_get_implementation",
# writerfilter/util/writerfilter.component
    "com_sun_star_comp_Writer_RtfFilter_get_implementation",
    "com_sun_star_comp_Writer_WriterFilter_get_implementation",
# writerperfect/source/writer/wpftwriter.component
    ("com_sun_star_comp_Writer_EPUBExportFilter_get_implementation", "#ifndef ENABLE_WASM_STRIP_EPUB"),
    ]

desktop_factory_list = [
    ]

desktop_constructor_list = [
    "com_sun_star_comp_bridge_BridgeFactory_get_implementation",
    "com_sun_star_comp_configuration_ConfigurationProvider_get_implementation",
    "com_sun_star_comp_configuration_ConfigurationRegistry_get_implementation",
    "com_sun_star_comp_configuration_DefaultProvider_get_implementation",
    "com_sun_star_comp_configuration_ReadOnlyAccess_get_implementation",
    "com_sun_star_comp_configuration_ReadWriteAccess_get_implementation",
    ("com_sun_star_comp_deployment_component_PackageRegistryBackend_get_implementation", "#ifndef ENABLE_WASM_STRIP_CLUCENE"),
    "com_sun_star_comp_deployment_configuration_PackageRegistryBackend_get_implementation",
    "com_sun_star_comp_deployment_executable_PackageRegistryBackend_get_implementation",
    "com_sun_star_comp_deployment_ExtensionManager_get_implementation",
    "com_sun_star_comp_deployment_ExtensionManager_get_implementation",
    ("com_sun_star_comp_deployment_help_PackageRegistryBackend_get_implementation","#ifndef ENABLE_WASM_STRIP_CLUCENE"),
    "com_sun_star_comp_deployment_PackageManagerFactory_get_implementation",
    "com_sun_star_comp_deployment_PackageManagerFactory_get_implementation",
    "com_sun_star_comp_deployment_ProgressLog_get_implementation",
    "com_sun_star_comp_deployment_script_PackageRegistryBackend_get_implementation",
    "com_sun_star_comp_deployment_sfwk_PackageRegistryBackend_get_implementation",
    "com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation",
    "com_sun_star_comp_frame_SessionListener_get_implementation",
    "com_sun_star_comp_framework_AddonsToolBarFactory_get_implementation",
    "com_sun_star_comp_framework_Desktop_get_implementation",
    "com_sun_star_comp_framework_DocumentAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_Frame_get_implementation",
    "com_sun_star_comp_framework_GenericPopupToolbarController_get_implementation",
    "com_sun_star_comp_framework_GenericPopupToolbarController_get_implementation",
    "com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_JobExecutor_get_implementation",
    "com_sun_star_comp_framework_LangSelectionStatusbarController_get_implementation",
    "com_sun_star_comp_framework_LayoutManager_get_implementation",
    "com_sun_star_comp_framework_MenuBarFactory_get_implementation",
    "com_sun_star_comp_framework_MenuBarFactory_get_implementation",
    "com_sun_star_comp_framework_ModuleAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_ModuleManager_get_implementation",
    "com_sun_star_comp_framework_ModuleUIConfigurationManager_get_implementation",
    "com_sun_star_comp_framework_ModuleUIConfigurationManagerSupplier_get_implementation",
    "com_sun_star_comp_framework_ObjectMenuController_get_implementation",
    "com_sun_star_comp_framework_PathSettings_get_implementation",
    "com_sun_star_comp_framework_PathSubstitution_get_implementation",
    "com_sun_star_comp_framework_PopupMenuControllerFactory_get_implementation",
    "com_sun_star_comp_framework_RecentFilesMenuController_get_implementation",
    "com_sun_star_comp_framework_RecentFilesMenuController_get_implementation",
    "com_sun_star_comp_framework_ResourceMenuController_get_implementation",
    "com_sun_star_comp_framework_SaveToolbarController_get_implementation",
    "com_sun_star_comp_framework_SaveToolbarController_get_implementation",
    "com_sun_star_comp_framework_StatusBarControllerFactory_get_implementation",
    "com_sun_star_comp_framework_StatusBarFactory_get_implementation",
    "com_sun_star_comp_framework_StatusIndicatorFactory_get_implementation",
    "com_sun_star_comp_framework_SubToolBarController_get_implementation",
    "com_sun_star_comp_framework_SubToolBarController_get_implementation",
    "com_sun_star_comp_framework_TaskCreator_get_implementation",
    "com_sun_star_comp_framework_ToolbarAsMenuController_get_implementation",
    "com_sun_star_comp_framework_ToolBarControllerFactory_get_implementation",
    "com_sun_star_comp_framework_ToolBarFactory_get_implementation",
    "com_sun_star_comp_framework_ToolBarFactory_get_implementation",
    "com_sun_star_comp_framework_UICommandDescription_get_implementation",
    "com_sun_star_comp_framework_UICommandDescription_get_implementation",
    "com_sun_star_comp_framework_UIConfigurationManager_get_implementation",
    "com_sun_star_comp_framework_UIElementFactoryManager_get_implementation",
    "com_sun_star_comp_framework_URLTransformer_get_implementation",
    "com_sun_star_comp_framework_WindowListMenuController_get_implementation",
    "com_sun_star_comp_framework_WindowListMenuController_get_implementation",
    "com_sun_star_comp_framework_WindowStateConfiguration_get_implementation",
    "com_sun_star_comp_graphic_GraphicProvider_get_implementation",
    "com_sun_star_comp_office_FrameLoader_get_implementation",
    "com_sun_star_comp_sfx2_AppDispatchProvider_get_implementation",
    ("com_sun_star_comp_sfx2_BackingComp_get_implementation", "#ifndef ENABLE_WASM_STRIP_RECENT"),
    "com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation",
    "com_sun_star_comp_sfx2_InsertSymbolToolBoxControl_get_implementation",
    "com_sun_star_comp_sfx2_InsertSymbolToolBoxControl_get_implementation",
    "com_sun_star_comp_stoc_CoreReflection_get_implementation",
    "com_sun_star_comp_stoc_Introspection_get_implementation",
    "com_sun_star_comp_svx_CharacterSpacingToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_ColorToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_CTLToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_FontNameToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_Impl_FindbarDispatcher_get_implementation",
    "com_sun_star_comp_svx_Impl_FindbarDispatcher_get_implementation",
    "com_sun_star_comp_svx_LineSpacingToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation",
    ("com_sun_star_comp_svx_RecoveryUI_get_implementation", "#ifndef ENABLE_WASM_STRIP_RECOVERYUI"),
    "com_sun_star_comp_svx_StyleToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_TableToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_TableToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_UnderlineToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_UndoRedoToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_UndoRedoToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_VertTextToolBoxControl_get_implementation",
    "com_sun_star_comp_svx_VertTextToolBoxControl_get_implementation",
    "com_sun_star_comp_uri_ExternalUriReferenceTranslator_get_implementation",
    "com_sun_star_comp_uri_UriReferenceFactory_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTexpand_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTscript_get_implementation",
    "com_sun_star_comp_uui_UUIInteractionHandler_get_implementation",
    "com_sun_star_extensions_xml_sax_Writer_get_implementation",
    "com_sun_star_frame_VCLSessionManagerClient_get_implementation",
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
    "com_sun_star_svx_DownSearchToolboxController_get_implementation",
    "com_sun_star_svx_ExitFindbarToolboxController_get_implementation",
    "com_sun_star_svx_FindAllToolboxController_get_implementation",
    "com_sun_star_svx_FindTextToolboxController_get_implementation",
    "com_sun_star_svx_FontHeightToolBoxController_get_implementation",
    "com_sun_star_svx_MatchCaseToolboxController_get_implementation",
    "com_sun_star_svx_SearchLabelToolboxController_get_implementation",
    "com_sun_star_svx_UpSearchToolboxController_get_implementation",
    "com_sun_star_text_DefaultNumberingProvider_get_implementation",
    "dtrans_CMimeContentTypeFactory_get_implementation",
    "filter_ConfigFlush_get_implementation",
    "filter_ContentHandlerFactory_get_implementation",
    "filter_FilterFactory_get_implementation",
    "filter_FrameLoaderFactory_get_implementation",
    "filter_TypeDetection_get_implementation",
    "framework_FooterMenuController_get_implementation",
    "framework_FooterMenuController_get_implementation",
    "framework_HeaderMenuController_get_implementation",
    "framework_HeaderMenuController_get_implementation",
    "framework_LanguageSelectionMenuController_get_implementation",
    "framework_LanguageSelectionMenuController_get_implementation",
    "framework_MacrosMenuController_get_implementation",
    "framework_MacrosMenuController_get_implementation",
    "framework_NewMenuController_get_implementation",
    "framework_NewMenuController_get_implementation",
    "framework_ServiceHandler_get_implementation",
    "framework_ServiceHandler_get_implementation",
    "framework_ToolbarsMenuController_get_implementation",
    "framework_ToolbarsMenuController_get_implementation",
    "i18npool_Collator_Unicode_get_implementation",
    "IndexedPropertyValuesContainer_get_implementation",
    ("lingucomponent_Hyphenator_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    ("lingucomponent_LangGuess_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    ("lingucomponent_SpellChecker_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    ("lingucomponent_Thesaurus_get_implementation", "#ifndef ENABLE_WASM_STRIP_HUNSPELL"),
    "linguistic_DicList_get_implementation",
    "linguistic_GrammarCheckingIterator_get_implementation",
    "linguistic_LinguProps_get_implementation",
    "linguistic_LngSvcMgr_get_implementation",
    "lo_writer_NavElementToolBoxController_get_implementation",
    "lo_writer_NextScrollToolboxController_get_implementation",
    "lo_writer_PreviousScrollToolboxController_get_implementation",
    "org_apache_openoffice_comp_framework_ContextChangeEventMultiplexer_get_implementation",
    "org_apache_openoffice_comp_framework_NewToolbarController_get_implementation",
    "org_apache_openoffice_comp_framework_NewToolbarController_get_implementation",
    "org_apache_openoffice_comp_svx_sidebar_PanelFactory_get_implementation",
    "package_ManifestReader_get_implementation",
    "package_OStorageFactory_get_implementation",
    "package_OZipFileAccess_get_implementation",
    "package_ZipPackage_get_implementation",
    "SfxDocumentMetaData_get_implementation",
    "shell_DesktopBackend_get_implementation",
    "shell_LocaleBackend_get_implementation",
    "stardiv_Toolkit_VCLXPopupMenu_get_implementation",
    "stardiv_Toolkit_VCLXToolkit_get_implementation",
    "stoc_FactoryImpl_get_implementation",
    "svl_FSStorageFactory_get_implementation",
    "ucb_expand_ExpandContentProviderImpl_get_implementation",
    "ucb_file_FileProvider_get_implementation",
    "ucb_OFileAccess_get_implementation",
    "ucb_tdoc_ContentProvider_get_implementation",
    "ucb_tdoc_DocumentContentFactory_get_implementation",
    "ucb_UcbContentProviderProxyFactory_get_implementation",
    "ucb_UcbStore_get_implementation",
    "ucb_UniversalContentBroker_get_implementation",
    "unotools_OTempFileService_get_implementation",
    "unoxml_CDocumentBuilder_get_implementation",
    "unoxml_CURI_get_implementation",
    "unoxml_CXPathAPI_get_implementation",
    "unoxml_rdfRepository_get_implementation",
    "vcl_SystemClipboard_get_implementation",
    "Writer_SwTextDocument_get_implementation",
    "com_sun_star_comp_svx_FrameToolBoxControl_get_implementation",
    "com_sun_star_i18n_InputSequenceChecker_get_implementation",
    "com_sun_star_comp_extensions_SimpleTextFormatter",
    "com_sun_star_comp_extensions_LoggerPool",
    "com_sun_star_comp_extensions_FileHandler",
    "com_sun_star_comp_extensions_PlainTextFormatter",
    "desktop_SplashScreen_get_implementation",
    ]

factory_map = {
    'core' : core_factory_list,
    'edit' : edit_factory_list,
    'math' : math_factory_list,
    'calc' : calc_factory_list,
    'draw' : draw_factory_list,
    'writer' : writer_factory_list,
    }

constructor_map = {
    'core' : core_constructor_list,
    'edit' : edit_constructor_list,
    'math' : math_constructor_list,
    'calc' : calc_constructor_list,
    'draw' : draw_constructor_list,
    'writer' : writer_constructor_list,
    }

custom_widgets = [
    'NotebookbarTabControl',
    'NotebookbarToolBox',
    ]

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
            component_name = re.sub(r'^vnd.sun.star.expand:\$LO_LIB_DIR/([^.]*).so$', r'\1.a', uri)
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
opts.add_option("-C", "--pure-c", action="store_true", help="do not print extern \"C\"", dest="pure_c", default=False)
opts.add_option("-c", "--constructors", help="file with the list of constructors", dest="constructors_file")

(options, args) = opts.parse_args()

# dict of all the constructors that we need according to -g's
full_constructor_map = {}
if options.groups:
    for constructor_group in options.groups:
        for constructor in constructor_map[constructor_group]:
            if type(constructor) is tuple:
                full_constructor_map[constructor[0]] = constructor[1]
            else:
                full_constructor_map[constructor] = True

if not options.groups and options.constructors_file:
    options.groups = factory_map.keys()

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

if options.constructors_file:
    with open(options.constructors_file, "r") as constructors:
        for line in constructors:
            full_constructor_map[line.strip()] = True

print ("""/*
 * This is a generated file. Do not edit.
 *
 * File generated by solenv/bin/native-code.py
 */

#include <config_features.h>
#include <config_fuzzers.h>
#include <config_gpgme.h>
#include <config_locales.h>
#include <osl/detail/component-mapping.h>
#include <string.h>

#ifdef ENABLE_WASM_STRIP_LOCALES
#define WITH_LOCALE_ALL 0
#endif

""")
if not options.pure_c:
    print ("""extern "C" {""")

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

print ('')
for entry in sorted(custom_widgets):
    print ('void make' + entry + '();')
print ('typedef void (*custom_widget_func)();')
print ('#if !ENABLE_FUZZERS')
print ('static struct { const char *name; custom_widget_func func; } custom_widgets[] = {')
for entry in sorted(custom_widgets):
    print ('    { "make' + entry + '", make' + entry + ' },')
print ('};')
print ('#endif')
print ('')
print ("""
custom_widget_func lo_get_custom_widget_func(const char* name)
{
#if ENABLE_FUZZERS
    (void)name;
    return nullptr;
#else
    for (size_t i = 0; i < sizeof(custom_widgets) / sizeof(custom_widgets[0]); i++)
        if (strcmp(name, custom_widgets[i].name) == 0)
            return custom_widgets[i].func;
    return nullptr;
#endif
}
""")

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
    extern void Java_org_libreoffice_kit_LibreOfficeKit_initializeNative();
    volatile void *p = (void *) Java_org_libreoffice_kit_LibreOfficeKit_initializeNative;

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

""")
if not options.pure_c:
    print("""}""")

# vim:set shiftwidth=4 softtabstop=4 expandtab:
