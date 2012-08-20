# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotoshumans,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Humans))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotoshumans,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotoshumans,\
	1471425079_b3b53bdf5f_b.jpg \
	1472456323_62f1a7dad1_o.jpg \
	1473308070_30ac88194b_o.jpg \
	1481457389_86532082cb_b.jpg \
	1482314200_1f933fe66f_b.jpg \
	morguefiles_com_pindiyath100_A1200509.JPG \
	morguefiles_com_pindiyath100_A1200513.JPG \
	morguefiles_com_pindiyath100_A1220569.JPG \
	morguefiles_com_pindiyath100_Fun_time.JPG \
	morguefiles_com_pindiyath100_Lady_with_THE_UMBRELLA.jpg \
	morguefiles_com_pindiyath100_PIC1092760844134.jpg \
))

# vim: set noet sw=4 ts=4:
