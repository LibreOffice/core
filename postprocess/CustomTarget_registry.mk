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

postprocess_XCS := registry/schema/org/openoffice
postprocess_XCU := registry/data/org/openoffice
postprocess_MOD := registry/spool
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
	korea.xcd \
	librelogo.xcd \
	lingucomponent.xcd \
	main.xcd \
	math.xcd \
	pyuno.xcd \
	writer.xcd \
	xsltfilter.xcd

postprocess_DEPS_base := main
postprocess_FILES_base := \
	$(postprocess_MOD)/fcfg_database_filters.xcu \
	$(postprocess_MOD)/fcfg_database_others.xcu \
	$(postprocess_MOD)/fcfg_database_types.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-base.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-base.xcu

postprocess_DEPS_calc := main
postprocess_FILES_calc := \
	$(postprocess_XCS)/Office/UI/CalcCommands.xcs \
	$(postprocess_XCS)/Office/UI/CalcWindowState.xcs \
	$(postprocess_XCU)/Office/UI/CalcCommands.xcu \
	$(postprocess_XCU)/Office/UI/CalcWindowState.xcu \
	$(postprocess_MOD)/fcfg_calc_filters.xcu \
	$(postprocess_MOD)/fcfg_calc_types.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-calc.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-calc.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-calc.xcu

ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
postprocess_FILES_calc += \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-calc.xcu
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
	$(postprocess_MOD)/fcfg_draw_filters.xcu \
	$(postprocess_MOD)/fcfg_draw_types.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-draw.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-draw.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-draw.xcu

postprocess_DEPS_graphicfilter := main
postprocess_FILES_graphicfilter := \
	$(postprocess_MOD)/fcfg_drawgraphics_filters.xcu \
	$(postprocess_MOD)/fcfg_drawgraphics_types.xcu \
	$(postprocess_MOD)/fcfg_impressgraphics_filters.xcu \
	$(postprocess_MOD)/fcfg_impressgraphics_types.xcu \
	$(postprocess_MOD)/fcfg_writergraphics_filters.xcu \
	$(postprocess_MOD)/fcfg_writergraphics_types.xcu

postprocess_DEPS_impress := main
postprocess_FILES_impress := \
	$(postprocess_XCS)/Office/PresenterScreen.xcs \
	$(postprocess_XCS)/Office/UI/Effects.xcs \
	$(postprocess_XCS)/Office/UI/ImpressWindowState.xcs \
	$(postprocess_XCU)/Office/PresenterScreen.xcu \
	$(postprocess_XCU)/Office/UI/Effects.xcu \
	$(postprocess_XCU)/Office/UI/ImpressWindowState.xcu \
	$(postprocess_MOD)/fcfg_impress_filters.xcu \
	$(postprocess_MOD)/fcfg_impress_types.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Jobs-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Office/ProtocolHandler-impress.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-impress.xcu

postprocess_DEPS_librelogo := main writer
postprocess_FILES_librelogo := \
	$(postprocess_MOD)/org/openoffice/Office/Addons-librelogo.xcu \
	$(postprocess_MOD)/org/openoffice/Office/UI/WriterWindowState-librelogo.xcu

postprocess_DEPS_korea := main
postprocess_FILES_korea := \
	$(postprocess_MOD)/org/openoffice/Office/Common-korea.xcu

postprocess_DEPS_lingucomponent := main
postprocess_FILES_lingucomponent := \
	$(postprocess_XCU)/Office/Linguistic-lingucomponent-hyphenator.xcu \
	$(postprocess_XCU)/Office/Linguistic-lingucomponent-spellchecker.xcu \
	$(postprocess_XCU)/Office/Linguistic-lingucomponent-thesaurus.xcu \

