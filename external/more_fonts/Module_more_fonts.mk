# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,more_fonts))

$(eval $(call gb_Module_add_targets,more_fonts,\
	ExternalPackage_alef \
	ExternalPackage_amiri \
	ExternalPackage_caladea \
	ExternalPackage_carlito \
	ExternalPackage_culmus \
	ExternalPackage_dejavu \
	ExternalPackage_gentium \
	ExternalPackage_kacst \
	ExternalPackage_liberation \
	ExternalPackage_liberation_narrow \
	ExternalPackage_libertineg \
	ExternalPackage_libre_hebrew \
	ExternalPackage_sourcecode \
	ExternalPackage_sourcesans \
	ExternalPackage_noto \
	ExternalPackage_reem \
	ExternalPackage_scheherazade \
	ExternalPackage_EmojiOne_Color \
	UnpackedTarball_alef \
	UnpackedTarball_amiri \
	UnpackedTarball_caladea \
	UnpackedTarball_carlito \
	UnpackedTarball_culmus \
	UnpackedTarball_dejavu \
	UnpackedTarball_gentium \
	UnpackedTarball_kacst \
	UnpackedTarball_liberation \
	UnpackedTarball_liberation_narrow \
	UnpackedTarball_libertineg \
	UnpackedTarball_libre_hebrew \
	UnpackedTarball_sourcecode \
	UnpackedTarball_sourcesans \
	UnpackedTarball_noto \
	UnpackedTarball_reem \
	UnpackedTarball_scheherazade \
	UnpackedTarball_EmojiOne_Color \
))

# vim: set noet sw=4 ts=4:
