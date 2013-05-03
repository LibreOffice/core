# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,web,$(SRCDIR)/wizards/com/sun/star/wizards/web))

$(eval $(call gb_Pyuno_add_files,web,wizards/web,\
	CallWizard.py \
	BackgroundsDialog.py \
	ErrorHandler.py \
	AbstractErrorHandler.py \
	FTPDialog.py \
	FTPDialogResources.py \
	IconsDialog.py \
	ImageListDialog.py \
	LogTaskListener.py \
	Process.py \
	ProcessErrorHandler.py \
	ProcessErrors.py \
	ProcessStatusRenderer.py \
	StatusDialog.py \
	StylePreview.py \
	TOCPreview.py \
	WWD_Events.py \
	WWD_General.py \
	WWD_Startup.py \
	WWHID.py \
	WebWizard.py \
	WebWizardConst.py \
	WebWizardDialog.py \
	WebWizardDialogResources.py \
	TypeDetection.py \
	ExtensionVerifier.py\
	WebConfigSet.py\
	__init__.py \
	data/CGArgument.py \
	data/CGContent.py \
	data/CGDesign.py \
	data/CGDocument.py \
	data/CGExporter.py \
	data/CGFilter.py \
	data/CGGeneralInfo.py \
	data/CGIconSet.py \
	data/CGImage.py \
	data/CGLayout.py \
	data/CGPublish.py \
	data/CGSession.py \
	data/CGSessionName.py \
	data/CGSettings.py \
	data/CGStyle.py\
	data/__init__.py \
	export/Exporter.py \
	export/AbstractExporter.py \
	export/CopyExporter.py \
	export/FilterExporter.py \
	export/ConfiguredExporter.py \
	export/ImpressHTMLExporter.py \
	export/__init__.py \
))
$(eval $(call gb_Pyuno_set_componentfile_full,web,wizards/com/sun/star/wizards/web/web,vnd.openoffice.pymodule:wizards.web,.CallWizard))
