# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangZip_AllLangZip,accessoriestemplates,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/lang))

$(eval $(call gb_AllLangZip_add_dependencies,accessoriestemplates,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_AllLangZip_add_files,accessoriestemplates,\
	cs/dummy_templates.txt \
	de/dummy_templates.txt \
	en-US/dummy_templates.txt \
	es/dummy_templates.txt \
	fi/dummy_templates.txt \
	fr/dummy_templates.txt \
	hu/dummy_templates.txt \
	it/dummy_templates.txt \
	ja/dummy_templates.txt \
	ka/dummy_templates.txt \
	pl/dummy_templates.txt \
	pt-BR/dummy_templates.txt \
	sv/dummy_templates.txt \
	tr/dummy_templates.txt \
	zh-CN/dummy_templates.txt \
))

# vim: set noet sw=4 ts=4:
