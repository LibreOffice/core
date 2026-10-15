/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_crypto.h>
#include <config_features.h>
#include <config_fuzzers.h>
#include <config_gio.h>
#include <config_java.h>
#include <config_locales.h>
#include <osl/detail/component-mapping.h>
#include <string>
#include <iostream>

/**
Prepate the linker script used to create the unitedlibs library.

Generates a linker script as documented at https://sourceware.org/binutils/docs/ld/VERSION.html.

We are not assigning symbols to versions here, which is the normal use of such a script, rather,
we are using it to limit symbol visibility, which reduces relocations, speeds loading time,
and improves performance a little.

TODO: It would be better to generate most of this script by parsing and extracting symbol names
 from all of the *.component files.
*/

static void line(std::string_view sym) { std::cout << "        " << sym << ";" << std::endl; }

int main()
{
    // no need for a name/version string here, since we are using this just to limit symbol visibility.
    std::cout << "{" << std::endl;
    std::cout << "    global:" << std::endl;

    // other things that are not in .component files
    line("basicide_choose_macro");
    line("create_SalInstanceGen");
    line("create_SalInstanceGtk3");
    line("makeNotebookbarToolBox");
    line("sal_getFullTextEncodingData");
    line("unit_kit_process_events_to_idle");
    line("uno_initEnvironment");
    line("uno_ext_getMapping");
    line("CreateUnoWrapper");
    line("ImportDOC");
    line("ImportDOCX");
    line("ImportRTF");
    line("ExportDOC");
    line("ExportRTF");
    line("ScFilterCreate");
    line("sal_detail_initialize");
    line("sal_detail_deinitialize");

    // stuff we need when running from online or coda
    line("kit_open_urandom");
    line("cok_preinit_2");
    line("cokit_hook_2");
    line("kit_preload_hook_sc");
    line("kit_preload_hook_sd");
    line("kit_preload_hook_sw");

    // stuff we need when running the classic UI, i.e. from soffice.bin
    line("soffice_main");

    // the language tables from i18npool
    line("getAllCalendars*");
    line("getAllCurrencies*");
    line("getAllFormats0*");
    line("getAllFormats1*");
    line("getBreakIteratorRules*");
    line("getCollatorImplementation*");
    line("getContinuousNumberingLevels*");
    line("getDateAcceptancePatterns*");
    line("getForbiddenCharacters*");
    line("getIndexAlgorithm*");
    line("getLocaleItem*");
    line("getLCInfo*");
    line("getOutlineNumberingLevels*");
    line("getReservedWords*");

    // core_factory_list
    line("i18npool_component_getFactory");
    line("svt_component_getFactory");

    // animations/source/animcore/animcore.component
    line("com_sun_star_animations_AnimatePhysics_get_implementation");
    line("com_sun_star_animations_AnimateColor_get_implementation");
    line("com_sun_star_animations_AnimateMotion_get_implementation");
    line("com_sun_star_animations_AnimateSet_get_implementation");
    line("com_sun_star_animations_AnimateTransform_get_implementation");
    line("com_sun_star_animations_Animate_get_implementation");
    line("com_sun_star_animations_Audio_get_implementation");
    line("com_sun_star_animations_Command_get_implementation");
    line("com_sun_star_animations_IterateContainer_get_implementation");
    line("com_sun_star_animations_ParallelTimeContainer_get_implementation");
    line("com_sun_star_animations_SequenceTimeContainer_get_implementation");
    line("com_sun_star_animations_TransitionFilter_get_implementation");
#if HAVE_FEATURE_AVMEDIA
#ifdef MACOSX
    // avmedia/source/macavf/avmediaMacAVF.component
    line("com_sun_star_comp_avmedia_Manager_MacAVF_get_implementation");
#endif
#ifdef _WIN32
    // avmedia/source/win/avmediawin.component
    line("avmedia_Manager_DirectX_get_implementation");
#endif
    // avmedia/util/avmedia.component
    line("com_sun_star_comp_framework_SoundHandler_get_implementation");
#endif
#if HAVE_FEATURE_SCRIPTING
    // basctl/util/basctl.component
    line("com_sun_star_comp_basic_BasicID_get_implementation");
    // basic/util/sb.component
    line("com_sun_star_comp_sfx2_DialogLibraryContainer_get_implementation");
    line("com_sun_star_comp_sfx2_ScriptLibraryContainer_get_implementation");
#endif
    // binaryurp/source/binaryurp.component
    line("com_sun_star_comp_bridge_BridgeFactory_get_implementation");
    // UnoControls/util/ctl.component
    line("stardiv_UnoControls_FrameControl_get_implementation");
    line("stardiv_UnoControls_ProgressBar_get_implementation");
    line("stardiv_UnoControls_ProgressMonitor_get_implementation");
    line("stardiv_UnoControls_StatusIndicator_get_implementation");
    // chart2/source/chart2.component
    line("com_sun_star_chart2_ExponentialScaling_get_implementation");
    line("com_sun_star_chart2_LinearScaling_get_implementation");
    line("com_sun_star_chart2_LogarithmicScaling_get_implementation");
    line("com_sun_star_chart2_PowerScaling_get_implementation");
    line("com_sun_star_comp_chart_AreaChartType_get_implementation");
    line("com_sun_star_comp_chart_BarChartType_get_implementation");
    line("com_sun_star_comp_chart_BoxWhiskerChartType_get_implementation");
    line("com_sun_star_comp_chart_BubbleChartType_get_implementation");
    line("com_sun_star_comp_chart_CorrelationCircleChartType_get_implementation");
    line("com_sun_star_comp_chart_CachedDataSequence_get_implementation");
    line("com_sun_star_comp_chart_CandleStickChartType_get_implementation");
    line("com_sun_star_comp_chart_ChartTypeManager_get_implementation");
    line("com_sun_star_comp_chart_ClusteredColumnChartType_get_implementation");
    line("com_sun_star_comp_chart_ColumnChartType_get_implementation");
    line("com_sun_star_comp_chart_HistogramChartType_get_implementation");
    line("com_sun_star_comp_chart_DataSeries_get_implementation");
    line("com_sun_star_comp_chart_DataSource_get_implementation");
    line("com_sun_star_comp_chart_FilledNetChartType_get_implementation");
    line("com_sun_star_comp_chart_FormattedString_get_implementation");
    line("com_sun_star_comp_chart_FunnelChartType_get_implementation");
    line("com_sun_star_comp_chart_InternalDataProvider_get_implementation");
    line("com_sun_star_comp_chart_LineChartType_get_implementation");
    line("com_sun_star_comp_chart_NetChartType_get_implementation");
    line("com_sun_star_comp_chart_ParetoLineChartType_get_implementation");
    line("com_sun_star_comp_chart_PieChartType_get_implementation");
    line("com_sun_star_comp_chart_RegionMapChartType_get_implementation");
    line("com_sun_star_comp_chart_ScatterChartType_get_implementation");
    line("com_sun_star_comp_chart_SunburstChartType_get_implementation");
    line("com_sun_star_comp_chart_TreemapChartType_get_implementation");
    line("com_sun_star_comp_chart_WaterfallChartType_get_implementation");
    line("com_sun_star_comp_chart2_Axis_get_implementation");
    line("com_sun_star_comp_chart2_CartesianCoordinateSystem2d_get_implementation");
    line("com_sun_star_comp_chart2_CartesianCoordinateSystem3d_get_implementation");
    line("com_sun_star_comp_chart2_ChartModel_get_implementation");
    line("com_sun_star_comp_chart2_ChartView_get_implementation");
    line("com_sun_star_comp_chart2_ConfigDefaultColorScheme_get_implementation");
    line("com_sun_star_comp_chart2_Diagram_get_implementation");
    line("com_sun_star_comp_chart2_ErrorBar_get_implementation");
    line("com_sun_star_comp_chart2_ExponentialRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_GridProperties_get_implementation");
    line("com_sun_star_comp_chart2_LabeledDataSequence_get_implementation");
    line("com_sun_star_comp_chart2_Legend_get_implementation");
    line("com_sun_star_comp_chart2_DataTable_get_implementation");
    line("com_sun_star_comp_chart2_LinearRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_LogarithmicRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_MeanValueRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_MovingAverageRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_PageBackground_get_implementation");
    line("com_sun_star_comp_chart2_PolarCoordinateSystem2d_get_implementation");
    line("com_sun_star_comp_chart2_PolarCoordinateSystem3d_get_implementation");
    line("com_sun_star_comp_chart2_PolynomialRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_PotentialRegressionCurve_get_implementation");
    line("com_sun_star_comp_chart2_RegressionEquation_get_implementation");
    line("com_sun_star_comp_chart2_Title_get_implementation");
    line("com_sun_star_comp_chart2_XMLFilter_get_implementation");
    line("com_sun_star_comp_chart2_report_XMLFilter_get_implementation");
    line("com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation");
    line("com_sun_star_comp_chart2_ChartFrameLoader_get_implementation");
    line("com_sun_star_comp_chart2_WizardDialog_get_implementation");
    line("org_libreoffice_chart2_Chart2ToolboxController");
    line("org_libreoffice_comp_chart2_sidebar_ChartPanelFactory");
    // chart2/source/controller/chartcontroller.component
    line("com_sun_star_comp_chart_ElementSelectorToolbarController_get_implementation");
    line("com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation");
    line("com_sun_star_comp_chart2_ChartFrameLoader_get_implementation");
    line("com_sun_star_comp_chart2_ChartTypeDialog_get_implementation");
    line("com_sun_star_comp_chart2_WizardDialog_get_implementation");
    line("org_libreoffice_comp_chart2_sidebar_ChartPanelFactory");
    line("org_libreoffice_chart2_Chart2ToolboxController");
    line("com_sun_star_comp_chart2_ChartColorPaletteControl_get_implementation");
    line("com_sun_star_comp_chart2_ChartThemeControl_get_implementation");
    line("com_sun_star_comp_chart2_ChartGradientPaletteControl_get_implementation");
    // comphelper/util/comphelp.component
    line("com_sun_star_comp_embed_InstanceLocker");
    line("com_sun_star_comp_MemoryStream");
    line("com_sun_star_comp_task_OfficeRestartManager");
    line("AnyCompareFactory_get_implementation");
    line("IndexedPropertyValuesContainer_get_implementation");
    line("NamedPropertyValuesContainer_get_implementation");
    line("com_sun_star_comp_comphelper_OPropertyBag");
    line("com_sun_star_comp_SequenceInputStreamService");
    line("com_sun_star_comp_SequenceOutputStreamService");
    line("com_sun_star_comp_util_OfficeInstallationDirectories");
    line("org_openoffice_comp_comphelper_EnumerableMap");
    // configmgr/source/configmgr.component
    line("com_sun_star_comp_configuration_ConfigurationProvider_get_implementation");
    line("com_sun_star_comp_configuration_ConfigurationRegistry_get_implementation");
    line("com_sun_star_comp_configuration_DefaultProvider_get_implementation");
    line("com_sun_star_comp_configuration_ReadOnlyAccess_get_implementation");
    line("com_sun_star_comp_configuration_ReadWriteAccess_get_implementation");
    line("com_sun_star_comp_configuration_Update_get_implementation");
    // connectivity/source/cpool/dbpool2.component
    line("connectivity_OPoolCollection_get_implementation");
    // connectivity/source/dbtools/dbtools.component
    line("connectivity_dbtools_ODatabaseMetaDataResultSet_get_implementation");
    line("connectivity_dbtools_ParameterSubstitution_get_implementation");
#ifdef _WIN32
    // connectivity/source/drivers/ado/ado.component
    line("connectivity_ado_ODriver_get_implementation");
#endif
    // connectivity/source/drivers/calc/calc.component
    line("connectivity_calc_ODriver");
    // connectivity/source/drivers/evoab2/evoab.component
    // TODO this needs a config_foo.h entry
    line("connectivity_OEvoabDriver_get_implementation");
    // connectivity/source/drivers/flat/flat.component
    line("connectivity_flat_ODriver");
#ifdef MACOSX
    // connectivity/source/drivers/macab/macab1.component
    line("connectivity_MacabDriver_get_implementation");
#endif
    // connectivity/source/drivers/mozab/bootstrap/mozbootstrap.component
    line("connectivity_moz_MozillaBootstrap_get_implementation");
#if HAVE_FEATURE_DBCONNECTIVITY
    // connectivity/source/drivers/dbase/dbase.component
    line("connectivity_dbase_ODriver");
    // connectivity/source/drivers/odbc/odbc.component
    line("connectivity_odbc_ORealOdbcDriver_get_implementation");
    // connectivity/source/drivers/writer/writer.component
    line("connectivity_writer_ODriver");
    // connectivity/source/manager/sdbc2.component
    line("connectivity_OSDBCDriverManager_get_implementation");
#endif
    // cppcanvas/source/uno/mtfrenderer.component
    line("com_sun_star_comp_rendering_MtfRenderer_get_implementation");
#if !ENABLE_FUZZERS
    // cui/util/cui.component
    line("com_sun_star_cui_GetCreateDialogFactoryService");
#endif
#if HAVE_FEATURE_DBCONNECTIVITY
    // dbaccess/source/filter/xml/dbaxml.component
    line("com_sun_star_comp_sdb_DBExportFilter_get_implementation");
    line("com_sun_star_comp_sdb_DBFilter_get_implementation");
    line("com_sun_star_comp_sdb_XMLFullExporter_get_implementation");
    line("com_sun_star_comp_sdb_XMLSettingsExporter_get_implementation");
    line("org_openoffice_comp_dbflt_DBContentLoader2_get_implementation");
    line("org_openoffice_comp_dbflt_DBTypeDetection_get_implementation");
    // dbaccess/util/dba.component
    line("com_sun_star_comp_dba_DataAccessDescriptorFactory");
    line("com_sun_star_comp_dba_OCommandDefinition");
    line("com_sun_star_comp_dba_OComponentDefinition");
    line("com_sun_star_comp_dba_ODatabaseContext_get_implementation");
    line("com_sun_star_comp_dba_ODatabaseDocument");
    line("com_sun_star_comp_dba_ODatabaseSource");
    line("com_sun_star_comp_dba_ORowSet_get_implementation");
    // dbaccess/util/dbu.component
    line("com_sun_star_comp_dbaccess_DatabaseInteractionHandler_get_implementation");
    line("com_sun_star_comp_dbaccess_LegacyInteractionHandler_get_implementation");
    line("com_sun_star_comp_dbaccess_OTextConnectionSettingsDialog_get_implementation");
    line("com_sun_star_comp_dbu_SbaXGridControl_get_implementation");
    line("com_sun_star_uno_comp_sdb_RowsetFilterDialog_get_implementation");
    line("com_sun_star_uno_comp_sdb_RowsetOrderDialog_get_implementation");
    line("org_openoffice_comp_dbu_CopyTableWizard_get_implementation");
    line("org_openoffice_comp_dbu_DBContentLoader_get_implementation");
    line("org_openoffice_comp_dbu_ODBTypeWizDialog_get_implementation");
    line("org_openoffice_comp_dbu_ODatasourceAdministrationDialog_get_implementation");
    line("org_openoffice_comp_dbu_ODatasourceBrowser_get_implementation");
    line("org_openoffice_comp_dbu_OSQLMessageDialog_get_implementation");
    // dbaccess/util/sdbt.component
    line("com_sun_star_comp_dbaccess_ConnectionTools_get_implementation");
#endif
    // desktop/kitclipboard.component
    line("desktop_KitClipboard_get_implementation");
    // desktop/source/offacc/offacc.component
    line("desktop_Acceptor_get_implementation");
    // desktop/source/splash/spl.component
    line("desktop_SplashScreen_get_implementation");
    line("desktop_UnxSplash_get_implementation");
    // drawinglayer/drawinglayer.component
    line("drawinglayer_XPrimitive2DRenderer");
    // embeddedobj/source/msole/emboleobj.component
    line("embeddedobj_OleEmbeddedObjectFactory_get_implementation");
    // embeddedobj/util/embobj.component
    line("embeddedobj_UNOEmbeddedObjectCreator_get_implementation");
    line("embeddedobj_OOoEmbeddedObjectFactory_get_implementation");
    line("embeddedobj_OOoSpecialEmbeddedObjectFactory_get_implementation");
#ifdef _WIN32
    // embedserv/util/emser.component
    line("embedserv_EmbedServer");
#endif
    // emfio/emfio.component
    line("emfio_emfreader_XEmfParser_get_implementation");
    // eventattacher/source/evtatt.component
    line("eventattacher_EventAttacher");
    // extensions/source/abpilot/abp.component
    line("org_openoffice_comp_abp_OAddressBookSourcePilot");
#if !ENABLE_FUZZERS
    // extensions/source/bibliography/bib.component
    line("extensions_BibliographyLoader_get_implementation");
#endif
#ifdef _WIN32
    // extensions/source/config/WinUserInfo/WinUserInfoBe.component
    line("extensions_WinUserInfoBe_get_implementation");
#endif
    // extensions/source/dbpilots/dbp.component
    line("extensions_dbp_OGridWizard_get_implementation");
    line("extensions_dbp_OGroupBoxWizard_get_implementation");
    line("extensions_dbp_OListComboWizard_get_implementation");
    // extensions/source/logging/log.component
    line("com_sun_star_comp_extensions_ConsoleHandler");
    line("com_sun_star_comp_extensions_CsvFormatter");
    line("com_sun_star_comp_extensions_FileHandler");
    line("com_sun_star_comp_extensions_LoggerPool");
    line("com_sun_star_comp_extensions_PlainTextFormatter");
    line("com_sun_star_comp_extensions_SimpleTextFormatter");
    // extensions/source/propctrlr/pcr.component
    line("extensions_propctrlr_StringRepresentation_get_implementation");
    line("extensions_propctrlr_ButtonNavigationHandler_get_implementation");
    line("extensions_propctrlr_CellBindingPropertyHandler_get_implementation");
    line("extensions_propctrlr_EFormsPropertyHandler_get_implementation");
    line("extensions_propctrlr_EditPropertyHandler_get_implementation");
    line("extensions_propctrlr_EventHandler_get_implementation");
    line("extensions_propctrlr_FormComponentPropertyHandler_get_implementation");
    line("extensions_propctrlr_FormGeometryHandler_get_implementation");
    line("extensions_propctrlr_GenericPropertyHandler_get_implementation");
    line("extensions_propctrlr_SubmissionPropertyHandler_get_implementation");
    line("extensions_propctrlr_XSDValidationPropertyHandler_get_implementation");
    line("extensions_propctrlr_DefaultFormComponentInspectorModel_get_implementation");
    line("extensions_propctrlr_DefaultHelpProvider_get_implementation");
    line("extensions_propctrlr_DialogController_get_implementation");
    line("extensions_propctrlr_FormController_get_implementation");
    line("extensions_propctrlr_OPropertyBrowserController_get_implementation");
    line("extensions_propctrlr_ObjectInspectorModel_get_implementation");
    line("extensions_propctrlr_MasterDetailLinkDialog_get_implementation");
    line("extensions_propctrlr_OControlFontDialog_get_implementation");
    line("extensions_propcrltr_OTabOrderDialog_get_implementation");
    // extensions/source/scanner/scn.component
    line("extensions_ScannerManager_get_implementation");
    // filter/source/config/cache/filterconfig1.component
    line("filter_ConfigFlush_get_implementation");
    line("filter_TypeDetection_get_implementation");
    line("filter_FrameLoaderFactory_get_implementation");
    line("filter_FilterFactory_get_implementation");
    line("filter_ContentHandlerFactory_get_implementation");
    // filter/source/graphic/graphicfilter.component
    line("filter_GraphicExportFilter_get_implementation");
    // filter/source/msfilter/msfilter.component
    line("filter_VBAMacroResolver_get_implementation");
    // filter/source/odfflatxml/odfflatxml.component
    line("filter_OdfFlatXml_get_implementation");
    // filter/source/pdf/pdffilter.component
    line("filter_PdfDecomposer_get_implementation");
    line("filter_PDFExportInteractionHandler_get_implementation");
    line("filter_PDFFilter_get_implementation");
    line("filter_PDFDialog_get_implementation");
    // filter/source/storagefilterdetect/storagefd.component
    line("filter_StorageFilterDetect_get_implementation");
    // filter/source/svg/svgfilter.component
    line("filter_SVGFilter_get_implementation");
    line("filter_SVGWriter_get_implementation");
    // filter/source/t602/t602filter.component
    line("filter_T602ImportFilter_get_implementation");
    line("filter_T602ImportFilterDialog_get_implementation");
    // filter/source/textfilterdetect/textfd.component
    line("com_sun_star_comp_filters_PlainTextFilterDetect_get_implementation");
    // filter/source/xmlfilteradaptor/xmlfa.component
    line("filter_XmlFilterAdaptor_get_implementation");
    // filter/source/xmlfilterdetect/xmlfd.component
    line("filter_XMLFilterDetect_get_implementation");
    // filter/source/xsltdialog/xsltdlg.component
    line("filter_XSLTFilterDialog_get_implementation");
    // filter/source/xsltfilter/xsltfilter.component
    line("filter_XSLTFilter_get_implementation");
    line("filter_LibXSLTTransformer_get_implementation");
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    // forms/util/frm.component
    line("com_sun_star_comp_form_ONavigationBarControl_get_implementation");
    line("com_sun_star_comp_form_ONavigationBarModel_get_implementation");
    line("com_sun_star_comp_form_ORichTextControl_get_implementation");
    line("com_sun_star_comp_forms_FormOperations_get_implementation");
    line("com_sun_star_comp_forms_ODatabaseForm_get_implementation");
    line("com_sun_star_comp_forms_OFilterControl_get_implementation");
    line("com_sun_star_comp_forms_OFormattedFieldWrapper_ForcedFormatted_get_implementation");
    line("com_sun_star_comp_forms_ORichTextModel_get_implementation");
    line("com_sun_star_comp_forms_OScrollBarModel_get_implementation");
    line("com_sun_star_comp_forms_OSpinButtonModel_get_implementation");
    line("com_sun_star_comp_xml_xpath_XFormsExtension_get_implementation");
    line("com_sun_star_form_ImageProducer_get_implementation");
    line("com_sun_star_form_Model_get_implementation");
    line("com_sun_star_form_OButtonControl_get_implementation");
    line("com_sun_star_form_OButtonModel_get_implementation");
    line("com_sun_star_form_OCheckBoxControl_get_implementation");
    line("com_sun_star_form_OCheckBoxModel_get_implementation");
    line("com_sun_star_form_OComboBoxControl_get_implementation");
    line("com_sun_star_form_OComboBoxModel_get_implementation");
    line("com_sun_star_form_OCurrencyControl_get_implementation");
    line("com_sun_star_form_OCurrencyModel_get_implementation");
    line("com_sun_star_form_ODateControl_get_implementation");
    line("com_sun_star_form_ODateModel_get_implementation");
    line("com_sun_star_form_OEditControl_get_implementation");
    line("com_sun_star_form_OFileControlModel_get_implementation");
    line("com_sun_star_form_OEditModel_get_implementation");
    line("com_sun_star_form_OFixedTextModel_get_implementation");
    line("com_sun_star_form_OFormattedControl_get_implementation");
    line("com_sun_star_form_OFormattedFieldWrapper_get_implementation");
    line("com_sun_star_form_OFormsCollection_get_implementation");
    line("com_sun_star_form_OGridControlModel_get_implementation");
    line("com_sun_star_form_OGroupBoxControl_get_implementation");
    line("com_sun_star_form_OGroupBoxModel_get_implementation");
    line("com_sun_star_form_OHiddenModel_get_implementation");
    line("com_sun_star_form_OImageButtonControl_get_implementation");
    line("com_sun_star_form_OImageButtonModel_get_implementation");
    line("com_sun_star_form_OImageControlControl_get_implementation");
    line("com_sun_star_form_OImageControlModel_get_implementation");
    line("com_sun_star_form_OListBoxControl_get_implementation");
    line("com_sun_star_form_OListBoxModel_get_implementation");
    line("com_sun_star_form_ONumericModel_get_implementation");
    line("com_sun_star_form_ONumericControl_get_implementation");
    line("com_sun_star_form_OPatternControl_get_implementation");
    line("com_sun_star_form_OPatternModel_get_implementation");
    line("com_sun_star_form_ORadioButtonControl_get_implementation");
    line("com_sun_star_form_ORadioButtonModel_get_implementation");
    line("com_sun_star_form_OTimeControl_get_implementation");
    line("com_sun_star_form_OTimeModel_get_implementation");
    line("com_sun_star_form_XForms_get_implementation");
#endif
    // formula/util/for.component
    line("simple_formula_FormulaOpCodeMapperObj");
#ifdef MACOS
    // fpicker/source/aqua/fps_aqua.component
    line("fpicker_SalAquaFilePicker_get_implementation");
    line("fpicker_SalAquaFolderPicker_get_implementation");
#endif
    // fpicker/source/office/fps_office.component
    line("fpicker_SvtFilePicker_get_implementation");
    line("fpicker_SvtRemoteFilePicker_get_implementation");
    line("fpicker_SvtFolderPicker_get_implementation");
#ifdef _WIN32
    // fpicker/source/win32/fps.component
    line("fpicker_win32_FilePicker_get_implementation");
    line("fpicker_win32_FolderPicker_get_implementation");
#endif
    // framework/util/fwk.component
    line("com_sun_star_comp_frame_SessionListener_get_implementation");
    line("com_sun_star_comp_framework_AddonsToolBarFactory_get_implementation");
    line("com_sun_star_comp_framework_AutoRecovery_get_implementation");
    line("com_sun_star_comp_framework_ControlMenuController_get_implementation");
    line("com_sun_star_comp_framework_Desktop_get_implementation");
    line("com_sun_star_comp_framework_DocumentAcceleratorConfiguration_get_implementation");
    line("org_apache_openoffice_comp_framework_ContextChangeEventMultiplexer_get_implementation");
    line("com_sun_star_comp_framework_Frame_get_implementation");
    line("com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation");
    line("com_sun_star_comp_framework_ImageManager_get_implementation");
    line("com_sun_star_comp_framework_JobExecutor_get_implementation");
    line("com_sun_star_comp_framework_LangSelectionStatusbarController_get_implementation");
    line("com_sun_star_comp_framework_LayoutManager_get_implementation");
    line("com_sun_star_comp_framework_MenuBarFactory_get_implementation");
    line("com_sun_star_comp_framework_ModuleAcceleratorConfiguration_get_implementation");
    line("com_sun_star_comp_framework_ModuleManager_get_implementation");
    line("com_sun_star_comp_framework_ModuleUIConfigurationManager_get_implementation");
    line("com_sun_star_comp_framework_ModuleUIConfigurationManagerSupplier_get_implementation");
    line("com_sun_star_comp_framework_ObjectMenuController_get_implementation");
    line("com_sun_star_comp_framework_PathSettings_get_implementation");
    line("com_sun_star_comp_framework_PathSubstitution_get_implementation");
    line("com_sun_star_comp_framework_PopupMenuControllerFactory_get_implementation");
    line("com_sun_star_comp_framework_RecentFilesMenuController_get_implementation");
    line("com_sun_star_comp_framework_SaveAsMenuController_get_implementation");
    line("com_sun_star_comp_framework_WindowListMenuController_get_implementation");
    line("com_sun_star_comp_framework_StatusBarControllerFactory_get_implementation");
    line("com_sun_star_comp_framework_StatusBarFactory_get_implementation");
    line("com_sun_star_comp_framework_StatusIndicatorFactory_get_implementation");
    line("com_sun_star_comp_framework_TaskCreator_get_implementation");
    line("com_sun_star_comp_framework_ToolBarControllerFactory_get_implementation");
    line("com_sun_star_comp_framework_ToolBarFactory_get_implementation");
    line("com_sun_star_comp_framework_UICategoryDescription_get_implementation");
    line("com_sun_star_comp_framework_UICommandDescription_get_implementation");
    line("com_sun_star_comp_framework_UIConfigurationManager_get_implementation");
    line("com_sun_star_comp_framework_UIElementFactoryManager_get_implementation");
    line("com_sun_star_comp_framework_URLTransformer_get_implementation");
    line("com_sun_star_comp_framework_WindowContentFactoryManager_get_implementation");
    line("com_sun_star_comp_framework_WindowStateConfiguration_get_implementation");
    line("com_sun_star_comp_framework_jobs_JobDispatch_get_implementation");
    line("com_sun_star_comp_framework_GenericPopupToolbarController_get_implementation");
    line("com_sun_star_comp_framework_SaveToolbarController_get_implementation");
    line("org_apache_openoffice_comp_framework_NewToolbarController_get_implementation");
    line("com_sun_star_comp_framework_SubToolBarController_get_implementation");
    line("com_sun_star_comp_framework_ThesaurusMenuController_get_implementation");
    line("com_sun_star_comp_framework_ToolbarAsMenuController_get_implementation");
    line("com_sun_star_comp_framework_ResourceMenuController_get_implementation");
    line("framework_LanguageSelectionMenuController_get_implementation");
    line("framework_FontMenuController_get_implementation");
    line("framework_FontSizeMenuController_get_implementation");
    line("framework_FooterMenuController_get_implementation");
    line("framework_HeaderMenuController_get_implementation");
    line("framework_MacrosMenuController_get_implementation");
    line("framework_MailToDispatcher_get_implementation");
    line("framework_MediaTypeDetectionHelper_get_implementation");
    line("framework_NewMenuController_get_implementation");
    line("framework_PopupMenuDispatcher_get_implementation");
    line("framework_ServiceHandler_get_implementation");
    line("framework_ToolbarsMenuController_get_implementation");
    line("framework_UriAbbreviation_get_implementation");
    line("framework_DispatchHelper_get_implementation");
    line("framework_DispatchDisabler_get_implementation");
    line("framework_HelpOnStartup_get_implementation");
    line("framework_ShellJob_get_implementation");
    line("framework_SystemExecute_get_implementation");
    // hwpfilter/source/hwp.component
    line("hwpfilter_HwpImportFilter_get_implementation");
    // i18npool/util/i18npool.component
    line("com_sun_star_i18n_BreakIterator_get_implementation");
    line("com_sun_star_i18n_BreakIterator_Unicode_get_implementation");
    line("i18npool_BreakIterator_ja_get_implementation");
    line("i18npool_BreakIterator_ko_get_implementation");
    line("i18npool_BreakIterator_zh_get_implementation");
    line("i18npool_BreakIterator_zh_TW_get_implementation");
    line("i18npool_CalendarImpl_get_implementation");
    line("i18npool_Calendar_ROC_get_implementation");
    line("i18npool_Calendar_dangi_get_implementation");
    line("i18npool_Calendar_buddhist_get_implementation");
    line("i18npool_Calendar_gengou_get_implementation");
    line("i18npool_Calendar_gregorian_get_implementation");
    line("i18npool_Calendar_hanja_get_implementation");
    line("i18npool_Calendar_hanja_yoil_get_implementation");
    line("i18npool_Calendar_hijri_get_implementation");
    line("i18npool_Calendar_jewish_get_implementation");
    line("i18npool_ChapterCollator_get_implementation");
    line("com_sun_star_i18n_CharacterClassification_get_implementation");
    line("com_sun_star_i18n_CharacterClassification_Unicode_get_implementation");
    line("com_sun_star_i18n_Collator_get_implementation");
    line("i18npool_Collator_Unicode_get_implementation");
    line("i18npool_IndexEntrySupplier_get_implementation");
    line("i18npool_IndexEntrySupplier_Unicode_get_implementation");
    line("i18npool_IndexEntrySupplier_asian_get_implementation");
    line("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant_get_implementation");
    line("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable_get_implementation");
    line("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant_get_implementation");
    line("i18npool_IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable_get_implementation");
    line("com_sun_star_i18n_InputSequenceChecker_get_implementation");
    line("i18npool_InputSequenceChecker_hi_get_implementation");
    line("i18npool_InputSequenceChecker_th_get_implementation");
    line("com_sun_star_i18n_LocaleDataImpl_get_implementation");
    line("com_sun_star_i18n_NativeNumberSupplier_get_implementation");
    line("com_sun_star_i18n_NumberFormatCodeMapper_get_implementation");
    line("com_sun_star_i18n_OrdinalSuffix_get_implementation");
    line("com_sun_star_i18n_ScriptTypeDetector_get_implementation");
    line("com_sun_star_i18n_TextConversion_get_implementation");
    line("i18npool_TextConversion_ko_get_implementation");
    line("i18npool_TextConversion_zh_get_implementation");
    line("com_sun_star_i18n_Transliteration_get_implementation");
    line("i18npool_CharToNumEastIndic_ar_get_implementation");
    line("i18npool_CharToNumFullwidth_get_implementation");
    line("i18npool_CharToNumHangul_ko_get_implementation");
    line("i18npool_CharToNumIndic_ar_get_implementation");
    line("i18npool_CharToNumIndic_hi_get_implementation");
    line("i18npool_CharToNumKanjiShort_ja_JP_get_implementation");
    line("i18npool_CharToNumKanjiTraditional_ja_JP_get_implementation");
    line("i18npool_CharToNumLower_ko_get_implementation");
    line("i18npool_CharToNumLower_zh_CN_get_implementation");
    line("i18npool_CharToNumLower_zh_TW_get_implementation");
    line("i18npool_CharToNumUpper_ko_get_implementation");
    line("i18npool_CharToNumUpper_zh_CN_get_implementation");
    line("i18npool_CharToNumUpper_zh_TW_get_implementation");
    line("i18npool_CharToNum_th_get_implementation");
    line("i18npool_FULLWIDTHKATAKANA_HALFWIDTHKATAKANA_get_implementation");
    line("i18npool_FULLWIDTH_HALFWIDTH_LIKE_ASC_get_implementation");
    line("i18npool_HALFWIDTHKATAKANA_FULLWIDTHKATAKANA_get_implementation");
    line("i18npool_HALFWIDTH_FULLWIDTH_LIKE_JIS_get_implementation");
    line("com_sun_star_i18n_Transliteration_IGNORE_CASE_get_implementation");
    line("com_sun_star_i18n_Transliteration_IGNORE_KANA_get_implementation");
    line("com_sun_star_i18n_Transliteration_IGNORE_WIDTH_get_implementation");
    line("i18npool_NumToCharEastIndic_ar_get_implementation");
    line("i18npool_NumToTextAIUFullWidth_ja_JP_get_implementation");
    line("i18npool_NumToTextAIUHalfWidth_ja_JP_get_implementation");
    line("i18npool_NumToTextFormalHangul_ko_get_implementation");
    line("i18npool_NumToTextFormalLower_ko_get_implementation");
    line("i18npool_NumToTextFormalUpper_ko_get_implementation");
    line("i18npool_NumToTextFullwidth_zh_CN_get_implementation");
    line("i18npool_NumToTextFullwidth_zh_TW_get_implementation");
    line("i18npool_NumToTextHangulCircledJamo_ko_get_implementation");
    line("i18npool_NumToTextHangulCircledSyllable_ko_get_implementation");
    line("i18npool_NumToTextHangulJamo_ko_get_implementation");
    line("i18npool_NumToTextHangulSyllable_ko_get_implementation");
    line("i18npool_NumToTextIROHAFullWidth_ja_JP_get_implementation");
    line("i18npool_NumToTextIROHAHalfWidth_ja_JP_get_implementation");
    line("i18npool_NumToTextInformalHangul_ko_get_implementation");
    line("i18npool_NumToTextInformalLower_ko_get_implementation");
    line("i18npool_NumToTextInformalUpper_ko_get_implementation");
    line("i18npool_NumToTextLower_zh_CN_get_implementation");
    line("i18npool_NumToTextLower_zh_TW_get_implementation");
    line("i18npool_NumToTextUpper_zh_CN_get_implementation");
    line("i18npool_NumToTextUpper_zh_TW_get_implementation");
    line("com_sun_star_text_DefaultNumberingProvider_get_implementation");
    // i18nsearch/source/search/i18nsearch.component
    line("i18npool_TextSearch_get_implementation");
    // io/source/io.component
    line("io_Pump_get_implementation");
    line("io_ODataInputStream_get_implementation");
    line("io_ODataOutputStream_get_implementation");
    line("io_OMarkableInputStream_get_implementation");
    line("io_OMarkableOutputStream_get_implementation");
    line("io_OObjectInputStream_get_implementation");
    line("io_OObjectOutputStream_get_implementation");
    line("io_OPipeImpl_get_implementation");
    line("io_OAcceptor_get_implementation");
    line("io_OConnector_get_implementation");
    line("io_OTextInputStream_get_implementation");
    line("io_OTextOutputStream_get_implementation");
    // lingucomponent/source/hyphenator/hyphen/hyphen.component
    line("lingucomponent_Hyphenator_get_implementation");
    // lingucomponent/source/languageguessing/guesslang.component
    line("lingucomponent_LangGuess_get_implementation");
#if !ENABLE_FUZZERS
    // lingucomponent/source/numbertext/numbertext.component
    line("lingucomponent_NumberText_get_implementation");
    // lingucomponent/source/spellcheck/languagetool/LanguageTool.component
    line("lingucomponent_LanguageToolGrammarChecker_get_implementation");
#endif
    // lingucomponent/source/spellcheck/lightproof/Lightproof.component
    line("lingucomponent_Lightproof_get_implementation");
#ifdef MACOSX
    // lingucomponent/source/spellcheck/macosxspell/MacOSXSpell.component
    line("lingucomponent_MacSpellChecker_get_implementation");
#endif
    // lingucomponent/source/spellcheck/spell/spell.component
    line("lingucomponent_SpellChecker_get_implementation");
    // lingucomponent/source/thesaurus/libnth/lnth.component
    line("lingucomponent_Thesaurus_get_implementation");
    line("lingucomponent_SpellChecker_get_implementation");
    line("lingucomponent_LangGuess_get_implementation");
    line("lingucomponent_Hyphenator_get_implementation");
    // linguistic/source/lng.component
    line("linguistic_ConvDicList_get_implementation");
    line("linguistic_DicList_get_implementation");
    line("linguistic_LinguProps_get_implementation");
    line("linguistic_LngSvcMgr_get_implementation");
    line("linguistic_GrammarCheckingIterator_get_implementation");
    // oox/util/oox.component
    line("com_sun_star_comp_oox_core_FastTokenHandler_get_implementation");
    line("com_sun_star_comp_oox_FormatDetector_get_implementation");
    line("com_sun_star_comp_oox_docprop_DocumentPropertiesImporter_get_implementation");
    line("com_sun_star_comp_oox_ppt_PowerPointImport_get_implementation");
    line("com_sun_star_comp_oox_crypto_StrongEncryptionDataSpace_get_implementation");
    // package/source/xstor/xstor.component
    line("package_OStorageFactory_get_implementation");
    // package/util/package2.component
    line("package_OZipFileAccess_get_implementation");
    line("package_ZipPackage_get_implementation");
    line("package_ManifestReader_get_implementation");
    line("package_ManifestWriter_get_implementation");
    // pyuno/source/loader/pythonloader.component
    // TODO need a config_foo.h define for this
    line("pyuno_Loader_get_implementation");
    // remotebridges/source/unourl_resolver/uuresolver.component
    line("remotebridges_ResolverImpl_get_implementation");
    // sax/source/expatwrap/expwrap.component
    line("com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation");
    line("com_sun_star_extensions_xml_sax_Writer_get_implementation");
    line("com_sun_star_comp_extensions_xml_sax_FastParser_get_implementation");
    line("com_sun_star_comp_extensions_xml_sax_LegacyFastParser_get_implementation");
    // scaddins/source/analysis/analysis.component
    line("scaddins_AnalysisAddIn_get_implementation");
    // scaddins/source/datefunc/date.component
    line("scaddins_ScaDateAddIn_get_implementation");
    // scaddins/source/pricing/pricing.component
    line("scaddins_ScaPricingAddIn_get_implementation");
    // scripting/source/protocolhandler/protocolhandler.component
    line("scripting_ScriptProtocolHandler_get_implementation");
#if HAVE_FEATURE_SCRIPTING
    // scripting/source/basprov/basprov.component
    line("scripting_BasicProviderImpl_get_implementation");
    // scripting/source/dlgprov/dlgprov.component
    line("scripting_DialogProviderImpl_get_implementation");
    line("scripting_DialogModelProvider_get_implementation");
    // scripting/source/stringresource/stringresource.component
    line("scripting_StringResourcePersistenceImpl_implementation");
    line("scripting_StringResourceWithLocationImpl_get_implementation");
    line("scripting_StringResourceWithStorageImpl_get_implementation");
    // scripting/source/vbaevents/vbaevents.component
    line("ooo_vba_VBAToOOEventDesc_get_implementation");
    line("ooo_vba_EventListener_get_implementation");
    // scripting/util/scriptframe.component
    line("scripting_BrowseNodeFactoryImpl_get_implementation");
    line("scripting_MasterScriptProvider_get_implementation");
    line("scripting_MasterScriptProviderFactory_get_implementation");
    line("scripting_ScriptingFrameworkURIHelper_get_implementation");
#endif
    // sc/source/solver/solver.component
    line("com_sun_star_comp_Calc_CoinMPSolver_get_implementation");
    line("com_sun_star_comp_Calc_SLPSolver_get_implementation");
    line("com_sun_star_comp_Calc_SwarmSolver_get_implementation");
    // sc/util/sc.component
    line("Calc_FilterOptionsDialog_get_implementation");
    line("Calc_SpreadsheetDocument_get_implementation");
    line("Calc_XMLContentExporter_get_implementation");
    line("Calc_XMLExporter_get_implementation");
    line("Calc_XMLMetaExporter_get_implementation");
    line("Calc_XMLOasisContentExporter_get_implementation");
    line("Calc_XMLOasisContentImporter_get_implementation");
    line("Calc_XMLOasisExporter_get_implementation");
    line("Calc_XMLOasisImporter_get_implementation");
    line("Calc_XMLOasisMetaExporter_get_implementation");
    line("Calc_XMLOasisMetaImporter_get_implementation");
    line("Calc_XMLOasisSettingsExporter_get_implementation");
    line("Calc_XMLOasisSettingsImporter_get_implementation");
    line("Calc_XMLOasisStylesExporter_get_implementation");
    line("Calc_XMLOasisStylesImporter_get_implementation");
    line("Calc_XMLSettingsExporter_get_implementation");
    line("Calc_XMLStylesExporter_get_implementation");
    line("ScAutoFormatsObj_get_implementation");
    line("ScFunctionAccess_get_implementation");
    line("ScFunctionListObj_get_implementation");
    line("ScRecentFunctionsObj_get_implementation");
    line("Calc_ScSpreadsheetSettings_get_implementation");
    line("ScPanelFactory_get_implementation");
    line("com_sun_star_comp_oox_xls_ExcelFilter_get_implementation");
    line("com_sun_star_comp_oox_xls_FormulaParser_get_implementation");
    line("com_sun_star_comp_sc_OrcusFormatDetect_get_implementation");
    // sc/util/scd.component
    line("com_sun_star_comp_calc_ExcelBiffFormatDetector_get_implementation");
    line("com_sun_star_comp_calc_FormatDetector_get_implementation");
#if !ENABLE_FUZZERS
    // sc/util/scui.component
    line("com_sun_star_sheet_CreateDialogFactoryService_get_implementation");
#endif
#if HAVE_FEATURE_SCRIPTING
    // sc/util/vbaobj.component
    line("Calc_ScVbaApplication_get_implementation");
    line("ScVbaEventsHelper_get_implementation");
    line("ScVbaGlobals_get_implementation");
    line("Calc_ScVbaHyperlink_get_implementation");
    line("ScVbaTextFrame_get_implementation");
    line("Calc_ScVbaWindow_get_implementation");
    line("Calc_ScVbaWorkbook_get_implementation");
    line("Calc_ScVbaWorksheet_get_implementation");
    line("Calc_ScVbaRange_get_implementation");
    /// scriptinterop/util/scriptinterop.component
    line("scriptinterop_Cool_get_implementation");
#endif
#if HAVE_FEATURE_PDFIMPORT
    // sdext/source/pdfimport/pdfimport.component
    line("sdext_PDFIHybridAdaptor_get_implementation");
    line("sdext_PDFDetector_get_implementation");
#endif
    // sd/util/sd.component
    line("sd_DrawingDocument_get_implementation");
    line("com_sun_star_comp_Draw_DrawingModule_get_implementation");
    line("sd_PresentationDocument_get_implementation");
    line("org_openoffice_comp_Draw_framework_PanelFactory_get_implementation");
    line("com_sun_star_comp_sd_SlideMasterPagesToolBoxControl_get_implementation");
    line("com_sun_star_comp_sd_InsertSlideController_get_implementation");
    line("com_sun_star_comp_sd_SlideTransitionsToolBoxControl_get_implementation");
    line("com_sun_star_comp_sd_SlideLayoutController_get_implementation");
    line("com_sun_star_comp_sd_DisplayModeController_get_implementation");
    line("RandomAnimationNode_get_implementation");
    line("css_comp_Impress_oox_PowerPointExport");
    // sd/util/sdd.component
    line("com_sun_star_comp_draw_FormatDetector_get_implementation");
#if !ENABLE_FUZZERS
    // sd/util/sdui.component
    line("com_sun_star_presentation_CreateDialogFactoryService_get_implementation");
#endif
    // sfx2/util/sfx.component
    line("com_sun_star_comp_sfx2_BackingComp_get_implementation");
    line("SfxDocumentMetaData_get_implementation");
    line("CompatWriterDocPropsImpl_get_implementation");
    line("com_sun_star_comp_document_OwnSubFilter_get_implementation");
    line("com_sun_star_comp_embed_PackageStructureCreator_get_implementation");
    line("com_sun_star_comp_office_FrameLoader_get_implementation");
    line("com_sun_star_comp_sfx2_AppDispatchProvider_get_implementation");
    line("com_sun_star_comp_sfx2_ApplicationDialogLibraryContainer_get_implementation");
    line("com_sun_star_comp_sfx2_ApplicationScriptLibraryContainer_get_implementation");
    line("com_sun_star_comp_sfx2_DocumentTemplates_get_implementation");
    line("com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation");
    line("com_sun_star_comp_sfx2_IFrameObject_get_implementation");
    line("com_sun_star_comp_sfx2_SfxMacroLoader_get_implementation");
    line("com_sun_star_sfx2_ClassificationCategoriesController_get_implementation");
    line("com_sun_star_comp_sfx2_InsertSymbolToolBoxControl_get_implementation");
    // shell/source/backends/desktopbe/desktopbe1.component
    line("shell_DesktopBackend_get_implementation");
    // shell/source/backends/localebe/localebe1.component
    line("shell_LocaleBackend_get_implementation");
#ifdef MACOSX
    // shell/source/backends/macbe/macbe1.component
    line("shell_MacOSXBackend_get_implementation");
#endif
#ifdef _WIN32
    // shell/source/backends/wininetbe/wininetbe1.component
    line("shell_WinInetBackend_get_implementation");
    // shell/source/cmdmail/cmdmail.component
    line("shell_CmdMailSuppl_get_implementation");
#endif
    // shell/source/sessioninstall/losessioninstall.component
    line("shell_sessioninstall_get_implementation");
    // shell/source/unix/exec/syssh.component
    line("shell_ShellExec_get_implementation");
#ifdef _WIN32
    // shell/source/win32/jumplist/jumplist.component
    line("shell_JumpListExec_get_implementation");
    // shell/source/win32/simplemail/smplmail.component
    line("shell_CSmplMailSuppl_get_implementation");
    // shell/source/win32/syssh.component
    line("shell_CSysShExec_get_implementation");
#endif
    // sot/util/sot.component
    line("com_sun_star_comp_embed_OLESimpleStorage");
    // starmath/util/sm.component
    line("Math_FormulaDocument_get_implementation");
    line("Math_XMLContentExporter_get_implementation");
    line("Math_XMLExporter_get_implementation");
    line("Math_XMLImporter_get_implementation");
    line("Math_XMLMetaExporter_get_implementation");
    line("Math_XMLOasisMetaExporter_get_implementation");
    line("Math_XMLOasisMetaImporter_get_implementation");
    line("Math_XMLOasisSettingsExporter_get_implementation");
    line("Math_XMLOasisSettingsImporter_get_implementation");
    line("Math_XMLSettingsExporter_get_implementation");
    line("com_sun_star_comp_Math_MathTypeFilter_get_implementation");
    line("org_libreoffice_comp_Math_sidebar_SmPanelFactory");
    line("com_sun_star_comp_starmath_NotebookbarElementsController_get_implementation");
    // starmath/util/smd.component
    line("math_FormatDetector_get_implementation");
    // stoc/source/corereflection/reflection.component
    line("com_sun_star_comp_stoc_CoreReflection_get_implementation");
    line("com_sun_star_comp_stoc_Dump_get_implementation");
    // stoc/source/inspect/introspection.component
    line("com_sun_star_comp_stoc_Introspection_get_implementation");
    // stoc/source/invocation_adapter/invocadapt.component
    line("stoc_invocation_adapter_get_implementation");
    // stoc/source/invocation/invocation.component
    line("stoc_InvocationService_get_implementation");
#if HAVE_FEATURE_JAVA
    // stoc/source/javaloader/javaloader.component
    line("stoc_JavaComponentLoader_get_implementation");
    // stoc/source/javavm/javavm.component
    line("stoc_JavaVM_get_implementation");
#endif
    // stoc/source/namingservice/namingservice.component
    line("stoc_NamingService_Impl_get_implementation");
    // stoc/source/proxy_factory/proxyfac.component
    line("stoc_FactoryImpl_get_implementation");
    // stoc/util/bootstrap.component
    line("com_sun_star_comp_stoc_DLLComponentLoader_get_implementation");
    line("com_sun_star_comp_stoc_ImplementationRegistration_get_implementation");
    line("com_sun_star_comp_stoc_NestedRegistry_get_implementation");
    line("com_sun_star_comp_stoc_ORegistryServiceManager_get_implementation");
    line("com_sun_star_comp_stoc_OServiceManager_get_implementation");
    line("com_sun_star_comp_stoc_OServiceManagerWrapper_get_implementation");
    line("com_sun_star_comp_stoc_SimpleRegistry_get_implementation");
    // stoc/util/stocservices.component
    line("com_sun_star_comp_stoc_OServiceManagerWrapper_get_implementation");
    line("com_sun_star_comp_stoc_TypeConverter_get_implementation");
    line("com_sun_star_comp_uri_ExternalUriReferenceTranslator_get_implementation");
    line("com_sun_star_comp_uri_UriReferenceFactory_get_implementation");
    line("com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTexpand_get_implementation");
    line("com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTscript_get_implementation");
    line("com_sun_star_comp_uri_VndSunStarPkgUrlReferenceFactory_get_implementation");
    // svgio/svgio.component
    line("svgio_XSvgParser_get_implementation");
    // svl/source/fsstor/fsstorage.component
    line("svl_FSStorageFactory_get_implementation");
    // svl/source/passwordcontainer/passwordcontainer.component
    line("svl_PasswordContainer_get_implementation");
    // svl/util/svl.component
    line("com_sun_star_comp_svl_PathService_get_implementation");
    line("com_sun_star_uno_util_numbers_SvNumberFormatsSupplierServiceObject_get_implementation");
    line("com_sun_star_uno_util_numbers_SvNumberFormatterServiceObject_get_implementation");
    // svtools/util/svt.component
    line("com_sun_star_comp_graphic_GraphicRendererVCL_get_implementation");
    line("com_sun_star_comp_svtools_OAddressBookSourceDialogUno_get_implementation");
    line("com_sun_star_comp_svtools_uno_Wizard_get_implementation");
    line("com_sun_star_svtools_SvFilterOptionsDialog_get_implementation");
    line("com_sun_star_comp_embed_DocumentCloser_get_implementation");
    // svx/util/svx.component
    line("com_sun_star_comp_gallery_GalleryThemeProvider_get_implementation");
    line("com_sun_star_comp_svx_Impl_FindbarDispatcher_get_implementation");
    line("com_sun_star_drawing_SvxShapeCollection_get_implementation");
    line("com_sun_star_drawing_SvxUnoColorTable_get_implementation");
    line("com_sun_star_svx_DownSearchToolboxController_get_implementation");
    line("com_sun_star_svx_FindTextToolboxController_get_implementation");
    line("com_sun_star_svx_MatchCaseToolboxController_get_implementation");
    line("com_sun_star_svx_MatchDiacriticsToolboxController_get_implementation");
    line("com_sun_star_svx_SearchFormattedToolboxController_get_implementation");
    line("com_sun_star_svx_FindAllToolboxController_get_implementation");
    line("com_sun_star_svx_SearchLabelToolboxController_get_implementation");
    line("com_sun_star_svx_FontHeightToolBoxController_get_implementation");
    line("com_sun_star_svx_UpSearchToolboxController_get_implementation");
    line("com_sun_star_svx_ExitFindbarToolboxController_get_implementation");
    line("org_apache_openoffice_comp_svx_sidebar_PanelFactory_get_implementation");
    line("com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_SmartTagMenuController_get_implementation");
    line("com_sun_star_comp_svx_LineSpacingToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_CharacterSpacingToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_TableToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_ColumnsToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_UnderlineToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_UndoRedoToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_CTLToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_VertTextToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_NotebookbarLineController_get_implementation");
    line("com_sun_star_comp_svx_ThemeColorsToolBoxControl_get_implementation");
    // svx/util/svxcore.component
    line("com_sun_star_comp_Draw_GraphicExporter_get_implementation");
    line("com_sun_star_comp_svx_ExtrusionDepthController_get_implementation");
    line("com_sun_star_comp_svx_ExtrusionDirectionControl_get_implementation");
    line("com_sun_star_comp_svx_ExtrusionLightingControl_get_implementation");
    line("com_sun_star_comp_svx_ExtrusionSurfaceControl_get_implementation");
    line("com_sun_star_comp_svx_FontworkAlignmentControl_get_implementation");
    line("com_sun_star_comp_svx_FontworkCharacterSpacingControl_get_implementation");
    line("com_sun_star_comp_svx_FrameToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_ColorToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_CurrencyToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_FontNameToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_StyleToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_LineEndToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_LineStyleToolBoxControl_get_implementation");
    line("com_sun_star_comp_graphic_PrimitiveFactory2D_get_implementation");
    line("com_sun_star_comp_svx_StylesPreviewToolBoxControl_get_implementation");
    line("com_sun_star_comp_svx_TextColumns_get_implementation");
    line("com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation");
    // sw/util/msword.component
    line("com_sun_star_comp_Writer_RtfExport_get_implementation");
    line("com_sun_star_comp_Writer_DocxExport_get_implementation");
    // sw/util/sw.component
    line("lo_writer_NavElementToolBoxController_get_implementation");
    line("lo_writer_PreviousScrollToolboxController_get_implementation");
    line("lo_writer_NextScrollToolboxController_get_implementation");
    line("SwXAutoTextContainer_get_implementation");
    line("SwXMailMerge_get_implementation");
    line("lo_writer_MMCurrentEntryController_get_implementation");
    line("lo_writer_MMExcludeEntryController_get_implementation");
    line("lo_writer_PageSizeToolBoxControl_get_implementation");
    line("lo_writer_PageColumnToolBoxControl_get_implementation");
    line("lo_writer_PageMarginToolBoxControl_get_implementation");
    line("lo_writer_PageOrientationToolBoxControl_get_implementation");
    line("SwXModule_get_implementation");
    line("com_sun_star_comp_Writer_FilterOptionsDialog_get_implementation");
    line("com_sun_star_comp_Writer_GlobalDocument_get_implementation");
    line("Writer_SwTextDocument_get_implementation");
    line("com_sun_star_comp_Writer_WebDocument_get_implementation");
    line("com_sun_star_comp_Writer_WriterModule_get_implementation");
    line("com_sun_star_comp_Writer_XMLContentExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLMetaExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisContentExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisContentImporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisImporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisMetaExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisMetaImporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisSettingsExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisSettingsImporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisStylesExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisStylesImporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLSettingsExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLStylesExporter_get_implementation");
    line("com_sun_star_util_comp_FinalThreadManager_get_implementation");
    line("com_sun_star_comp_Writer_IndexingExportFilter_get_implementation");
    line("com_sun_star_comp_Writer_LayoutDump_get_implementation");
    line("org_apache_openoffice_comp_sw_sidebar_SwPanelFactory_get_implementation");
    // sw/util/swd.component
    line("com_sun_star_comp_writer_FormatDetector_get_implementation");
#if !ENABLE_FUZZERS
    // sw/util/swui.component
    line("com_sun_star_text_DialogFactoryService_get_implementation");
#endif
    // sw/util/sw_writerfilter.component
    line("com_sun_star_comp_Writer_RtfFilter_get_implementation");
    line("com_sun_star_comp_Writer_WriterFilter_get_implementation");
#if HAVE_FEATURE_SCRIPTING
    // sw/util/vbaswobj.component
    line("Writer_SwVbaDocument_get_implementation");
    line("Writer_SwVbaEventsHelper_get_implementation");
    line("Writer_SwVbaGlobals_get_implementation");
    line("Writer_SwVbaWrapFormat_get_implementation");
#endif
// these are not included
//    // unotest/source/testuno/testuno.component
//    line("com_sun_star_comp_testuno_BridgeTest_get_implementation");
//    line("com_sun_star_comp_testuno_Test_get_implementation");
//    line("com_sun_star_comp_testuno_Constructors_get_implementation");
//    line("com_sun_star_comp_testuno_Singleton_get_implementation");
    // toolkit/util/tk.component
    line("com_sun_star_awt_comp_AsyncCallback_get_implementation");
    line("com_sun_star_comp_embed_HatchWindowFactory_get_implementation");
    line("stardiv_Toolkit_UnoControlTabPage_get_implementation");
    line("stardiv_Toolkit_UnoControlTabPageModel_get_implementation");
    line("stardiv_Toolkit_UnoControlTabPageContainerModel_get_implementation");
    line("stardiv_Toolkit_UnoControlTabPageContainer_get_implementation");
    line("stardiv_Toolkit_DefaultGridColumnModel_get_implementation");
    line("stardiv_Toolkit_DefaultGridDataModel_get_implementation");
    line("org_openoffice_comp_toolkit_GridColumn_get_implementation");
    line("org_openoffice_comp_toolkit_SortableGridDataModel_get_implementation");
    line("stardiv_Toolkit_GridControl_get_implementation");
    line("stardiv_Toolkit_GridControlModel_get_implementation");
    line("stardiv_Toolkit_MutableTreeDataModel_get_implementation");
    line("stardiv_Toolkit_StdTabController_get_implementation");
    line("stardiv_Toolkit_StdTabControllerModel_get_implementation");
    line("stardiv_Toolkit_TreeControl_get_implementation");
    line("stardiv_Toolkit_TreeControlModel_get_implementation");
    line("stardiv_Toolkit_UnoButtonControl_get_implementation");
    line("stardiv_Toolkit_UnoCheckBoxControl_get_implementation");
    line("stardiv_Toolkit_UnoComboBoxControl_get_implementation");
    line("stardiv_Toolkit_UnoControlButtonModel_get_implementation");
    line("stardiv_Toolkit_UnoControlCheckBoxModel_get_implementation");
    line("stardiv_Toolkit_UnoControlComboBoxModel_get_implementation");
    line("stardiv_Toolkit_UnoControlContainer_get_implementation");
    line("stardiv_Toolkit_UnoControlContainerModel_get_implementation");
    line("stardiv_Toolkit_UnoControlCurrencyFieldModel_get_implementation");
    line("stardiv_Toolkit_UnoControlDateFieldModel_get_implementation");
    line("stardiv_Toolkit_UnoControlDialogModel_get_implementation");
    line("stardiv_Toolkit_UnoControlEditModel_get_implementation");
    line("stardiv_Toolkit_UnoControlFileControlModel_get_implementation");
    line("stardiv_Toolkit_UnoControlFixedHyperlinkModel_get_implementation");
    line("stardiv_Toolkit_UnoControlFixedLineModel_get_implementation");
    line("stardiv_Toolkit_UnoControlFixedTextModel_get_implementation");
    line("stardiv_Toolkit_UnoControlFormattedFieldModel_get_implementation");
    line("stardiv_Toolkit_UnoControlGroupBoxModel_get_implementation");
    line("stardiv_Toolkit_UnoControlImageControlModel_get_implementation");
    line("stardiv_Toolkit_UnoControlListBoxModel_get_implementation");
    line("stardiv_Toolkit_UnoControlNumericFieldModel_get_implementation");
    line("stardiv_Toolkit_UnoControlPatternFieldModel_get_implementation");
    line("stardiv_Toolkit_UnoControlProgressBarModel_get_implementation");
    line("stardiv_Toolkit_UnoControlRadioButtonModel_get_implementation");
    line("stardiv_Toolkit_UnoControlRoadmapModel_get_implementation");
    line("stardiv_Toolkit_UnoControlScrollBarModel_get_implementation");
    line("stardiv_Toolkit_UnoControlTimeFieldModel_get_implementation");
    line("stardiv_Toolkit_UnoCurrencyFieldControl_get_implementation");
    line("stardiv_Toolkit_UnoDateFieldControl_get_implementation");
    line("stardiv_Toolkit_UnoDialogControl_get_implementation");
    line("stardiv_Toolkit_UnoEditControl_get_implementation");
    line("stardiv_Toolkit_UnoFileControl_get_implementation");
    line("stardiv_Toolkit_UnoFixedHyperlinkControl_get_implementation");
    line("stardiv_Toolkit_UnoFixedLineControl_get_implementation");
    line("stardiv_Toolkit_UnoFixedTextControl_get_implementation");
    line("stardiv_Toolkit_UnoFormattedFieldControl_get_implementation");
    line("stardiv_Toolkit_UnoGroupBoxControl_get_implementation");
    line("stardiv_Toolkit_UnoImageControlControl_get_implementation");
    line("stardiv_Toolkit_UnoListBoxControl_get_implementation");
    line("stardiv_Toolkit_UnoNumericFieldControl_get_implementation");
    line("stardiv_Toolkit_UnoPatternFieldControl_get_implementation");
    line("stardiv_Toolkit_UnoProgressBarControl_get_implementation");
    line("stardiv_Toolkit_UnoRadioButtonControl_get_implementation");
    line("stardiv_Toolkit_UnoRoadmapControl_get_implementation");
    line("stardiv_Toolkit_UnoScrollBarControl_get_implementation");
    line("org_openoffice_comp_toolkit_SpinningProgressControlModel_get_implementation");
    line("org_openoffice_comp_toolkit_AnimatedImagesControlModel_get_implementation");
    line("org_openoffice_comp_toolkit_AnimatedImagesControl_get_implementation");
    line("stardiv_Toolkit_UnoSpinButtonControl_get_implementation");
    line("stardiv_Toolkit_UnoSpinButtonModel_get_implementation");
    line("stardiv_Toolkit_UnoTimeFieldControl_get_implementation");
    line("stardiv_Toolkit_VCLXMenuBar_get_implementation");
    line("stardiv_Toolkit_VCLXPointer_get_implementation");
    line("stardiv_Toolkit_VCLXPopupMenu_get_implementation");
    line("stardiv_Toolkit_VCLXToolkit_get_implementation");
    line("stardiv_Toolkit_UnoMultiPageModel_get_implementation");
    line("stardiv_Toolkit_UnoMultiPageControl_get_implementation");
    line("stardiv_Toolkit_UnoPageModel_get_implementation");
    line("stardiv_Toolkit_UnoPageControl_get_implementation");
    line("stardiv_Toolkit_UnoFrameModel_get_implementation");
    line("stardiv_Toolkit_UnoFrameControl_get_implementation");
    // ucb/source/cacher/cached1.component
    line("ucb_CachedContentResultSetFactory_get_implementation");
    line("ucb_CachedContentResultSetStubFactory_get_implementation");
    line("ucb_CachedDynamicResultSetFactory_get_implementation");
    line("ucb_CachedDynamicResultSetStubFactory_get_implementation");
    // ucb/source/core/ucb1.component
    line("ucb_UcbCommandEnvironment_get_implementation");
    line("ucb_UcbContentProviderProxyFactory_get_implementation");
    line("ucb_UcbPropertiesManager_get_implementation");
    line("ucb_UcbStore_get_implementation");
    line("ucb_UniversalContentBroker_get_implementation");
    line("ucb_OFileAccess_get_implementation");
    // ucb/source/sorter/srtrs1.component
    line("ucb_SortedDynamicResultSetFactory_get_implementation");
    // ucb/source/tdoc/ucptdoc1.component
    line("ucb_tdoc_ContentProvider_get_implementation");
    line("ucb_tdoc_DocumentContentFactory_get_implementation");
    // ucb/source/ucp/cmis/ucpcmis1.component
    // TODO this needs an entry in config_foo.h
    line("ucb_cmis_ContentProvider_get_implementation");
    // ucb/source/ucp/expand/ucpexpand1.component
    line("ucb_expand_ExpandContentProviderImpl_get_implementation");
    // ucb/source/ucp/file/ucpfile1.component
    line("ucb_file_FileProvider_get_implementation");
#if ENABLE_GIO
    // ucb/source/ucp/gio/ucpgio.component
    line("ucb_gio_ContentProvider_get_implementation");
#endif
    // ucb/source/ucp/ucphier1.component
    line("ucb_HierarchyContentProvider_get_implementation");
    line("ucb_HierarchyDataSource_get_implementation");
    // ucb/source/ucp/image/ucpimage.component
    line("com_sun_star_comp_ucb_ImageContentProvider_get_implementation");
    // ucb/source/ucp/package/ucppkg1
    line("ucb_package_ContentProvider_get_implementation");
    // ucb/source/ucp/tdoc/ucptdoc1.component
    line("ucb_tdoc_ContentProvider_get_implementation");
    line("ucb_tdoc_DocumentContentFactory_get_implementation");
    // ucb/source/ucp/webdav-curl/ucpdav1.component
    line("ucb_webdav_ContentProvider_get_implementation");
    line("ucb_webdav_manager_get_implementation");
    // unotools/util/utl.component
    line("unotools_ServiceDocument_get_implementation");
    line("unotools_OTempFileService_get_implementation");
    // unoxml/source/rdf/unordf.component
    line("unoxml_rdfRepository_get_implementation");
    line("unoxml_CURI_get_implementation");
    line("unoxml_CLiteral_get_implementation");
    line("unoxml_CBlankNode_get_implementation");
    // unoxml/source/service/unoxml.component
    line("unoxml_CXPathAPI_get_implementation");
    line("unoxml_CSAXDocumentBuilder_get_implementation");
    line("unoxml_CDocumentBuilder_get_implementation");
    // uui/util/uui.component
    line("com_sun_star_comp_uui_UUIInteractionHandler_get_implementation");
    line("com_sun_star_comp_uui_UUIInteractionRequestStringResolver_get_implementation");
    line("com_sun_star_comp_uui_PasswordContainerInteractionHandler_get_implementation");
#if HAVE_FEATURE_SCRIPTING
    // vbahelper/util/msforms.component
    line("ControlProviderImpl_get_implementation");
    line("ScVbaUserForm_get_implementation");
#endif
    // vcl/vcl.common.component
    line("com_sun_star_comp_graphic_GraphicMapper_get_implementation");
    line("com_sun_star_comp_graphic_GraphicProvider_get_implementation");
#if !ENABLE_FUZZERS
    line("com_sun_star_cui_ColorPicker_get_implementation");
#endif
    line("vcl_SystemClipboard_get_implementation");
    line("dtrans_CMimeContentTypeFactory_get_implementation");
    line("com_sun_star_frame_VCLSessionManagerClient_get_implementation");
    line("com_sun_star_graphic_GraphicObject_get_implementation");
    line("UITest_get_implementation");
    line("vcl_FontIdentificator_get_implementation");
#ifdef _WIN32
    // vcl/vclplug_win.component
    line("dtrans_CDataFormatTranslatorUNO_get_implementation");
    line("dtrans_DragSource_get_implementation");
    line("dtrans_DropTarget_get_implementation");
    line("dtrans_ClipboardManager_get_implementation");
    line("dtrans_GenericClipboard_get_implementation");
    // winaccessibility/source/service/winaccessibility.component
    line("winaccessibility_MSAAServiceImpl_get_implementation");
#endif
    // writerperfect/source/calc/wpftcalc.component
    line("com_sun_star_comp_Calc_MSWorksCalcImportFilter_get_implementation");
    line("com_sun_star_comp_Calc_MWAWCalcImportFilter_get_implementation");
    line("org_libreoffice_comp_Calc_NumbersImportFilter_get_implementation");
    line("org_libreoffice_comp_Calc_StarOfficeCalcImportFilter_get_implementation");
    // writerperfect/source/draw/wpftdraw.component
    line("com_sun_star_comp_Draw_CDRImportFilter_get_implementation");
    line("com_sun_star_comp_Draw_CMXImportFilter_get_implementation");
    line("com_sun_star_comp_Draw_FreehandImportFilter_get_implementation");
    line("com_sun_star_comp_Draw_MSPUBImportFilter_get_implementation");
    line("com_sun_star_comp_Draw_MWAWDrawImportFilter_get_implementation");
    line("org_libreoffice_comp_Draw_StarOfficeDrawImportFilter_get_implementation");
    line("com_sun_star_comp_Draw_VisioImportFilter_get_implementation");
    line("com_sun_star_comp_Draw_WPGImportFilter_get_implementation");
    line("org_libreoffice_comp_Draw_PageMakerImportFilter_get_implementation");
    line("org_libreoffice_comp_Draw_QXPImportFilter_get_implementation");
    line("org_libreoffice_comp_Draw_ZMFImportFilter_get_implementation");
    // writerperfect/source/impress/wpftimpress.component
    line("org_libreoffice_comp_Impress_KeynoteImportFilter_get_implementation");
    line("com_sun_star_comp_Impress_MWAWPresentationImportFilter_get_implementation");
    line("org_libreoffice_comp_Presentation_StarOfficePresentationImportFilter_get_implementation");
    // writerperfect/source/writer/wpftwriter.component
    line("com_sun_star_comp_Writer_AbiWordImportFilter_get_implementation");
    line("com_sun_star_comp_Writer_MSWorksImportFilter_get_implementation");
    line("com_sun_star_comp_Writer_MWAWImportFilter_get_implementation");
    line("com_sun_star_comp_Writer_WordPerfectImportFilter_get_implementation");
    line("org_libreoffice_comp_Writer_EBookImportFilter_get_implementation");
    line("org_libreoffice_comp_Writer_PagesImportFilter_get_implementation");
    line("org_libreoffice_comp_Writer_StarOfficeWriterImportFilter_get_implementation");
    line("com_sun_star_comp_Writer_EPUBExportFilter_get_implementation");
    line("com_sun_star_comp_Writer_EPUBExportUIComponent_get_implementation");
#if HAVE_FEATURE_XMLHELP
    // xmlhelp/util/ucpchelp1.component
    line("CHelpContentProvider_get_implementation");
    line("com_sun_star_help_TreeViewImpl_get_implementation");
#endif
    // xmloff/source/transform/xof.component
    line("xmloff_XMLCalcContentImportOOO_get_implementation");
    line("xmloff_XMLCalcImportOOO_get_implementation");
    line("xmloff_XMLCalcMetaImportOOO_get_implementation");
    line("xmloff_XMLCalcSettingsImportOOO_get_implementation");
    line("xmloff_XMLCalcStylesImportOOO_get_implementation");
    line("xmloff_XMLChartContentImportOOO_get_implementation");
    line("xmloff_XMLChartImportOOO_get_implementation");
    line("xmloff_XMLChartStylesImportOOO_get_implementation");
    line("xmloff_XMLDrawContentImportOOO_get_implementation");
    line("xmloff_XMLDrawImportOOO_get_implementation");
    line("xmloff_XMLDrawMetaImportOOO_get_implementation");
    line("xmloff_XMLDrawSettingsImportOOO_get_implementation");
    line("xmloff_XMLDrawStylesImportOOO_get_implementation");
    line("xmloff_XMLImpressContentImportOOO_get_implementation");
    line("xmloff_XMLImpressImportOOO_get_implementation");
    line("xmloff_XMLImpressMetaImportOOO_get_implementation");
    line("xmloff_XMLImpressSettingsImportOOO_get_implementation");
    line("xmloff_XMLImpressStylesImportOOO_get_implementation");
    line("xmloff_XMLMathMetaImportOOO_get_implementation");
    line("xmloff_XMLMathSettingsImportOOO_get_implementation");
    line("xmloff_OOo2OasisTransformer_get_implementation");
    line("xmloff_Oasis2OOoTransformer_get_implementation");
    line("xmloff_XMLAutoTextEventImportOOO_get_implementation");
    line("xmloff_XMLWriterContentImportOOO_get_implementation");
    line("xmloff_XMLWriterImportOOO_get_implementation");
    line("xmloff_XMLWriterMetaImportOOO_get_implementation");
    line("xmloff_XMLWriterSettingsImportOOO_get_implementation");
    line("xmloff_XMLWriterStylesImportOOO_get_implementation");
    line("xmloff_XMLMetaImportOOO_get_implementation");
    // xmloff/util/xo.component
    line("XMLMetaExportComponent_get_implementation");
    line("XMLMetaExportOOo_get_implementation");
    line("XMLMetaImportComponent_get_implementation");
    line("XMLVersionListPersistence_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisImporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisExporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisStylesExporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisContentExporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisMetaExporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisMetaImporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisContentImporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisStylesImporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisSettingsExporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLOasisSettingsImporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisSettingsExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisMetaExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisContentExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisStylesExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisExporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisImporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisStylesImporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisContentImporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisMetaImporter_get_implementation");
    line("com_sun_star_comp_Draw_XMLOasisSettingsImporter_get_implementation");
    line("com_sun_star_comp_Xmloff_AnimationsImport");
    line("com_sun_star_comp_DrawingLayer_XMLExporter_get_implementation");
    line("com_sun_star_comp_Impress_XMLClipboardExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisImporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisMetaImporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisMetaExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisStylesExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisContentExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLStylesExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLContentExporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisStylesImporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisContentImporter_get_implementation");
    line("com_sun_star_comp_Chart_XMLOasisExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisAutotextEventsExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLAutotextEventsExporter_get_implementation");
    line("com_sun_star_comp_Writer_XMLOasisAutotextEventsImporter_get_implementation");
    line("com_sun_star_comp_Svx_GraphicExportHelper_get_implementation");
    line("com_sun_star_comp_Svx_GraphicImportHelper_get_implementation");
    line("com_sun_star_comp_Svx_XPropertyTableExporter_get_implementation");
    line("com_sun_star_comp_Svx_XPropertyTableImporter_get_implementation");
    // xmlscript/util/xmlscript.component
    line("com_sun_star_comp_xml_input_SaxDocumentHandler_get_implementation");
    line("com_sun_star_comp_xmlscript_XMLBasicExporter");
    line("com_sun_star_comp_xmlscript_XMLOasisBasicExporter");
#if HAVE_FEATURE_NSS
    // xmlsecurity/util/xmlsecurity.component
    line("com_sun_star_security_CertificateContainer_get_implementation");
    line("com_sun_star_security_DocumentDigitalSignatures_get_implementation");
    // xmlsecurity/util/xsec_xmlsec.component
    line("com_sun_star_xml_crypto_NSSInitializer_get_implementation");
    line("com_sun_star_xml_crypto_SEInitializer_get_implementation");
    line("com_sun_star_xml_crypto_SecurityEnvironment_get_implementation");
    line("com_sun_star_xml_wrapper_XMLDocumentWrapper_get_implementation");
    line("com_sun_star_xml_wrapper_XMLElementWrapper_get_implementation");
    line("com_sun_star_xml_crypto_XMLSecurityContext_get_implementation");
    line("com_sun_star_xml_crypto_XMLSignature_get_implementation");
#endif

    std::cout << std::endl << "    local: *;" << std::endl << "};" << std::endl;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
