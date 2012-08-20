# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangZip_AllLangZip,accessoriessamples,$(call gb_UnpackedTarball_get_dir,samples-pack)/samples/lang))

$(eval $(call gb_AllLangZip_add_dependencies,accessoriessamples,\
	$(call gb_UnpackedTarball_get_target,samples-pack) \
))

$(eval $(call gb_AllLangZip_add_files,accessoriessamples,\
	cs/.nametranslation.table \
	de/.nametranslation.table \
	en-US/JacketPotato.odb \
	en-US/.nametranslation.table \
	es/.nametranslation.table \
	fi/.nametranslation.table \
	fr/.nametranslation.table \
	hu/.nametranslation.table \
	hu/naptar.ods \
	it/.nametranslation.table \
	ja/.nametranslation.table \
	ka/.nametranslation.table \
	nl/.nametranslation.table \
	pl/dummy_samples.txt \
	pt-BR/.nametranslation.table \
	sv/.nametranslation.table \
	tr/.nametranslation.table \
	zh-CN/.nametranslation.table \
))

# vim: set noet sw=4 ts=4:
