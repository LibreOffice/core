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
    ("libcomphelper.a", "comphelp_component_getFactory"),
    ("libconfigmgrlo.a", "configmgr_component_getFactory"),
    ("libdrawinglayerlo.a", "drawinglayer_component_getFactory"),
    ("libemfiolo.a", "emfio_component_getFactory"),
    ("libfilterconfiglo.a", "filterconfig1_component_getFactory"),
    ("libfsstoragelo.a", "fsstorage_component_getFactory"),
    ("libhyphenlo.a", "hyphen_component_getFactory"),
    ("libi18npoollo.a", "i18npool_component_getFactory"),
    ("libi18nsearchlo.a", "i18nsearch_component_getFactory"),
    ("libinvocadaptlo.a", "invocadapt_component_getFactory"),
    ("liblnglo.a", "lng_component_getFactory"),
    ("liblnthlo.a", "lnth_component_getFactory"),
    ("liblocalebe1lo.a", "localebe1_component_getFactory"),
    ("libpackage2.a", "package2_component_getFactory"),
    ("libsmlo.a", "sm_component_getFactory"),
    ("libsrtrs1.a", "srtrs1_component_getFactory"),
    ("libstoragefdlo.a", "storagefd_component_getFactory"),
    ("libucb1.a", "ucb_component_getFactory"),
    ("libucpfile1.a", "ucpfile_component_getFactory"),
    ("libucphier1.a", "ucphier1_component_getFactory"),
    ("libucptdoc1lo.a", "ucptdoc1_component_getFactory"),
    ("libunordflo.a", "unordf_component_getFactory"),
    ("libunoxmllo.a", "unoxml_component_getFactory"),
    ("libutllo.a", "utl_component_getFactory"),
    ("libxoflo.a", "xof_component_getFactory"),
    ("libxolo.a", "xo_component_getFactory"),
    ("libxstor.a", "xstor_component_getFactory"),
    ("libvclcanvaslo.a", "vclcanvas_component_getFactory"),
    ("libmtfrendererlo.a", "mtfrenderer_component_getFactory"),
    ("libxmlfdlo.a", "xmlfd_component_getFactory"),
    ("libxmlfalo.a", "xmlfa_component_getFactory"),
    ("libodfflatxmllo.a", "odfflatxml_component_getFactory"),
    ("libxmlscriptlo.a", "xmlscript_component_getFactory"),
    ("libmcnttype.a", "mcnttype_component_getFactory"),
    ("libvcllo.a", "vcl_component_getFactory"),
    ("libspelllo.a", "spell_component_getFactory"),
    ("libpdffilterlo.a", "pdffilter_component_getFactory"),
    ("libsvgiolo.a", "svgio_component_getFactory"),
    ("libsvtlo.a", "svt_component_getFactory")
    ]

