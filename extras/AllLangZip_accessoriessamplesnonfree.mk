# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangZip_AllLangZip,accessoriessamplesnonfree,$(call gb_UnpackedTarball_get_dir,samples-pack)/samples-nonfree/lang))

$(eval $(call gb_AllLangZip_add_dependencies,accessoriessamplesnonfree,\
	$(call gb_UnpackedTarball_get_target,samples-pack) \
))

$(eval $(call gb_AllLangZip_add_files,accessoriessamplesnonfree,\
	cs/dummy_samples-nonfree.txt \
	de/dummy_samples-nonfree.txt \
	en-US/dummy_samples-nonfree.txt \
	es/dummy_samples-nonfree.txt \
	fi/dummy_samples-nonfree.txt \
	fr/dummy_samples-nonfree.txt \
	hu/dummy_samples-nonfree.txt \
	it/dummy_samples-nonfree.txt \
	ja/dummy_samples-nonfree.txt \
	ka/dummy_samples-nonfree.txt \
	nl/dummy_samples-nonfree.txt \
	pl/dummy_samples-nonfree.txt \
	pt-BR/dummy_samples-nonfree.txt \
	sv/dummy_samples-nonfree.txt \
	tr/dummy_samples-nonfree.txt \
	zh-CN/dummy_samples-nonfree.txt \
))

# vim: set noet sw=4 ts=4:
