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

MY_XCS := registry/schema/org/openoffice
MY_XCU := registry/data/org/openoffice
MY_MOD := registry/spool
MY_DRIVERS :=

MY_XCDS := \
	base.xcd \
	calc.xcd \
	cjk.xcd \
	ctl.xcd \
	ctlseqcheck.xcd \
	draw.xcd \
	graphicfilter.xcd \
	impress.xcd \
	korea.xcd \
	lingucomponent.xcd \
	main.xcd \
	math.xcd \
	palm.xcd \
	pocketexcel.xcd \
	pocketword.xcd \
	pyuno.xcd \
	writer.xcd \
	xsltfilter.xcd

MY_DEPS_base := main
MY_FILES_base := \
	$(MY_MOD)/fcfg_database_filters.xcu \
	$(MY_MOD)/fcfg_database_others.xcu \
	$(MY_MOD)/fcfg_database_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-base.xcu \
	$(MY_MOD)/org/openoffice/Setup-base.xcu

MY_DEPS_calc := main
MY_FILES_calc := \
	$(MY_XCS)/Office/UI/CalcCommands.xcs \
	$(MY_XCS)/Office/UI/CalcWindowState.xcs \
	$(MY_XCU)/Office/UI/CalcCommands.xcu \
	$(MY_XCU)/Office/UI/CalcWindowState.xcu \
	$(MY_MOD)/fcfg_calc_filters.xcu \
	$(MY_MOD)/fcfg_calc_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-calc.xcu \
	$(MY_MOD)/org/openoffice/Office/Embedding-calc.xcu \
	$(MY_MOD)/org/openoffice/Setup-calc.xcu

ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
MY_FILES_calc += \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-calc.xcu
MY_DRIVERS += calc
endif

MY_DEPS_cjk := main
MY_FILES_cjk := \
	$(MY_MOD)/org/openoffice/Office/Common-cjk.xcu \
	$(MY_MOD)/org/openoffice/Office/Writer-cjk.xcu

MY_DEPS_ctl := main
MY_FILES_ctl := \
	$(MY_MOD)/org/openoffice/Office/Common-ctl.xcu

MY_DEPS_draw := main
MY_FILES_draw := \
	$(MY_XCS)/Office/UI/DrawWindowState.xcs \
	$(MY_XCU)/Office/UI/DrawWindowState.xcu \
	$(MY_MOD)/fcfg_draw_filters.xcu \
	$(MY_MOD)/fcfg_draw_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-draw.xcu \
	$(MY_MOD)/org/openoffice/Office/Embedding-draw.xcu \
	$(MY_MOD)/org/openoffice/Setup-draw.xcu

MY_DEPS_graphicfilter := main
MY_FILES_graphicfilter := \
	$(MY_MOD)/fcfg_drawgraphics_filters.xcu \
	$(MY_MOD)/fcfg_drawgraphics_types.xcu \
	$(MY_MOD)/fcfg_impressgraphics_filters.xcu \
	$(MY_MOD)/fcfg_impressgraphics_types.xcu

MY_DEPS_impress := main
MY_FILES_impress := \
	$(MY_XCS)/Office/UI/Effects.xcs \
	$(MY_XCS)/Office/UI/ImpressWindowState.xcs \
	$(MY_XCU)/Office/UI/Effects.xcu \
	$(MY_XCU)/Office/UI/ImpressWindowState.xcu \
	$(MY_MOD)/fcfg_impress_filters.xcu \
	$(MY_MOD)/fcfg_impress_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-impress.xcu \
	$(MY_MOD)/org/openoffice/Office/Embedding-impress.xcu \
	$(MY_MOD)/org/openoffice/Setup-impress.xcu

MY_DEPS_korea := main
MY_FILES_korea := \
	$(MY_MOD)/org/openoffice/Office/Common-korea.xcu

MY_DEPS_lingucomponent := main
MY_FILES_lingucomponent := \
	$(MY_XCU)/Office/Linguistic-lingucomponent-hyphenator.xcu \
	$(MY_XCU)/Office/Linguistic-lingucomponent-spellchecker.xcu \
	$(MY_XCU)/Office/Linguistic-lingucomponent-thesaurus.xcu \

MY_FILES_main := \
	$(MY_XCS)/FirstStartWizard.xcs \
	$(MY_XCS)/Inet.xcs \
	$(MY_XCS)/Interaction.xcs \
	$(MY_XCS)/LDAP.xcs \
	$(MY_XCS)/Office/Accelerators.xcs \
	$(MY_XCS)/Office/Addons.xcs \
	$(MY_XCS)/Office/Calc.xcs \
	$(MY_XCS)/Office/CalcAddIns.xcs \
	$(MY_XCS)/Office/Canvas.xcs \
	$(MY_XCS)/Office/Chart.xcs \
	$(MY_XCS)/Office/Commands.xcs \
	$(MY_XCS)/Office/Common.xcs \
	$(MY_XCS)/Office/Compatibility.xcs \
	$(MY_XCS)/Office/DataAccess/Drivers.xcs \
	$(MY_XCS)/Office/DataAccess.xcs \
	$(MY_XCS)/Office/Draw.xcs \
	$(MY_XCS)/Office/Embedding.xcs \
	$(MY_XCS)/Office/Events.xcs \
	$(MY_XCS)/Office/ExtendedColorScheme.xcs \
	$(MY_XCS)/Office/ExtensionDependencies.xcs \
	$(MY_XCS)/Office/ExtensionManager.xcs \
	$(MY_XCS)/Office/FormWizard.xcs \
	$(MY_XCS)/Office/Histories.xcs \
	$(MY_XCS)/Office/Impress.xcs \
	$(MY_XCS)/Office/Java.xcs \
	$(MY_XCS)/Office/Jobs.xcs \
	$(MY_XCS)/Office/Labels.xcs \
	$(MY_XCS)/Office/Linguistic.xcs \
	$(MY_XCS)/Office/Logging.xcs \
	$(MY_XCS)/Office/Math.xcs \
	$(MY_XCS)/Office/OOoImprovement/Settings.xcs \
	$(MY_XCS)/Office/OptionsDialog.xcs \
	$(MY_XCS)/Office/Paths.xcs \
	$(MY_XCS)/Office/ProtocolHandler.xcs \
	$(MY_XCS)/Office/Recovery.xcs \
	$(MY_XCS)/Office/SFX.xcs \
	$(MY_XCS)/Office/Scripting.xcs \
	$(MY_XCS)/Office/Security.xcs \
	$(MY_XCS)/Office/Substitution.xcs \
	$(MY_XCS)/Office/TabBrowse.xcs \
	$(MY_XCS)/Office/TableWizard.xcs \
	$(MY_XCS)/Office/TypeDetection.xcs \
	$(MY_XCS)/Office/UI/BaseWindowState.xcs \
	$(MY_XCS)/Office/UI/BasicIDECommands.xcs \
	$(MY_XCS)/Office/UI/BasicIDEWindowState.xcs \
	$(MY_XCS)/Office/UI/BibliographyCommands.xcs \
	$(MY_XCS)/Office/UI/BibliographyWindowState.xcs \
	$(MY_XCS)/Office/UI/Category.xcs \
	$(MY_XCS)/Office/UI/ChartCommands.xcs \
	$(MY_XCS)/Office/UI/ChartWindowState.xcs \
	$(MY_XCS)/Office/UI/Commands.xcs \
	$(MY_XCS)/Office/UI/Controller.xcs \
	$(MY_XCS)/Office/UI/DbBrowserWindowState.xcs \
	$(MY_XCS)/Office/UI/DbQueryWindowState.xcs \
	$(MY_XCS)/Office/UI/DbRelationWindowState.xcs \
	$(MY_XCS)/Office/UI/DbTableDataWindowState.xcs \
	$(MY_XCS)/Office/UI/DbTableWindowState.xcs \
	$(MY_XCS)/Office/UI/DbuCommands.xcs \
	$(MY_XCS)/Office/UI/DrawImpressCommands.xcs \
	$(MY_XCS)/Office/UI/Factories.xcs \
	$(MY_XCS)/Office/UI/GenericCategories.xcs \
	$(MY_XCS)/Office/UI/GenericCommands.xcs \
	$(MY_XCS)/Office/UI/GlobalSettings.xcs \
	$(MY_XCS)/Office/UI/StartModuleCommands.xcs \
	$(MY_XCS)/Office/UI/StartModuleWindowState.xcs \
	$(MY_XCS)/Office/UI/WindowContentFactories.xcs \
	$(MY_XCS)/Office/UI/WindowState.xcs \
	$(MY_XCS)/Office/UI.xcs \
	$(MY_XCS)/Office/Views.xcs \
	$(MY_XCS)/Office/WebWizard.xcs \
	$(MY_XCS)/Office/Writer.xcs \
	$(MY_XCS)/Office/WriterWeb.xcs \
	$(MY_XCS)/Setup.xcs \
	$(MY_XCS)/System.xcs \
	$(MY_XCS)/TypeDetection/Filter.xcs \
	$(MY_XCS)/TypeDetection/GraphicFilter.xcs \
	$(MY_XCS)/TypeDetection/Misc.xcs \
	$(MY_XCS)/TypeDetection/Types.xcs \
	$(MY_XCS)/TypeDetection/UISort.xcs \
	$(MY_XCS)/UserProfile.xcs \
	$(MY_XCS)/VCL.xcs \
	$(MY_XCS)/ucb/Configuration.xcs \
	$(MY_XCS)/ucb/Hierarchy.xcs \
	$(MY_XCS)/ucb/InteractionHandler.xcs \
	$(MY_XCS)/ucb/Store.xcs \
	$(MY_XCU)/FirstStartWizard.xcu \
	$(MY_XCU)/Inet.xcu \
	$(MY_XCU)/Interaction.xcu \
	$(MY_XCU)/Office/Accelerators.xcu \
	$(MY_XCU)/Office/Calc.xcu \
	$(MY_XCU)/Office/Canvas.xcu \
	$(MY_XCU)/Office/Common.xcu \
	$(MY_XCU)/Office/Compatibility.xcu \
	$(MY_XCU)/Office/DataAccess.xcu \
	$(MY_XCU)/Office/Embedding.xcu \
	$(MY_XCU)/Office/ExtensionDependencies.xcu \
	$(MY_XCU)/Office/ExtensionManager.xcu \
	$(MY_XCU)/Office/FormWizard.xcu \
	$(MY_XCU)/Office/Histories.xcu \
	$(MY_XCU)/Office/Impress.xcu \
	$(MY_XCU)/Office/Jobs.xcu \
	$(MY_XCU)/Office/Labels.xcu \
	$(MY_XCU)/Office/Logging.xcu \
	$(MY_XCU)/Office/Math.xcu \
	$(MY_XCU)/Office/Paths.xcu \
	$(MY_XCU)/Office/ProtocolHandler.xcu \
	$(MY_XCU)/Office/SFX.xcu \
	$(MY_XCU)/Office/Scripting.xcu \
	$(MY_XCU)/Office/Security.xcu \
	$(MY_XCU)/Office/TableWizard.xcu \
	$(MY_XCU)/Office/UI/BaseWindowState.xcu \
	$(MY_XCU)/Office/UI/BasicIDECommands.xcu \
	$(MY_XCU)/Office/UI/BasicIDEWindowState.xcu \
	$(MY_XCU)/Office/UI/BibliographyCommands.xcu \
	$(MY_XCU)/Office/UI/ChartCommands.xcu \
	$(MY_XCU)/Office/UI/ChartWindowState.xcu \
	$(MY_XCU)/Office/UI/Controller.xcu \
	$(MY_XCU)/Office/UI/DbBrowserWindowState.xcu \
	$(MY_XCU)/Office/UI/DbQueryWindowState.xcu \
	$(MY_XCU)/Office/UI/DbRelationWindowState.xcu \
	$(MY_XCU)/Office/UI/DbTableDataWindowState.xcu \
	$(MY_XCU)/Office/UI/DbTableWindowState.xcu \
	$(MY_XCU)/Office/UI/DbuCommands.xcu \
	$(MY_XCU)/Office/UI/DrawImpressCommands.xcu \
	$(MY_XCU)/Office/UI/Factories.xcu \
	$(MY_XCU)/Office/UI/GenericCategories.xcu \
	$(MY_XCU)/Office/UI/GenericCommands.xcu \
	$(MY_XCU)/Office/UI/StartModuleCommands.xcu \
	$(MY_XCU)/Office/UI/StartModuleWindowState.xcu \
	$(MY_XCU)/Office/UI.xcu \
	$(MY_XCU)/Office/Views.xcu \
	$(MY_XCU)/Office/WebWizard.xcu \
	$(MY_XCU)/Office/Writer.xcu \
	$(MY_XCU)/Setup.xcu \
	$(MY_XCU)/System.xcu \
	$(MY_XCU)/TypeDetection/UISort.xcu \
	$(MY_XCU)/UserProfile.xcu \
	$(MY_XCU)/VCL.xcu \
	$(MY_XCU)/ucb/Configuration.xcu \
	$(MY_MOD)/fcfg_base_filters.xcu \
	$(MY_MOD)/fcfg_base_others.xcu \
	$(MY_MOD)/fcfg_base_types.xcu \
	$(MY_MOD)/fcfg_chart_filters.xcu \
	$(MY_MOD)/fcfg_chart_others.xcu \
	$(MY_MOD)/fcfg_chart_types.xcu \
	$(MY_MOD)/fcfg_internalgraphics_filters.xcu \
	$(MY_MOD)/fcfg_internalgraphics_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Embedding-chart.xcu \
	$(MY_MOD)/org/openoffice/Setup-start.xcu \
	$(MY_MOD)/org/openoffice/TypeDetection/UISort-calc.xcu \
	$(MY_MOD)/org/openoffice/TypeDetection/UISort-draw.xcu \
	$(MY_MOD)/org/openoffice/TypeDetection/UISort-impress.xcu \
	$(MY_MOD)/org/openoffice/TypeDetection/UISort-math.xcu \
	$(MY_MOD)/org/openoffice/TypeDetection/UISort-writer.xcu
ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
MY_FILES_main += \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-dbase.xcu \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-flat.xcu \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-mysql.xcu \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-odbc.xcu
MY_DRIVERS += dbase flat mysql odbc
endif
ifeq (aqua,$(GUIBASE))
MY_FILES_main += \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-macab.xcu \
	$(MY_MOD)/org/openoffice/Inet-macosx.xcu \
	$(MY_MOD)/org/openoffice/Office/Accelerators-macosx.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-macosx.xcu \
	$(MY_MOD)/org/openoffice/Office/Paths-macosx.xcu
		# Inet-macosx.xcu must come after Inet.xcu
MY_DRIVERS += macab
else ifeq (unx,$(GUIBASE))
MY_FILES_main += \
	$(MY_MOD)/org/openoffice/Inet-unixdesktop.xcu \
	$(MY_MOD)/org/openoffice/Office/Accelerators-unxwnt.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-UseOOoFileDialogs.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-unixdesktop.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-unx.xcu \
	$(MY_MOD)/org/openoffice/Office/Paths-unixdesktop.xcu \
	$(MY_MOD)/org/openoffice/Office/Paths-unxwnt.xcu \
	$(MY_MOD)/org/openoffice/UserProfile-unixdesktop.xcu \
	$(MY_MOD)/org/openoffice/VCL-unixdesktop.xcu
		# Inet-unixdesktop.xcu must come after Inet.xcu
		# VCL-unixdesktop.xcu must come after VCL.xcu
else ifeq (WIN,$(GUIBASE))
MY_FILES_main += \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-ado.xcu \
	$(MY_MOD)/org/openoffice/Inet-wnt.xcu \
	$(MY_MOD)/org/openoffice/Office/Accelerators-unxwnt.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-wnt.xcu \
	$(MY_MOD)/org/openoffice/Office/Paths-unxwnt.xcu \
	$(MY_MOD)/org/openoffice/ucb/Configuration-win.xcu
		# Inet-wnt.xcu must come after Inet.xcu
MY_DRIVERS += ado
else ifeq (cocoatouch,$(GUIBASE))
# ?
else ifeq (android,$(GUIBASE))
# ?
else ifeq (headless,$(GUIBASE))
# ?
else
$(error unknown GUIBASE)
endif
ifeq ($(DISABLE_NEON),$(false))
MY_FILES_main += $(MY_MOD)/org/openoffice/ucb/Configuration-neon.xcu
endif
ifeq ($(ENABLE_EVOAB2),TRUE)
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-evoab2.xcu
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/DataAccess-evoab2.xcu
MY_DRIVERS += evoab
endif
ifeq ($(SOLAR_JAVA),TRUE)
MY_FILES_main += \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-hsqldb.xcu \
	$(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-jdbc.xcu
MY_DRIVERS += hsqldb jdbc
endif
ifeq ($(ENABLE_TDEAB),TRUE)
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-tdeab.xcu
endif
ifeq ($(ENABLE_KAB),TRUE)
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-kab.xcu
MY_DRIVERS += kab
endif

ifeq (WNT,$(OS))
.IF "$(SYSTEM_MOZILLA)" != "YES" && "$(WITH_MOZILLA)" != "NO"
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-mozab.xcu
MY_DRIVERS += mozab
.END
else ifeq (DESKTOP,$(filter DESKTOP,$(BUILD_TYPE)))
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-mork.xcu
MY_DRIVERS += mork
endif

ifneq (,$(SYSTEM_LIBEXTTEXTCAT_DATA))
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/Paths-externallibexttextcatdata.xcu
else
MY_FILES_main += $(MY_MOD)/org/openoffice/Office/Paths-internallibexttextcatdata.xcu
endif

MY_DEPS_math := main
MY_FILES_math := \
	$(MY_XCS)/Office/UI/MathCommands.xcs \
	$(MY_XCS)/Office/UI/MathWindowState.xcs \
	$(MY_XCU)/Office/UI/MathCommands.xcu \
	$(MY_XCU)/Office/UI/MathWindowState.xcu \
	$(MY_MOD)/fcfg_math_filters.xcu \
	$(MY_MOD)/fcfg_math_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-math.xcu \
	$(MY_MOD)/org/openoffice/Office/Embedding-math.xcu \
	$(MY_MOD)/org/openoffice/Setup-math.xcu

MY_DEPS_palm := main
MY_FILES_palm := \
	$(MY_MOD)/fcfg_palm_filters.xcu \
	$(MY_MOD)/fcfg_palm_types.xcu

MY_DEPS_pocketexcel := main
MY_FILES_pocketexcel := \
	$(MY_MOD)/fcfg_pocketexcel_filters.xcu \
	$(MY_MOD)/fcfg_pocketexcel_types.xcu

MY_DEPS_pocketword := main
MY_FILES_pocketword := \
	$(MY_MOD)/fcfg_pocketword_filters.xcu \
	$(MY_MOD)/fcfg_pocketword_types.xcu

MY_DEPS_pyuno := main
MY_FILES_pyuno := \
	$(MY_MOD)/org/openoffice/Office/Scripting-python.xcu

MY_DEPS_writer := main
MY_FILES_writer := \
	$(MY_XCS)/Office/UI/WriterCommands.xcs \
	$(MY_XCS)/Office/UI/WriterFormWindowState.xcs \
	$(MY_XCS)/Office/UI/WriterGlobalWindowState.xcs \
	$(MY_XCS)/Office/UI/WriterReportWindowState.xcs \
	$(MY_XCS)/Office/UI/WriterWebWindowState.xcs \
	$(MY_XCS)/Office/UI/WriterWindowState.xcs \
	$(MY_XCS)/Office/UI/XFormsWindowState.xcs \
	$(MY_XCU)/Office/UI/WriterCommands.xcu \
	$(MY_XCU)/Office/UI/WriterFormWindowState.xcu \
	$(MY_XCU)/Office/UI/WriterGlobalWindowState.xcu \
	$(MY_XCU)/Office/UI/WriterReportWindowState.xcu \
	$(MY_XCU)/Office/UI/WriterWebWindowState.xcu \
	$(MY_XCU)/Office/UI/WriterWindowState.xcu \
	$(MY_XCU)/Office/UI/XFormsWindowState.xcu \
	$(MY_MOD)/fcfg_global_filters.xcu \
	$(MY_MOD)/fcfg_global_types.xcu \
	$(MY_MOD)/fcfg_web_filters.xcu \
	$(MY_MOD)/fcfg_web_types.xcu \
	$(MY_MOD)/fcfg_writer_filters.xcu \
	$(MY_MOD)/fcfg_writer_types.xcu \
	$(MY_MOD)/org/openoffice/Office/Common-writer.xcu \
	$(MY_MOD)/org/openoffice/Office/Embedding-writer.xcu \
	$(MY_MOD)/org/openoffice/Setup-writer.xcu

MY_DEPS_xsltfilter := main
MY_FILES_xsltfilter := \
	$(MY_MOD)/fcfg_xslt_filters.xcu \
	$(MY_MOD)/fcfg_xslt_types.xcu

ifeq ($(BUILD_POSTGRESQL_SDBC),YES)
MY_XCDS += postgresqlsdbc.xcd
MY_DEPS_postgresqlsdbc := main
MY_FILES_postgresqlsdbc := $(MY_MOD)/org/openoffice/Office/DataAccess/Drivers-postgresql.xcu
MY_DRIVERS += postgresql
endif

ifeq (unx,$(GUIBASE))
ifneq (,$(or $(filter TRUEYES,$(ENABLE_GCONF)$(ENABLE_LOCKDOWN))$(filter TRUE,$(ENABLE_GIO))))
MY_XCDS += gnome.xcd
MY_DEPS_gnome := main
ifeq ($(ENABLE_GCONF)$(ENABLE_LOCKDOWN),TRUEYES)
MY_FILES_gnome += \
	$(MY_MOD)/org/openoffice/Office/Common-gconflockdown.xcu \
	$(MY_MOD)/org/openoffice/Office/Recovery-gconflockdown.xcu \
	$(MY_MOD)/org/openoffice/VCL-gconflockdown.xcu
endif
ifeq ($(ENABLE_GIO),TRUE)
MY_FILES_gnome += \
	$(MY_MOD)/org/openoffice/ucb/Configuration-gio.xcu
endif
endif
endif # unx == $(GUIBASE)

ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
MY_XCDS += onlineupdate.xcd
MY_DEPS_onlineupdate := main
MY_FILES_onlineupdate := \
	$(MY_MOD)/org/openoffice/Office/Addons-onlineupdate.xcu \
	$(MY_MOD)/org/openoffice/Office/Jobs-onlineupdate.xcu
endif

ifeq ($(ENABLE_OPENGL),TRUE)
MY_XCDS += ogltrans.xcd
MY_DEPS_ogltrans := main
MY_FILES_ogltrans := \
	$(MY_MOD)/org/openoffice/Office/Impress-ogltrans.xcu
endif

ifeq (WIN,$(GUIBASE))
MY_XCDS += forcedefault.xcd
MY_DEPS_forcedefault := main
MY_FILES_forcedefault := \
	$(MY_MOD)/org/openoffice/Office/Linguistic-ForceDefaultLanguage.xcu
endif

MY_DRIVERS := $(foreach driver,$(MY_DRIVERS),driver_$(driver))

#
# Targets
#

$(eval $(call gb_CustomTarget_register_targets,postprocess/registry,\
	$(MY_XCDS) \
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
	$(foreach file,$(MY_FILES_$(1)),$(OUTDIR)/xml/$(file)) \
	$(SRCDIR)/postprocess/CustomTarget_registry.mk \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

endef
$(foreach xcd,$(MY_XCDS),$(eval $(call postprocess_xcd_deps,$(basename $(xcd)))))

define postprocess_lang_deps
$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-$(1).list : \
	$(call gb_XcuModuleTarget_get_outdir_target,Langpack-$(1).xcu) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/.dir

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).list : \
	$(call gb_Zip_get_target,fcfg_langpack_$(1)) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$(1).unzip/.dir

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).xcd : \
	$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).list

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).list : \
	$(call gb_Zip_get_target,registry_$(1)) \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)), \
		$(foreach driver,$(MY_DRIVERS),$(call gb_Zip_get_target,$(driver)_$(1))) \
	) \
	$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)), \
		$(call gb_Zip_get_target,updchk_$(1)) \
	) \
	| $(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$(1).unzip/.dir \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)), \
		$(foreach driver,$(MY_DRIVERS), \
			$(call gb_CustomTarget_get_workdir,postprocess/registry)/$(driver)_$(1).unzip/.dir ) \
	) \
	$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)), \
		$(call gb_CustomTarget_get_workdir,postprocess/registry)/updchk_$(1).unzip/.dir \
	)

endef
$(foreach lang,$(gb_Configuration_LANGS),$(eval $(call postprocess_lang_deps,$(lang))))

#
# Rules
#

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-%.xcd :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(gb_XSLTPROC) --nonet --stringparam prefix $(call gb_XcuModuleTarget_get_outdir_target) -o $@ \
		$(SOLARENV)/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/Langpack-%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list><dependency file="main"/><filename>Langpack-$*.xcu</filename></list>' > $@

$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_%.xcd :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(gb_XSLTPROC) --nonet --stringparam prefix $(call gb_CustomTarget_get_workdir,postprocess/registry)/ \
		-o $@ $(SOLARENV)/bin/packregistry.xslt $< \
	)

# It can happen that localized fcfg_langpack_*.zip contains
# zero-sized org/openoffice/TypeDectection/Filter.xcu; filter them out in the
# find shell command below (see issue 110041):
$(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	$(call gb_Helper_abbreviate_dirs, \
	cd $(call gb_CustomTarget_get_workdir,postprocess/registry)/fcfg_langpack_$*.unzip \
	&& rm -rf * \
	&& unzip $< \
	&& cd .. \
	&& echo '<list>' > $@ \
	&& find fcfg_langpack_$*.unzip -name *.xcu -size +0c -printf '<filename>%p</filename>\n' >> $@ \
	&& echo '</list>' >> $@ \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_%.xcd :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(gb_XSLTPROC) --nonet --stringparam prefix $(call gb_CustomTarget_get_workdir,postprocess/registry)/ \
		-o $@ $(SOLARENV)/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list>' > $@
	# Add registry_*.zip content to *.list:
	$(call gb_Helper_abbreviate_dirs, \
	cd $(call gb_CustomTarget_get_workdir,postprocess/registry)/registry_$*.unzip \
	&& rm -rf * \
	&& unzip $(call gb_Zip_get_target,registry_$*) \
	&& cd .. \
	&& find registry_$*.unzip -name *.xcu -printf '<filename>%p</filename>\n' >> $@ \
	)
ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
	# Add fcfg_drivers_*.zip content to *.list:
	$(call gb_Helper_abbreviate_dirs, \
	true \
	$(foreach driver,$(MY_DRIVERS), \
		&& cd $(call gb_CustomTarget_get_workdir,postprocess/registry)/$(driver)_$*.unzip \
		&& rm -rf * \
		&& unzip $(call gb_Zip_get_target,$(driver)_$*) \
		&& cd .. \
		&& find $(driver)_$*.unzip -name *.xcu -printf '<filename>%p</filename>\n' >> $@ \
	) \
	)
endif
ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
	# Add updchk_*.zip content to *.list:
	$(call gb_Helper_abbreviate_dirs, \
	cd $(call gb_CustomTarget_get_workdir,postprocess/registry)/updchk_$*.unzip \
	&& rm -rf * \
	&& unzip $(call gb_Zip_get_target,updchk_$*) \
	&& cd .. \
	&& find updchk_$*.unzip -name *.xcu -printf '<filename>%p</filename>\n' >> $@ \
	)
endif
	echo '</list>' >> $@

$(call gb_CustomTarget_get_workdir,postprocess/registry)/%.xcd :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XCD,3)
	$(call gb_Helper_abbreviate_dirs, \
	$(gb_XSLTPROC) --nonet --stringparam prefix $(OUTDIR)/xml/ \
		-o $@ $(SOLARENV)/bin/packregistry.xslt $< \
	)

$(call gb_CustomTarget_get_workdir,postprocess/registry)/%.list :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,2)
	echo '<list>' $(foreach i,$(MY_DEPS_$*), '<dependency file="$i"/>') \
		$(foreach i,$(MY_FILES_$*), '<filename>$i</filename>') '</list>' > $@

# vim: set noet sw=4 ts=4:
