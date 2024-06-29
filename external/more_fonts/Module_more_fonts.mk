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
	$(if $(MPL_SUBSET),,ExternalPackage_culmus) \
	ExternalPackage_dejavu \
	ExternalPackage_gentium \
	ExternalPackage_liberation \
	ExternalPackage_liberation_narrow \
	ExternalPackage_libertineg \
	ExternalPackage_libre_hebrew \
	ExternalPackage_noto_kufi_arabic \
	ExternalPackage_noto_naskh_arabic \
	ExternalPackage_noto_sans \
	ExternalPackage_noto_sans_arabic \
	ExternalPackage_noto_sans_armenian \
	ExternalPackage_noto_sans_georgian \
	ExternalPackage_noto_sans_hebrew \
	ExternalPackage_noto_sans_lao \
	ExternalPackage_noto_sans_lisu \
	ExternalPackage_noto_serif \
	ExternalPackage_noto_serif_armenian \
	ExternalPackage_noto_serif_georgian \
	ExternalPackage_noto_serif_hebrew \
	ExternalPackage_noto_serif_lao \
	ExternalPackage_reem \
	ExternalPackage_scheherazade \
	ExternalPackage_agdasima \
	ExternalPackage_bacasime_antique \
	ExternalPackage_belanosima \
	ExternalPackage_caprasimo \
	ExternalPackage_lugrasimo \
	ExternalPackage_lumanosimo \
	ExternalPackage_lunasima \
	UnpackedTarball_alef \
	UnpackedTarball_amiri \
	UnpackedTarball_caladea \
	UnpackedTarball_carlito \
	$(if $(MPL_SUBSET),,UnpackedTarball_culmus) \
	UnpackedTarball_dejavu \
	UnpackedTarball_gentium \
	UnpackedTarball_liberation \
	UnpackedTarball_liberation_narrow \
	UnpackedTarball_libertineg \
	UnpackedTarball_libre_hebrew \
	UnpackedTarball_noto_kufi_arabic \
	UnpackedTarball_noto_naskh_arabic \
	UnpackedTarball_noto_sans \
	UnpackedTarball_noto_sans_arabic \
	UnpackedTarball_noto_sans_armenian \
	UnpackedTarball_noto_sans_georgian \
	UnpackedTarball_noto_sans_hebrew \
	UnpackedTarball_noto_sans_lao \
	UnpackedTarball_noto_sans_lisu \
	UnpackedTarball_noto_serif \
	UnpackedTarball_noto_serif_armenian \
	UnpackedTarball_noto_serif_georgian \
	UnpackedTarball_noto_serif_hebrew \
	UnpackedTarball_noto_serif_lao \
	UnpackedTarball_reem \
	UnpackedTarball_scheherazade \
	UnpackedTarball_agdasima \
	UnpackedTarball_bacasime_antique \
	UnpackedTarball_belanosima \
	UnpackedTarball_caprasimo \
	UnpackedTarball_lugrasimo \
	UnpackedTarball_lumanosimo \
	UnpackedTarball_lunasima \
))

# vim: set noet sw=4 ts=4:
