#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Configuration_Configuration,registry))

$(eval $(call gb_Configuration_add_schemas,registry,officecfg/registry/schema,\
	org/openoffice/LDAP.xcs \
	org/openoffice/Inet.xcs \
	org/openoffice/Setup.xcs \
	org/openoffice/System.xcs \
	org/openoffice/UserProfile.xcs \
	org/openoffice/VCL.xcs \
	org/openoffice/FirstStartWizard.xcs \
	org/openoffice/Interaction.xcs \
	org/openoffice/Office/Accelerators.xcs \
	org/openoffice/Office/Addons.xcs \
	org/openoffice/Office/Calc.xcs \
	org/openoffice/Office/CalcAddIns.xcs \
	org/openoffice/Office/Canvas.xcs \
	org/openoffice/Office/Chart.xcs \
	org/openoffice/Office/Commands.xcs \
	org/openoffice/Office/Common.xcs \
	org/openoffice/Office/Compatibility.xcs \
	org/openoffice/Office/DataAccess.xcs \
	org/openoffice/Office/Draw.xcs \
	org/openoffice/Office/Events.xcs \
	org/openoffice/Office/Embedding.xcs \
	org/openoffice/Office/ExtensionManager.xcs \
	org/openoffice/Office/Impress.xcs \
	org/openoffice/Office/Java.xcs \
	org/openoffice/Office/Jobs.xcs \
	org/openoffice/Office/Labels.xcs \
	org/openoffice/Office/Linguistic.xcs \
	org/openoffice/Office/Logging.xcs \
	org/openoffice/Office/Math.xcs \
	org/openoffice/Office/OptionsDialog.xcs \
	org/openoffice/Office/ProtocolHandler.xcs \
	org/openoffice/Office/SFX.xcs \
	org/openoffice/Office/Substitution.xcs \
	org/openoffice/Office/TableWizard.xcs \
	org/openoffice/Office/UI.xcs \
	org/openoffice/Office/Views.xcs \
	org/openoffice/Office/Writer.xcs \
	org/openoffice/Office/WriterWeb.xcs \
	org/openoffice/Office/Scripting.xcs \
	org/openoffice/Office/Security.xcs \
	org/openoffice/Office/WebWizard.xcs \
	org/openoffice/Office/Recovery.xcs \
	org/openoffice/Office/FormWizard.xcs \
	org/openoffice/Office/ExtendedColorScheme.xcs \
	org/openoffice/Office/TypeDetection.xcs \
	org/openoffice/Office/TabBrowse.xcs \
	org/openoffice/Office/Paths.xcs \
	org/openoffice/Office/Histories.xcs \
	org/openoffice/Office/UI/Controller.xcs \
	org/openoffice/Office/UI/Factories.xcs \
	org/openoffice/Office/UI/Commands.xcs \
	org/openoffice/Office/UI/BasicIDECommands.xcs \
	org/openoffice/Office/UI/BibliographyCommands.xcs \
	org/openoffice/Office/UI/CalcCommands.xcs \
	org/openoffice/Office/UI/ChartCommands.xcs \
	org/openoffice/Office/UI/DbuCommands.xcs \
	org/openoffice/Office/UI/BaseWindowState.xcs \
	org/openoffice/Office/UI/WriterFormWindowState.xcs \
	org/openoffice/Office/UI/WriterReportWindowState.xcs \
	org/openoffice/Office/UI/DbBrowserWindowState.xcs \
	org/openoffice/Office/UI/DbTableDataWindowState.xcs \
	org/openoffice/Office/UI/DrawImpressCommands.xcs \
	org/openoffice/Office/UI/GenericCommands.xcs \
	org/openoffice/Office/UI/MathCommands.xcs \
	org/openoffice/Office/UI/StartModuleCommands.xcs \
	org/openoffice/Office/UI/WriterCommands.xcs \
	org/openoffice/Office/UI/WindowState.xcs \
	org/openoffice/Office/UI/BasicIDEWindowState.xcs \
	org/openoffice/Office/UI/BibliographyWindowState.xcs \
	org/openoffice/Office/UI/CalcWindowState.xcs \
	org/openoffice/Office/UI/ChartWindowState.xcs \
	org/openoffice/Office/UI/DbQueryWindowState.xcs \
	org/openoffice/Office/UI/DbRelationWindowState.xcs \
	org/openoffice/Office/UI/DbTableWindowState.xcs \
	org/openoffice/Office/UI/DrawWindowState.xcs \
	org/openoffice/Office/UI/ImpressWindowState.xcs \
	org/openoffice/Office/UI/MathWindowState.xcs \
	org/openoffice/Office/UI/StartModuleWindowState.xcs \
	org/openoffice/Office/UI/WriterWindowState.xcs \
	org/openoffice/Office/UI/XFormsWindowState.xcs \
	org/openoffice/Office/UI/WriterGlobalWindowState.xcs \
	org/openoffice/Office/UI/WriterWebWindowState.xcs \
	org/openoffice/Office/UI/Effects.xcs \
	org/openoffice/Office/UI/Category.xcs \
	org/openoffice/Office/UI/GenericCategories.xcs \
	org/openoffice/Office/UI/GlobalSettings.xcs \
	org/openoffice/Office/UI/WindowContentFactories.xcs \
	org/openoffice/Office/DataAccess/Drivers.xcs \
	org/openoffice/Office/OOoImprovement/Settings.xcs \
	org/openoffice/TypeDetection/Types.xcs \
	org/openoffice/TypeDetection/Filter.xcs \
	org/openoffice/TypeDetection/GraphicFilter.xcs \
	org/openoffice/TypeDetection/Misc.xcs \
	org/openoffice/TypeDetection/UISort.xcs \
	org/openoffice/ucb/Configuration.xcs \
	org/openoffice/ucb/Hierarchy.xcs \
	org/openoffice/ucb/InteractionHandler.xcs \
	org/openoffice/ucb/Store.xcs \
))

ifeq ($(ENABLE_SDREMOTE),YES)
$(eval $(call gb_Configuration_add_schemas,registry,officecfg/registry/schema,\
        org/openoffice/Office/Impress-sdremote.xcs \
))
endif

$(eval $(call gb_Configuration_add_datas,registry,officecfg/registry/data,\
	org/openoffice/Inet.xcu \
	org/openoffice/System.xcu \
	org/openoffice/UserProfile.xcu \
	org/openoffice/VCL.xcu \
	org/openoffice/FirstStartWizard.xcu \
	org/openoffice/Interaction.xcu \
	org/openoffice/Office/Calc.xcu \
	org/openoffice/Office/Canvas.xcu \
	org/openoffice/Office/Compatibility.xcu \
	org/openoffice/Office/ExtensionManager.xcu \
	org/openoffice/Office/Impress.xcu \
	org/openoffice/Office/Jobs.xcu \
	org/openoffice/Office/Labels.xcu \
	org/openoffice/Office/Linguistic.xcu \
	org/openoffice/Office/Logging.xcu \
	org/openoffice/Office/Math.xcu \
	org/openoffice/Office/ProtocolHandler.xcu \
	org/openoffice/Office/Security.xcu \
	org/openoffice/Office/Scripting.xcu \
	org/openoffice/Office/SFX.xcu \
	org/openoffice/Office/Views.xcu \
	org/openoffice/Office/Paths.xcu \
	org/openoffice/Office/Histories.xcu \
	org/openoffice/Office/UI/Controller.xcu \
	org/openoffice/Office/UI/Factories.xcu \
	org/openoffice/TypeDetection/UISort.xcu \
	org/openoffice/ucb/Configuration.xcu \
))

$(eval $(call gb_Configuration_add_spool_modules,registry,officecfg/registry/data,\
	org/openoffice/Inet-macosx.xcu \
	org/openoffice/Inet-unixdesktop.xcu \
	org/openoffice/Inet-wnt.xcu \
	org/openoffice/Setup-writer.xcu \
	org/openoffice/Setup-calc.xcu \
	org/openoffice/Setup-draw.xcu \
	org/openoffice/Setup-impress.xcu \
	org/openoffice/Setup-base.xcu \
	org/openoffice/Setup-math.xcu \
	org/openoffice/Setup-report.xcu \
	org/openoffice/Setup-start.xcu \
	org/openoffice/UserProfile-unixdesktop.xcu \
	org/openoffice/VCL-gconflockdown.xcu \
	org/openoffice/VCL-unixdesktop.xcu \
	org/openoffice/Office/Accelerators-macosx.xcu \
	org/openoffice/Office/Accelerators-unxwnt.xcu \
	org/openoffice/Office/Common-writer.xcu \
	org/openoffice/Office/Common-calc.xcu \
	org/openoffice/Office/Common-draw.xcu \
	org/openoffice/Office/Common-impress.xcu \
	org/openoffice/Office/Common-base.xcu \
	org/openoffice/Office/Common-math.xcu \
	org/openoffice/Office/Common-unx.xcu \
	org/openoffice/Office/Common-unixdesktop.xcu \
	org/openoffice/Office/Common-gconflockdown.xcu \
	org/openoffice/Office/Common-macosx.xcu \
	org/openoffice/Office/Common-wnt.xcu \
	org/openoffice/Office/Common-UseOOoFileDialogs.xcu \
	org/openoffice/Office/Linguistic-ForceDefaultLanguage.xcu \
	org/openoffice/Office/Scripting-python.xcu \
	org/openoffice/Office/Common-cjk.xcu \
	org/openoffice/Office/Common-ctl.xcu \
	org/openoffice/Office/Common-korea.xcu \
	org/openoffice/Office/DataAccess-evoab2.xcu \
	org/openoffice/Office/Paths-macosx.xcu \
	org/openoffice/Office/Paths-unxwnt.xcu \
	org/openoffice/Office/Paths-unixdesktop.xcu \
	org/openoffice/Office/Paths-internallibexttextcatdata.xcu \
	org/openoffice/Office/Paths-externallibexttextcatdata.xcu \
	org/openoffice/Office/Writer-cjk.xcu \
	org/openoffice/Office/Impress-ogltrans.xcu \
	org/openoffice/Office/Embedding-calc.xcu \
	org/openoffice/Office/Embedding-chart.xcu \
	org/openoffice/Office/Embedding-draw.xcu \
	org/openoffice/Office/Embedding-impress.xcu \
	org/openoffice/Office/Embedding-math.xcu \
	org/openoffice/Office/Embedding-base.xcu \
	org/openoffice/Office/Embedding-writer.xcu \
	org/openoffice/Office/Recovery-gconflockdown.xcu \
	org/openoffice/TypeDetection/UISort-writer.xcu \
	org/openoffice/TypeDetection/UISort-calc.xcu \
	org/openoffice/TypeDetection/UISort-draw.xcu \
	org/openoffice/TypeDetection/UISort-impress.xcu \
	org/openoffice/TypeDetection/UISort-math.xcu \
	org/openoffice/ucb/Configuration-gio.xcu \
	org/openoffice/ucb/Configuration-neon.xcu \
	org/openoffice/ucb/Configuration-win.xcu \
))

# perhaps this file should be moved 2 levels up?
$(eval $(call gb_Configuration_add_spool_langpack,registry,officecfg/registry/data/org/openoffice,\
	Langpack.xcu \
))

$(eval $(call gb_Configuration_add_localized_datas,registry,officecfg/registry/data,\
	org/openoffice/Setup.xcu \
	org/openoffice/Office/Accelerators.xcu \
	org/openoffice/Office/Common.xcu \
	org/openoffice/Office/SFX.xcu \
	org/openoffice/Office/DataAccess.xcu \
	org/openoffice/Office/TableWizard.xcu \
	org/openoffice/Office/UI.xcu \
	org/openoffice/Office/Embedding.xcu \
	org/openoffice/Office/WebWizard.xcu \
	org/openoffice/Office/FormWizard.xcu \
	org/openoffice/Office/Writer.xcu \
	org/openoffice/Office/UI/BasicIDECommands.xcu \
	org/openoffice/Office/UI/BibliographyCommands.xcu \
	org/openoffice/Office/UI/CalcCommands.xcu \
	org/openoffice/Office/UI/ChartCommands.xcu \
	org/openoffice/Office/UI/ChartWindowState.xcu \
	org/openoffice/Office/UI/DbuCommands.xcu \
	org/openoffice/Office/UI/BaseWindowState.xcu \
	org/openoffice/Office/UI/WriterFormWindowState.xcu \
	org/openoffice/Office/UI/WriterReportWindowState.xcu \
	org/openoffice/Office/UI/DbQueryWindowState.xcu \
	org/openoffice/Office/UI/DbTableWindowState.xcu \
	org/openoffice/Office/UI/DbRelationWindowState.xcu \
	org/openoffice/Office/UI/DbBrowserWindowState.xcu \
	org/openoffice/Office/UI/DbTableDataWindowState.xcu \
	org/openoffice/Office/UI/DrawImpressCommands.xcu \
	org/openoffice/Office/UI/Effects.xcu \
	org/openoffice/Office/UI/GenericCommands.xcu \
	org/openoffice/Office/UI/MathCommands.xcu \
	org/openoffice/Office/UI/StartModuleCommands.xcu \
	org/openoffice/Office/UI/BasicIDEWindowState.xcu \
	org/openoffice/Office/UI/CalcWindowState.xcu \
	org/openoffice/Office/UI/DrawWindowState.xcu \
	org/openoffice/Office/UI/ImpressWindowState.xcu \
	org/openoffice/Office/UI/MathWindowState.xcu \
	org/openoffice/Office/UI/StartModuleWindowState.xcu \
	org/openoffice/Office/UI/WriterWindowState.xcu \
	org/openoffice/Office/UI/XFormsWindowState.xcu \
	org/openoffice/Office/UI/WriterGlobalWindowState.xcu \
	org/openoffice/Office/UI/WriterWebWindowState.xcu \
	org/openoffice/Office/UI/WriterCommands.xcu \
	org/openoffice/Office/UI/GenericCategories.xcu \
))

