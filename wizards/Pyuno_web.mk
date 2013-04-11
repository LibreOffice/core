# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
	export/__init__.py \
))
$(eval $(call gb_Pyuno_set_componentfile_full,web,wizards/com/sun/star/wizards/web/web,vnd.openoffice.pymodule:wizards.web,.CallWizard))