core_constructor_list = [
# canvas/source/factory/canvasfactory.component
    "com_sun_star_comp_rendering_CanvasFactory_get_implementation",
# chart2/source/chartcore.component
    "com_sun_star_chart2_ExponentialScaling_get_implementation",
    "com_sun_star_chart2_LinearScaling_get_implementation",
    "com_sun_star_chart2_LogarithmicScaling_get_implementation",
    "com_sun_star_chart2_PowerScaling_get_implementation",
    "com_sun_star_comp_chart_AreaChartType_get_implementation",
    "com_sun_star_comp_chart_BarChartType_get_implementation",
    "com_sun_star_comp_chart_BubbleChartType_get_implementation",
    "com_sun_star_comp_chart_CachedDataSequence_get_implementation",
    "com_sun_star_comp_chart_CandleStickChartType_get_implementation",
    "com_sun_star_comp_chart_ChartTypeManager_get_implementation",
    "com_sun_star_comp_chart_ColumnChartType_get_implementation",
    "com_sun_star_comp_chart_DataSeries_get_implementation",
    "com_sun_star_comp_chart_DataSource_get_implementation",
    "com_sun_star_comp_chart_FilledNetChartType_get_implementation",
    "com_sun_star_comp_chart_FormattedString_get_implementation",
    "com_sun_star_comp_chart_InternalDataProvider_get_implementation",
    "com_sun_star_comp_chart_LineChartType_get_implementation",
    "com_sun_star_comp_chart_NetChartType_get_implementation",
    "com_sun_star_comp_chart_PieChartType_get_implementation",
    "com_sun_star_comp_chart_ScatterChartType_get_implementation",
    "com_sun_star_comp_chart2_Axis_get_implementation",
    "com_sun_star_comp_chart2_CartesianCoordinateSystem2d_get_implementation",
    "com_sun_star_comp_chart2_CartesianCoordinateSystem3d_get_implementation",
    "com_sun_star_comp_chart2_ChartController_get_implementation",
    "com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation",
    "com_sun_star_comp_chart2_ChartModel_get_implementation",
    "com_sun_star_comp_chart2_ChartView_get_implementation",
    "com_sun_star_comp_chart2_ConfigDefaultColorScheme_get_implementation",
    "com_sun_star_comp_chart2_Diagram_get_implementation",
    "com_sun_star_comp_chart2_ErrorBar_get_implementation",
    "com_sun_star_comp_chart2_ExponentialRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_GridProperties_get_implementation",
    "com_sun_star_comp_chart2_LabeledDataSequence_get_implementation",
    "com_sun_star_comp_chart2_Legend_get_implementation",
    "com_sun_star_comp_chart2_LinearRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_LogarithmicRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_MeanValueRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_MovingAverageRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_PageBackground_get_implementation",
    "com_sun_star_comp_chart2_PolarCoordinateSystem2d_get_implementation",
    "com_sun_star_comp_chart2_PolarCoordinateSystem3d_get_implementation",
    "com_sun_star_comp_chart2_PolynomialRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_PotentialRegressionCurve_get_implementation",
    "com_sun_star_comp_chart2_RegressionEquation_get_implementation",
    "com_sun_star_comp_chart2_Title_get_implementation",
    "com_sun_star_comp_chart2_XMLFilter_get_implementation",
# chart2/source/controller/chartcontroller.component
    "com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation",
    "com_sun_star_comp_chart2_ChartFrameLoader_get_implementation",
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
# dbaccess/util/dba.component
    ("com_sun_star_comp_dba_ORowSet_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
# forms/util/frm.component
    ("com_sun_star_comp_forms_FormOperations_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_comp_forms_ODatabaseForm_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_comp_forms_OFormattedFieldWrapper_ForcedFormatted_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_comp_forms_ORichTextModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_comp_forms_OScrollBarModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_comp_forms_OSpinButtonModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_Model_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OButtonControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OButtonModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OCheckBoxControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OCheckBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OComboBoxControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OComboBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_ODateControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_ODateModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OEditControl_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OEditModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OFixedTextModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OFormsCollection_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OGridControlModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OGroupBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_OListBoxModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_ONumericModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_ORadioButtonModel_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
    ("com_sun_star_form_XForms_get_implementation", "#if HAVE_FEATURE_DBCONNECTIVITY"),
# framework/util/fwk.component
    "com_sun_star_comp_framework_AutoRecovery_get_implementation",
    "com_sun_star_comp_framework_Desktop_get_implementation",
    "com_sun_star_comp_framework_DocumentAcceleratorConfiguration_get_implementation",
    "com_sun_star_comp_framework_Frame_get_implementation",
    "com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation",
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
    "com_sun_star_i18n_NativeNumberSupplier_get_implementation",
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
# sot/util/sot.component
    "com_sun_star_comp_embed_OLESimpleStorage",
# stoc/source/inspect/introspection.component
    "com_sun_star_comp_stoc_Introspection_get_implementation",
# stoc/source/corereflection/reflection.component
    "com_sun_star_comp_stoc_CoreReflection_get_implementation",
# stoc/util/stocservices.component
    "com_sun_star_comp_stoc_OServiceManagerWrapper_get_implementation",
    "com_sun_star_comp_stoc_TypeConverter_get_implementation",
    "com_sun_star_comp_uri_ExternalUriReferenceTranslator_get_implementation",
    "com_sun_star_comp_uri_UriReferenceFactory_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTexpand_get_implementation",
    "com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTscript_get_implementation",
# vcl/vcl.android.component
    "com_sun_star_graphic_GraphicObject_get_implementation",
    "com_sun_star_comp_graphic_GraphicProvider_get_implementation",
# svx/util/svx.component
    "com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation",
    "com_sun_star_drawing_SvxShapeCollection_get_implementation",
# svx/util/svxcore.component
    "com_sun_star_comp_graphic_PrimitiveFactory2D_get_implementation",
    "com_sun_star_comp_Draw_GraphicExporter_get_implementation",
    "com_sun_star_comp_Svx_GraphicExportHelper_get_implementation",
    "com_sun_star_comp_Svx_GraphicImportHelper_get_implementation",
# toolkit/util/tk.component
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
    "stardiv_Toolkit_UnoSpinButtonModel_get_implementation",
    "stardiv_Toolkit_VCLXPointer_get_implementation",
    "stardiv_Toolkit_VCLXToolkit_get_implementation",
# uui/util/uui.component
    "com_sun_star_comp_uui_UUIInteractionHandler_get_implementation",
# xmloff/util/xo.component
    "XMLVersionListPersistence_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisImporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisStylesExporter_get_implementation",
    "com_sun_star_comp_Impress_XMLOasisContentExporter_get_implementation",
# xmlscript/util/xmlscript.component
    "com_sun_star_comp_xmlscript_XMLBasicExporter",
    "com_sun_star_comp_xmlscript_XMLBasicImporter",
    "com_sun_star_comp_xmlscript_XMLOasisBasicExporter",
    "com_sun_star_comp_xmlscript_XMLOasisBasicImporter",
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
    "com_sun_star_comp_oox_ShapeContextHandler_get_implementation",
    ]

# edit group for apps, where you can edit documents
edit_factory_list = [
    ]

edit_constructor_list = [
# framework/util/fwk.component
    "com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation",
# i18npool/util/i18npool.component
    "com_sun_star_i18n_InputSequenceChecker_get_implementation",
    "com_sun_star_i18n_OrdinalSuffix_get_implementation",
# sc/util/sc.component
    "Calc_XMLOasisContentExporter_get_implementation",
    "Calc_XMLOasisExporter_get_implementation",
    "Calc_XMLOasisMetaExporter_get_implementation",
    "Calc_XMLOasisSettingsExporter_get_implementation",
    "Calc_XMLOasisStylesExporter_get_implementation",
# starmath/util/sm.component
    "Math_XMLOasisMetaExporter_get_implementation",
    "Math_XMLOasisSettingsExporter_get_implementation",
    "Math_XMLImporter_get_implementation",
    "Math_XMLOasisMetaImporter_get_implementation",
    "Math_XMLOasisSettingsImporter_get_implementation",
# starmath/util/smd.component
    "math_FormatDetector_get_implementation",
# sw/util/sw.component
    "com_sun_star_comp_Writer_XMLOasisContentExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisMetaExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisSettingsExporter_get_implementation",
    "com_sun_star_comp_Writer_XMLOasisStylesExporter_get_implementation",
    "com_sun_star_comp_Writer_WriterModule_get_implementation",
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
    ("libsclo.a", "sc_component_getFactory"),
    ("libanalysislo.a", "analysis_component_getFactory"),
    ("libdatelo.a", "date_component_getFactory"),
    ("libpricinglo.a", "pricing_component_getFactory"),
    ]

calc_constructor_list = [
# avmedia/util/avmedia.component
    "com_sun_star_comp_framework_SoundHandler_get_implementation",
# sc/util/scd.component
    "com_sun_star_comp_calc_ExcelBiffFormatDetector_get_implementation",
    "com_sun_star_comp_calc_FormatDetector_get_implementation",
# sc/util/scfilt.component
    "com_sun_star_comp_oox_xls_ExcelFilter_get_implementation",
    "com_sun_star_comp_oox_xls_FormulaParser_get_implementation",
# scripting/source/vbaevents/vbaevents.component
    ("ooo_vba_VBAToOOEventDesc_get_implementation", "#if HAVE_FEATURE_SCRIPTING"),
# svl/util/svl.component
    "com_sun_star_uno_util_numbers_SvNumberFormatsSupplierServiceObject_get_implementation",
    "com_sun_star_uno_util_numbers_SvNumberFormatterServiceObject_get_implementation",
    ]

draw_factory_list = [
    ("libsdlo.a", "sd_component_getFactory"),
    ("libsvgfilterlo.a", "svgfilter_component_getFactory"),
    ("libdeployment.a", "deployment_component_getFactory"),
    ("libemboleobj.a", "emboleobj_component_getFactory"),
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
# sd/util/sd.component
    "RandomAnimationNode_get_implementation",
    "com_sun_star_comp_Draw_framework_BasicPaneFactory_get_implementation",
    "com_sun_star_comp_Draw_framework_BasicToolBarFactory_get_implementation",
    "com_sun_star_comp_Draw_framework_BasicViewFactory_get_implementation",
    "com_sun_star_comp_Draw_framework_PresentationFactoryProvider_get_implementation",
    "com_sun_star_comp_Draw_framework_ResourceID_get_implementation",
    "com_sun_star_comp_Draw_framework_configuration_ConfigurationController_get_implementation",
    "com_sun_star_comp_Draw_framework_module_ModuleController_get_implementation",
# sd/util/sdd.component
    "com_sun_star_comp_draw_FormatDetector_get_implementation",
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
    "com_sun_star_util_comp_FinalThreadManager_get_implementation",
# sw/util/swd.component
    "com_sun_star_comp_writer_FormatDetector_get_implementation",
# sw/util/msword.component
    "com_sun_star_comp_Writer_RtfExport_get_implementation",
    "com_sun_star_comp_Writer_DocxExport_get_implementation",
# writerfilter/util/writerfilter.component
    "com_sun_star_comp_Writer_RtfFilter_get_implementation",
    "com_sun_star_comp_Writer_WriterFilter_get_implementation",
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
    'ArgEdit',
    'BookmarksBox',
    'CategoryListBox',
    'ClassificationEditView',
    'ColorConfigCtrl',
    'ColumnEdit',
    'ConditionEdit',
    'ContentListBox',
    'ContextVBox',
    'CustomAnimationList',
    'CustomPropertiesControl',
    'DataTreeListBox',
    'DriverListControl',
    'DropdownBox',
    'EditBox',
    'EmojiView',
    'ExtBoxWithBtns',
    'ExtensionBox',
    'FontNameBox',
    'FontSizeBox',
    'FontStyleBox',
    'FormulaListBox',
    'IndexBox',
    'IndexBox',
    'IntellectualPropertyPartEdit',
    'LightButton',
    'LookUpComboBox',
    'ManagedMenuButton',
    'MultiLineEditSyntaxHighlight',
    'NumFormatListBox',
    'OFileURLControl',
    'OptionalBox',
    'PageNumberListBox',
    'PaperSizeListBox',
    'PriorityHBox',
    'PriorityMergedHBox',
    'PropertyControl',
    'RecentDocsView',
    'RefButton',
    'RefEdit',
    'ReplaceEdit',
    'RowEdit',
    'RubyEdit',
    'RubyPreview',
    'SameContentListBox',
    'ScCondFormatList',
    'ScCsvTableBox',
    'ScCursorRefEdit',
    'ScDataTableView',
    'ScDoubleField',
    'ScEditWindow',
    'ScPivotLayoutTreeList',
    'ScPivotLayoutTreeListData',
    'ScPivotLayoutTreeListLabel',
    'ScRefButtonEx',
    'SdPageObjsTLB',
    'SearchBox',
    'SearchResultsBox',
    'SelectionListBox',
    'SentenceEditWindow',
    'ShowNupOrderWindow',
    'ShowNupOrderWindow',
    'SidebarDialControl',
    'SidebarToolBox',
    'SmallButton',
    'SpacingListBox',
    'StatusBar',
    'StructListBox',
    'SvSimpleTableContainer',
    'SvTreeListBox',
    'SvtFileView',
    'SvtIconChoiceCtrl',
    'SvtURLBox',
    'Svx3DPreviewControl',
    'SvxCharViewControl',
    'SvxCheckListBox',
    'SvxColorListBox',
    'SvxColorValueSet',
    'SvxFillAttrBox',
    'SvxFillTypeBox',
    'SvxFontPrevWindow',
    'SvxHlmarkTreeLBox',
    'SvxHyperURLBox',
    'SvxLanguageBox',
    'SvxLanguageComboBox',
    'SvxLightCtl3D',
    'SvxNoSpaceEdit',
    'SvxPathControl',
    'SvxRelativeField',
    'SvxTextEncodingBox',
    'SvxTextEncodingBox',
    'SwAddressPreview',
    'SwGlTreeListBox',
    'SwMarkPreview',
    'SwNavHelpToolBox',
    'TableValueSet',
    'TemplateDefaultView',
    'TemplateLocalView',
    'TemplateSearchView',
    'ThesaurusAlternativesCtrl',
    'ValueSet',
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
opts.add_option("-C", "--pure-c", action="store_true", help="do not print extern \"C\"", dest="pure_c", default=False)

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
 *
 * File generated by solenv/bin/native-code.py
 */

#include <config_features.h>
#include <config_fuzzers.h>
#include <config_gpgme.h>
#include <osl/detail/component-mapping.h>
#include <string.h>

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