postprocess_FILES_main := \
	$(postprocess_XCS)/FirstStartWizard.xcs \
	$(postprocess_XCS)/Inet.xcs \
	$(postprocess_XCS)/Interaction.xcs \
	$(postprocess_XCS)/LDAP.xcs \
	$(postprocess_XCS)/Office/Accelerators.xcs \
	$(postprocess_XCS)/Office/Addons.xcs \
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
	$(postprocess_XCS)/Office/Substitution.xcs \
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
	$(postprocess_XCS)/Office/UI/Sidebar.xcs \
	$(postprocess_XCS)/Office/UI/StartModuleCommands.xcs \
	$(postprocess_XCS)/Office/UI/StartModuleWindowState.xcs \
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
	$(postprocess_XCU)/FirstStartWizard.xcu \
	$(postprocess_XCU)/Inet.xcu \
	$(postprocess_XCU)/Interaction.xcu \
	$(postprocess_XCU)/Office/Accelerators.xcu \
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
	$(postprocess_XCU)/Office/UI/Sidebar.xcu \
	$(postprocess_XCU)/Office/UI/StartModuleCommands.xcu \
	$(postprocess_XCU)/Office/UI/StartModuleWindowState.xcu \
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
	$(postprocess_MOD)/fcfg_base_filters.xcu \
	$(postprocess_MOD)/fcfg_base_others.xcu \
	$(postprocess_MOD)/fcfg_base_types.xcu \
	$(postprocess_MOD)/fcfg_chart_filters.xcu \
	$(postprocess_MOD)/fcfg_chart_others.xcu \
	$(postprocess_MOD)/fcfg_chart_types.xcu \
	$(postprocess_MOD)/fcfg_internalgraphics_filters.xcu \
	$(postprocess_MOD)/fcfg_internalgraphics_types.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-chart.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-start.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-calc.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-draw.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-impress.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-math.xcu \
	$(postprocess_MOD)/org/openoffice/TypeDetection/UISort-writer.xcu
ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
postprocess_FILES_main += \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-dbase.xcu \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-flat.xcu \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-mysql.xcu \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-odbc.xcu
postprocess_DRIVERS += dbase flat mysql odbc
ifeq (WNT,$(OS))
ifeq ($(WITH_MOZAB4WIN),YES)
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-mozab.xcu
postprocess_DRIVERS += mozab
endif
else ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-mork.xcu
postprocess_DRIVERS += mork
endif
endif
endif
ifeq (MACOSX,$(OS))
postprocess_FILES_main += \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-macab.xcu \
	$(postprocess_MOD)/org/openoffice/Inet-macosx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-macosx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-macosx.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Paths-macosx.xcu
		# Inet-macosx.xcu must come after Inet.xcu
postprocess_DRIVERS += macab
else ifeq (unx,$(GUIBASE))
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
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-ado.xcu \
	$(postprocess_MOD)/org/openoffice/Inet-wnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-unxwnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-wnt.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Paths-unxwnt.xcu \
	$(postprocess_MOD)/org/openoffice/ucb/Configuration-win.xcu
		# Inet-wnt.xcu must come after Inet.xcu
postprocess_DRIVERS += ado
endif
ifeq ($(DISABLE_NEON),$(false))
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/ucb/Configuration-neon.xcu
endif
ifeq ($(ENABLE_EVOAB2),TRUE)
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-evoab2.xcu
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess-evoab2.xcu
postprocess_DRIVERS += evoab
endif
ifeq ($(ENABLE_JAVA),TRUE)
postprocess_FILES_main += \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-hsqldb.xcu \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-jdbc.xcu
postprocess_DRIVERS += hsqldb jdbc
endif
ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)
postprocess_FILES_main += \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-firebird.xcu
postprocess_DRIVERS += firebird_sdbc
endif
ifeq ($(ENABLE_TDEAB),TRUE)
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-tdeab.xcu
endif
ifeq ($(ENABLE_KAB),TRUE)
postprocess_FILES_main += $(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-kab.xcu
postprocess_DRIVERS += kab
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
	$(postprocess_MOD)/fcfg_math_filters.xcu \
	$(postprocess_MOD)/fcfg_math_types.xcu \
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
	$(postprocess_XCU)/TypeDetection/Filter.xcu \
	$(postprocess_XCU)/TypeDetection/Types.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Accelerators-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/DataAccess-reportbuilder.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-reportbuilder.xcu
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
	$(postprocess_MOD)/fcfg_global_filters.xcu \
	$(postprocess_MOD)/fcfg_global_types.xcu \
	$(postprocess_MOD)/fcfg_web_filters.xcu \
	$(postprocess_MOD)/fcfg_web_types.xcu \
	$(postprocess_MOD)/fcfg_writer_filters.xcu \
	$(postprocess_MOD)/fcfg_writer_types.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Common-writer.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Embedding-writer.xcu \
	$(postprocess_MOD)/org/openoffice/Setup-writer.xcu

postprocess_DEPS_xsltfilter := main
postprocess_FILES_xsltfilter := \
	$(postprocess_MOD)/fcfg_xslt_filters.xcu \
	$(postprocess_MOD)/fcfg_xslt_types.xcu

ifeq ($(BUILD_POSTGRESQL_SDBC),YES)
postprocess_XCDS += postgresqlsdbc.xcd
postprocess_DEPS_postgresqlsdbc := main
postprocess_FILES_postgresqlsdbc := $(postprocess_MOD)/org/openoffice/Office/DataAccess/Drivers-postgresql.xcu
postprocess_DRIVERS += postgresql
endif

ifeq (unx,$(GUIBASE))
ifneq (,$(or $(filter TRUETRUE,$(ENABLE_GCONF)$(ENABLE_LOCKDOWN))$(filter TRUE,$(ENABLE_GIO))))
postprocess_XCDS += gnome.xcd
postprocess_DEPS_gnome := main
ifeq ($(ENABLE_GCONF)$(ENABLE_LOCKDOWN),TRUETRUE)
postprocess_FILES_gnome += \
	$(postprocess_MOD)/org/openoffice/Office/Common-gconflockdown.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Recovery-gconflockdown.xcu \
	$(postprocess_MOD)/org/openoffice/VCL-gconflockdown.xcu
endif
ifeq ($(ENABLE_GIO),TRUE)
postprocess_FILES_gnome += \
	$(postprocess_MOD)/org/openoffice/ucb/Configuration-gio.xcu
endif
endif
endif # unx == $(GUIBASE)

ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
postprocess_XCDS += onlineupdate.xcd
postprocess_DEPS_onlineupdate := main
postprocess_FILES_onlineupdate := \
	$(postprocess_MOD)/org/openoffice/Office/Addons-onlineupdate.xcu \
	$(postprocess_MOD)/org/openoffice/Office/Jobs-onlineupdate.xcu
endif

ifeq ($(ENABLE_OPENGL),TRUE)
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
postprocess_FILES_pdfimport := pdfimport/pdf_import_filter.xcu pdfimport/pdf_types.xcu
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

$(eval $(call gb_CustomTarget_register_targets,postprocess/registry,\
	$(postprocess_XCDS) \
	$(foreach lang,$(gb_Configuration_LANGS), \
		Langpack-$(lang).xcd \
		fcfg_langpack_$(lang).xcd \
		registry_$(lang).xcd \
	) \
))

#
# Dependencies
#

define postprocess_xcd_deps
$(call gb_CustomTarget_get_workdir,postprocess/registry)/$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/$(1).list : \
	$(foreach file,$(postprocess_FILES_$(1)),$(OUTDIR)/xml/$(file)) \
	$(SRCDIR)/postprocess/CustomTarget_registry.mk \
	$(call gb_Postprocess_get_target,AllPackages) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

endef
$(foreach xcd,$(postprocess_XCDS),$(eval $(call postprocess_xcd_deps,$(basename $(xcd)))))

define postprocess_lang_deps
$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).list : \
	$(call gb_XcuLangpackTarget_get_outdir_target,Langpack-$(1).xcu)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).list : \
	$(call gb_Configuration_get_target,fcfg_langpack)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).list : \
	$(call gb_Configuration_get_target,registry) \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),\
		$(foreach driver,$(postprocess_DRIVERS),$(call gb_Configuration_get_target,$(driver))) \
	) \
	$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)),$(call gb_Configuration_get_target,updchk))

endef
$(foreach lang,$(gb_Configuration_LANGS),$(eval $(call postprocess_lang_deps,$(lang))))

#
# Rules
#

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-%.xcd : \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ \
		$(SOLARENV)/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list><dependency file="main"/><filename>$(call gb_XcuLangpackTarget_get_outdir_target,Langpack-$*.xcu)</filename></list>' > $@

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_%.xcd : \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
		-o $@ $(SOLARENV)/bin/packregistry.xslt $< \
	)

# It can happen that localized fcfg_langpack_*.zip contains
# zero-sized org/openoffice/TypeDectection/Filter.xcu; filter them out in the
# find shell command below (see issue 110041):
$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	$(call gb_Helper_abbreviate_dirs, \
		echo '<list>' > $@ \
		&& ( find $(call gb_XcuResTarget_get_target,fcfg_langpack/$*/) -name *.xcu -size +0c -print0 | xargs -n1 -0 -I '{}' echo '<filename>{}</filename>') >> $@ \
		&& echo '</list>' >> $@ \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_%.xcd : \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
		-o $@ $(SOLARENV)/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list>' > $@ \
	&& $(call gb_Helper_abbreviate_dirs, \
		(find $(call gb_XcuResTarget_get_target,registry/$*/) -name *.xcu -print0 | xargs -n1 -0 -I '{}' echo '<filename>{}</filename>') >> $@ \
		$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),\
			$(foreach driver,$(postprocess_DRIVERS), \
				&& (find $(call gb_XcuResTarget_get_target,$(driver)/$*/) -name *.xcu -print0 | xargs -n1 -0 -I '{}' echo '<filename>{}</filename>') >> $@ \
			) \
		) \
		$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)),\
			&& (find $(call gb_XcuResTarget_get_target,updchk/$*/) -name *.xcu -print0 | xargs -n1 -0 -I '{}' echo '<filename>{}</filename>') >> $@ \
		) \
	) \
	&& echo '</list>' >> $@

$(call gb_CustomTarget_get_workdir,postprocess/registry)/%.xcd : \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
		-o $@ $(SOLARENV)/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list>' $(foreach i,$(postprocess_DEPS_$*), '<dependency file="$i"/>') \
		$(foreach i,$(postprocess_OPTDEPS_$*), '<dependency file="$i" optional="true"/>') \
		$(foreach i,$(postprocess_FILES_$*), '<filename>$(OUTDIR)/xml/$(i)</filename>') '</list>' > $@

# vim: set noet sw=4 ts=4:
