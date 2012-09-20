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
	$(addprefix org/openoffice/,$(addsuffix .xcs,$(officecfg_XCSFILES))) \
))

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

