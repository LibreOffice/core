# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosspace,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Space))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosspace,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosspace,\
	Wikimedia_Space_120px-Atlantis_taking_off_on_STS-27.jpg \
	Wikimedia_Space_1995-18-b-large_web.jpg \
	Wikimedia_Space_2005-06-b-large_web.jpg \
	Wikimedia_Space_2005-37-a-large_web.jpg \
	Wikimedia_Space_2006-10-a-large_web.jpg \
	Wikimedia_Space_395px-Mir_as_seen_from_Discovery_during_STS-63.jpg \
	Wikimedia_Space_433px-Baikal_liftoff.jpg \
	Wikimedia_Space_477px-Ap11-KSC-69PC-442.jpg \
	Wikimedia_Space_516px-Shuttle.jpg \
	Wikimedia_Space_594px-Apollo_11_bootprint.jpg \
	Wikimedia_Space_595px-Tarantula_nebula_detail.jpg \
	Wikimedia_Space_600px-Mars_Valles_Marineris.jpeg \
	Wikimedia_Space_682px-Atlantis_Docked_to_Mir.jpg \
	Wikimedia_Space_740px-Hst_sts82.jpg \
	Wikimedia_Space_792px-NASA_Space_Shuttle_Discovery_STS-92.jpg \
	Wikimedia_Space_As08-16-2593.jpg \
	Wikimedia_Space_heic0405a.jpg \
	Wikimedia_Space_NASA-Apollo8-Dec24-Earthrise.jpg \
	Wikimedia_Space_Sputnik.jpg \
))

# vim: set noet sw=4 ts=4:
