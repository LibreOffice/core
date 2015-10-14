# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,postprocess/registry))

#
# Variables
#

postprocess_XCS := $(call gb_XcsTarget_get_target,org/openoffice)
postprocess_XCU := $(call gb_XcuDataTarget_get_target,officecfg/registry/data/org/openoffice)
postprocess_MOD := $(call gb_XcuModuleTarget_get_target,officecfg/registry/data)
postprocess_DRIVERS :=

postprocess_XCDS := \
	base.xcd \
	calc.xcd \
	cjk.xcd \
	ctl.xcd \
	ctlseqcheck.xcd \
	draw.xcd \
	graphicfilter.xcd \
	impress.xcd \
	librelogo.xcd \
	lingucomponent.xcd \
	main.xcd \
	math.xcd \
	pyuno.xcd \
	writer.xcd \
	xsltfilter.xcd

postprocess_DEPS_base := main
postprocess_FILES_base := \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_database_filters.xcu) \
	$(call gb_XcuFilterOthersTarget_get_target,fcfg_database_others.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_database_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Common-base.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-base.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-base.xcu

postprocess_DEPS_calc := main
postprocess_FILES_calc := \
	$(postprocess_XCS)/Office/UI/CalcCommands.xcs \
	$(postprocess_XCS)/Office/UI/CalcWindowState.xcs \
	$(postprocess_XCU)/Office/UI/CalcCommands.xcu \
	$(postprocess_XCU)/Office/UI/CalcWindowState.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_calc_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_calc_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Common-calc.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-calc.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-calc.xcu

ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
postprocess_FILES_calc += \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/calc)/org/openoffice/Office/DataAccess/Drivers-calc.xcu
postprocess_DRIVERS += calc
endif

postprocess_DEPS_cjk := main
postprocess_FILES_cjk := \
	$(postprocess_MOD)/org/openoffice/Office/Common-cjk.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Writer-cjk.xcu

postprocess_DEPS_ctl := main
postprocess_FILES_ctl := \
	$(postprocess_MOD)/org/openoffice/Office/Common-ctl.xcu

postprocess_DEPS_draw := main
postprocess_FILES_draw := \
	$(postprocess_XCS)/Office/UI/DrawWindowState.xcs \
	$(postprocess_XCU)/Office/UI/DrawWindowState.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_draw_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_draw_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Common-draw.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-draw.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-draw.xcu

postprocess_DEPS_graphicfilter := main
postprocess_FILES_graphicfilter := \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_drawgraphics_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_drawgraphics_types.xcu) \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_impressgraphics_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_impressgraphics_types.xcu) \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_calcgraphics_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_calcgraphics_types.xcu) \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_writergraphics_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_writergraphics_types.xcu)

postprocess_DEPS_impress := main
postprocess_FILES_impress := \
	$(postprocess_XCS)/Office/PresentationMinimizer.xcs \
	$(postprocess_XCS)/Office/PresenterScreen.xcs \
	$(postprocess_XCS)/Office/UI/Effects.xcs \
	$(postprocess_XCS)/Office/UI/ImpressWindowState.xcs \
	$(postprocess_XCU)/Office/PresentationMinimizer.xcu \
	$(postprocess_XCU)/Office/PresenterScreen.xcu \
	$(postprocess_XCU)/Office/UI/Effects.xcu \
	$(postprocess_XCU)/Office/UI/ImpressWindowState.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_impress_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_impress_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Common-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Jobs-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Office/ProtocolHandler-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-impress.xcu

postprocess_DEPS_librelogo := main writer
postprocess_FILES_librelogo := \
	$(postprocess_MOD)/org/openoffice/Office/Addons-librelogo.xcu \
	$(postprocess_MOD)/org/openoffice/Office/UI/WriterWindowState-librelogo.xcu

postprocess_DEPS_lingucomponent := main
postprocess_FILES_lingucomponent := \
	$(SRCDIR)/lingucomponent/config/Linguistic-lingucomponent-hyphenator.xcu \
	$(SRCDIR)/lingucomponent/config/Linguistic-lingucomponent-spellchecker.xcu \
	$(SRCDIR)/lingucomponent/config/Linguistic-lingucomponent-thesaurus.xcu \

postprocess_FILES_main := \
	$(postprocess_XCS)/Inet.xcs \
	$(postprocess_XCS)/Interaction.xcs \
	$(postprocess_XCS)/LDAP.xcs \
	$(postprocess_XCS)/Office/Accelerators.xcs \
	$(postprocess_XCS)/Office/Addons.xcs \
	$(postprocess_XCS)/Office/BasicIDE.xcs \
	$(postprocess_XCS)/Office/Calc.xcs \
	$(postprocess_XCS)/Office/CalcAddIns.xcs \
	$(postprocess_XCS)/Office/Canvas.xcs \
	$(postprocess_XCS)/Office/Chart.xcs \
	$(postprocess_XCS)/Office/Commands.xcs \
	$(postprocess_XCS)/Office/Common.xcs \
	$(postprocess_XCS)/Office/Compatibility.xcs \
	$(postprocess_XCS)/Office/DataAccess/Drivers.xcs \
	$(postprocess_XCS)/Office/DataAccess.xcs \
	$(postprocess_XCS)/Office/Draw.xcs \
	$(postprocess_XCS)/Office/Embedding.xcs \
	$(postprocess_XCS)/Office/Events.xcs \
	$(postprocess_XCS)/Office/ExtendedColorScheme.xcs \
	$(postprocess_XCS)/Office/ExtensionDependencies.xcs \
	$(postprocess_XCS)/Office/ExtensionManager.xcs \
	$(postprocess_XCS)/Office/FormWizard.xcs \
	$(postprocess_XCS)/Office/Histories.xcs \
	$(postprocess_XCS)/Office/Impress.xcs \
	$(postprocess_XCS)/Office/Java.xcs \
	$(postprocess_XCS)/Office/Jobs.xcs \
	$(postprocess_XCS)/Office/Labels.xcs \
	$(postprocess_XCS)/Office/Linguistic.xcs \
	$(postprocess_XCS)/Office/Logging.xcs \
	$(postprocess_XCS)/Office/Math.xcs \
	$(postprocess_XCS)/Office/OOoImprovement/Settings.xcs \
	$(postprocess_XCS)/Office/OptionsDialog.xcs \
	$(postprocess_XCS)/Office/Paths.xcs \
	$(postprocess_XCS)/Office/ProtocolHandler.xcs \
	$(postprocess_XCS)/Office/Recovery.xcs \
	$(postprocess_XCS)/Office/Scripting.xcs \
	$(postprocess_XCS)/Office/Security.xcs \
	$(postprocess_XCS)/Office/TabBrowse.xcs \
	$(postprocess_XCS)/Office/TableWizard.xcs \
	$(postprocess_XCS)/Office/TypeDetection.xcs \
	$(postprocess_XCS)/Office/UI/BaseWindowState.xcs \
	$(postprocess_XCS)/Office/UI/BasicIDECommands.xcs \
	$(postprocess_XCS)/Office/UI/BasicIDEWindowState.xcs \
	$(postprocess_XCS)/Office/UI/BibliographyCommands.xcs \
	$(postprocess_XCS)/Office/UI/BibliographyWindowState.xcs \
	$(postprocess_XCS)/Office/UI/Category.xcs \
	$(postprocess_XCS)/Office/UI/ChartCommands.xcs \
	$(postprocess_XCS)/Office/UI/ChartWindowState.xcs \
	$(postprocess_XCS)/Office/UI/Commands.xcs \
	$(postprocess_XCS)/Office/UI/Controller.xcs \
	$(postprocess_XCS)/Office/UI/DbBrowserWindowState.xcs \
	$(postprocess_XCS)/Office/UI/DbQueryWindowState.xcs \
	$(postprocess_XCS)/Office/UI/DbRelationWindowState.xcs \
	$(postprocess_XCS)/Office/UI/DbTableDataWindowState.xcs \
	$(postprocess_XCS)/Office/UI/DbTableWindowState.xcs \
	$(postprocess_XCS)/Office/UI/DbuCommands.xcs \
	$(postprocess_XCS)/Office/UI/DrawImpressCommands.xcs \
	$(postprocess_XCS)/Office/UI/Factories.xcs \
	$(postprocess_XCS)/Office/UI/GenericCategories.xcs \
	$(postprocess_XCS)/Office/UI/GenericCommands.xcs \
	$(postprocess_XCS)/Office/UI/GlobalSettings.xcs \
	$(postprocess_XCS)/Office/UI/Notebookbar.xcs \
	$(postprocess_XCS)/Office/UI/Sidebar.xcs \
	$(postprocess_XCS)/Office/UI/StartModuleCommands.xcs \
	$(postprocess_XCS)/Office/UI/StartModuleWindowState.xcs \
	$(postprocess_XCS)/Office/UI/ToolbarMode.xcs \
	$(postprocess_XCS)/Office/UI/WindowContentFactories.xcs \
	$(postprocess_XCS)/Office/UI/WindowState.xcs \
	$(postprocess_XCS)/Office/UI.xcs \
	$(postprocess_XCS)/Office/Views.xcs \
	$(postprocess_XCS)/Office/WebWizard.xcs \
	$(postprocess_XCS)/Office/Writer.xcs \
	$(postprocess_XCS)/Office/WriterWeb.xcs \
	$(postprocess_XCS)/Setup.xcs \
	$(postprocess_XCS)/System.xcs \
	$(postprocess_XCS)/TypeDetection/Filter.xcs \
	$(postprocess_XCS)/TypeDetection/GraphicFilter.xcs \
	$(postprocess_XCS)/TypeDetection/Misc.xcs \
	$(postprocess_XCS)/TypeDetection/Types.xcs \
	$(postprocess_XCS)/TypeDetection/UISort.xcs \
	$(postprocess_XCS)/UserProfile.xcs \
	$(postprocess_XCS)/VCL.xcs \
	$(postprocess_XCS)/ucb/Configuration.xcs \
	$(postprocess_XCS)/ucb/Hierarchy.xcs \
	$(postprocess_XCS)/ucb/InteractionHandler.xcs \
	$(postprocess_XCS)/ucb/Store.xcs \
	$(postprocess_XCU)/Inet.xcu \
	$(postprocess_XCU)/Interaction.xcu \
	$(postprocess_XCU)/Office/Accelerators.xcu \
	$(postprocess_XCU)/Office/BasicIDE.xcu \
	$(postprocess_XCU)/Office/Calc.xcu \
	$(postprocess_XCU)/Office/Canvas.xcu \
	$(postprocess_XCU)/Office/Common.xcu \
	$(postprocess_XCU)/Office/Compatibility.xcu \
	$(postprocess_XCU)/Office/DataAccess.xcu \
	$(postprocess_XCU)/Office/Embedding.xcu \
	$(postprocess_XCU)/Office/ExtensionDependencies.xcu \
	$(postprocess_XCU)/Office/ExtensionManager.xcu \
	$(postprocess_XCU)/Office/FormWizard.xcu \
	$(postprocess_XCU)/Office/Histories.xcu \
	$(postprocess_XCU)/Office/Impress.xcu \
	$(postprocess_XCU)/Office/Jobs.xcu \
	$(postprocess_XCU)/Office/Logging.xcu \
	$(postprocess_XCU)/Office/Math.xcu \
	$(postprocess_XCU)/Office/Paths.xcu \
	$(postprocess_XCU)/Office/ProtocolHandler.xcu \
	$(postprocess_XCU)/Office/Scripting.xcu \
	$(postprocess_XCU)/Office/Security.xcu \
	$(postprocess_XCU)/Office/TableWizard.xcu \
	$(postprocess_XCU)/Office/UI/BaseWindowState.xcu \
	$(postprocess_XCU)/Office/UI/BasicIDECommands.xcu \
	$(postprocess_XCU)/Office/UI/BasicIDEWindowState.xcu \
	$(postprocess_XCU)/Office/UI/BibliographyCommands.xcu \
	$(postprocess_XCU)/Office/UI/ChartCommands.xcu \
	$(postprocess_XCU)/Office/UI/ChartWindowState.xcu \
	$(postprocess_XCU)/Office/UI/Controller.xcu \
	$(postprocess_XCU)/Office/UI/DbBrowserWindowState.xcu \
	$(postprocess_XCU)/Office/UI/DbQueryWindowState.xcu \
	$(postprocess_XCU)/Office/UI/DbRelationWindowState.xcu \
	$(postprocess_XCU)/Office/UI/DbTableDataWindowState.xcu \
	$(postprocess_XCU)/Office/UI/DbTableWindowState.xcu \
	$(postprocess_XCU)/Office/UI/DbuCommands.xcu \
	$(postprocess_XCU)/Office/UI/DrawImpressCommands.xcu \
	$(postprocess_XCU)/Office/UI/Factories.xcu \
	$(postprocess_XCU)/Office/UI/GenericCategories.xcu \
	$(postprocess_XCU)/Office/UI/GenericCommands.xcu \
	$(postprocess_XCU)/Office/UI/Notebookbar.xcu \
	$(postprocess_XCU)/Office/UI/Sidebar.xcu \
	$(postprocess_XCU)/Office/UI/StartModuleCommands.xcu \
	$(postprocess_XCU)/Office/UI/StartModuleWindowState.xcu \
	$(postprocess_XCU)/Office/UI/ToolbarMode.xcu \
	$(postprocess_XCU)/Office/UI.xcu \
	$(postprocess_XCU)/Office/Views.xcu \
	$(postprocess_XCU)/Office/WebWizard.xcu \
	$(postprocess_XCU)/Office/Writer.xcu \
	$(postprocess_XCU)/Setup.xcu \
	$(postprocess_XCU)/System.xcu \
	$(postprocess_XCU)/TypeDetection/UISort.xcu \
	$(postprocess_XCU)/UserProfile.xcu \
	$(postprocess_XCU)/VCL.xcu \
	$(postprocess_XCU)/ucb/Configuration.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_base_filters.xcu) \
	$(call gb_XcuFilterOthersTarget_get_target,fcfg_base_others.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_base_types.xcu) \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_chart_filters.xcu) \
	$(call gb_XcuFilterOthersTarget_get_target,fcfg_chart_others.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_chart_types.xcu) \
	$(call gb_XcuFilterInternalTarget_get_target,fcfg_internalgraphics_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_internalgraphics_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-chart.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-start.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-calc.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-draw.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-impress.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-math.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-writer.xcu
ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
postprocess_FILES_main += \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/dbase)/org/openoffice/Office/DataAccess/Drivers-dbase.xcu \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/flat)/org/openoffice/Office/DataAccess/Drivers-flat.xcu \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/mysql)/org/openoffice/Office/DataAccess/Drivers-mysql.xcu \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/odbc)/org/openoffice/Office/DataAccess/Drivers-odbc.xcu
postprocess_DRIVERS += dbase flat mysql odbc
ifeq (WNT,$(OS))
else ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
postprocess_FILES_main += $(call gb_XcuModuleTarget_get_target,connectivity/registry/mork)/org/openoffice/Office/DataAccess/Drivers-mork.xcu
postprocess_DRIVERS += mork
endif
endif
endif
ifeq (MACOSX,$(OS))
postprocess_FILES_main += \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/macab)/org/openoffice/Office/DataAccess/Drivers-macab.xcu \
	$(postprocess_MOD)/org/openoffice/Inet-macosx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-macosx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-macosx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Paths-macosx.xcu
		# Inet-macosx.xcu must come after Inet.xcu
postprocess_DRIVERS += macab
else ifeq ($(USING_X11), TRUE)
postprocess_FILES_main += \
	$(postprocess_MOD)/org/openoffice/Inet-unixdesktop.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-unxwnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-UseOOoFileDialogs.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-unixdesktop.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-unx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Paths-unixdesktop.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Paths-unxwnt.xcu \
	$(postprocess_MOD)/org/openoffice/UserProfile-unixdesktop.xcu \
	$(postprocess_MOD)/org/openoffice/VCL-unixdesktop.xcu
		# Inet-unixdesktop.xcu must come after Inet.xcu
		# VCL-unixdesktop.xcu must come after VCL.xcu
else ifeq (WNT,$(OS))
postprocess_FILES_main += \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/ado)/org/openoffice/Office/DataAccess/Drivers-ado.xcu \
	$(postprocess_MOD)/org/openoffice/Inet-wnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-unxwnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-wnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Paths-unxwnt.xcu \
	$(postprocess_MOD)/org/openoffice/ucb/Configuration-win.xcu
		# Inet-wnt.xcu must come after Inet.xcu
postprocess_DRIVERS += ado
endif
ifneq ($(WITH_WEBDAV),)
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/ucb/Configuration-webdav.xcu
endif
ifeq ($(ENABLE_EVOAB2),TRUE)
postprocess_FILES_main += $(call gb_XcuModuleTarget_get_target,connectivity/registry/evoab2)/org/openoffice/Office/DataAccess/Drivers-evoab2.xcu
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess-evoab2.xcu
postprocess_DRIVERS += evoab
endif
ifeq ($(ENABLE_JAVA),TRUE)
postprocess_FILES_main += \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/hsqldb)/org/openoffice/Office/DataAccess/Drivers-hsqldb.xcu \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/jdbc)/org/openoffice/Office/DataAccess/Drivers-jdbc.xcu
postprocess_DRIVERS += hsqldb jdbc
endif
ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)
postprocess_FILES_main += \
	$(call gb_XcuModuleTarget_get_target,connectivity/registry/firebird)/org/openoffice/Office/DataAccess/Drivers-firebird.xcu
postprocess_DRIVERS += firebird_sdbc
endif
ifeq ($(ENABLE_TDEAB),TRUE)
postprocess_FILES_main += $(call gb_XcuModuleTarget_get_target,connectivity/registry/tdeab)/org/openoffice/Office/DataAccess/Drivers-tdeab.xcu
endif

ifneq (,$(SYSTEM_LIBEXTTEXTCAT_DATA))
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/Paths-externallibexttextcatdata.xcu
else
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/Paths-internallibexttextcatdata.xcu
endif

postprocess_DEPS_math := main
postprocess_FILES_math := \
	$(postprocess_XCS)/Office/UI/MathCommands.xcs \
	$(postprocess_XCS)/Office/UI/MathWindowState.xcs \
	$(postprocess_XCU)/Office/UI/MathCommands.xcu \
	$(postprocess_XCU)/Office/UI/MathWindowState.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_math_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_math_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Common-math.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-math.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-math.xcu

postprocess_DEPS_pyuno := main
postprocess_FILES_pyuno := \
	$(postprocess_MOD)/org/openoffice/Office/Scripting-python.xcu

ifeq ($(ENABLE_REPORTBUILDER),TRUE)
postprocess_XCDS += reportbuilder.xcd
postprocess_DEPS_reportbuilder := main
postprocess_FILES_reportbuilder := \
	$(postprocess_XCS)/Office/ReportDesign.xcs \
	$(postprocess_XCS)/Office/UI/DbReportWindowState.xcs \
	$(postprocess_XCS)/Office/UI/ReportCommands.xcs \
	$(postprocess_XCU)/Office/ExtendedColorScheme.xcu \
	$(postprocess_XCU)/Office/ReportDesign.xcu \
	$(postprocess_XCU)/Office/UI/DbReportWindowState.xcu \
	$(postprocess_XCU)/Office/UI/ReportCommands.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_reportbuilder_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_reportbuilder_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Setup-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/UI/Controller-reportbuilder.xcu
endif

postprocess_DEPS_writer := main
postprocess_FILES_writer := \
	$(postprocess_XCS)/Office/UI/WriterCommands.xcs \
	$(postprocess_XCS)/Office/UI/WriterFormWindowState.xcs \
	$(postprocess_XCS)/Office/UI/WriterGlobalWindowState.xcs \
	$(postprocess_XCS)/Office/UI/WriterReportWindowState.xcs \
	$(postprocess_XCS)/Office/UI/WriterWebWindowState.xcs \
	$(postprocess_XCS)/Office/UI/WriterWindowState.xcs \
	$(postprocess_XCS)/Office/UI/XFormsWindowState.xcs \
	$(postprocess_XCU)/Office/UI/WriterCommands.xcu \
	$(postprocess_XCU)/Office/UI/WriterFormWindowState.xcu \
	$(postprocess_XCU)/Office/UI/WriterGlobalWindowState.xcu \
	$(postprocess_XCU)/Office/UI/WriterReportWindowState.xcu \
	$(postprocess_XCU)/Office/UI/WriterWebWindowState.xcu \
	$(postprocess_XCU)/Office/UI/WriterWindowState.xcu \
	$(postprocess_XCU)/Office/UI/XFormsWindowState.xcu \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_global_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_global_types.xcu) \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_web_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_web_types.xcu) \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_writer_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_writer_types.xcu) \
	$(postprocess_MOD)/org/openoffice/Office/Common-writer.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-writer.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-writer.xcu

postprocess_DEPS_xsltfilter := main
postprocess_OPTDEPS_xsltfilter := calc writer
postprocess_FILES_xsltfilter := \
	$(call gb_XcuFilterFiltersTarget_get_target,fcfg_xslt_filters.xcu) \
	$(call gb_XcuFilterTypesTarget_get_target,fcfg_xslt_types.xcu)

ifneq ($(BUILD_POSTGRESQL_SDBC),)
postprocess_XCDS += postgresql.xcd
postprocess_DEPS_postgresql := main
postprocess_FILES_postgresql := $(call gb_XcuModuleTarget_get_target,connectivity/registry/postgresql)/org/openoffice/Office/DataAccess/Drivers-postgresql.xcu
postprocess_DRIVERS += postgresql
endif

ifneq (,$(and $(USING_X11), $(ENABLE_GIO)))
postprocess_XCDS += gnome.xcd
postprocess_DEPS_gnome := main
postprocess_FILES_gnome += $(postprocess_MOD)/org/openoffice/ucb/Configuration-gio.xcu
endif

ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
postprocess_XCDS += onlineupdate.xcd
postprocess_DEPS_onlineupdate := main
postprocess_FILES_onlineupdate := \
	$(call gb_XcuModuleTarget_get_target,extensions/source/update/check)/org/openoffice/Office/Addons-onlineupdate.xcu \
	$(call gb_XcuModuleTarget_get_target,extensions/source/update/check)/org/openoffice/Office/Jobs-onlineupdate.xcu
endif

ifeq ($(ENABLE_OPENGL_TRANSITIONS),TRUE)
postprocess_XCDS += ogltrans.xcd
postprocess_DEPS_ogltrans := main
postprocess_FILES_ogltrans := \
	$(postprocess_MOD)/org/openoffice/Office/Impress-ogltrans.xcu
endif

ifeq ($(ENABLE_PDFIMPORT),TRUE)
postprocess_XCDS += pdfimport.xcd
postprocess_OPTDEPS_pdfimport := calc draw impress math writer
	# HACK: for all fcfg_X_types.xcu in filter/Configuration_filter.mk that
	# include pdf_Portable_Document_Format (i.e., X in calc, draw, global,
	# impress, math, web, writer), add optional dependencies on those XCDS that
	# include those fcfg_X_types.xcu
postprocess_FILES_pdfimport := \
	$(SRCDIR)/sdext/source/pdfimport/config/pdf_import_filter.xcu \
	$(SRCDIR)/sdext/source/pdfimport/config/pdf_types.xcu
endif

ifeq (WNT,$(OS))
postprocess_XCDS += forcedefault.xcd
postprocess_DEPS_forcedefault := main
postprocess_FILES_forcedefault := \
	$(postprocess_MOD)/org/openoffice/Office/Linguistic-ForceDefaultLanguage.xcu
endif

postprocess_DRIVERS := $(foreach driver,$(postprocess_DRIVERS),driver_$(driver))

#
# Targets
#

$(call gb_CustomTarget_get_target,postprocess/registry) : \
	$(foreach target,$(postprocess_XCDS) \
		$(foreach lang,$(gb_Configuration_LANGS), \
			Langpack-$(lang).xcd \
			fcfg_langpack_$(lang).xcd \
			registry_$(lang).xcd \
		) \
	,$(call gb_XcdTarget_get_target,$(target)))

#
# Dependencies
#

define postprocess_xcd_deps
$(call gb_XcdTarget_get_target,$(1)).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/$(1).list : \
	$(postprocess_FILES_$(1)) \
	$(SRCDIR)/postprocess/CustomTarget_registry.mk \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

endef
$(foreach xcd,$(postprocess_XCDS),$(eval $(call postprocess_xcd_deps,$(basename $(xcd)))))

define postprocess_lang_deps
$(call gb_XcdTarget_get_target,Langpack-$(1).xcd) : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).list : \
	$(call gb_XcuLangpackTarget_get_target,Langpack-$(1).xcu) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

$(call gb_XcdTarget_get_target,fcfg_langpack_$(1).xcd) : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).list \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).list : \
	$(call gb_Configuration_get_target,fcfg_langpack) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

$(call gb_XcdTarget_get_target,registry_$(1).xcd) : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).list : \
	$(call gb_Configuration_get_target,registry) \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),\
		$(foreach driver,$(postprocess_DRIVERS),$(call gb_Configuration_get_target,$(driver))) \
	) \
	$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)),$(call gb_Configuration_get_target,updchk)) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

endef
$(foreach lang,$(gb_Configuration_LANGS),$(eval $(call postprocess_lang_deps,$(lang))))

#
# Rules
#

postprocess_main_SED := \
	-e 's,$${ABOUTBOXPRODUCTVERSION},$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO)$(LIBO_VERSION_SUFFIX),g' \
	-e 's,$${ABOUTBOXPRODUCTVERSIONSUFFIX},$(LIBO_VERSION_SUFFIX_SUFFIX),g' \
	-e 's,$${DICT_REPO_URL},http://extensions.libreoffice.org/dictionaries/,g' \
	-e 's,$${OOOVENDOR},$(if $(OOO_VENDOR),$(subst $(COMMA),\x2c,$(OOO_VENDOR)),The Document Foundation),g' \
	-e 's,$${PRODUCTNAME},$(PRODUCTNAME),g' \
	-e 's,$${PRODUCTVERSION},$(PRODUCTVERSION),g' \
	-e 's,$${PRODUCTEXTENSION},.$(LIBO_VERSION_MICRO)$(LIBO_VERSION_SUFFIX),g' \
	-e 's,$${STARTCENTER_ADDFEATURE_URL},http://extensions.libreoffice.org/,g' \
	-e 's,$${STARTCENTER_INFO_URL},https://www.libreoffice.org/,g' \
	-e 's,$${STARTCENTER_HIDE_EXTERNAL_LINKS},0,g' \
	-e 's,$${STARTCENTER_TEMPLREP_URL},http://templates.libreoffice.org/,g' \
	-e 's,$${SYSTEM_LIBEXTTEXTCAT_DATA},$(SYSTEM_LIBEXTTEXTCAT_DATA),g' \

$(call gb_XcdTarget_get_target,main.xcd) \
		: $(BUILDDIR)/config_host.mk.stamp \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,main,$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
			$(SRCDIR)/solenv/bin/packregistry.xslt \
			$(call gb_CustomTarget_get_workdir,postprocess/registry)/main.list \
		|  sed $(postprocess_main_SED) > $@ \
	)

$(call gb_XcdTarget_get_target,%.xcd) : \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*,$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
			-o $@ $(SRCDIR)/solenv/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list><dependency file="main"/><filename>$(call gb_XcuLangpackTarget_get_target,Langpack-$*.xcu)</filename></list>' > $@

# It can happen that localized fcfg_langpack_*.zip contains
# zero-sized org/openoffice/TypeDectection/Filter.xcu; filter them out in the
# find shell command below (see issue 110041):
$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AWK,2)
	$(call gb_Helper_abbreviate_dirs,\
	    $(FIND) $(call gb_XcuResTarget_get_target,fcfg_langpack/$*/) \
	         -name *.xcu -size +0c \
	        | $(gb_AWK) 'BEGIN{print "<list>"} \
	                    {print "<filename>"$$0"</filename>"} \
	               END  {print "</list>"}' > $@ \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AWK,2)
	$(call gb_Helper_abbreviate_dirs,\
	    $(FIND) $(call gb_XcuResTarget_get_target,registry/$*/) \
	         $(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),\
	             $(foreach driver,$(postprocess_DRIVERS),\
	                 $(call gb_XcuResTarget_get_target,$(driver)/$*/)))\
	         $(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)),\
	             $(call gb_XcuResTarget_get_target,updchk/$*/))\
	         -name *.xcu \
	        | $(gb_AWK) 'BEGIN{print "<list>"} \
	                    {print "<filename>"$$0"</filename>"} \
	               END  {print "</list>"}' > $@ \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	mv $(call var2file,$@.tmp,70,<list> $(foreach i,$(postprocess_DEPS_$*), <dependency file='$i'/>) \
		   $(foreach i,$(postprocess_OPTDEPS_$*), <dependency file='$i' optional='true'/>) \
		   $(foreach i,$(postprocess_FILES_$*), <filename>$(i)</filename>) </list>) \
	   $@

# vim: set noet sw=4 ts=4:
