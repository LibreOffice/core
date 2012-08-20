# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangZip_AllLangZip,accessoriessamplesadvertisement,$(call gb_UnpackedTarball_get_dir,samples-pack)/samples/advertisement/lang))

$(eval $(call gb_AllLangZip_add_dependencies,accessoriessamplesadvertisement,\
	$(call gb_UnpackedTarball_get_target,samples-pack) \
))

$(eval $(call gb_AllLangZip_add_files,accessoriessamplesadvertisement,\
	cs/dummy_samples.txt \
	de/dummy_samples.txt \
	en-US/ooo2prodflyera3en.odt \
	en-US/ooo2prodflyera4en.odt \
	en-US/ooo2prodintroen.odp \
	en-US/ooo2prodspeca4en.odt \
	es/dummy_samples.txt \
	fi/dummy_samples.txt \
	fr/dummy_samples.txt \
	hu/ooo2prodflyera3hu.odt \
	hu/ooo2prodspeca4hu.odt \
	hu/OOo_Ossszehasonlitas.odt \
	hu/OOo_Reklam4.odt \
	it/dummy_samples.txt \
	ja/dummy_samples.txt \
	ka/dummy_samples.txt \
	nl/dummy_samples.txt \
	pl/dummy_samples.txt \
	pt-BR/dummy_samples.txt \
	sv/dummy_samples.txt \
	tr/dummy_samples.txt \
	zh-CN/dummy_samples.txt \
))

# vim: set noet sw=4 ts=4:
