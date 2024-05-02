# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_templates,$(gb_CustomTarget_workdir)/extras/source/templates))

$(eval $(call gb_Package_add_files_with_dir,extras_templates,$(LIBO_SHARE_FOLDER)/template/common,\
	officorr/Modern_business_letter_sans_serif.ott \
	officorr/Modern_business_letter_serif.ott \
	offimisc/Businesscard-with-logo.ott \
	personal/CV.ott \
	personal/Resume1page.ott \
	styles/Default.ott \
	styles/Modern.ott \
	styles/Simple.ott \
	draw/bpmn.otg \
	l10n/zh_CN_ott_normal.ott \
	l10n/ja_ott_normal.ott \
))

# vim: set noet sw=4 ts=4:
